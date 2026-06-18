/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/proto/ax25_loopback_payload.c */

#include <sys/types.h>

#include <stddef.h>

#include "kilonode/ax25_loopback_payload.h"
#include "kilonode/ax25_sequence.h"

enum kn_ax25_loopback_payload_error
kn_ax25_loopback_payload_record(struct kn_ax25_loopback_endpoint *endpoint,
	const struct kn_ax25_i_frame_decoded *decoded, uint8_t accepted,
	const char *reason)
{
	if (endpoint == NULL || decoded == NULL)
		return KN_AX25_LOOPBACK_PAYLOAD_ERR_INVALID_ARGUMENT;
	if (kn_ax25_payload_delivery_queue_record(&endpoint->deliveries,
	    endpoint->name, endpoint->port_name, &decoded->source,
	    &decoded->destination, decoded->ns, decoded->nr,
	    decoded->payload, decoded->payload_len, accepted, reason,
	    NULL) != KN_AX25_PAYLOAD_DELIVERY_OK)
		return KN_AX25_LOOPBACK_PAYLOAD_ERR_BUFFER;
	return KN_AX25_LOOPBACK_PAYLOAD_OK;
}

enum kn_ax25_loopback_payload_error
kn_ax25_loopback_payload_send_i(struct kn_ax25_loopback_endpoint *endpoint,
	const uint8_t *payload, size_t payload_len, uint8_t ns_override,
	uint8_t use_ns_override, uint8_t *out, size_t out_len,
	size_t *written)
{
	struct kn_ax25_connection_record *record;
	struct kn_ax25_sequence_state sequence;
	struct kn_ax25_i_frame_request request;
	size_t window_size;
	uint64_t retransmit_id;
	uint8_t ns;

	if (endpoint == NULL || out == NULL || written == NULL)
		return KN_AX25_LOOPBACK_PAYLOAD_ERR_INVALID_ARGUMENT;
	if (payload == NULL && payload_len > 0)
		return KN_AX25_LOOPBACK_PAYLOAD_ERR_INVALID_ARGUMENT;
	if (endpoint->table.count == 0)
		return KN_AX25_LOOPBACK_PAYLOAD_ERR_STATE;
	record = &endpoint->table.records[0];
	if (record->connection.state != KN_AX25_CONNECTION_CONNECTED)
		return KN_AX25_LOOPBACK_PAYLOAD_ERR_STATE;
	if (payload_len > record->params.max_info_len)
		return KN_AX25_LOOPBACK_PAYLOAD_ERR_INVALID_VALUE;
	window_size = record->params.window_size;
	if (window_size == 0)
		return KN_AX25_LOOPBACK_PAYLOAD_ERR_INVALID_VALUE;

	kn_ax25_i_frame_request_clear(&request);
	request.source = endpoint->local;
	request.destination = endpoint->peer;
	request.modulo_mode = record->params.modulo_mode;
	request.nr = record->connection.receive_state;
	request.pid = KN_AX25_PID_NO_LAYER_3;
	request.payload = payload;
	request.payload_len = payload_len;
	request.max_info_len = record->params.max_info_len;
	if (use_ns_override != 0) {
		if (ns_override > 7)
			return KN_AX25_LOOPBACK_PAYLOAD_ERR_INVALID_VALUE;
		request.ns = ns_override;
	} else {
		sequence.send_state = record->connection.send_state;
		if (kn_ax25_sequence_send_i_mod8(&sequence, &ns) !=
		    KN_AX25_SEQUENCE_OK)
			return KN_AX25_LOOPBACK_PAYLOAD_ERR_INVALID_VALUE;
		request.ns = ns;
	}
	if (kn_ax25_i_frame_build(&request, out, out_len, written) !=
	    KN_AX25_I_FRAME_OK)
		return KN_AX25_LOOPBACK_PAYLOAD_ERR_BUFFER;
	if (kn_ax25_loopback_window_record(&endpoint->window, request.ns,
	    request.nr, payload_len, endpoint->i_frames_sent,
	    window_size) != KN_AX25_LOOPBACK_WINDOW_OK) {
		endpoint->window_blocked = endpoint->window.blocked;
		return KN_AX25_LOOPBACK_PAYLOAD_ERR_STATE;
	}
	if (kn_ax25_loopback_retransmit_record(&endpoint->retransmit,
	    request.ns, request.nr, payload_len, endpoint->i_frames_sent,
	    out, *written, &retransmit_id) !=
	    KN_AX25_LOOPBACK_RETRANSMIT_OK) {
		endpoint->retransmit_full = endpoint->retransmit.full;
		return KN_AX25_LOOPBACK_PAYLOAD_ERR_STATE;
	}
	if (use_ns_override == 0)
		record->connection.send_state =
		    kn_ax25_sequence_increment_mod8(
		    record->connection.send_state);
	endpoint->outstanding_frames =
	    kn_ax25_loopback_window_in_flight(&endpoint->window);
	endpoint->outstanding_max_seen = endpoint->window.max_in_flight_seen;
	endpoint->retransmit_buffered = endpoint->retransmit.recorded;
	endpoint->retransmit_needed =
	    kn_ax25_loopback_retransmit_count_retry_needed(
	    &endpoint->retransmit);
	endpoint->i_frames_sent++;
	return KN_AX25_LOOPBACK_PAYLOAD_OK;
}
