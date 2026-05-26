/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_control_socket.c */

#define _POSIX_C_SOURCE 200809L

#include <sys/types.h>
#include <sys/wait.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "kilonode/control.h"

static int child_ping(const char *);
static int test_invalid_path(void);
static int test_missing_socket_connect(void);
static int test_ping_round_trip(void);
static int test_server_start_close(void);
static int test_too_long_path(void);

int
main(void)
{
	if (test_invalid_path() != 0)
		return 1;
	if (test_too_long_path() != 0)
		return 1;
	if (test_missing_socket_connect() != 0)
		return 1;
	if (test_server_start_close() != 0)
		return 1;
	if (test_ping_round_trip() != 0)
		return 1;

	return 0;
}

static int
child_ping(const char *path)
{
	char response[64];
	int fd;
	ssize_t nread;
	size_t i;

	for (i = 0; i < 200000; i++) {
		if (kn_control_socket_connect(path, &fd) == KN_CONTROL_OK) {
			if (kn_control_socket_write(fd, "PING\n",
			    5) != KN_CONTROL_OK) {
				(void)close(fd);
				return 1;
			}
			nread = read(fd, response, sizeof(response) - 1);
			if (nread <= 0) {
				(void)close(fd);
				return 1;
			}
			response[nread] = '\0';
			(void)close(fd);
			return strcmp(response, "OK PONG\n") == 0 ? 0 : 1;
		}
	}

	return 1;
}

static int
test_invalid_path(void)
{
	if (kn_control_socket_path_valid("") != 0)
		return 1;
	if (kn_control_socket_path_valid(NULL) != 0)
		return 1;

	return 0;
}

static int
test_missing_socket_connect(void)
{
	char path[128];
	int fd;

	(void)snprintf(path, sizeof(path), "/tmp/kilonode-control-missing-%ld.sock",
	    (long)getpid());
	(void)unlink(path);

	return kn_control_socket_connect(path, &fd) == KN_CONTROL_ERR_CONNECT ?
	    0 : 1;
}

static int
test_ping_round_trip(void)
{
	struct kn_control_socket control;
	char command[KN_CONTROL_COMMAND_MAX];
	char path[128];
	size_t command_len;
	int status;
	pid_t pid;

	(void)snprintf(path, sizeof(path), "/tmp/kilonode-control-%ld.sock",
	    (long)getpid());
	(void)unlink(path);

	if (kn_control_socket_open(&control, path) != KN_CONTROL_OK)
		return 1;

	pid = fork();
	if (pid < 0) {
		kn_control_socket_close(&control);
		return 1;
	}

	if (pid == 0)
		_exit(child_ping(path));

	if (kn_control_socket_read_command(&control, command, sizeof(command),
	    &command_len) != KN_CONTROL_OK) {
		kn_control_socket_close(&control);
		(void)waitpid(pid, &status, 0);
		return 1;
	}

	if (strcmp(command, "PING") != 0 || command_len != 4) {
		kn_control_socket_close(&control);
		(void)waitpid(pid, &status, 0);
		return 1;
	}

	if (kn_control_socket_write(control.client_fd, "OK PONG\n",
	    8) != KN_CONTROL_OK) {
		kn_control_socket_close(&control);
		(void)waitpid(pid, &status, 0);
		return 1;
	}

	kn_control_socket_close(&control);
	(void)waitpid(pid, &status, 0);

	return WIFEXITED(status) && WEXITSTATUS(status) == 0 ? 0 : 1;
}

static int
test_server_start_close(void)
{
	struct kn_control_socket control;
	char path[128];

	(void)snprintf(path, sizeof(path), "/tmp/kilonode-control-close-%ld.sock",
	    (long)getpid());
	(void)unlink(path);

	if (kn_control_socket_open(&control, path) != KN_CONTROL_OK)
		return 1;
	if (kn_control_socket_fd(&control) < 0) {
		kn_control_socket_close(&control);
		return 1;
	}

	kn_control_socket_close(&control);
	return access(path, F_OK) != 0 ? 0 : 1;
}

static int
test_too_long_path(void)
{
	char path[300];

	memset(path, 'a', sizeof(path));
	path[sizeof(path) - 1] = '\0';

	return kn_control_socket_path_valid(path) == 0 ? 0 : 1;
}
