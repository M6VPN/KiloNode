/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/ax25_segmenter.h */

#ifndef KILONODE_AX25_SEGMENTER_H
#define KILONODE_AX25_SEGMENTER_H

#include <sys/types.h>

#include <stdint.h>

#define KN_AX25_SEGMENTER_MAX_SEGMENTS 64

enum kn_ax25_segmenter_error {
	KN_AX25_SEGMENTER_OK = 0,
	KN_AX25_SEGMENTER_ERR_INVALID_ARGUMENT,
	KN_AX25_SEGMENTER_ERR_INVALID_VALUE,
	KN_AX25_SEGMENTER_ERR_TOO_MANY_SEGMENTS,
	KN_AX25_SEGMENTER_ERR_OUTPUT_FULL
};

struct kn_ax25_segment {
	size_t index;
	size_t offset;
	size_t len;
	uint8_t final;
	const uint8_t *data;
};

enum kn_ax25_segmenter_error kn_ax25_segmenter_split(
	const uint8_t *, size_t, size_t, struct kn_ax25_segment *,
	size_t, size_t *);

#endif
