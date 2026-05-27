/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/proto/ax25_params.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "kilonode/ax25_params.h"

#define KN_AX25_PARAMS_MAX_INFO_LEN 2048
#define KN_AX25_PARAMS_MAX_PACLEN   2048
#define KN_AX25_PARAMS_MAX_RETRIES  16
#define KN_AX25_PARAMS_MAX_T1_MS    600000U
#define KN_AX25_PARAMS_MAX_T2_MS    600000U
#define KN_AX25_PARAMS_MAX_T3_MS    3600000U

void
kn_ax25_params_default(struct kn_ax25_params *params)
{
	if (params == NULL)
		return;

	memset(params, 0, sizeof(*params));
	params->allow_connected_mode = 0;
	params->modulo_mode = KN_AX25_MODULO_8;
	params->t1_ms = 3000;
	params->t2_ms = 1000;
	params->t3_ms = 180000;
	params->n2_retry_count = 10;
	params->window_size = 1;
	params->max_info_len = 256;
	params->paclen = 256;
}

enum kn_ax25_params_error
kn_ax25_params_format(const struct kn_ax25_params *params, char *buf,
	size_t bufsiz)
{
	int needed;

	if (params == NULL || buf == NULL || bufsiz == 0)
		return KN_AX25_PARAMS_ERR_INVALID_ARGUMENT;

	needed = snprintf(buf, bufsiz,
	    "enabled=%s modulo=%s t1_ms=%u t2_ms=%u t3_ms=%u n2=%u "
	    "window=%u max_info=%llu paclen=%llu",
	    params->allow_connected_mode != 0 ? "true" : "false",
	    kn_ax25_params_modulo_name(params->modulo_mode),
	    (unsigned int)params->t1_ms,
	    (unsigned int)params->t2_ms,
	    (unsigned int)params->t3_ms,
	    (unsigned int)params->n2_retry_count,
	    (unsigned int)params->window_size,
	    (unsigned long long)params->max_info_len,
	    (unsigned long long)params->paclen);
	if (needed < 0 || (size_t)needed >= bufsiz)
		return KN_AX25_PARAMS_ERR_BUFFER;

	return KN_AX25_PARAMS_OK;
}

const char *
kn_ax25_params_modulo_name(enum kn_ax25_modulo_mode mode)
{
	switch (mode) {
	case KN_AX25_MODULO_8:
		return "modulo-8";
	case KN_AX25_MODULO_128:
		return "modulo-128";
	}

	return "unknown";
}

enum kn_ax25_params_error
kn_ax25_params_validate(const struct kn_ax25_params *params)
{
	if (params == NULL)
		return KN_AX25_PARAMS_ERR_INVALID_ARGUMENT;
	if (params->allow_connected_mode > 1)
		return KN_AX25_PARAMS_ERR_INVALID_VALUE;
	if (params->modulo_mode != KN_AX25_MODULO_8 &&
	    params->modulo_mode != KN_AX25_MODULO_128)
		return KN_AX25_PARAMS_ERR_INVALID_VALUE;
	if (params->t1_ms == 0 || params->t1_ms > KN_AX25_PARAMS_MAX_T1_MS)
		return KN_AX25_PARAMS_ERR_INVALID_VALUE;
	if (params->t2_ms == 0 || params->t2_ms > KN_AX25_PARAMS_MAX_T2_MS)
		return KN_AX25_PARAMS_ERR_INVALID_VALUE;
	if (params->t3_ms == 0 || params->t3_ms > KN_AX25_PARAMS_MAX_T3_MS)
		return KN_AX25_PARAMS_ERR_INVALID_VALUE;
	if (params->n2_retry_count == 0 ||
	    params->n2_retry_count > KN_AX25_PARAMS_MAX_RETRIES)
		return KN_AX25_PARAMS_ERR_INVALID_VALUE;
	if (params->window_size == 0)
		return KN_AX25_PARAMS_ERR_INVALID_VALUE;
	if (params->modulo_mode == KN_AX25_MODULO_8 &&
	    params->window_size > 7)
		return KN_AX25_PARAMS_ERR_INVALID_VALUE;
	if (params->modulo_mode == KN_AX25_MODULO_128 &&
	    params->window_size > 63)
		return KN_AX25_PARAMS_ERR_INVALID_VALUE;
	if (params->max_info_len == 0 ||
	    params->max_info_len > KN_AX25_PARAMS_MAX_INFO_LEN)
		return KN_AX25_PARAMS_ERR_INVALID_VALUE;
	if (params->paclen == 0 ||
	    params->paclen > KN_AX25_PARAMS_MAX_PACLEN)
		return KN_AX25_PARAMS_ERR_INVALID_VALUE;

	return KN_AX25_PARAMS_OK;
}
