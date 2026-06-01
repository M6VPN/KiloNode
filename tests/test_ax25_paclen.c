/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_ax25_paclen.c */

#include <sys/types.h>

#include <string.h>

#include "kilonode/ax25_paclen.h"

static int test_default_paclen(void);
static int test_format(void);
static int test_one_valid(void);
static int test_overlarge_rejected(void);
static int test_zero_rejected(void);

int
main(void)
{
	if (test_default_paclen() != 0)
		return 1;
	if (test_one_valid() != 0)
		return 1;
	if (test_zero_rejected() != 0)
		return 1;
	if (test_overlarge_rejected() != 0)
		return 1;
	if (test_format() != 0)
		return 1;
	return 0;
}

static int
test_default_paclen(void)
{
	struct kn_ax25_params params;
	size_t paclen;

	kn_ax25_params_default(&params);
	if (kn_ax25_paclen_derive(&params, &paclen) != KN_AX25_PACLEN_OK)
		return 1;
	return paclen == params.paclen ? 0 : 1;
}

static int
test_format(void)
{
	struct kn_ax25_params params;
	char buf[80];

	kn_ax25_params_default(&params);
	if (kn_ax25_paclen_format(&params, buf, sizeof(buf)) !=
	    KN_AX25_PACLEN_OK)
		return 1;
	return strstr(buf, "paclen=256") != NULL ? 0 : 1;
}

static int
test_one_valid(void)
{
	return kn_ax25_paclen_validate(1, 256) == KN_AX25_PACLEN_OK ? 0 : 1;
}

static int
test_overlarge_rejected(void)
{
	if (kn_ax25_paclen_validate(257, 256) !=
	    KN_AX25_PACLEN_ERR_INVALID_VALUE)
		return 1;
	return kn_ax25_paclen_validate(KN_AX25_PACLEN_MAX + 1,
	    KN_AX25_PACLEN_MAX) == KN_AX25_PACLEN_ERR_INVALID_VALUE ? 0 : 1;
}

static int
test_zero_rejected(void)
{
	return kn_ax25_paclen_validate(0, 256) ==
	    KN_AX25_PACLEN_ERR_INVALID_VALUE ? 0 : 1;
}
