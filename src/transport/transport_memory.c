/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/transport/transport_memory.c */

#include <sys/types.h>

#include <string.h>

#include "kilonode/transport_memory.h"

void
kn_transport_memory_close(struct kn_transport_memory *transport)
{
	if (transport == NULL)
		return;

	transport->open = 0;
}

void
kn_transport_memory_free(struct kn_transport_memory *transport)
{
	if (transport == NULL)
		return;

	memset(transport, 0, sizeof(*transport));
}

enum kn_transport_memory_error
kn_transport_memory_init(struct kn_transport_memory *transport,
	size_t capacity)
{
	if (transport == NULL || capacity == 0 ||
	    capacity > KN_TRANSPORT_MEMORY_CAPACITY_MAX)
		return KN_TRANSPORT_MEMORY_ERR_INVALID_ARGUMENT;

	memset(transport, 0, sizeof(*transport));
	transport->capacity = capacity;
	return KN_TRANSPORT_MEMORY_OK;
}

enum kn_transport_memory_error
kn_transport_memory_open(struct kn_transport_memory *transport)
{
	if (transport == NULL || transport->capacity == 0)
		return KN_TRANSPORT_MEMORY_ERR_INVALID_ARGUMENT;

	transport->open = 1;
	return KN_TRANSPORT_MEMORY_OK;
}

void
kn_transport_memory_reset(struct kn_transport_memory *transport)
{
	if (transport == NULL)
		return;

	transport->len = 0;
	transport->bytes_written = 0;
	transport->write_calls = 0;
	transport->force_failure = 0;
	transport->force_short_write = 0;
	memset(transport->data, 0, sizeof(transport->data));
}

enum kn_transport_memory_error
kn_transport_memory_write(struct kn_transport_memory *transport,
	const uint8_t *buf, size_t buf_len)
{
	size_t write_len;

	if (transport == NULL || (buf == NULL && buf_len > 0))
		return KN_TRANSPORT_MEMORY_ERR_INVALID_ARGUMENT;
	if (transport->open == 0)
		return KN_TRANSPORT_MEMORY_ERR_CLOSED;

	transport->write_calls++;
	if (transport->force_failure != 0)
		return KN_TRANSPORT_MEMORY_ERR_FORCED;

	write_len = buf_len;
	if (transport->force_short_write != 0 &&
	    transport->force_short_write < write_len)
		write_len = transport->force_short_write;
	if (write_len > transport->capacity ||
	    transport->len > transport->capacity - write_len)
		return KN_TRANSPORT_MEMORY_ERR_FULL;

	if (write_len > 0)
		memcpy(transport->data + transport->len, buf, write_len);
	transport->len += write_len;
	transport->bytes_written += (uint64_t)write_len;

	if (write_len != buf_len)
		return KN_TRANSPORT_MEMORY_ERR_SHORT_WRITE;
	return KN_TRANSPORT_MEMORY_OK;
}
