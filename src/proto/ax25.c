/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/proto/ax25.c */

#include <sys/types.h>

#include <stdint.h>
#include <string.h>

#include "kilonode/ax25.h"

static enum kn_ax25_error address_valid(const struct kn_ax25_addr *);
static uint8_t control_has_pid(uint8_t);
static enum kn_ax25_error frame_addresses_decode(const uint8_t *, size_t,
	struct kn_ax25_frame *, size_t *);
static enum kn_ax25_error rc_from_buffer(int);

static enum kn_ax25_error
address_valid(const struct kn_ax25_addr *addr)
{
	size_t i;

	if (addr == NULL)
		return KN_AX25_ERR_INVALID_ARGUMENT;

	if (addr->callsign.call[0] == '\0')
		return KN_AX25_ERR_MALFORMED_ADDRESS;

	if (addr->callsign.ssid > 15)
		return KN_AX25_ERR_INVALID_SSID;

	for (i = 0; addr->callsign.call[i] != '\0'; i++) {
		if (i >= KN_CALLSIGN_MAX)
			return KN_AX25_ERR_MALFORMED_ADDRESS;
		if (!((addr->callsign.call[i] >= 'A' &&
			addr->callsign.call[i] <= 'Z') ||
			(addr->callsign.call[i] >= '0' &&
			addr->callsign.call[i] <= '9')))
			return KN_AX25_ERR_MALFORMED_ADDRESS;
	}

	return KN_AX25_OK;
}

static uint8_t
control_has_pid(uint8_t control)
{
	if ((control & 0x01u) == 0)
		return 1;

	if ((control & 0xefu) == KN_AX25_CONTROL_UI)
		return 1;

	return 0;
}

static enum kn_ax25_error
frame_addresses_decode(const uint8_t *data, size_t data_len,
	struct kn_ax25_frame *frame, size_t *addr_len)
{
	size_t offset;
	uint8_t final;

	if (data_len < KN_AX25_ADDR_LEN * 2)
		return KN_AX25_ERR_SHORT_FRAME;

	offset = 0;
	final = 0;
	if (kn_ax25_address_decode(data + offset, data_len - offset,
		&frame->destination, &final) != KN_AX25_OK)
		return KN_AX25_ERR_MALFORMED_ADDRESS;
	offset += KN_AX25_ADDR_LEN;

	if (final != 0)
		return KN_AX25_ERR_MALFORMED_ADDRESS;

	if (kn_ax25_address_decode(data + offset, data_len - offset,
		&frame->source, &final) != KN_AX25_OK)
		return KN_AX25_ERR_MALFORMED_ADDRESS;
	offset += KN_AX25_ADDR_LEN;

	while (final == 0) {
		if (frame->digipeater_count >= KN_AX25_MAX_DIGIS)
			return KN_AX25_ERR_TOO_MANY_DIGIS;

		if (data_len - offset < KN_AX25_ADDR_LEN)
			return KN_AX25_ERR_UNTERMINATED_ADDRESS;

		if (kn_ax25_address_decode(data + offset, data_len - offset,
			&frame->digipeaters[frame->digipeater_count],
			&final) != KN_AX25_OK)
			return KN_AX25_ERR_MALFORMED_ADDRESS;

		frame->digipeater_count++;
		offset += KN_AX25_ADDR_LEN;
	}

	*addr_len = offset;
	return KN_AX25_OK;
}

enum kn_ax25_error
kn_ax25_address_decode(const uint8_t *data, size_t data_len,
	struct kn_ax25_addr *out, uint8_t *final)
{
	size_t i;
	size_t call_len;
	uint8_t ch;
	uint8_t padding;

	if (data == NULL || out == NULL || final == NULL)
		return KN_AX25_ERR_INVALID_ARGUMENT;

	if (data_len < KN_AX25_ADDR_LEN)
		return KN_AX25_ERR_SHORT_FRAME;

	memset(out, 0, sizeof(*out));
	call_len = 0;
	padding = 0;

	for (i = 0; i < KN_CALLSIGN_MAX; i++) {
		if ((data[i] & 0x01u) != 0)
			return KN_AX25_ERR_MALFORMED_ADDRESS;

		ch = (uint8_t)(data[i] >> 1);
		if (ch == ' ') {
			padding = 1;
			continue;
		}

		if (padding != 0)
			return KN_AX25_ERR_MALFORMED_ADDRESS;

		if (!((ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9')))
			return KN_AX25_ERR_MALFORMED_ADDRESS;

		out->callsign.call[call_len++] = (char)ch;
	}

	if (call_len == 0)
		return KN_AX25_ERR_MALFORMED_ADDRESS;

	if ((data[6] & 0x60u) != 0x60u)
		return KN_AX25_ERR_MALFORMED_ADDRESS;

	out->callsign.call[call_len] = '\0';
	out->callsign.ssid = (uint8_t)((data[6] >> 1) & 0x0fu);
	out->repeated = (uint8_t)((data[6] & 0x80u) != 0);
	*final = (uint8_t)(data[6] & 0x01u);

	return KN_AX25_OK;
}

enum kn_ax25_error
kn_ax25_address_encode(const struct kn_ax25_addr *addr, uint8_t final,
	struct kn_buffer *out)
{
	uint8_t encoded[KN_AX25_ADDR_LEN];
	size_t call_len;
	size_t i;
	enum kn_ax25_error rc;

	if (out == NULL)
		return KN_AX25_ERR_INVALID_ARGUMENT;

	rc = address_valid(addr);
	if (rc != KN_AX25_OK)
		return rc;

	memset(encoded, 0, sizeof(encoded));
	call_len = strlen(addr->callsign.call);

	for (i = 0; i < KN_CALLSIGN_MAX; i++) {
		if (i < call_len)
			encoded[i] = (uint8_t)((uint8_t)addr->callsign.call[i] << 1);
		else
			encoded[i] = (uint8_t)(' ' << 1);
	}

	encoded[6] = (uint8_t)(0x60u |
		((unsigned int)addr->callsign.ssid << 1));
	if (addr->repeated != 0)
		encoded[6] = (uint8_t)(encoded[6] | 0x80u);
	if (final != 0)
		encoded[6] = (uint8_t)(encoded[6] | 0x01u);

	return rc_from_buffer(kn_buffer_append(out, encoded, sizeof(encoded)));
}

enum kn_ax25_error
kn_ax25_frame_decode(const uint8_t *data, size_t data_len,
	struct kn_ax25_frame *out)
{
	size_t offset;
	enum kn_ax25_error rc;

	if (data == NULL || out == NULL)
		return KN_AX25_ERR_INVALID_ARGUMENT;

	kn_ax25_frame_reset(out);

	rc = frame_addresses_decode(data, data_len, out, &offset);
	if (rc != KN_AX25_OK)
		return rc;

	if (offset >= data_len)
		return KN_AX25_ERR_SHORT_FRAME;

	out->control = data[offset++];
	out->has_pid = control_has_pid(out->control);

	if (out->has_pid != 0) {
		if (offset >= data_len)
			return KN_AX25_ERR_MISSING_PID;
		out->pid = data[offset++];
	}

	out->payload = data + offset;
	out->payload_len = data_len - offset;

	return KN_AX25_OK;
}

void
kn_ax25_frame_reset(struct kn_ax25_frame *frame)
{
	if (frame == NULL)
		return;

	memset(frame, 0, sizeof(*frame));
}

enum kn_ax25_error
kn_ax25_ui_frame_encode(const struct kn_ax25_frame *frame,
	struct kn_buffer *out)
{
	size_t i;
	enum kn_ax25_error rc;

	if (frame == NULL || out == NULL)
		return KN_AX25_ERR_INVALID_ARGUMENT;

	if (frame->digipeater_count > KN_AX25_MAX_DIGIS)
		return KN_AX25_ERR_TOO_MANY_DIGIS;

	if (frame->payload == NULL && frame->payload_len > 0)
		return KN_AX25_ERR_INVALID_ARGUMENT;

	rc = kn_ax25_address_encode(&frame->destination, 0, out);
	if (rc != KN_AX25_OK)
		return rc;

	rc = kn_ax25_address_encode(&frame->source,
		(uint8_t)(frame->digipeater_count == 0), out);
	if (rc != KN_AX25_OK)
		return rc;

	for (i = 0; i < frame->digipeater_count; i++) {
		rc = kn_ax25_address_encode(&frame->digipeaters[i],
			(uint8_t)(i + 1 == frame->digipeater_count), out);
		if (rc != KN_AX25_OK)
			return rc;
	}

	rc = rc_from_buffer(kn_buffer_append_byte(out, KN_AX25_CONTROL_UI));
	if (rc != KN_AX25_OK)
		return rc;

	rc = rc_from_buffer(kn_buffer_append_byte(out, frame->pid));
	if (rc != KN_AX25_OK)
		return rc;

	return rc_from_buffer(kn_buffer_append(out, frame->payload,
		frame->payload_len));
}

static enum kn_ax25_error
rc_from_buffer(int rc)
{
	if (rc == 0)
		return KN_AX25_OK;

	return KN_AX25_ERR_BUFFER;
}
