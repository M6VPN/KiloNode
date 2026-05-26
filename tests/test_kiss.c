/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_kiss.c */

#include <sys/types.h>

#include <stdint.h>
#include <string.h>

#include "kilonode/kiss.h"

static int test_kiss_escape(void);
static int test_kiss_unescape_invalid(void);

int
main(void)
{
	if (test_kiss_escape() != 0)
		return 1;

	if (test_kiss_unescape_invalid() != 0)
		return 1;

	return 0;
}

static int
test_kiss_escape(void)
{
	struct kn_buffer escaped;
	struct kn_buffer unescaped;
	const uint8_t input[] = { 0x01, KN_KISS_FEND, 0x02, KN_KISS_FESC, 0x03 };
	const uint8_t expected[] = {
		0x01,
		KN_KISS_FESC,
		KN_KISS_TFEND,
		0x02,
		KN_KISS_FESC,
		KN_KISS_TFESC,
		0x03
	};

	if (kn_buffer_init(&escaped, 0) != 0)
		return 1;
	if (kn_buffer_init(&unescaped, 0) != 0) {
		kn_buffer_free(&escaped);
		return 1;
	}

	if (kn_kiss_escape(input, sizeof(input), &escaped) != 0) {
		kn_buffer_free(&escaped);
		kn_buffer_free(&unescaped);
		return 1;
	}

	if (escaped.len != sizeof(expected) ||
	    memcmp(escaped.data, expected, sizeof(expected)) != 0) {
		kn_buffer_free(&escaped);
		kn_buffer_free(&unescaped);
		return 1;
	}

	if (kn_kiss_unescape(escaped.data, escaped.len, &unescaped) != 0) {
		kn_buffer_free(&escaped);
		kn_buffer_free(&unescaped);
		return 1;
	}

	if (unescaped.len != sizeof(input) ||
	    memcmp(unescaped.data, input, sizeof(input)) != 0) {
		kn_buffer_free(&escaped);
		kn_buffer_free(&unescaped);
		return 1;
	}

	kn_buffer_free(&escaped);
	kn_buffer_free(&unescaped);
	return 0;
}

static int
test_kiss_unescape_invalid(void)
{
	struct kn_buffer out;
	const uint8_t dangling[] = { KN_KISS_FESC };
	const uint8_t invalid[] = { KN_KISS_FESC, 0x00 };

	if (kn_buffer_init(&out, 0) != 0)
		return 1;

	if (kn_kiss_unescape(dangling, sizeof(dangling), &out) == 0) {
		kn_buffer_free(&out);
		return 1;
	}

	kn_buffer_clear(&out);
	if (kn_kiss_unescape(invalid, sizeof(invalid), &out) == 0) {
		kn_buffer_free(&out);
		return 1;
	}

	kn_buffer_free(&out);
	return 0;
}
