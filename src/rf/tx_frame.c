/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/rf/tx_frame.c */

#include <sys/types.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "kilonode/buffer.h"
#include "kilonode/kiss.h"
#include "kilonode/tx_frame.h"

static enum kn_tx_frame_error callsign_text(const struct kn_callsign *,
	char *, size_t);
static enum kn_tx_frame_error copy_port(char *, size_t, const char *);
static enum kn_tx_frame_error encode_kiss(struct kn_tx_frame *);
static enum kn_tx_frame_error format_path(const struct kn_tx_frame *,
	char *, size_t);
static enum kn_tx_frame_error format_preview(const uint8_t *, size_t, size_t,
	struct kn_tx_frame *);
static uint8_t payload_printable(const uint8_t *, size_t);
static enum kn_tx_frame_error store_ax25(struct kn_tx_frame *,
	const uint8_t *, size_t);

enum kn_tx_frame_error
kn_tx_frame_build_raw_ax25(struct kn_tx_frame *frame, uint64_t id,
	uint64_t now, const char *port_name, uint8_t kiss_port,
	uint8_t kiss_command, const uint8_t *ax25, size_t ax25_len,
	size_t preview_bytes)
{
	if (frame == NULL || port_name == NULL || (ax25 == NULL &&
	    ax25_len > 0))
		return KN_TX_FRAME_ERR_INVALID_ARGUMENT;
	if (kiss_port > 15 || kiss_command > 15)
		return KN_TX_FRAME_ERR_INVALID_VALUE;

	kn_tx_frame_clear(frame);
	frame->id = id;
	frame->created = now;
	frame->kiss_port = kiss_port;
	frame->kiss_command = kiss_command;
	frame->kind = KN_TX_FRAME_RAW_AX25;
	frame->status = KN_TX_FRAME_QUEUED;
	frame->payload_len = ax25_len;

	if (copy_port(frame->port_name, sizeof(frame->port_name),
	    port_name) != KN_TX_FRAME_OK)
		return KN_TX_FRAME_ERR_INVALID_VALUE;
	if (store_ax25(frame, ax25, ax25_len) != KN_TX_FRAME_OK)
		return KN_TX_FRAME_ERR_TOO_LARGE;
	if (format_preview(ax25, ax25_len, preview_bytes, frame) !=
	    KN_TX_FRAME_OK)
		return KN_TX_FRAME_ERR_BUFFER;
	return encode_kiss(frame);
}

enum kn_tx_frame_error
kn_tx_frame_build_ui(struct kn_tx_frame *frame, uint64_t id, uint64_t now,
	const char *port_name, uint8_t kiss_port, const char *source,
	const char *destination, const char *const *digipeaters,
	size_t digipeater_count, uint8_t pid, const uint8_t *payload,
	size_t payload_len, const struct kn_tx_policy *policy)
{
	struct kn_ax25_frame ax25;
	struct kn_buffer out;
	size_t i;
	enum kn_ax25_error ax25_rc;
	enum kn_tx_policy_error policy_rc;

	if (frame == NULL || port_name == NULL || source == NULL ||
	    destination == NULL || (payload == NULL && payload_len > 0))
		return KN_TX_FRAME_ERR_INVALID_ARGUMENT;
	if (kiss_port > 15 || digipeater_count > KN_AX25_MAX_DIGIS)
		return KN_TX_FRAME_ERR_INVALID_VALUE;
	if (digipeater_count > 0 && digipeaters == NULL)
		return KN_TX_FRAME_ERR_INVALID_ARGUMENT;
	if (policy != NULL) {
		policy_rc = kn_tx_policy_allow_ui(policy, payload_len);
		if (policy_rc == KN_TX_POLICY_ERR_TOO_LARGE)
			return KN_TX_FRAME_ERR_TOO_LARGE;
		if (policy_rc != KN_TX_POLICY_OK)
			return KN_TX_FRAME_ERR_POLICY;
	}

	kn_tx_frame_clear(frame);
	frame->id = id;
	frame->created = now;
	frame->kiss_port = kiss_port;
	frame->kiss_command = 0;
	frame->kind = KN_TX_FRAME_UI;
	frame->status = policy != NULL && policy->dry_run != 0 ?
	    KN_TX_FRAME_DRY_RUN : KN_TX_FRAME_QUEUED;
	frame->control = KN_AX25_CONTROL_UI;
	frame->pid = pid;
	frame->has_pid = 1;
	frame->payload_len = payload_len;

	if (copy_port(frame->port_name, sizeof(frame->port_name),
	    port_name) != KN_TX_FRAME_OK)
		return KN_TX_FRAME_ERR_INVALID_VALUE;
	if (kn_callsign_parse(source, &frame->source) != 0 ||
	    kn_callsign_parse(destination, &frame->destination) != 0)
		return KN_TX_FRAME_ERR_INVALID_VALUE;
	frame->digipeater_count = digipeater_count;
	for (i = 0; i < digipeater_count; i++) {
		if (digipeaters[i] == NULL ||
		    kn_callsign_parse(digipeaters[i],
		    &frame->digipeaters[i]) != 0)
			return KN_TX_FRAME_ERR_INVALID_VALUE;
	}
	if (format_path(frame, frame->path, sizeof(frame->path)) !=
	    KN_TX_FRAME_OK)
		return KN_TX_FRAME_ERR_BUFFER;
	if (format_preview(payload, payload_len,
	    policy == NULL ? KN_TX_POLICY_PREVIEW_DEFAULT :
	    policy->payload_preview_bytes, frame) != KN_TX_FRAME_OK)
		return KN_TX_FRAME_ERR_BUFFER;

	kn_ax25_frame_reset(&ax25);
	ax25.source.callsign = frame->source;
	ax25.destination.callsign = frame->destination;
	ax25.digipeater_count = frame->digipeater_count;
	for (i = 0; i < frame->digipeater_count; i++)
		ax25.digipeaters[i].callsign = frame->digipeaters[i];
	ax25.control = KN_AX25_CONTROL_UI;
	ax25.pid = pid;
	ax25.has_pid = 1;
	ax25.payload = payload;
	ax25.payload_len = payload_len;

	if (kn_buffer_init(&out, 0) != 0)
		return KN_TX_FRAME_ERR_BUFFER;
	ax25_rc = kn_ax25_ui_frame_encode(&ax25, &out);
	if (ax25_rc != KN_AX25_OK) {
		kn_buffer_free(&out);
		return KN_TX_FRAME_ERR_INVALID_VALUE;
	}
	if (store_ax25(frame, out.data, out.len) != KN_TX_FRAME_OK) {
		kn_buffer_free(&out);
		return KN_TX_FRAME_ERR_TOO_LARGE;
	}
	kn_buffer_free(&out);

	return encode_kiss(frame);
}

void
kn_tx_frame_clear(struct kn_tx_frame *frame)
{
	if (frame == NULL)
		return;

	memset(frame, 0, sizeof(*frame));
}

enum kn_tx_frame_error
kn_tx_frame_format_brief(const struct kn_tx_frame *frame, char *buf,
	size_t bufsiz)
{
	char source[KN_CALLSIGN_MAX + 4];
	char destination[KN_CALLSIGN_MAX + 4];
	int needed;

	if (frame == NULL || buf == NULL || bufsiz == 0)
		return KN_TX_FRAME_ERR_INVALID_ARGUMENT;

	if (frame->kind == KN_TX_FRAME_RAW_AX25) {
		needed = snprintf(buf, bufsiz,
		    "TX FRAME id=%llu port=%s kind=%s status=%s "
		    "payload_len=%llu %s=%s",
		    (unsigned long long)frame->id, frame->port_name,
		    kn_tx_frame_kind_name(frame->kind),
		    kn_tx_frame_status_name(frame->status),
		    (unsigned long long)frame->payload_len,
		    frame->preview_binary != 0 ? "preview_hex" : "preview",
		    frame->preview);
		return needed >= 0 && (size_t)needed < bufsiz ?
		    KN_TX_FRAME_OK : KN_TX_FRAME_ERR_BUFFER;
	}

	if (callsign_text(&frame->source, source, sizeof(source)) !=
	    KN_TX_FRAME_OK ||
	    callsign_text(&frame->destination, destination,
	    sizeof(destination)) != KN_TX_FRAME_OK)
		return KN_TX_FRAME_ERR_INVALID_VALUE;

	needed = snprintf(buf, bufsiz,
	    "TX FRAME id=%llu port=%s kind=%s status=%s from=%s to=%s "
	    "payload_len=%llu %s=%s",
	    (unsigned long long)frame->id, frame->port_name,
	    kn_tx_frame_kind_name(frame->kind),
	    kn_tx_frame_status_name(frame->status), source, destination,
	    (unsigned long long)frame->payload_len,
	    frame->preview_binary != 0 ? "preview_hex" : "preview",
	    frame->preview);
	return needed >= 0 && (size_t)needed < bufsiz ?
	    KN_TX_FRAME_OK : KN_TX_FRAME_ERR_BUFFER;
}

enum kn_tx_frame_error
kn_tx_frame_format_full(const struct kn_tx_frame *frame, char *buf,
	size_t bufsiz)
{
	char source[KN_CALLSIGN_MAX + 4];
	char destination[KN_CALLSIGN_MAX + 4];
	char pid[16];
	int needed;

	if (frame == NULL || buf == NULL || bufsiz == 0)
		return KN_TX_FRAME_ERR_INVALID_ARGUMENT;

	if (frame->has_pid != 0)
		(void)snprintf(pid, sizeof(pid), "0x%02x",
		    (unsigned int)frame->pid);
	else
		(void)snprintf(pid, sizeof(pid), "none");

	if (frame->kind == KN_TX_FRAME_RAW_AX25) {
		needed = snprintf(buf, bufsiz,
		    "TX FRAME id=%llu time=%llu port=%s kiss_port=%u "
		    "kiss_cmd=%u kind=%s status=%s payload_len=%llu "
		    "%s=%s ax25_len=%llu kiss_len=%llu",
		    (unsigned long long)frame->id,
		    (unsigned long long)frame->created, frame->port_name,
		    (unsigned int)frame->kiss_port,
		    (unsigned int)frame->kiss_command,
		    kn_tx_frame_kind_name(frame->kind),
		    kn_tx_frame_status_name(frame->status),
		    (unsigned long long)frame->payload_len,
		    frame->preview_binary != 0 ? "preview_hex" : "preview",
		    frame->preview, (unsigned long long)frame->ax25_len,
		    (unsigned long long)frame->kiss_len);
		return needed >= 0 && (size_t)needed < bufsiz ?
		    KN_TX_FRAME_OK : KN_TX_FRAME_ERR_BUFFER;
	}

	if (callsign_text(&frame->source, source, sizeof(source)) !=
	    KN_TX_FRAME_OK ||
	    callsign_text(&frame->destination, destination,
	    sizeof(destination)) != KN_TX_FRAME_OK)
		return KN_TX_FRAME_ERR_INVALID_VALUE;

	needed = snprintf(buf, bufsiz,
	    "TX FRAME id=%llu time=%llu port=%s kiss_port=%u kiss_cmd=%u "
	    "kind=%s status=%s from=%s to=%s via=%s control=0x%02x "
	    "pid=%s payload_len=%llu %s=%s ax25_len=%llu kiss_len=%llu",
	    (unsigned long long)frame->id,
	    (unsigned long long)frame->created, frame->port_name,
	    (unsigned int)frame->kiss_port, (unsigned int)frame->kiss_command,
	    kn_tx_frame_kind_name(frame->kind),
	    kn_tx_frame_status_name(frame->status), source, destination,
	    frame->path[0] == '\0' ? "-" : frame->path,
	    (unsigned int)frame->control, pid,
	    (unsigned long long)frame->payload_len,
	    frame->preview_binary != 0 ? "preview_hex" : "preview",
	    frame->preview, (unsigned long long)frame->ax25_len,
	    (unsigned long long)frame->kiss_len);
	return needed >= 0 && (size_t)needed < bufsiz ?
	    KN_TX_FRAME_OK : KN_TX_FRAME_ERR_BUFFER;
}

const char *
kn_tx_frame_kind_name(enum kn_tx_frame_kind kind)
{
	switch (kind) {
	case KN_TX_FRAME_UI:
		return "UI";
	case KN_TX_FRAME_RAW_AX25:
		return "raw-ax25";
	case KN_TX_FRAME_UNKNOWN:
		return "unknown";
	}

	return "unknown";
}

const char *
kn_tx_frame_status_name(enum kn_tx_frame_status status)
{
	switch (status) {
	case KN_TX_FRAME_QUEUED:
		return "queued";
	case KN_TX_FRAME_SENT:
		return "sent";
	case KN_TX_FRAME_DROPPED:
		return "dropped";
	case KN_TX_FRAME_FAILED:
		return "failed";
	case KN_TX_FRAME_DRY_RUN:
		return "dry-run";
	}

	return "unknown";
}

static enum kn_tx_frame_error
callsign_text(const struct kn_callsign *callsign, char *buf, size_t bufsiz)
{
	if (kn_callsign_format(callsign, buf, bufsiz) != 0)
		return KN_TX_FRAME_ERR_INVALID_VALUE;

	return KN_TX_FRAME_OK;
}

static enum kn_tx_frame_error
copy_port(char *dst, size_t dst_len, const char *src)
{
	size_t len;

	len = strlen(src);
	if (len == 0 || len >= dst_len)
		return KN_TX_FRAME_ERR_INVALID_VALUE;

	memcpy(dst, src, len + 1);
	return KN_TX_FRAME_OK;
}

static enum kn_tx_frame_error
encode_kiss(struct kn_tx_frame *frame)
{
	struct kn_buffer body;
	struct kn_buffer escaped;
	uint8_t type;
	enum kn_tx_frame_error rc;

	if (kn_buffer_init(&body, 0) != 0)
		return KN_TX_FRAME_ERR_BUFFER;
	if (kn_buffer_init(&escaped, 0) != 0) {
		kn_buffer_free(&body);
		return KN_TX_FRAME_ERR_BUFFER;
	}

	rc = KN_TX_FRAME_ERR_BUFFER;
	type = (uint8_t)(((unsigned int)frame->kiss_port << 4) |
	    ((unsigned int)frame->kiss_command & 0x0fU));
	if (kn_buffer_append_byte(&body, type) != 0)
		goto out;
	if (kn_buffer_append(&body, frame->ax25, frame->ax25_len) != 0)
		goto out;
	if (kn_kiss_escape(body.data, body.len, &escaped) != 0)
		goto out;
	if (escaped.len + 2 > sizeof(frame->kiss)) {
		rc = KN_TX_FRAME_ERR_TOO_LARGE;
		goto out;
	}

	frame->kiss[0] = KN_KISS_FEND;
	memcpy(frame->kiss + 1, escaped.data, escaped.len);
	frame->kiss[escaped.len + 1] = KN_KISS_FEND;
	frame->kiss_len = escaped.len + 2;
	rc = KN_TX_FRAME_OK;

out:
	kn_buffer_free(&body);
	kn_buffer_free(&escaped);
	return rc;
}

static enum kn_tx_frame_error
format_path(const struct kn_tx_frame *frame, char *buf, size_t bufsiz)
{
	char call[KN_CALLSIGN_MAX + 4];
	size_t i;
	size_t offset;
	int needed;

	if (buf == NULL || bufsiz == 0)
		return KN_TX_FRAME_ERR_INVALID_ARGUMENT;

	if (frame->digipeater_count == 0) {
		(void)snprintf(buf, bufsiz, "-");
		return KN_TX_FRAME_OK;
	}

	offset = 0;
	buf[0] = '\0';
	for (i = 0; i < frame->digipeater_count; i++) {
		if (kn_callsign_format(&frame->digipeaters[i], call,
		    sizeof(call)) != 0)
			return KN_TX_FRAME_ERR_INVALID_VALUE;
		needed = snprintf(buf + offset, bufsiz - offset, "%s%s",
		    i == 0 ? "" : ",", call);
		if (needed < 0 || (size_t)needed >= bufsiz - offset)
			return KN_TX_FRAME_ERR_BUFFER;
		offset += (size_t)needed;
	}

	return KN_TX_FRAME_OK;
}

static enum kn_tx_frame_error
format_preview(const uint8_t *payload, size_t payload_len,
	size_t preview_bytes, struct kn_tx_frame *frame)
{
	size_t limit;
	size_t i;
	size_t offset;
	int needed;

	if (payload == NULL && payload_len > 0)
		return KN_TX_FRAME_ERR_INVALID_ARGUMENT;

	if (preview_bytes == 0 || preview_bytes > KN_TX_POLICY_PREVIEW_MAX)
		preview_bytes = KN_TX_POLICY_PREVIEW_DEFAULT;

	limit = payload_len < preview_bytes ? payload_len : preview_bytes;
	frame->preview_len = limit;
	frame->preview_binary = payload_printable(payload, limit) == 0 ? 1 : 0;

	if (frame->preview_binary == 0) {
		offset = 0;
		frame->preview[offset++] = '"';
		for (i = 0; i < limit; i++) {
			if (payload[i] == '"' || payload[i] == '\\') {
				if (offset + 2 >= sizeof(frame->preview))
					return KN_TX_FRAME_ERR_BUFFER;
				frame->preview[offset++] = '\\';
				frame->preview[offset++] = (char)payload[i];
			} else {
				if (offset + 1 >= sizeof(frame->preview))
					return KN_TX_FRAME_ERR_BUFFER;
				frame->preview[offset++] = (char)payload[i];
			}
		}
		if (offset + 1 >= sizeof(frame->preview))
			return KN_TX_FRAME_ERR_BUFFER;
		frame->preview[offset++] = '"';
		frame->preview[offset] = '\0';
		return KN_TX_FRAME_OK;
	}

	offset = 0;
	for (i = 0; i < limit; i++) {
		needed = snprintf(frame->preview + offset,
		    sizeof(frame->preview) - offset, "%02x",
		    (unsigned int)payload[i]);
		if (needed < 0 ||
		    (size_t)needed >= sizeof(frame->preview) - offset)
			return KN_TX_FRAME_ERR_BUFFER;
		offset += (size_t)needed;
	}
	if (limit == 0)
		(void)snprintf(frame->preview, sizeof(frame->preview), "-");

	return KN_TX_FRAME_OK;
}

static uint8_t
payload_printable(const uint8_t *payload, size_t len)
{
	size_t i;

	if (len == 0)
		return 1;

	for (i = 0; i < len; i++) {
		if (payload[i] < 0x20 || payload[i] > 0x7e)
			return 0;
	}

	return 1;
}

static enum kn_tx_frame_error
store_ax25(struct kn_tx_frame *frame, const uint8_t *ax25, size_t ax25_len)
{
	if (ax25 == NULL && ax25_len > 0)
		return KN_TX_FRAME_ERR_INVALID_ARGUMENT;
	if (ax25_len == 0 || ax25_len > sizeof(frame->ax25))
		return KN_TX_FRAME_ERR_TOO_LARGE;

	memcpy(frame->ax25, ax25, ax25_len);
	frame->ax25_len = ax25_len;
	return KN_TX_FRAME_OK;
}
