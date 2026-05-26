/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_bbs_area.c */

#include <sys/types.h>

#include <string.h>

#include "kilonode/bbs_area.h"

static int test_empty_rejected(void);
static int test_invalid_rejected(void);
static int test_lowercase_normalized(void);
static int test_valid_area(void);

int
main(void)
{
	if (test_valid_area() != 0)
		return 1;
	if (test_lowercase_normalized() != 0)
		return 1;
	if (test_invalid_rejected() != 0)
		return 1;
	if (test_empty_rejected() != 0)
		return 1;

	return 0;
}

static int
test_empty_rejected(void)
{
	char out[KN_MESSAGE_AREA_MAX + 1];

	return kn_bbs_area_normalize("", out, sizeof(out)) ==
	    KN_BBS_AREA_ERR_INVALID_NAME ? 0 : 1;
}

static int
test_invalid_rejected(void)
{
	char out[KN_MESSAGE_AREA_MAX + 1];

	return kn_bbs_area_normalize("../BAD", out, sizeof(out)) ==
	    KN_BBS_AREA_ERR_INVALID_NAME ? 0 : 1;
}

static int
test_lowercase_normalized(void)
{
	char out[KN_MESSAGE_AREA_MAX + 1];

	if (kn_bbs_area_normalize("general", out, sizeof(out)) !=
	    KN_BBS_AREA_OK)
		return 1;

	return strcmp(out, "GENERAL") == 0 ? 0 : 1;
}

static int
test_valid_area(void)
{
	return kn_bbs_area_valid("GENERAL-1") != 0 ? 0 : 1;
}
