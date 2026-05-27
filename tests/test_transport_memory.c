/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_transport_memory.c */

#include <sys/types.h>

#include <string.h>

#include "kilonode/transport_memory.h"

static int test_capacity_limit(void);
static int test_forced_failure(void);
static int test_forced_short_write(void);
static int test_init_empty(void);
static int test_reset(void);
static int test_write_readback(void);

int
main(void)
{
	if (test_init_empty() != 0)
		return 1;
	if (test_write_readback() != 0)
		return 1;
	if (test_reset() != 0)
		return 1;
	if (test_capacity_limit() != 0)
		return 1;
	if (test_forced_failure() != 0)
		return 1;
	if (test_forced_short_write() != 0)
		return 1;

	return 0;
}

static int
test_capacity_limit(void)
{
	const uint8_t data[] = { 1, 2, 3, 4 };
	struct kn_transport_memory transport;

	if (kn_transport_memory_init(&transport, 3) !=
	    KN_TRANSPORT_MEMORY_OK)
		return 1;
	if (kn_transport_memory_open(&transport) != KN_TRANSPORT_MEMORY_OK)
		return 1;

	return kn_transport_memory_write(&transport, data, sizeof(data)) ==
	    KN_TRANSPORT_MEMORY_ERR_FULL ? 0 : 1;
}

static int
test_forced_failure(void)
{
	const uint8_t data[] = { 1 };
	struct kn_transport_memory transport;

	if (kn_transport_memory_init(&transport, 8) !=
	    KN_TRANSPORT_MEMORY_OK)
		return 1;
	if (kn_transport_memory_open(&transport) != KN_TRANSPORT_MEMORY_OK)
		return 1;
	transport.force_failure = 1;

	return kn_transport_memory_write(&transport, data, sizeof(data)) ==
	    KN_TRANSPORT_MEMORY_ERR_FORCED ? 0 : 1;
}

static int
test_forced_short_write(void)
{
	const uint8_t data[] = { 1, 2, 3, 4 };
	struct kn_transport_memory transport;

	if (kn_transport_memory_init(&transport, 8) !=
	    KN_TRANSPORT_MEMORY_OK)
		return 1;
	if (kn_transport_memory_open(&transport) != KN_TRANSPORT_MEMORY_OK)
		return 1;
	transport.force_short_write = 2;
	if (kn_transport_memory_write(&transport, data, sizeof(data)) !=
	    KN_TRANSPORT_MEMORY_ERR_SHORT_WRITE)
		return 1;
	if (transport.len != 2 || transport.bytes_written != 2)
		return 1;

	return memcmp(transport.data, data, 2) == 0 ? 0 : 1;
}

static int
test_init_empty(void)
{
	struct kn_transport_memory transport;

	if (kn_transport_memory_init(&transport, 8) !=
	    KN_TRANSPORT_MEMORY_OK)
		return 1;
	if (transport.len != 0 || transport.write_calls != 0 ||
	    transport.open != 0)
		return 1;

	return transport.capacity == 8 ? 0 : 1;
}

static int
test_reset(void)
{
	const uint8_t data[] = { 1, 2 };
	struct kn_transport_memory transport;

	if (kn_transport_memory_init(&transport, 8) !=
	    KN_TRANSPORT_MEMORY_OK)
		return 1;
	if (kn_transport_memory_open(&transport) != KN_TRANSPORT_MEMORY_OK)
		return 1;
	if (kn_transport_memory_write(&transport, data, sizeof(data)) !=
	    KN_TRANSPORT_MEMORY_OK)
		return 1;
	kn_transport_memory_reset(&transport);

	return transport.len == 0 && transport.bytes_written == 0 &&
	    transport.write_calls == 0 ? 0 : 1;
}

static int
test_write_readback(void)
{
	const uint8_t data[] = { 0xc0, 0x00, 0xdb };
	struct kn_transport_memory transport;

	if (kn_transport_memory_init(&transport, 8) !=
	    KN_TRANSPORT_MEMORY_OK)
		return 1;
	if (kn_transport_memory_open(&transport) != KN_TRANSPORT_MEMORY_OK)
		return 1;
	if (kn_transport_memory_write(&transport, data, sizeof(data)) !=
	    KN_TRANSPORT_MEMORY_OK)
		return 1;
	if (transport.len != sizeof(data) ||
	    transport.bytes_written != sizeof(data) ||
	    transport.write_calls != 1)
		return 1;

	return memcmp(transport.data, data, sizeof(data)) == 0 ? 0 : 1;
}
