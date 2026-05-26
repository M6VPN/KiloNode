/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/transport/transport_unix.c */

#define _POSIX_C_SOURCE 200809L

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>

#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include "kilonode/transport.h"
#include "kilonode/transport_unix.h"

static enum kn_transport_error accept_one(struct kn_transport *);
static enum kn_transport_error sockaddr_from_path(const char *,
	struct sockaddr_un *);

static enum kn_transport_error
accept_one(struct kn_transport *transport)
{
	int fd;

	for (;;) {
		fd = accept(transport->listen_fd, NULL, NULL);
		if (fd >= 0)
			break;
		if (errno == EINTR)
			continue;
		transport->last_errno = errno;
		return KN_TRANSPORT_ERR_ACCEPT;
	}

	(void)close(transport->listen_fd);
	transport->listen_fd = -1;
	transport->read_fd = fd;
	transport->write_fd = fd;
	transport->open = 1;

	return KN_TRANSPORT_OK;
}

enum kn_transport_error
kn_transport_unix_connect_open(struct kn_transport *transport, const char *path)
{
	struct sockaddr_un addr;
	enum kn_transport_error rc;
	int fd;

	if (transport == NULL || path == NULL)
		return KN_TRANSPORT_ERR_INVALID_ARGUMENT;

	rc = sockaddr_from_path(path, &addr);
	if (rc != KN_TRANSPORT_OK)
		return rc;

	kn_transport_reset(transport);
	fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (fd < 0) {
		transport->last_errno = errno;
		return KN_TRANSPORT_ERR_OPEN;
	}

	if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
		transport->last_errno = errno;
		(void)close(fd);
		return KN_TRANSPORT_ERR_CONNECT;
	}

	transport->kind = KN_TRANSPORT_KIND_UNIX_CLIENT;
	transport->read_fd = fd;
	transport->write_fd = fd;
	transport->listen_fd = -1;
	transport->open = 1;

	return KN_TRANSPORT_OK;
}

enum kn_transport_error
kn_transport_unix_listen_open(struct kn_transport *transport, const char *path)
{
	struct sockaddr_un addr;
	enum kn_transport_error rc;
	int fd;

	if (transport == NULL || path == NULL)
		return KN_TRANSPORT_ERR_INVALID_ARGUMENT;

	rc = sockaddr_from_path(path, &addr);
	if (rc != KN_TRANSPORT_OK)
		return rc;

	kn_transport_reset(transport);
	fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (fd < 0) {
		transport->last_errno = errno;
		return KN_TRANSPORT_ERR_OPEN;
	}

	(void)unlink(path);

	if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) != 0 ||
	    listen(fd, 1) != 0) {
		transport->last_errno = errno;
		(void)close(fd);
		return KN_TRANSPORT_ERR_LISTEN;
	}

	transport->kind = KN_TRANSPORT_KIND_UNIX_SERVER;
	transport->listen_fd = fd;
	rc = accept_one(transport);
	if (rc != KN_TRANSPORT_OK)
		kn_transport_close(transport);

	return rc;
}

uint8_t
kn_transport_unix_path_valid(const char *path)
{
	struct sockaddr_un addr;

	return sockaddr_from_path(path, &addr) == KN_TRANSPORT_OK ? 1 : 0;
}

static enum kn_transport_error
sockaddr_from_path(const char *path, struct sockaddr_un *addr)
{
	size_t path_len;

	if (path == NULL || addr == NULL)
		return KN_TRANSPORT_ERR_INVALID_ARGUMENT;

	path_len = strlen(path);
	if (path_len == 0 || path_len >= sizeof(addr->sun_path))
		return KN_TRANSPORT_ERR_INVALID_CONFIG;

	memset(addr, 0, sizeof(*addr));
	addr->sun_family = AF_UNIX;
	memcpy(addr->sun_path, path, path_len + 1);

	return KN_TRANSPORT_OK;
}
