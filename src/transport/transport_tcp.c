/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/transport/transport_tcp.c */

#define _POSIX_C_SOURCE 200809L

#include <sys/socket.h>
#include <sys/types.h>

#include <errno.h>
#include <netdb.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "kilonode/transport.h"
#include "kilonode/transport_tcp.h"

static int socket_open(const struct addrinfo *);

enum kn_transport_error
kn_transport_tcp_connect_open(struct kn_transport *transport, const char *host,
	const char *port)
{
	struct addrinfo hints;
	struct addrinfo *result;
	struct addrinfo *rp;
	int fd;
	int gai_rc;

	if (transport == NULL || host == NULL || port == NULL)
		return KN_TRANSPORT_ERR_INVALID_ARGUMENT;

	if (host[0] == '\0' || kn_transport_tcp_port_valid(port) == 0)
		return KN_TRANSPORT_ERR_INVALID_CONFIG;

	kn_transport_reset(transport);
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	gai_rc = getaddrinfo(host, port, &hints, &result);
	if (gai_rc != 0)
		return KN_TRANSPORT_ERR_RESOLVE;

	for (rp = result; rp != NULL; rp = rp->ai_next) {
		fd = socket_open(rp);
		if (fd < 0)
			continue;

		if (connect(fd, rp->ai_addr, rp->ai_addrlen) == 0) {
			freeaddrinfo(result);
			transport->kind = KN_TRANSPORT_KIND_TCP_CLIENT;
			transport->read_fd = fd;
			transport->write_fd = fd;
			transport->listen_fd = -1;
			transport->open = 1;
			return KN_TRANSPORT_OK;
		}

		transport->last_errno = errno;
		(void)close(fd);
	}

	freeaddrinfo(result);
	return KN_TRANSPORT_ERR_CONNECT;
}

enum kn_transport_error
kn_transport_tcp_listen_open(struct kn_transport *transport, const char *host,
	const char *port)
{
	struct addrinfo hints;
	struct addrinfo *result;
	struct addrinfo *rp;
	int fd;
	int gai_rc;
	int yes;

	if (transport == NULL || host == NULL || port == NULL)
		return KN_TRANSPORT_ERR_INVALID_ARGUMENT;

	if (host[0] == '\0' || kn_transport_tcp_port_valid(port) == 0)
		return KN_TRANSPORT_ERR_INVALID_CONFIG;

	kn_transport_reset(transport);
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	gai_rc = getaddrinfo(host, port, &hints, &result);
	if (gai_rc != 0)
		return KN_TRANSPORT_ERR_RESOLVE;

	for (rp = result; rp != NULL; rp = rp->ai_next) {
		fd = socket_open(rp);
		if (fd < 0)
			continue;

		yes = 1;
		(void)setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes,
		    sizeof(yes));

		if (bind(fd, rp->ai_addr, rp->ai_addrlen) == 0 &&
		    listen(fd, 1) == 0) {
			freeaddrinfo(result);
			transport->kind = KN_TRANSPORT_KIND_TCP_SERVER;
			transport->listen_fd = fd;
			transport->read_fd = -1;
			transport->write_fd = -1;
			transport->open = 1;
			return KN_TRANSPORT_OK;
		}

		transport->last_errno = errno;
		(void)close(fd);
	}

	freeaddrinfo(result);
	return KN_TRANSPORT_ERR_LISTEN;
}

uint8_t
kn_transport_tcp_port_valid(const char *port)
{
	char *end;
	unsigned long value;

	if (port == NULL || port[0] == '\0')
		return 0;

	errno = 0;
	value = strtoul(port, &end, 10);
	if (errno != 0 || *end != '\0')
		return 0;

	if (value == 0 || value > 65535)
		return 0;

	return 1;
}

static int
socket_open(const struct addrinfo *addr)
{
	int fd;

	fd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
	if (fd < 0)
		return -1;

	return fd;
}
