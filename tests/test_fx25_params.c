/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_fx25_params.c */

#include <sys/types.h>

#include <string.h>

#include "kilonode/fx25_params.h"

static int test_default_disabled(void);
static int test_detect_only_valid(void);
static int test_formatting(void);
static int test_invalid_boolean(void);
static int test_invalid_detect_without_enable(void);
static int test_invalid_max_frame(void);

int
main(void)
{
	if (test_default_disabled() != 0)
		return 1;
	if (test_detect_only_valid() != 0)
		return 1;
	if (test_invalid_max_frame() != 0)
		return 1;
	if (test_invalid_boolean() != 0)
		return 1;
	if (test_invalid_detect_without_enable() != 0)
		return 1;
	if (test_formatting() != 0)
		return 1;

	return 0;
}

static int
test_default_disabled(void)
{
	struct kn_fx25_params params;

	kn_fx25_params_default(&params);
	if (params.enabled != 0)
		return 1;
	if (params.allow_fallback_ax25 != 1)
		return 1;

	return kn_fx25_params_validate(&params) == KN_FX25_PARAMS_OK ? 0 : 1;
}

static int
test_detect_only_valid(void)
{
	struct kn_fx25_params params;

	kn_fx25_params_default(&params);
	params.enabled = 1;
	params.detect_only = 1;

	return kn_fx25_params_validate(&params) == KN_FX25_PARAMS_OK ? 0 : 1;
}

static int
test_formatting(void)
{
	struct kn_fx25_params params;
	char out[160];

	kn_fx25_params_default(&params);
	if (kn_fx25_params_format(&params, out, sizeof(out)) !=
	    KN_FX25_PARAMS_OK)
		return 1;

	return strstr(out, "enabled=false") != NULL &&
	    strstr(out, "fallback_ax25=true") != NULL ? 0 : 1;
}

static int
test_invalid_boolean(void)
{
	struct kn_fx25_params params;

	kn_fx25_params_default(&params);
	params.strict_mode = 2;

	return kn_fx25_params_validate(&params) ==
	    KN_FX25_PARAMS_ERR_INVALID_VALUE ? 0 : 1;
}

static int
test_invalid_detect_without_enable(void)
{
	struct kn_fx25_params params;

	kn_fx25_params_default(&params);
	params.detect_only = 1;

	return kn_fx25_params_validate(&params) ==
	    KN_FX25_PARAMS_ERR_INVALID_VALUE ? 0 : 1;
}

static int
test_invalid_max_frame(void)
{
	struct kn_fx25_params params;

	kn_fx25_params_default(&params);
	params.max_frame_bytes = 0;

	return kn_fx25_params_validate(&params) ==
	    KN_FX25_PARAMS_ERR_INVALID_VALUE ? 0 : 1;
}
