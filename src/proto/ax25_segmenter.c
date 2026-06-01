/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/proto/ax25_segmenter.c */

#include <sys/types.h>

#include <string.h>

#include "kilonode/ax25_segmenter.h"

enum kn_ax25_segmenter_error
kn_ax25_segmenter_split(const uint8_t *payload, size_t payload_len,
	size_t paclen, struct kn_ax25_segment *segments,
	size_t max_segments, size_t *count)
{
	size_t needed;
	size_t i;
	size_t offset;
	size_t len;

	if (segments == NULL || count == NULL)
		return KN_AX25_SEGMENTER_ERR_INVALID_ARGUMENT;
	if (payload == NULL && payload_len > 0)
		return KN_AX25_SEGMENTER_ERR_INVALID_ARGUMENT;
	if (paclen == 0)
		return KN_AX25_SEGMENTER_ERR_INVALID_VALUE;
	if (max_segments == 0)
		return KN_AX25_SEGMENTER_ERR_OUTPUT_FULL;

	needed = payload_len == 0 ? 1 : (payload_len + paclen - 1) / paclen;
	if (needed > KN_AX25_SEGMENTER_MAX_SEGMENTS)
		return KN_AX25_SEGMENTER_ERR_TOO_MANY_SEGMENTS;
	if (needed > max_segments)
		return KN_AX25_SEGMENTER_ERR_OUTPUT_FULL;

	memset(segments, 0, sizeof(segments[0]) * max_segments);
	offset = 0;
	for (i = 0; i < needed; i++) {
		len = payload_len - offset;
		if (len > paclen)
			len = paclen;
		segments[i].index = i;
		segments[i].offset = offset;
		segments[i].len = len;
		segments[i].final = i + 1 == needed ? 1 : 0;
		segments[i].data = len == 0 ? payload : payload + offset;
		offset += len;
	}
	*count = needed;
	return KN_AX25_SEGMENTER_OK;
}
