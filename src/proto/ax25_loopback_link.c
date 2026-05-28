/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/proto/ax25_loopback_link.c */

#include <sys/types.h>

#include <string.h>

#include "kilonode/ax25_loopback_link.h"

void
kn_ax25_loopback_link_init(struct kn_ax25_loopback_link *link)
{
	if (link == NULL)
		return;

	memset(link, 0, sizeof(*link));
	link->max_frames_per_step = KN_AX25_LOOPBACK_LINK_STEP_MAX;
}

void
kn_ax25_loopback_link_reset(struct kn_ax25_loopback_link *link)
{
	kn_ax25_loopback_link_init(link);
}

enum kn_ax25_loopback_link_error
kn_ax25_loopback_link_transfer(struct kn_ax25_loopback_link *link,
	struct kn_ax25_loopback_endpoint *from,
	struct kn_ax25_loopback_endpoint *to, size_t *transferred)
{
	const struct kn_ax25_prepared_frame *frame;
	size_t count;
	size_t i;
	size_t moved;
	enum kn_ax25_loopback_endpoint_error endpoint_rc;

	if (link == NULL || from == NULL || to == NULL || transferred == NULL)
		return KN_AX25_LOOPBACK_LINK_ERR_INVALID_ARGUMENT;
	*transferred = 0;
	count = kn_ax25_prepared_queue_count(&from->prepared);
	moved = 0;
	for (i = from->sent_prepared_count; i < count; i++) {
		if (moved >= link->max_frames_per_step)
			break;
		frame = &from->prepared.frames[i];
		if (frame->status != KN_AX25_PREPARED_FRAME_STATUS_PREPARED ||
		    frame->raw_len == 0) {
			from->sent_prepared_count++;
			continue;
		}
		if (frame->raw_len > KN_AX25_LOOPBACK_LINK_FRAME_MAX)
			return KN_AX25_LOOPBACK_LINK_ERR_FRAME_TOO_LARGE;
		endpoint_rc = kn_ax25_loopback_endpoint_process_frame(to,
		    frame->raw, frame->raw_len);
		if (endpoint_rc != KN_AX25_LOOPBACK_ENDPOINT_OK)
			return KN_AX25_LOOPBACK_LINK_ERR_ENDPOINT;
		from->sent_prepared_count++;
		link->raw_ax25_frames_transferred++;
		moved++;
	}
	*transferred = moved;
	return KN_AX25_LOOPBACK_LINK_OK;
}

enum kn_ax25_loopback_link_error
kn_ax25_loopback_link_transfer_raw(struct kn_ax25_loopback_link *link,
	struct kn_ax25_loopback_endpoint *to, const uint8_t *data,
	size_t data_len)
{
	if (link == NULL || to == NULL || data == NULL || data_len == 0)
		return KN_AX25_LOOPBACK_LINK_ERR_INVALID_ARGUMENT;
	if (data_len > KN_AX25_LOOPBACK_LINK_FRAME_MAX)
		return KN_AX25_LOOPBACK_LINK_ERR_FRAME_TOO_LARGE;
	if (kn_ax25_loopback_endpoint_process_frame(to, data, data_len) !=
	    KN_AX25_LOOPBACK_ENDPOINT_OK) {
		link->malformed_frames++;
		return KN_AX25_LOOPBACK_LINK_ERR_ENDPOINT;
	}
	link->raw_ax25_frames_transferred++;
	return KN_AX25_LOOPBACK_LINK_OK;
}
