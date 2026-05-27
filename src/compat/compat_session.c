/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/compat/compat_session.c */

#define _POSIX_C_SOURCE 200809L

#include <sys/types.h>
#include <sys/socket.h>

#include <errno.h>
#include <netdb.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "kilonode/compat_session.h"

static enum kn_compat_session_error read_response(int,
	struct kn_compat_session_result *, unsigned int);
static uint8_t valid_host(const char *);
static uint8_t valid_port(const char *);

void
kn_compat_session_result_clear(struct kn_compat_session_result *result)
{
	if (result == NULL)
		return;

	memset(result, 0, sizeof(*result));
}

const char *
kn_compat_session_error_name(enum kn_compat_session_error error)
{
	switch (error) {
	case KN_COMPAT_SESSION_OK:
		return "ok";
	case KN_COMPAT_SESSION_ERR_INVALID_ARGUMENT:
		return "invalid-argument";
	case KN_COMPAT_SESSION_ERR_INVALID_HOST:
		return "invalid-host";
	case KN_COMPAT_SESSION_ERR_INVALID_PORT:
		return "invalid-port";
	case KN_COMPAT_SESSION_ERR_COMMAND_TOO_LARGE:
		return "command-too-large";
	case KN_COMPAT_SESSION_ERR_CONNECT:
		return "connect";
	case KN_COMPAT_SESSION_ERR_TIMEOUT:
		return "timeout";
	case KN_COMPAT_SESSION_ERR_IO:
		return "io";
	case KN_COMPAT_SESSION_ERR_RESPONSE_TOO_LARGE:
		return "response-too-large";
	}

	return "unknown";
}

enum kn_compat_session_error
kn_compat_session_tcp_line(const char *host, const char *port,
	const char *command, unsigned int timeout_seconds,
	struct kn_compat_session_result *result)
{
	struct addrinfo hints;
	struct addrinfo *res;
	struct addrinfo *ai;
	int fd;
	int gai;
	size_t command_len;
	enum kn_compat_session_error rc;

	if (command == NULL || result == NULL || timeout_seconds == 0)
		return KN_COMPAT_SESSION_ERR_INVALID_ARGUMENT;
	rc = kn_compat_session_validate_endpoint(host, port);
	if (rc != KN_COMPAT_SESSION_OK)
		return rc;
	command_len = strlen(command);
	if (command_len >= KN_COMPAT_SESSION_COMMAND_MAX)
		return KN_COMPAT_SESSION_ERR_COMMAND_TOO_LARGE;

	kn_compat_session_result_clear(result);
	memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_family = AF_UNSPEC;
	gai = getaddrinfo(host, port, &hints, &res);
	if (gai != 0)
		return KN_COMPAT_SESSION_ERR_CONNECT;

	fd = -1;
	for (ai = res; ai != NULL; ai = ai->ai_next) {
		fd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
		if (fd < 0)
			continue;
		if (connect(fd, ai->ai_addr, ai->ai_addrlen) == 0)
			break;
		(void)close(fd);
		fd = -1;
	}
	freeaddrinfo(res);
	if (fd < 0)
		return KN_COMPAT_SESSION_ERR_CONNECT;

	if (write(fd, command, command_len) < 0 ||
	    write(fd, "\n", 1) < 0) {
		(void)close(fd);
		return KN_COMPAT_SESSION_ERR_IO;
	}
	(void)shutdown(fd, SHUT_WR);
	rc = read_response(fd, result, timeout_seconds);
	(void)close(fd);
	return rc;
}

enum kn_compat_session_error
kn_compat_session_validate_endpoint(const char *host, const char *port)
{
	if (valid_host(host) == 0)
		return KN_COMPAT_SESSION_ERR_INVALID_HOST;
	if (valid_port(port) == 0)
		return KN_COMPAT_SESSION_ERR_INVALID_PORT;

	return KN_COMPAT_SESSION_OK;
}

static enum kn_compat_session_error
read_response(int fd, struct kn_compat_session_result *result,
	unsigned int timeout_seconds)
{
	struct pollfd pfd;
	int rv;
	ssize_t n;
	unsigned int elapsed;

	elapsed = 0;
	while (1) {
		pfd.fd = fd;
		pfd.events = POLLIN | POLLHUP;
		rv = poll(&pfd, 1, 100);
		if (rv < 0) {
			if (errno == EINTR)
				continue;
			return KN_COMPAT_SESSION_ERR_IO;
		}
		if (rv == 0) {
			elapsed++;
			if (elapsed >= timeout_seconds * 10)
				return KN_COMPAT_SESSION_ERR_TIMEOUT;
			continue;
		}
		if ((pfd.revents & POLLIN) != 0) {
			if (result->response_len + 1 >=
			    sizeof(result->response))
				return KN_COMPAT_SESSION_ERR_RESPONSE_TOO_LARGE;
			n = read(fd, result->response + result->response_len,
			    sizeof(result->response) - result->response_len - 1);
			if (n > 0) {
				result->response_len += (size_t)n;
				result->response[result->response_len] = '\0';
				continue;
			}
			if (n == 0)
				return KN_COMPAT_SESSION_OK;
			return KN_COMPAT_SESSION_ERR_IO;
		}
		if ((pfd.revents & POLLHUP) != 0)
			return KN_COMPAT_SESSION_OK;
	}
}

static uint8_t
valid_host(const char *host)
{
	size_t i;

	if (host == NULL || host[0] == '\0' || strlen(host) > 128)
		return 0;
	for (i = 0; host[i] != '\0'; i++) {
		if (host[i] == '/' || host[i] == '\\' || host[i] == ';')
			return 0;
	}

	return 1;
}

static uint8_t
valid_port(const char *port)
{
	char *end;
	long value;

	if (port == NULL || port[0] == '\0')
		return 0;
	value = strtol(port, &end, 10);
	if (*end != '\0')
		return 0;

	return value > 0 && value <= 65535 ? 1 : 0;
}
