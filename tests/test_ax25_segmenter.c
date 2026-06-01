/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_ax25_segmenter.c */

#include <sys/types.h>

#include <stddef.h>
#include <stdint.h>

#include "kilonode/ax25_segmenter.h"

static int test_binary_payload(void);
static int test_empty_payload(void);
static int test_exact_paclen(void);
static int test_output_full(void);
static int test_paclen_plus_one(void);
static int test_short_payload(void);
static int test_too_many_segments(void);
static int test_zero_paclen(void);

int
main(void)
{
	if (test_empty_payload() != 0)
		return 1;
	if (test_short_payload() != 0)
		return 1;
	if (test_exact_paclen() != 0)
		return 1;
	if (test_paclen_plus_one() != 0)
		return 1;
	if (test_binary_payload() != 0)
		return 1;
	if (test_too_many_segments() != 0)
		return 1;
	if (test_zero_paclen() != 0)
		return 1;
	if (test_output_full() != 0)
		return 1;
	return 0;
}

static int
test_binary_payload(void)
{
	const uint8_t payload[] = { 0x00, 0xff, 0x41, 0x42 };
	struct kn_ax25_segment segments[2];
	size_t count;

	if (kn_ax25_segmenter_split(payload, sizeof(payload), 2, segments, 2,
	    &count) != KN_AX25_SEGMENTER_OK)
		return 1;
	return count == 2 && segments[1].offset == 2 &&
	    segments[1].len == 2 && segments[1].final != 0 ? 0 : 1;
}

static int
test_empty_payload(void)
{
	struct kn_ax25_segment segments[1];
	size_t count;

	if (kn_ax25_segmenter_split(NULL, 0, 4, segments, 1, &count) !=
	    KN_AX25_SEGMENTER_OK)
		return 1;
	return count == 1 && segments[0].len == 0 &&
	    segments[0].final != 0 ? 0 : 1;
}

static int
test_exact_paclen(void)
{
	const uint8_t payload[] = { 'a', 'b', 'c', 'd' };
	struct kn_ax25_segment segments[2];
	size_t count;

	if (kn_ax25_segmenter_split(payload, sizeof(payload), 4, segments, 2,
	    &count) != KN_AX25_SEGMENTER_OK)
		return 1;
	return count == 1 && segments[0].len == 4 &&
	    segments[0].final != 0 ? 0 : 1;
}

static int
test_output_full(void)
{
	const uint8_t payload[] = { 'a', 'b', 'c' };
	struct kn_ax25_segment segments[1];
	size_t count;

	return kn_ax25_segmenter_split(payload, sizeof(payload), 1, segments,
	    1, &count) == KN_AX25_SEGMENTER_ERR_OUTPUT_FULL ? 0 : 1;
}

static int
test_paclen_plus_one(void)
{
	const uint8_t payload[] = { 'a', 'b', 'c', 'd', 'e' };
	struct kn_ax25_segment segments[2];
	size_t count;

	if (kn_ax25_segmenter_split(payload, sizeof(payload), 4, segments, 2,
	    &count) != KN_AX25_SEGMENTER_OK)
		return 1;
	return count == 2 && segments[0].len == 4 &&
	    segments[1].offset == 4 && segments[1].len == 1 &&
	    segments[1].final != 0 ? 0 : 1;
}

static int
test_short_payload(void)
{
	const uint8_t payload[] = { 'a', 'b' };
	struct kn_ax25_segment segments[2];
	size_t count;

	if (kn_ax25_segmenter_split(payload, sizeof(payload), 4, segments, 2,
	    &count) != KN_AX25_SEGMENTER_OK)
		return 1;
	return count == 1 && segments[0].offset == 0 &&
	    segments[0].len == 2 ? 0 : 1;
}

static int
test_too_many_segments(void)
{
	uint8_t payload[KN_AX25_SEGMENTER_MAX_SEGMENTS + 1];
	struct kn_ax25_segment segments[KN_AX25_SEGMENTER_MAX_SEGMENTS];
	size_t count;

	payload[0] = 0;
	return kn_ax25_segmenter_split(payload, sizeof(payload), 1, segments,
	    KN_AX25_SEGMENTER_MAX_SEGMENTS, &count) ==
	    KN_AX25_SEGMENTER_ERR_TOO_MANY_SEGMENTS ? 0 : 1;
}

static int
test_zero_paclen(void)
{
	const uint8_t payload[] = { 'a' };
	struct kn_ax25_segment segments[1];
	size_t count;

	return kn_ax25_segmenter_split(payload, sizeof(payload), 0, segments,
	    1, &count) == KN_AX25_SEGMENTER_ERR_INVALID_VALUE ? 0 : 1;
}
