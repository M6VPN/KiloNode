/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/control/control_socket.c */

#define _POSIX_C_SOURCE 200809L

#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "kilonode/control.h"

static enum kn_control_error sockaddr_from_path(const char *,
	struct sockaddr_un *);

void
kn_control_socket_close(struct kn_control_socket *control)
{
	if (control == NULL)
		return;

	if (control->client_fd >= 0)
		(void)close(control->client_fd);
	if (control->listen_fd >= 0)
		(void)close(control->listen_fd);
	if (control->path[0] != '\0')
		(void)unlink(control->path);

	kn_control_socket_init(control);
}

enum kn_control_error
kn_control_socket_connect(const char *path, int *fd_out)
{
	struct sockaddr_un addr;
	enum kn_control_error rc;
	int fd;

	if (path == NULL || fd_out == NULL)
		return KN_CONTROL_ERR_INVALID_ARGUMENT;

	rc = sockaddr_from_path(path, &addr);
	if (rc != KN_CONTROL_OK)
		return rc;

	fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (fd < 0)
		return KN_CONTROL_ERR_OPEN;

	if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
		(void)close(fd);
		return KN_CONTROL_ERR_CONNECT;
	}

	*fd_out = fd;
	return KN_CONTROL_OK;
}

int
kn_control_socket_fd(const struct kn_control_socket *control)
{
	if (control == NULL)
		return -1;

	if (control->client_fd >= 0)
		return control->client_fd;

	return control->listen_fd;
}

void
kn_control_socket_init(struct kn_control_socket *control)
{
	if (control == NULL)
		return;

	memset(control, 0, sizeof(*control));
	control->listen_fd = -1;
	control->client_fd = -1;
}

enum kn_control_error
kn_control_socket_open(struct kn_control_socket *control, const char *path)
{
	struct sockaddr_un addr;
	enum kn_control_error rc;
	int fd;
	int needed;

	if (control == NULL || path == NULL)
		return KN_CONTROL_ERR_INVALID_ARGUMENT;

	rc = sockaddr_from_path(path, &addr);
	if (rc != KN_CONTROL_OK)
		return rc;

	kn_control_socket_init(control);
	fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (fd < 0)
		return KN_CONTROL_ERR_OPEN;

	(void)unlink(path);
	if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) != 0 ||
	    listen(fd, 4) != 0) {
		(void)close(fd);
		return KN_CONTROL_ERR_OPEN;
	}

	(void)chmod(path, 0600);
	needed = snprintf(control->path, sizeof(control->path), "%s", path);
	if (needed < 0 || (size_t)needed >= sizeof(control->path)) {
		(void)close(fd);
		(void)unlink(path);
		return KN_CONTROL_ERR_INVALID_PATH;
	}
	control->listen_fd = fd;
	return KN_CONTROL_OK;
}

enum kn_control_error
kn_control_socket_read_command(struct kn_control_socket *control, char *buf,
	size_t bufsiz, size_t *command_len)
{
	size_t offset;
	ssize_t nread;
	char ch;
	int fd;

	if (control == NULL || buf == NULL || bufsiz == 0 ||
	    command_len == NULL)
		return KN_CONTROL_ERR_INVALID_ARGUMENT;

	*command_len = 0;

	if (control->client_fd < 0) {
		for (;;) {
			fd = accept(control->listen_fd, NULL, NULL);
			if (fd >= 0)
				break;
			if (errno == EINTR)
				continue;
			return KN_CONTROL_ERR_IO;
		}
		control->client_fd = fd;
	}

	offset = 0;
	for (;;) {
		nread = read(control->client_fd, &ch, 1);
		if (nread == 1) {
			if (ch == '\n')
				break;
			if (offset + 1 >= bufsiz)
				return KN_CONTROL_ERR_OVERLONG_COMMAND;
			buf[offset++] = ch == '\r' ? '\0' : ch;
			if (ch == '\r')
				break;
			continue;
		}
		if (nread == 0)
			break;
		if (errno == EINTR)
			continue;
		return KN_CONTROL_ERR_IO;
	}

	buf[offset] = '\0';
	*command_len = offset;
	return KN_CONTROL_OK;
}

enum kn_control_error
kn_control_socket_write(int fd, const char *buf, size_t buf_len)
{
	size_t offset;
	ssize_t nwritten;

	if (fd < 0 || (buf == NULL && buf_len > 0))
		return KN_CONTROL_ERR_INVALID_ARGUMENT;

	offset = 0;
	while (offset < buf_len) {
		nwritten = write(fd, buf + offset, buf_len - offset);
		if (nwritten > 0) {
			offset += (size_t)nwritten;
			continue;
		}
		if (nwritten < 0 && errno == EINTR)
			continue;
		return KN_CONTROL_ERR_IO;
	}

	return KN_CONTROL_OK;
}

uint8_t
kn_control_socket_path_valid(const char *path)
{
	struct sockaddr_un addr;

	return sockaddr_from_path(path, &addr) == KN_CONTROL_OK ? 1 : 0;
}

static enum kn_control_error
sockaddr_from_path(const char *path, struct sockaddr_un *addr)
{
	size_t path_len;

	if (path == NULL || addr == NULL)
		return KN_CONTROL_ERR_INVALID_ARGUMENT;

	path_len = strlen(path);
	if (path_len == 0 || path_len >= sizeof(addr->sun_path) ||
	    path_len >= KN_CONFIG_PATH_MAX)
		return KN_CONTROL_ERR_INVALID_PATH;

	memset(addr, 0, sizeof(*addr));
	addr->sun_family = AF_UNIX;
	memcpy(addr->sun_path, path, path_len + 1);

	return KN_CONTROL_OK;
}
