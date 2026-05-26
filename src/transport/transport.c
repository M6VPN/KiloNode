/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/transport/transport.c */

#define _POSIX_C_SOURCE 200809L

#include <sys/socket.h>
#include <sys/types.h>

#include <errno.h>
#include <stdint.h>
#include <unistd.h>

#include "kilonode/transport.h"

static enum kn_transport_error transport_accept(struct kn_transport *);

static enum kn_transport_error
transport_accept(struct kn_transport *transport)
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
	return KN_TRANSPORT_OK;
}

void
kn_transport_close(struct kn_transport *transport)
{
	if (transport == NULL)
		return;

	if (transport->read_fd >= 0)
		(void)close(transport->read_fd);
	if (transport->write_fd >= 0 && transport->write_fd != transport->read_fd)
		(void)close(transport->write_fd);
	if (transport->listen_fd >= 0)
		(void)close(transport->listen_fd);

	kn_transport_reset(transport);
}

const char *
kn_transport_error_name(enum kn_transport_error error)
{
	switch (error) {
	case KN_TRANSPORT_OK:
		return "ok";
	case KN_TRANSPORT_ERR_INVALID_ARGUMENT:
		return "invalid argument";
	case KN_TRANSPORT_ERR_INVALID_CONFIG:
		return "invalid config";
	case KN_TRANSPORT_ERR_RESOLVE:
		return "resolve failed";
	case KN_TRANSPORT_ERR_OPEN:
		return "open failed";
	case KN_TRANSPORT_ERR_CONNECT:
		return "connect failed";
	case KN_TRANSPORT_ERR_LISTEN:
		return "listen failed";
	case KN_TRANSPORT_ERR_ACCEPT:
		return "accept failed";
	case KN_TRANSPORT_ERR_IO:
		return "io failed";
	case KN_TRANSPORT_ERR_EOF:
		return "eof";
	case KN_TRANSPORT_ERR_NOT_OPEN:
		return "not open";
	}

	return "unknown";
}

int
kn_transport_fd(const struct kn_transport *transport)
{
	if (transport == NULL || transport->open == 0)
		return -1;

	if (transport->read_fd >= 0)
		return transport->read_fd;

	return transport->listen_fd;
}

const char *
kn_transport_kind_name(enum kn_transport_kind kind)
{
	switch (kind) {
	case KN_TRANSPORT_KIND_NONE:
		return "none";
	case KN_TRANSPORT_KIND_STDIO:
		return "stdio";
	case KN_TRANSPORT_KIND_TCP_CLIENT:
		return "tcp-client";
	case KN_TRANSPORT_KIND_TCP_SERVER:
		return "tcp-server";
	case KN_TRANSPORT_KIND_SERIAL:
		return "serial";
	case KN_TRANSPORT_KIND_PTY:
		return "pty";
	case KN_TRANSPORT_KIND_UNIX_CLIENT:
		return "unix-client";
	case KN_TRANSPORT_KIND_UNIX_SERVER:
		return "unix-server";
	}

	return "unknown";
}

uint8_t
kn_transport_kind_valid(enum kn_transport_kind kind)
{
	switch (kind) {
	case KN_TRANSPORT_KIND_STDIO:
	case KN_TRANSPORT_KIND_TCP_CLIENT:
	case KN_TRANSPORT_KIND_TCP_SERVER:
	case KN_TRANSPORT_KIND_SERIAL:
	case KN_TRANSPORT_KIND_PTY:
	case KN_TRANSPORT_KIND_UNIX_CLIENT:
	case KN_TRANSPORT_KIND_UNIX_SERVER:
		return 1;
	case KN_TRANSPORT_KIND_NONE:
		return 0;
	}

	return 0;
}

enum kn_transport_error
kn_transport_read(struct kn_transport *transport, uint8_t *buf, size_t bufsiz,
	size_t *read_len)
{
	ssize_t nread;

	if (read_len != NULL)
		*read_len = 0;

	if (transport == NULL || buf == NULL || read_len == NULL || bufsiz == 0)
		return KN_TRANSPORT_ERR_INVALID_ARGUMENT;

	if (transport->open == 0)
		return KN_TRANSPORT_ERR_NOT_OPEN;

	if (transport->read_fd < 0) {
		if (transport->listen_fd >= 0) {
			enum kn_transport_error rc;

			rc = transport_accept(transport);
			if (rc != KN_TRANSPORT_OK)
				return rc;
		} else {
			return KN_TRANSPORT_ERR_NOT_OPEN;
		}
	}

	for (;;) {
		nread = read(transport->read_fd, buf, bufsiz);
		if (nread > 0) {
			*read_len = (size_t)nread;
			return KN_TRANSPORT_OK;
		}

		if (nread == 0) {
			transport->eof = 1;
			return KN_TRANSPORT_ERR_EOF;
		}

		if (errno == EINTR)
			continue;

		transport->last_errno = errno;
		return KN_TRANSPORT_ERR_IO;
	}
}

void
kn_transport_reset(struct kn_transport *transport)
{
	if (transport == NULL)
		return;

	transport->kind = KN_TRANSPORT_KIND_NONE;
	transport->read_fd = -1;
	transport->write_fd = -1;
	transport->listen_fd = -1;
	transport->last_errno = 0;
	transport->open = 0;
	transport->eof = 0;
}

enum kn_transport_error
kn_transport_write(struct kn_transport *transport, const uint8_t *buf,
	size_t buf_len)
{
	size_t offset;
	ssize_t nwritten;

	if (transport == NULL || (buf == NULL && buf_len > 0))
		return KN_TRANSPORT_ERR_INVALID_ARGUMENT;

	if (transport->open == 0 || transport->write_fd < 0)
		return KN_TRANSPORT_ERR_NOT_OPEN;

	offset = 0;
	while (offset < buf_len) {
		nwritten = write(transport->write_fd, buf + offset,
		    buf_len - offset);
		if (nwritten > 0) {
			offset += (size_t)nwritten;
			continue;
		}

		if (nwritten < 0 && errno == EINTR)
			continue;

		transport->last_errno = errno;
		return KN_TRANSPORT_ERR_IO;
	}

	return KN_TRANSPORT_OK;
}
