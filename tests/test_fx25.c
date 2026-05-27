/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_fx25.c */

#include <sys/types.h>

#include <stdint.h>
#include <string.h>

#include "kilonode/fx25.h"

static int test_decode_disabled_not_fx25(void);
static int test_decode_placeholder_not_implemented(void);
static int test_empty_enabled_malformed(void);
static int test_formatting(void);
static int test_no_payload_extraction_claimed(void);
static int test_oversized_rejected(void);
static int test_status_names(void);

int
main(void)
{
	if (test_decode_disabled_not_fx25() != 0)
		return 1;
	if (test_decode_placeholder_not_implemented() != 0)
		return 1;
	if (test_empty_enabled_malformed() != 0)
		return 1;
	if (test_oversized_rejected() != 0)
		return 1;
	if (test_no_payload_extraction_claimed() != 0)
		return 1;
	if (test_formatting() != 0)
		return 1;
	if (test_status_names() != 0)
		return 1;

	return 0;
}

static int
test_decode_disabled_not_fx25(void)
{
	struct kn_fx25_params params;
	struct kn_fx25_decode_result result;
	const uint8_t frame[] = { 0x7e, 0x7e };

	kn_fx25_params_default(&params);
	if (kn_fx25_decode_placeholder(frame, sizeof(frame), &params,
	    &result) != KN_FX25_OK)
		return 1;

	return result.status == KN_FX25_DECODE_NOT_FX25 ? 0 : 1;
}

static int
test_decode_placeholder_not_implemented(void)
{
	struct kn_fx25_params params;
	struct kn_fx25_decode_result result;
	const uint8_t frame[] = { 0x7e, 0x01, 0x02, 0x7e };

	kn_fx25_params_default(&params);
	params.enabled = 1;
	params.detect_only = 1;

	if (kn_fx25_decode_placeholder(frame, sizeof(frame), &params,
	    &result) != KN_FX25_ERR_NOT_IMPLEMENTED)
		return 1;

	return result.status == KN_FX25_DECODE_NOT_IMPLEMENTED ? 0 : 1;
}

static int
test_empty_enabled_malformed(void)
{
	struct kn_fx25_params params;
	struct kn_fx25_decode_result result;

	kn_fx25_params_default(&params);
	params.enabled = 1;
	params.detect_only = 1;

	if (kn_fx25_decode_placeholder(NULL, 0, &params, &result) !=
	    KN_FX25_OK)
		return 1;

	return result.status == KN_FX25_DECODE_MALFORMED ? 0 : 1;
}

static int
test_formatting(void)
{
	struct kn_fx25_decode_result result;
	char out[160];

	kn_fx25_decode_result_clear(&result);
	if (kn_fx25_decode_result_format(&result, out, sizeof(out)) !=
	    KN_FX25_OK)
		return 1;

	return strstr(out, "status=not-fx25") != NULL &&
	    strstr(out, "corrected=false") != NULL ? 0 : 1;
}

static int
test_no_payload_extraction_claimed(void)
{
	struct kn_fx25_params params;
	struct kn_fx25_decode_result result;
	const uint8_t frame[] = { 0x7e, 0x01, 0x02, 0x7e };

	kn_fx25_params_default(&params);
	params.enabled = 1;
	params.detect_only = 1;
	(void)kn_fx25_decode_placeholder(frame, sizeof(frame), &params,
	    &result);

	if (result.ax25_payload != NULL || result.ax25_payload_len != 0)
		return 1;
	if (result.corrected != 0)
		return 1;

	return result.payload_relation == KN_FX25_PAYLOAD_UNKNOWN ? 0 : 1;
}

static int
test_oversized_rejected(void)
{
	struct kn_fx25_params params;
	struct kn_fx25_decode_result result;
	const uint8_t frame[] = { 0x01, 0x02 };

	kn_fx25_params_default(&params);
	params.max_frame_bytes = 1;

	return kn_fx25_decode_placeholder(frame, sizeof(frame), &params,
	    &result) == KN_FX25_ERR_TOO_LARGE ? 0 : 1;
}

static int
test_status_names(void)
{
	if (strcmp(kn_fx25_mode_name(KN_FX25_MODE_DETECT_ONLY),
	    "detect-only") != 0)
		return 1;
	if (strcmp(kn_fx25_decode_status_name(
	    KN_FX25_DECODE_NOT_IMPLEMENTED), "not-implemented") != 0)
		return 1;
	if (strcmp(kn_fx25_fec_profile_name(
	    KN_FX25_FEC_PROFILE_RS_PLANNED), "rs-planned") != 0)
		return 1;

	return strcmp(kn_fx25_payload_relation_name(
	    KN_FX25_PAYLOAD_EMBEDDED_AX25), "embedded-ax25") == 0 ? 0 : 1;
}
