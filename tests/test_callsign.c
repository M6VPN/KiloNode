/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_callsign.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "kilonode/callsign.h"

static int expect_invalid(const char *);
static int expect_valid(const char *, const char *, uint8_t);
static int test_callsign_invalid(void);
static int test_callsign_valid(void);

int
main(void)
{
	if (test_callsign_valid() != 0)
		return 1;

	if (test_callsign_invalid() != 0)
		return 1;

	return 0;
}

static int
expect_invalid(const char *input)
{
	struct kn_callsign call;

	return kn_callsign_parse(input, &call) == 0 ? 1 : 0;
}

static int
expect_valid(const char *input, const char *expected_call, uint8_t expected_ssid)
{
	struct kn_callsign call;

	if (kn_callsign_parse(input, &call) != 0)
		return 1;

	if (strcmp(call.call, expected_call) != 0)
		return 1;

	if (call.ssid != expected_ssid)
		return 1;

	return 0;
}

static int
test_callsign_invalid(void)
{
	if (expect_invalid("") != 0)
		return 1;
	if (expect_invalid("TOOLONG") != 0)
		return 1;
	if (expect_invalid("N0CALL-16") != 0)
		return 1;
	if (expect_invalid("N0CALL-") != 0)
		return 1;
	if (expect_invalid("N0CALL-A") != 0)
		return 1;
	if (expect_invalid("N0C@LL") != 0)
		return 1;
	if (expect_invalid("n0call") != 0)
		return 1;

	return 0;
}

static int
test_callsign_valid(void)
{
	if (expect_valid("M6VPN", "M6VPN", 0) != 0)
		return 1;
	if (expect_valid("M6VPN-1", "M6VPN", 1) != 0)
		return 1;
	if (expect_valid("N0CALL-15", "N0CALL", 15) != 0)
		return 1;

	return 0;
}
