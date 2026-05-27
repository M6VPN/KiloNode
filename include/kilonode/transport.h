/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/transport.h */

#ifndef KILONODE_TRANSPORT_H
#define KILONODE_TRANSPORT_H

#include <sys/types.h>

#include <stdint.h>

enum kn_transport_kind {
	KN_TRANSPORT_KIND_NONE = 0,
	KN_TRANSPORT_KIND_STDIO,
	KN_TRANSPORT_KIND_TCP_CLIENT,
	KN_TRANSPORT_KIND_TCP_SERVER,
	KN_TRANSPORT_KIND_SERIAL,
	KN_TRANSPORT_KIND_PTY,
	KN_TRANSPORT_KIND_UNIX_CLIENT,
	KN_TRANSPORT_KIND_UNIX_SERVER,
	KN_TRANSPORT_KIND_MEMORY_TEST
};

enum kn_transport_error {
	KN_TRANSPORT_OK = 0,
	KN_TRANSPORT_ERR_INVALID_ARGUMENT,
	KN_TRANSPORT_ERR_INVALID_CONFIG,
	KN_TRANSPORT_ERR_RESOLVE,
	KN_TRANSPORT_ERR_OPEN,
	KN_TRANSPORT_ERR_CONNECT,
	KN_TRANSPORT_ERR_LISTEN,
	KN_TRANSPORT_ERR_ACCEPT,
	KN_TRANSPORT_ERR_IO,
	KN_TRANSPORT_ERR_EOF,
	KN_TRANSPORT_ERR_NOT_OPEN
};

struct kn_transport {
	enum kn_transport_kind kind;
	int read_fd;
	int write_fd;
	int listen_fd;
	int last_errno;
	uint8_t open;
	uint8_t eof;
};

void kn_transport_close(struct kn_transport *);
const char *kn_transport_error_name(enum kn_transport_error);
int kn_transport_fd(const struct kn_transport *);
const char *kn_transport_kind_name(enum kn_transport_kind);
uint8_t kn_transport_kind_valid(enum kn_transport_kind);
enum kn_transport_error kn_transport_read(struct kn_transport *, uint8_t *,
	size_t, size_t *);
void kn_transport_reset(struct kn_transport *);
enum kn_transport_error kn_transport_write(struct kn_transport *,
	const uint8_t *, size_t);

#endif
