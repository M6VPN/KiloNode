/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/transport_memory.h */

#ifndef KILONODE_TRANSPORT_MEMORY_H
#define KILONODE_TRANSPORT_MEMORY_H

#include <sys/types.h>

#include <stdint.h>

#define KN_TRANSPORT_MEMORY_CAPACITY_DEFAULT 65536
#define KN_TRANSPORT_MEMORY_CAPACITY_MAX     262144

enum kn_transport_memory_error {
	KN_TRANSPORT_MEMORY_OK = 0,
	KN_TRANSPORT_MEMORY_ERR_INVALID_ARGUMENT,
	KN_TRANSPORT_MEMORY_ERR_CLOSED,
	KN_TRANSPORT_MEMORY_ERR_FULL,
	KN_TRANSPORT_MEMORY_ERR_SHORT_WRITE,
	KN_TRANSPORT_MEMORY_ERR_FORCED
};

struct kn_transport_memory {
	uint8_t data[KN_TRANSPORT_MEMORY_CAPACITY_MAX];
	size_t capacity;
	size_t len;
	uint64_t bytes_written;
	uint64_t write_calls;
	uint8_t open;
	uint8_t force_failure;
	size_t force_short_write;
};

void kn_transport_memory_close(struct kn_transport_memory *);
void kn_transport_memory_free(struct kn_transport_memory *);
enum kn_transport_memory_error kn_transport_memory_init(
	struct kn_transport_memory *, size_t);
enum kn_transport_memory_error kn_transport_memory_open(
	struct kn_transport_memory *);
void kn_transport_memory_reset(struct kn_transport_memory *);
enum kn_transport_memory_error kn_transport_memory_write(
	struct kn_transport_memory *, const uint8_t *, size_t);

#endif
