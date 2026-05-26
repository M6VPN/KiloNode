/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_buffer.c */

#include <sys/types.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "kilonode/buffer.h"

static int test_buffer_growth(void);

int
main(void)
{
	if (test_buffer_growth() != 0)
		return 1;

	return 0;
}

static int
test_buffer_growth(void)
{
	struct kn_buffer buf;
	uint8_t data[128];
	size_t i;

	if (kn_buffer_init(&buf, 1) != 0)
		return 1;

	for (i = 0; i < sizeof(data); i++)
		data[i] = (uint8_t)i;

	if (kn_buffer_append(&buf, data, sizeof(data)) != 0) {
		kn_buffer_free(&buf);
		return 1;
	}

	if (buf.len != sizeof(data) || buf.cap < sizeof(data)) {
		kn_buffer_free(&buf);
		return 1;
	}

	if (memcmp(buf.data, data, sizeof(data)) != 0) {
		kn_buffer_free(&buf);
		return 1;
	}

	kn_buffer_free(&buf);
	return 0;
}
