/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/proto/ax25_i_frame.c */

#include <sys/types.h>

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "kilonode/ax25_control.h"
#include "kilonode/ax25_i_frame.h"
#include "kilonode/buffer.h"

static enum kn_ax25_i_frame_error append_addresses(
	const struct kn_ax25_i_frame_request *, struct kn_buffer *);
static enum kn_ax25_i_frame_error ax25_rc(enum kn_ax25_error);
static uint8_t payload_is_text(const uint8_t *, size_t);
static void preview_set(struct kn_ax25_i_frame_decoded *);

static enum kn_ax25_i_frame_error
append_addresses(const struct kn_ax25_i_frame_request *request,
	struct kn_buffer *out)
{
	struct kn_ax25_addr addr;
	size_t i;
	enum kn_ax25_error rc;

	memset(&addr, 0, sizeof(addr));
	addr.callsign = request->destination;
	rc = kn_ax25_address_encode(&addr, 0, out);
	if (rc != KN_AX25_OK)
		return ax25_rc(rc);

	memset(&addr, 0, sizeof(addr));
	addr.callsign = request->source;
	rc = kn_ax25_address_encode(&addr,
	    (uint8_t)(request->digipeater_count == 0), out);
	if (rc != KN_AX25_OK)
		return ax25_rc(rc);

	for (i = 0; i < request->digipeater_count; i++) {
		memset(&addr, 0, sizeof(addr));
		addr.callsign = request->digipeaters[i];
		rc = kn_ax25_address_encode(&addr,
		    (uint8_t)(i + 1 == request->digipeater_count), out);
		if (rc != KN_AX25_OK)
			return ax25_rc(rc);
	}

	return KN_AX25_I_FRAME_OK;
}

static enum kn_ax25_i_frame_error
ax25_rc(enum kn_ax25_error rc)
{
	switch (rc) {
	case KN_AX25_OK:
		return KN_AX25_I_FRAME_OK;
	case KN_AX25_ERR_TOO_MANY_DIGIS:
		return KN_AX25_I_FRAME_ERR_TOO_MANY_DIGIS;
	case KN_AX25_ERR_BUFFER:
		return KN_AX25_I_FRAME_ERR_BUFFER;
	case KN_AX25_ERR_INVALID_ARGUMENT:
	case KN_AX25_ERR_SHORT_FRAME:
	case KN_AX25_ERR_UNTERMINATED_ADDRESS:
	case KN_AX25_ERR_MALFORMED_ADDRESS:
	case KN_AX25_ERR_INVALID_SSID:
	case KN_AX25_ERR_MISSING_PID:
		break;
	}

	return KN_AX25_I_FRAME_ERR_INVALID_VALUE;
}

static uint8_t
payload_is_text(const uint8_t *payload, size_t payload_len)
{
	size_t i;

	if (payload == NULL && payload_len > 0)
		return 0;
	for (i = 0; i < payload_len; i++) {
		if (payload[i] == '\0')
			return 0;
		if (payload[i] != '\n' && payload[i] != '\r' &&
		    payload[i] != '\t' && isprint(payload[i]) == 0)
			return 0;
	}
	return 1;
}

static void
preview_set(struct kn_ax25_i_frame_decoded *decoded)
{
	size_t limit;
	size_t i;

	decoded->payload_is_text = payload_is_text(decoded->payload,
	    decoded->payload_len);
	limit = decoded->payload_len;
	if (limit > KN_AX25_I_FRAME_PREVIEW_MAX)
		limit = KN_AX25_I_FRAME_PREVIEW_MAX;
	if (decoded->payload_is_text != 0) {
		for (i = 0; i < limit; i++)
			decoded->preview[i] = (char)decoded->payload[i];
		decoded->preview[limit] = '\0';
		return;
	}
	for (i = 0; i < limit && i * 2 + 2 <
	    sizeof(decoded->preview); i++)
		(void)snprintf(decoded->preview + i * 2,
		    sizeof(decoded->preview) - i * 2, "%02x",
		    decoded->payload[i]);
}

enum kn_ax25_i_frame_error
kn_ax25_i_frame_build(const struct kn_ax25_i_frame_request *request,
	uint8_t *out, size_t out_len, size_t *written)
{
	struct kn_buffer frame;
	enum kn_ax25_i_frame_error rc;
	uint8_t control;

	if (request == NULL || out == NULL || written == NULL)
		return KN_AX25_I_FRAME_ERR_INVALID_ARGUMENT;
	if (request->digipeater_count > KN_AX25_MAX_DIGIS)
		return KN_AX25_I_FRAME_ERR_TOO_MANY_DIGIS;
	if (request->modulo_mode != KN_AX25_MODULO_8)
		return KN_AX25_I_FRAME_ERR_UNSUPPORTED;
	if (request->payload == NULL && request->payload_len > 0)
		return KN_AX25_I_FRAME_ERR_INVALID_ARGUMENT;
	if (request->max_info_len == 0 ||
	    request->payload_len > request->max_info_len)
		return KN_AX25_I_FRAME_ERR_TOO_LARGE;
	if (kn_ax25_control_encode_i(request->ns, request->nr,
	    request->poll_final, &control) != KN_AX25_CONTROL_OK)
		return KN_AX25_I_FRAME_ERR_INVALID_VALUE;

	*written = 0;
	if (kn_buffer_init(&frame, 0) != 0)
		return KN_AX25_I_FRAME_ERR_BUFFER;

	rc = append_addresses(request, &frame);
	if (rc != KN_AX25_I_FRAME_OK)
		goto out;
	if (kn_buffer_append_byte(&frame, control) != 0 ||
	    kn_buffer_append_byte(&frame, request->pid) != 0 ||
	    kn_buffer_append(&frame, request->payload,
	    request->payload_len) != 0) {
		rc = KN_AX25_I_FRAME_ERR_BUFFER;
		goto out;
	}
	if (frame.len > out_len) {
		rc = KN_AX25_I_FRAME_ERR_TOO_LARGE;
		goto out;
	}
	memcpy(out, frame.data, frame.len);
	*written = frame.len;
	rc = KN_AX25_I_FRAME_OK;

out:
	kn_buffer_free(&frame);
	return rc;
}

void
kn_ax25_i_frame_decoded_clear(struct kn_ax25_i_frame_decoded *decoded)
{
	if (decoded == NULL)
		return;
	memset(decoded, 0, sizeof(*decoded));
}

enum kn_ax25_i_frame_error
kn_ax25_i_frame_decode_raw(const uint8_t *data, size_t data_len,
	struct kn_ax25_i_frame_decoded *decoded)
{
	struct kn_ax25_frame frame;
	struct kn_ax25_control_info control;
	size_t i;

	if (data == NULL || decoded == NULL)
		return KN_AX25_I_FRAME_ERR_INVALID_ARGUMENT;
	kn_ax25_i_frame_decoded_clear(decoded);
	if (kn_ax25_frame_decode(data, data_len, &frame) != KN_AX25_OK)
		return KN_AX25_I_FRAME_ERR_MALFORMED;
	kn_ax25_control_decode(frame.control, &control);
	if (control.class != KN_AX25_CONTROL_CLASS_I)
		return KN_AX25_I_FRAME_ERR_INVALID_VALUE;
	if (frame.has_pid == 0)
		return KN_AX25_I_FRAME_ERR_MALFORMED;

	decoded->source = frame.source.callsign;
	decoded->destination = frame.destination.callsign;
	decoded->digipeater_count = frame.digipeater_count;
	for (i = 0; i < frame.digipeater_count; i++)
		decoded->digipeaters[i] = frame.digipeaters[i].callsign;
	decoded->ns = control.ns;
	decoded->nr = control.nr;
	decoded->poll_final = control.poll_final;
	decoded->pid = frame.pid;
	decoded->payload = frame.payload;
	decoded->payload_len = frame.payload_len;
	preview_set(decoded);
	return KN_AX25_I_FRAME_OK;
}

void
kn_ax25_i_frame_request_clear(struct kn_ax25_i_frame_request *request)
{
	if (request == NULL)
		return;
	memset(request, 0, sizeof(*request));
	request->modulo_mode = KN_AX25_MODULO_8;
	request->pid = KN_AX25_PID_NO_LAYER_3;
	request->max_info_len = 256;
}
