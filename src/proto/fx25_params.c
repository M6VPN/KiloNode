/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/proto/fx25_params.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "kilonode/fx25_params.h"

#define KN_FX25_PARAMS_MAX_FRAME_BYTES 4096

void
kn_fx25_params_default(struct kn_fx25_params *params)
{
	if (params == NULL)
		return;

	memset(params, 0, sizeof(*params));
	params->enabled = 0;
	params->detect_only = 0;
	params->max_frame_bytes = 512;
	params->allow_fallback_ax25 = 1;
	params->strict_mode = 0;
}

enum kn_fx25_params_error
kn_fx25_params_format(const struct kn_fx25_params *params, char *buf,
	size_t bufsiz)
{
	int needed;

	if (params == NULL || buf == NULL || bufsiz == 0)
		return KN_FX25_PARAMS_ERR_INVALID_ARGUMENT;

	needed = snprintf(buf, bufsiz,
	    "enabled=%s detect_only=%s max_frame=%llu fallback_ax25=%s "
	    "strict=%s",
	    params->enabled != 0 ? "true" : "false",
	    params->detect_only != 0 ? "true" : "false",
	    (unsigned long long)params->max_frame_bytes,
	    params->allow_fallback_ax25 != 0 ? "true" : "false",
	    params->strict_mode != 0 ? "true" : "false");
	if (needed < 0 || (size_t)needed >= bufsiz)
		return KN_FX25_PARAMS_ERR_BUFFER;

	return KN_FX25_PARAMS_OK;
}

enum kn_fx25_params_error
kn_fx25_params_validate(const struct kn_fx25_params *params)
{
	if (params == NULL)
		return KN_FX25_PARAMS_ERR_INVALID_ARGUMENT;
	if (params->enabled > 1 || params->detect_only > 1 ||
	    params->allow_fallback_ax25 > 1 || params->strict_mode > 1)
		return KN_FX25_PARAMS_ERR_INVALID_VALUE;
	if (params->detect_only != 0 && params->enabled == 0)
		return KN_FX25_PARAMS_ERR_INVALID_VALUE;
	if (params->max_frame_bytes == 0 ||
	    params->max_frame_bytes > KN_FX25_PARAMS_MAX_FRAME_BYTES)
		return KN_FX25_PARAMS_ERR_INVALID_VALUE;

	return KN_FX25_PARAMS_OK;
}
