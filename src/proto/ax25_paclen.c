/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/proto/ax25_paclen.c */

#include <sys/types.h>

#include <stdio.h>

#include "kilonode/ax25_paclen.h"

enum kn_ax25_paclen_error
kn_ax25_paclen_derive(const struct kn_ax25_params *params, size_t *paclen)
{
	if (params == NULL || paclen == NULL)
		return KN_AX25_PACLEN_ERR_INVALID_ARGUMENT;
	if (kn_ax25_params_validate(params) != KN_AX25_PARAMS_OK)
		return KN_AX25_PACLEN_ERR_INVALID_VALUE;
	if (kn_ax25_paclen_validate(params->paclen,
	    params->max_info_len) != KN_AX25_PACLEN_OK)
		return KN_AX25_PACLEN_ERR_INVALID_VALUE;
	*paclen = params->paclen;
	return KN_AX25_PACLEN_OK;
}

enum kn_ax25_paclen_error
kn_ax25_paclen_format(const struct kn_ax25_params *params, char *buf,
	size_t bufsiz)
{
	int needed;

	if (params == NULL || buf == NULL || bufsiz == 0)
		return KN_AX25_PACLEN_ERR_INVALID_ARGUMENT;
	needed = snprintf(buf, bufsiz, "paclen=%llu max_info=%llu",
	    (unsigned long long)params->paclen,
	    (unsigned long long)params->max_info_len);
	if (needed < 0 || (size_t)needed >= bufsiz)
		return KN_AX25_PACLEN_ERR_BUFFER;
	return KN_AX25_PACLEN_OK;
}

enum kn_ax25_paclen_error
kn_ax25_paclen_validate(size_t paclen, size_t max_info_len)
{
	if (paclen < KN_AX25_PACLEN_MIN)
		return KN_AX25_PACLEN_ERR_INVALID_VALUE;
	if (paclen > KN_AX25_PACLEN_MAX)
		return KN_AX25_PACLEN_ERR_INVALID_VALUE;
	if (max_info_len == 0 || max_info_len > KN_AX25_PACLEN_MAX)
		return KN_AX25_PACLEN_ERR_INVALID_VALUE;
	if (paclen > max_info_len)
		return KN_AX25_PACLEN_ERR_INVALID_VALUE;
	return KN_AX25_PACLEN_OK;
}
