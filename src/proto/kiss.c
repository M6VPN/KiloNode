/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/proto/kiss.c */

#include <sys/types.h>

#include <errno.h>
#include <stdint.h>
#include <stddef.h>

#include "kilonode/kiss.h"

int
kn_kiss_escape(const uint8_t *input, size_t input_len, struct kn_buffer *out)
{
	size_t i;
	int rc;

	if (out == NULL || (input == NULL && input_len > 0))
		return EINVAL;

	for (i = 0; i < input_len; i++) {
		if (input[i] == KN_KISS_FEND) {
			rc = kn_buffer_append_byte(out, KN_KISS_FESC);
			if (rc != 0)
				return rc;
			rc = kn_buffer_append_byte(out, KN_KISS_TFEND);
		} else if (input[i] == KN_KISS_FESC) {
			rc = kn_buffer_append_byte(out, KN_KISS_FESC);
			if (rc != 0)
				return rc;
			rc = kn_buffer_append_byte(out, KN_KISS_TFESC);
		} else {
			rc = kn_buffer_append_byte(out, input[i]);
		}

		if (rc != 0)
			return rc;
	}

	return 0;
}

int
kn_kiss_unescape(const uint8_t *input, size_t input_len, struct kn_buffer *out)
{
	size_t i;
	int rc;

	if (out == NULL || (input == NULL && input_len > 0))
		return EINVAL;

	for (i = 0; i < input_len; i++) {
		if (input[i] != KN_KISS_FESC) {
			rc = kn_buffer_append_byte(out, input[i]);
			if (rc != 0)
				return rc;
			continue;
		}

		i++;
		if (i >= input_len)
			return EINVAL;

		if (input[i] == KN_KISS_TFEND)
			rc = kn_buffer_append_byte(out, KN_KISS_FEND);
		else if (input[i] == KN_KISS_TFESC)
			rc = kn_buffer_append_byte(out, KN_KISS_FESC);
		else
			return EINVAL;

		if (rc != 0)
			return rc;
	}

	return 0;
}
