/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/compat/compat_process.c */

#define _POSIX_C_SOURCE 200809L

#include <sys/types.h>
#include <sys/wait.h>

#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "kilonode/compat_process.h"

static enum kn_compat_process_error append_fd(int, char *, size_t *,
	size_t);
static void close_pair(int *);
static int set_nonblock(int);
static enum kn_compat_process_error wait_capture(pid_t, int, int,
	struct kn_compat_process_result *, unsigned int);

void
kn_compat_process_result_clear(struct kn_compat_process_result *result)
{
	if (result == NULL)
		return;

	memset(result, 0, sizeof(*result));
	result->exit_status = -1;
}

const char *
kn_compat_process_error_name(enum kn_compat_process_error error)
{
	switch (error) {
	case KN_COMPAT_PROCESS_OK:
		return "ok";
	case KN_COMPAT_PROCESS_ERR_INVALID_ARGUMENT:
		return "invalid-argument";
	case KN_COMPAT_PROCESS_ERR_NOT_EXECUTABLE:
		return "not-executable";
	case KN_COMPAT_PROCESS_ERR_PIPE:
		return "pipe";
	case KN_COMPAT_PROCESS_ERR_FORK:
		return "fork";
	case KN_COMPAT_PROCESS_ERR_TIMEOUT:
		return "timeout";
	case KN_COMPAT_PROCESS_ERR_OUTPUT_TOO_LARGE:
		return "output-too-large";
	case KN_COMPAT_PROCESS_ERR_CHILD:
		return "child";
	}

	return "unknown";
}

enum kn_compat_process_error
kn_compat_process_run(const char *binary, const char *config,
	const char *input, unsigned int timeout_seconds,
	struct kn_compat_process_result *result)
{
	int in_pipe[2];
	int out_pipe[2];
	int err_pipe[2];
	pid_t pid;
	const char *argv[3];
	size_t input_len;

	if (binary == NULL || binary[0] == '\0' || result == NULL ||
	    timeout_seconds == 0)
		return KN_COMPAT_PROCESS_ERR_INVALID_ARGUMENT;
	if (access(binary, X_OK) != 0)
		return KN_COMPAT_PROCESS_ERR_NOT_EXECUTABLE;

	kn_compat_process_result_clear(result);
	in_pipe[0] = in_pipe[1] = -1;
	out_pipe[0] = out_pipe[1] = -1;
	err_pipe[0] = err_pipe[1] = -1;
	if (pipe(in_pipe) != 0 || pipe(out_pipe) != 0 ||
	    pipe(err_pipe) != 0) {
		close_pair(in_pipe);
		close_pair(out_pipe);
		close_pair(err_pipe);
		return KN_COMPAT_PROCESS_ERR_PIPE;
	}

	pid = fork();
	if (pid < 0) {
		close_pair(in_pipe);
		close_pair(out_pipe);
		close_pair(err_pipe);
		return KN_COMPAT_PROCESS_ERR_FORK;
	}
	if (pid == 0) {
		(void)dup2(in_pipe[0], STDIN_FILENO);
		(void)dup2(out_pipe[1], STDOUT_FILENO);
		(void)dup2(err_pipe[1], STDERR_FILENO);
		close_pair(in_pipe);
		close_pair(out_pipe);
		close_pair(err_pipe);
		argv[0] = binary;
		if (config != NULL && config[0] != '\0') {
			argv[1] = config;
			argv[2] = NULL;
		} else {
			argv[1] = NULL;
		}
		(void)execv(binary, (char * const *)argv);
		_exit(127);
	}

	(void)close(in_pipe[0]);
	(void)close(out_pipe[1]);
	(void)close(err_pipe[1]);
	in_pipe[0] = out_pipe[1] = err_pipe[1] = -1;

	input_len = input == NULL ? 0 : strlen(input);
	if (input_len > 0)
		(void)write(in_pipe[1], input, input_len);
	(void)close(in_pipe[1]);
	in_pipe[1] = -1;

	if (set_nonblock(out_pipe[0]) != 0 || set_nonblock(err_pipe[0]) != 0) {
		(void)kill(pid, SIGKILL);
		(void)waitpid(pid, NULL, 0);
		close_pair(out_pipe);
		close_pair(err_pipe);
		return KN_COMPAT_PROCESS_ERR_CHILD;
	}

	return wait_capture(pid, out_pipe[0], err_pipe[0], result,
	    timeout_seconds);
}

static enum kn_compat_process_error
append_fd(int fd, char *buf, size_t *len, size_t cap)
{
	ssize_t n;

	if (*len + 1 >= cap)
		return KN_COMPAT_PROCESS_ERR_OUTPUT_TOO_LARGE;
	n = read(fd, buf + *len, cap - *len - 1);
	if (n > 0) {
		*len += (size_t)n;
		buf[*len] = '\0';
		return KN_COMPAT_PROCESS_OK;
	}
	if (n == 0)
		return KN_COMPAT_PROCESS_ERR_CHILD;
	if (errno == EAGAIN || errno == EWOULDBLOCK)
		return KN_COMPAT_PROCESS_OK;

	return KN_COMPAT_PROCESS_ERR_CHILD;
}

static void
close_pair(int *fds)
{
	if (fds[0] >= 0)
		(void)close(fds[0]);
	if (fds[1] >= 0)
		(void)close(fds[1]);
}

static int
set_nonblock(int fd)
{
	int flags;

	flags = fcntl(fd, F_GETFL, 0);
	if (flags < 0)
		return -1;
	return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

static enum kn_compat_process_error
wait_capture(pid_t pid, int out_fd, int err_fd,
	struct kn_compat_process_result *result, unsigned int timeout_seconds)
{
	struct pollfd fds[2];
	time_t deadline;
	enum kn_compat_process_error rc;
	int status;
	int child_done;
	int nfds;

	deadline = time(NULL) + (time_t)timeout_seconds;
	child_done = 0;
	status = 0;
	while (out_fd >= 0 || err_fd >= 0 || child_done == 0) {
		if (time(NULL) > deadline) {
			result->timed_out = 1;
			(void)kill(pid, SIGKILL);
			(void)waitpid(pid, &status, 0);
			result->terminated = 1;
			if (out_fd >= 0)
				(void)close(out_fd);
			if (err_fd >= 0)
				(void)close(err_fd);
			return KN_COMPAT_PROCESS_ERR_TIMEOUT;
		}

		nfds = 0;
		if (out_fd >= 0) {
			fds[nfds].fd = out_fd;
			fds[nfds].events = POLLIN | POLLHUP;
			nfds++;
		}
		if (err_fd >= 0) {
			fds[nfds].fd = err_fd;
			fds[nfds].events = POLLIN | POLLHUP;
			nfds++;
		}
		if (nfds > 0)
			(void)poll(fds, (nfds_t)nfds, 50);

		if (out_fd >= 0) {
			rc = append_fd(out_fd, result->stdout_text,
			    &result->stdout_len,
			    sizeof(result->stdout_text));
			if (rc == KN_COMPAT_PROCESS_ERR_OUTPUT_TOO_LARGE)
				goto too_large;
			if (rc == KN_COMPAT_PROCESS_ERR_CHILD) {
				(void)close(out_fd);
				out_fd = -1;
			}
		}
		if (err_fd >= 0) {
			rc = append_fd(err_fd, result->stderr_text,
			    &result->stderr_len,
			    sizeof(result->stderr_text));
			if (rc == KN_COMPAT_PROCESS_ERR_OUTPUT_TOO_LARGE)
				goto too_large;
			if (rc == KN_COMPAT_PROCESS_ERR_CHILD) {
				(void)close(err_fd);
				err_fd = -1;
			}
		}
		if (child_done == 0 && waitpid(pid, &status, WNOHANG) == pid)
			child_done = 1;
	}

	result->exit_status = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
	return result->exit_status == 0 ? KN_COMPAT_PROCESS_OK :
	    KN_COMPAT_PROCESS_ERR_CHILD;

too_large:
	(void)kill(pid, SIGKILL);
	(void)waitpid(pid, NULL, 0);
	if (out_fd >= 0)
		(void)close(out_fd);
	if (err_fd >= 0)
		(void)close(err_fd);
	return KN_COMPAT_PROCESS_ERR_OUTPUT_TOO_LARGE;
}
