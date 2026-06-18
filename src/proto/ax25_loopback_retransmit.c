/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/proto/ax25_loopback_retransmit.c */

#include <sys/types.h>

#include <string.h>

#include "kilonode/ax25_loopback_retransmit.h"

static uint8_t is_acked_by_rr(uint8_t, uint8_t);
static struct kn_ax25_loopback_retransmit_frame *next_retry(
	struct kn_ax25_loopback_retransmit_buffer *);

static uint8_t
is_acked_by_rr(uint8_t ns, uint8_t nr)
{
	uint8_t distance;

	if (ns > 7 || nr > 7)
		return 0;
	distance = (uint8_t)((nr + 8U - ns) & 0x07U);
	return distance > 0 ? 1 : 0;
}

static struct kn_ax25_loopback_retransmit_frame *
next_retry(struct kn_ax25_loopback_retransmit_buffer *buffer)
{
	size_t i;

	if (buffer == NULL)
		return NULL;
	for (i = 0; i < buffer->count; i++) {
		if (buffer->frames[i].status ==
		    KN_AX25_LOOPBACK_RETRANSMIT_STATUS_RETRY_NEEDED)
			return &buffer->frames[i];
	}
	for (i = 0; i < buffer->count; i++) {
		if (buffer->frames[i].status ==
		    KN_AX25_LOOPBACK_RETRANSMIT_STATUS_IN_FLIGHT)
			return &buffer->frames[i];
	}
	return NULL;
}

enum kn_ax25_loopback_retransmit_error
kn_ax25_loopback_retransmit_ack_rr(
	struct kn_ax25_loopback_retransmit_buffer *buffer, uint8_t nr,
	size_t *acked)
{
	size_t i;
	size_t count;

	if (buffer == NULL || acked == NULL)
		return KN_AX25_LOOPBACK_RETRANSMIT_ERR_INVALID_ARGUMENT;
	if (nr > 7)
		return KN_AX25_LOOPBACK_RETRANSMIT_ERR_INVALID_VALUE;

	count = 0;
	for (i = 0; i < buffer->count; i++) {
		if (buffer->frames[i].status ==
		    KN_AX25_LOOPBACK_RETRANSMIT_STATUS_ACKED)
			continue;
		if (is_acked_by_rr(buffer->frames[i].ns, nr) == 0)
			continue;
		buffer->frames[i].status =
		    KN_AX25_LOOPBACK_RETRANSMIT_STATUS_ACKED;
		count++;
	}
	buffer->acked += count;
	*acked = count;
	return KN_AX25_LOOPBACK_RETRANSMIT_OK;
}

size_t
kn_ax25_loopback_retransmit_count_in_flight(
	const struct kn_ax25_loopback_retransmit_buffer *buffer)
{
	size_t i;
	size_t count;

	if (buffer == NULL)
		return 0;
	count = 0;
	for (i = 0; i < buffer->count; i++) {
		if (buffer->frames[i].status ==
		    KN_AX25_LOOPBACK_RETRANSMIT_STATUS_IN_FLIGHT ||
		    buffer->frames[i].status ==
		    KN_AX25_LOOPBACK_RETRANSMIT_STATUS_RETRY_NEEDED ||
		    buffer->frames[i].status ==
		    KN_AX25_LOOPBACK_RETRANSMIT_STATUS_REPLAYED)
			count++;
	}
	return count;
}

size_t
kn_ax25_loopback_retransmit_count_retry_needed(
	const struct kn_ax25_loopback_retransmit_buffer *buffer)
{
	size_t i;
	size_t count;

	if (buffer == NULL)
		return 0;
	count = 0;
	for (i = 0; i < buffer->count; i++) {
		if (buffer->frames[i].status ==
		    KN_AX25_LOOPBACK_RETRANSMIT_STATUS_RETRY_NEEDED)
			count++;
	}
	return count;
}

void
kn_ax25_loopback_retransmit_init(
	struct kn_ax25_loopback_retransmit_buffer *buffer)
{
	if (buffer == NULL)
		return;

	memset(buffer, 0, sizeof(*buffer));
	buffer->next_id = 1;
}

enum kn_ax25_loopback_retransmit_error
kn_ax25_loopback_retransmit_mark_rej(
	struct kn_ax25_loopback_retransmit_buffer *buffer, uint8_t nr,
	size_t *marked)
{
	size_t i;
	size_t count;

	if (buffer == NULL || marked == NULL)
		return KN_AX25_LOOPBACK_RETRANSMIT_ERR_INVALID_ARGUMENT;
	if (nr > 7)
		return KN_AX25_LOOPBACK_RETRANSMIT_ERR_INVALID_VALUE;

	count = 0;
	for (i = 0; i < buffer->count; i++) {
		if (buffer->frames[i].ns != nr)
			continue;
		if (buffer->frames[i].status ==
		    KN_AX25_LOOPBACK_RETRANSMIT_STATUS_ACKED)
			continue;
		buffer->frames[i].status =
		    KN_AX25_LOOPBACK_RETRANSMIT_STATUS_RETRY_NEEDED;
		count++;
	}
	buffer->retry_marked += count;
	*marked = count;
	return KN_AX25_LOOPBACK_RETRANSMIT_OK;
}

enum kn_ax25_loopback_retransmit_error
kn_ax25_loopback_retransmit_next(
	struct kn_ax25_loopback_retransmit_buffer *buffer, uint8_t *out,
	size_t out_len, size_t *written)
{
	struct kn_ax25_loopback_retransmit_frame *frame;

	if (buffer == NULL || out == NULL || written == NULL)
		return KN_AX25_LOOPBACK_RETRANSMIT_ERR_INVALID_ARGUMENT;
	frame = next_retry(buffer);
	if (frame == NULL)
		return KN_AX25_LOOPBACK_RETRANSMIT_ERR_NOT_FOUND;
	if (frame->raw_len > out_len)
		return KN_AX25_LOOPBACK_RETRANSMIT_ERR_BUFFER;

	memcpy(out, frame->raw, frame->raw_len);
	*written = frame->raw_len;
	frame->status = KN_AX25_LOOPBACK_RETRANSMIT_STATUS_REPLAYED;
	frame->replay_count++;
	buffer->replayed++;
	return KN_AX25_LOOPBACK_RETRANSMIT_OK;
}

enum kn_ax25_loopback_retransmit_error
kn_ax25_loopback_retransmit_record(
	struct kn_ax25_loopback_retransmit_buffer *buffer, uint8_t ns,
	uint8_t nr, size_t payload_len, size_t segment_index,
	const uint8_t *raw, size_t raw_len, uint64_t *id)
{
	struct kn_ax25_loopback_retransmit_frame *frame;

	if (buffer == NULL || raw == NULL || id == NULL)
		return KN_AX25_LOOPBACK_RETRANSMIT_ERR_INVALID_ARGUMENT;
	if (ns > 7 || nr > 7 || raw_len == 0 ||
	    raw_len > KN_AX25_LOOPBACK_RETRANSMIT_RAW_MAX)
		return KN_AX25_LOOPBACK_RETRANSMIT_ERR_INVALID_VALUE;
	if (buffer->count >= KN_AX25_LOOPBACK_RETRANSMIT_MAX) {
		buffer->full++;
		return KN_AX25_LOOPBACK_RETRANSMIT_ERR_FULL;
	}

	frame = &buffer->frames[buffer->count++];
	memset(frame, 0, sizeof(*frame));
	frame->id = buffer->next_id++;
	frame->ns = ns;
	frame->nr = nr;
	frame->payload_len = payload_len;
	frame->segment_index = segment_index;
	frame->raw_len = raw_len;
	memcpy(frame->raw, raw, raw_len);
	frame->status = KN_AX25_LOOPBACK_RETRANSMIT_STATUS_IN_FLIGHT;
	buffer->recorded++;
	*id = frame->id;
	return KN_AX25_LOOPBACK_RETRANSMIT_OK;
}

void
kn_ax25_loopback_retransmit_reset(
	struct kn_ax25_loopback_retransmit_buffer *buffer)
{
	kn_ax25_loopback_retransmit_init(buffer);
}
