/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/proto/ax25_retry.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "kilonode/ax25_retry.h"

uint8_t
kn_ax25_retry_exhausted(const struct kn_ax25_retry *retry)
{
	if (retry == NULL || retry->max_retries == 0)
		return 1;

	return retry->count >= retry->max_retries ? 1 : 0;
}

enum kn_ax25_retry_error
kn_ax25_retry_format(const struct kn_ax25_retry *retry, char *buf,
	size_t bufsiz)
{
	int needed;

	if (retry == NULL || buf == NULL || bufsiz == 0)
		return KN_AX25_RETRY_ERR_INVALID_ARGUMENT;

	needed = snprintf(buf, bufsiz, "count=%u max=%u exhausted=%s",
	    (unsigned int)retry->count,
	    (unsigned int)retry->max_retries,
	    kn_ax25_retry_exhausted(retry) != 0 ? "true" : "false");
	if (needed < 0 || (size_t)needed >= bufsiz)
		return KN_AX25_RETRY_ERR_BUFFER;

	return KN_AX25_RETRY_OK;
}

enum kn_ax25_retry_error
kn_ax25_retry_increment(struct kn_ax25_retry *retry)
{
	if (retry == NULL)
		return KN_AX25_RETRY_ERR_INVALID_ARGUMENT;
	if (kn_ax25_retry_validate_max(retry->max_retries) !=
	    KN_AX25_RETRY_OK)
		return KN_AX25_RETRY_ERR_INVALID_VALUE;
	if (retry->count >= retry->max_retries)
		return KN_AX25_RETRY_ERR_INVALID_VALUE;

	retry->count++;
	return KN_AX25_RETRY_OK;
}

enum kn_ax25_retry_error
kn_ax25_retry_init(struct kn_ax25_retry *retry, uint8_t max_retries)
{
	if (retry == NULL)
		return KN_AX25_RETRY_ERR_INVALID_ARGUMENT;
	if (kn_ax25_retry_validate_max(max_retries) != KN_AX25_RETRY_OK)
		return KN_AX25_RETRY_ERR_INVALID_VALUE;

	memset(retry, 0, sizeof(*retry));
	retry->max_retries = max_retries;
	return KN_AX25_RETRY_OK;
}

void
kn_ax25_retry_reset(struct kn_ax25_retry *retry)
{
	if (retry == NULL)
		return;

	retry->count = 0;
}

uint8_t
kn_ax25_retry_under_limit(const struct kn_ax25_retry *retry)
{
	if (retry == NULL || retry->max_retries == 0)
		return 0;

	return retry->count < retry->max_retries ? 1 : 0;
}

enum kn_ax25_retry_error
kn_ax25_retry_validate_max(uint8_t max_retries)
{
	if (max_retries == 0 || max_retries > KN_AX25_RETRY_MAX)
		return KN_AX25_RETRY_ERR_INVALID_VALUE;

	return KN_AX25_RETRY_OK;
}
