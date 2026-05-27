/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/proto/fx25.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "kilonode/fx25.h"

enum kn_fx25_error
kn_fx25_decode_placeholder(const uint8_t *data, size_t data_len,
	const struct kn_fx25_params *params,
	struct kn_fx25_decode_result *result)
{
	if (result == NULL)
		return KN_FX25_ERR_INVALID_ARGUMENT;
	if (data == NULL && data_len > 0)
		return KN_FX25_ERR_INVALID_ARGUMENT;

	kn_fx25_decode_result_clear(result);

	if (params != NULL) {
		if (kn_fx25_params_validate(params) != KN_FX25_PARAMS_OK)
			return KN_FX25_ERR_INVALID_ARGUMENT;
		if (data_len > params->max_frame_bytes)
			return KN_FX25_ERR_TOO_LARGE;
		if (params->enabled == 0) {
			result->status = KN_FX25_DECODE_NOT_FX25;
			return KN_FX25_OK;
		}
	} else if (data_len == 0) {
		result->status = KN_FX25_DECODE_NOT_FX25;
		return KN_FX25_OK;
	}

	if (data_len == 0) {
		result->status = KN_FX25_DECODE_MALFORMED;
		return KN_FX25_OK;
	}

	result->status = KN_FX25_DECODE_NOT_IMPLEMENTED;
	result->payload_relation = KN_FX25_PAYLOAD_UNKNOWN;
	result->fec_profile = KN_FX25_FEC_PROFILE_RS_PLANNED;

	return KN_FX25_ERR_NOT_IMPLEMENTED;
}

void
kn_fx25_decode_result_clear(struct kn_fx25_decode_result *result)
{
	if (result == NULL)
		return;

	memset(result, 0, sizeof(*result));
	result->status = KN_FX25_DECODE_NOT_FX25;
	result->payload_relation = KN_FX25_PAYLOAD_UNKNOWN;
	result->fec_profile = KN_FX25_FEC_PROFILE_NONE;
}

enum kn_fx25_error
kn_fx25_decode_result_format(const struct kn_fx25_decode_result *result,
	char *buf, size_t bufsiz)
{
	int needed;

	if (result == NULL || buf == NULL || bufsiz == 0)
		return KN_FX25_ERR_INVALID_ARGUMENT;

	needed = snprintf(buf, bufsiz,
	    "status=%s payload=%s fec=%s corrected=%s ax25_len=%llu",
	    kn_fx25_decode_status_name(result->status),
	    kn_fx25_payload_relation_name(result->payload_relation),
	    kn_fx25_fec_profile_name(result->fec_profile),
	    result->corrected != 0 ? "true" : "false",
	    (unsigned long long)result->ax25_payload_len);
	if (needed < 0 || (size_t)needed >= bufsiz)
		return KN_FX25_ERR_BUFFER;

	return KN_FX25_OK;
}

const char *
kn_fx25_decode_status_name(enum kn_fx25_decode_status status)
{
	switch (status) {
	case KN_FX25_DECODE_NOT_FX25:
		return "not-fx25";
	case KN_FX25_DECODE_CANDIDATE:
		return "candidate";
	case KN_FX25_DECODE_VALID:
		return "valid";
	case KN_FX25_DECODE_CORRECTED:
		return "corrected";
	case KN_FX25_DECODE_UNCORRECTABLE:
		return "uncorrectable";
	case KN_FX25_DECODE_MALFORMED:
		return "malformed";
	case KN_FX25_DECODE_NOT_IMPLEMENTED:
		return "not-implemented";
	}

	return "unknown";
}

const char *
kn_fx25_fec_profile_name(enum kn_fx25_fec_profile profile)
{
	switch (profile) {
	case KN_FX25_FEC_PROFILE_NONE:
		return "none";
	case KN_FX25_FEC_PROFILE_RS_PLANNED:
		return "rs-planned";
	}

	return "unknown";
}

const char *
kn_fx25_mode_name(enum kn_fx25_mode mode)
{
	switch (mode) {
	case KN_FX25_MODE_DISABLED:
		return "disabled";
	case KN_FX25_MODE_DETECT_ONLY:
		return "detect-only";
	case KN_FX25_MODE_ENCODE_PLANNED:
		return "encode-planned";
	case KN_FX25_MODE_DECODE_PLANNED:
		return "decode-planned";
	}

	return "unknown";
}

const char *
kn_fx25_payload_relation_name(enum kn_fx25_payload_relation relation)
{
	switch (relation) {
	case KN_FX25_PAYLOAD_UNKNOWN:
		return "unknown";
	case KN_FX25_PAYLOAD_EMBEDDED_AX25:
		return "embedded-ax25";
	}

	return "unknown";
}
