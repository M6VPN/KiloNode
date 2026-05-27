/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_ax25_params.c */

#include <sys/types.h>

#include <string.h>

#include "kilonode/ax25_params.h"

static int test_default_valid(void);
static int test_formatting(void);
static int test_invalid_max_info(void);
static int test_invalid_modulo(void);
static int test_invalid_retry(void);
static int test_invalid_timer(void);
static int test_invalid_window(void);

int
main(void)
{
	if (test_default_valid() != 0)
		return 1;
	if (test_invalid_window() != 0)
		return 1;
	if (test_invalid_retry() != 0)
		return 1;
	if (test_invalid_timer() != 0)
		return 1;
	if (test_invalid_max_info() != 0)
		return 1;
	if (test_invalid_modulo() != 0)
		return 1;
	if (test_formatting() != 0)
		return 1;

	return 0;
}

static int
test_default_valid(void)
{
	struct kn_ax25_params params;

	kn_ax25_params_default(&params);
	if (kn_ax25_params_validate(&params) != KN_AX25_PARAMS_OK)
		return 1;

	return params.allow_connected_mode == 0 ? 0 : 1;
}

static int
test_formatting(void)
{
	struct kn_ax25_params params;
	char out[160];

	kn_ax25_params_default(&params);
	if (kn_ax25_params_format(&params, out, sizeof(out)) !=
	    KN_AX25_PARAMS_OK)
		return 1;
	if (strstr(out, "enabled=false") == NULL)
		return 1;
	if (strstr(out, "modulo=modulo-8") == NULL)
		return 1;

	return strcmp(kn_ax25_params_modulo_name(99), "unknown") == 0 ? 0 : 1;
}

static int
test_invalid_max_info(void)
{
	struct kn_ax25_params params;

	kn_ax25_params_default(&params);
	params.max_info_len = 0;

	return kn_ax25_params_validate(&params) ==
	    KN_AX25_PARAMS_ERR_INVALID_VALUE ? 0 : 1;
}

static int
test_invalid_modulo(void)
{
	struct kn_ax25_params params;

	kn_ax25_params_default(&params);
	params.modulo_mode = 99;

	return kn_ax25_params_validate(&params) ==
	    KN_AX25_PARAMS_ERR_INVALID_VALUE ? 0 : 1;
}

static int
test_invalid_retry(void)
{
	struct kn_ax25_params params;

	kn_ax25_params_default(&params);
	params.n2_retry_count = 0;

	return kn_ax25_params_validate(&params) ==
	    KN_AX25_PARAMS_ERR_INVALID_VALUE ? 0 : 1;
}

static int
test_invalid_timer(void)
{
	struct kn_ax25_params params;

	kn_ax25_params_default(&params);
	params.t1_ms = 0;

	return kn_ax25_params_validate(&params) ==
	    KN_AX25_PARAMS_ERR_INVALID_VALUE ? 0 : 1;
}

static int
test_invalid_window(void)
{
	struct kn_ax25_params params;

	kn_ax25_params_default(&params);
	params.window_size = 8;

	return kn_ax25_params_validate(&params) ==
	    KN_AX25_PARAMS_ERR_INVALID_VALUE ? 0 : 1;
}
