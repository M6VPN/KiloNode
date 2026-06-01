/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/proto/ax25_loopback_segment.c */

#include <sys/types.h>

#include <stddef.h>

#include "kilonode/ax25_loopback_payload.h"
#include "kilonode/ax25_loopback_segment.h"
#include "kilonode/ax25_paclen.h"
#include "kilonode/ax25_reassembly.h"
#include "kilonode/ax25_segmenter.h"

enum kn_ax25_loopback_segment_error
kn_ax25_loopback_segment_send(struct kn_ax25_loopback_endpoint *from,
	struct kn_ax25_loopback_endpoint *to,
	struct kn_ax25_loopback_link *link, const uint8_t *payload,
	size_t payload_len, size_t *segments_sent)
{
	struct kn_ax25_connection_record *record;
	struct kn_ax25_segment segments[KN_AX25_SEGMENTER_MAX_SEGMENTS];
	uint8_t frame[KN_AX25_LOOPBACK_LINK_FRAME_MAX];
	size_t segment_count;
	size_t paclen;
	size_t written;
	size_t i;
	size_t moved;
	uint64_t accepted_before;

	if (from == NULL || to == NULL || link == NULL ||
	    segments_sent == NULL)
		return KN_AX25_LOOPBACK_SEGMENT_ERR_INVALID_ARGUMENT;
	if (payload == NULL && payload_len > 0)
		return KN_AX25_LOOPBACK_SEGMENT_ERR_INVALID_ARGUMENT;
	if (from->table.count == 0)
		return KN_AX25_LOOPBACK_SEGMENT_ERR_STATE;
	record = &from->table.records[0];
	if (record->connection.state != KN_AX25_CONNECTION_CONNECTED)
		return KN_AX25_LOOPBACK_SEGMENT_ERR_STATE;
	if (record->params.window_size != 1)
		return KN_AX25_LOOPBACK_SEGMENT_ERR_UNSUPPORTED;
	if (kn_ax25_paclen_derive(&record->params, &paclen) !=
	    KN_AX25_PACLEN_OK)
		return KN_AX25_LOOPBACK_SEGMENT_ERR_INVALID_VALUE;
	if (kn_ax25_segmenter_split(payload, payload_len, paclen, segments,
	    KN_AX25_SEGMENTER_MAX_SEGMENTS, &segment_count) !=
	    KN_AX25_SEGMENTER_OK)
		return KN_AX25_LOOPBACK_SEGMENT_ERR_INVALID_VALUE;

	*segments_sent = 0;
	for (i = 0; i < segment_count; i++) {
		accepted_before = to->deliveries.accepted_count;
		if (kn_ax25_loopback_endpoint_send_i(from, segments[i].data,
		    segments[i].len, 0, 0, frame, sizeof(frame), &written) !=
		    KN_AX25_LOOPBACK_ENDPOINT_OK)
			return KN_AX25_LOOPBACK_SEGMENT_ERR_STATE;
		if (kn_ax25_loopback_link_transfer_raw(link, to, frame,
		    written) != KN_AX25_LOOPBACK_LINK_OK)
			return KN_AX25_LOOPBACK_SEGMENT_ERR_TRANSFER;
		if (to->deliveries.accepted_count != accepted_before + 1)
			return KN_AX25_LOOPBACK_SEGMENT_ERR_TRANSFER;
		if (kn_ax25_loopback_link_transfer(link, to, from, &moved) !=
		    KN_AX25_LOOPBACK_LINK_OK)
			return KN_AX25_LOOPBACK_SEGMENT_ERR_TRANSFER;
		if (moved == 0)
			return KN_AX25_LOOPBACK_SEGMENT_ERR_TRANSFER;
		(*segments_sent)++;
	}

	if (kn_ax25_reassembly_queue_record(&to->reassemblies, to->name,
	    to->port_name, &from->local, &to->local, segment_count,
	    segment_count, payload, payload_len, 1, "complete",
	    NULL) != KN_AX25_REASSEMBLY_OK)
		return KN_AX25_LOOPBACK_SEGMENT_ERR_REASSEMBLY;
	to->reassembled_payloads++;
	to->segments_received += segment_count;
	from->segments_sent += segment_count;
	return KN_AX25_LOOPBACK_SEGMENT_OK;
}
