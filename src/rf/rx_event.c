/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/rf/rx_event.c */

#include <sys/types.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "kilonode/ax25.h"
#include "kilonode/callsign.h"
#include "kilonode/rx_event.h"

static enum kn_rx_event_error callsign_text(const struct kn_callsign *,
	char *, size_t);
static enum kn_rx_event_error copy_port(char *, size_t, const char *);
static enum kn_rx_event_error format_path(const struct kn_ax25_frame *,
	char *, size_t);
static enum kn_rx_event_error format_preview(const uint8_t *, size_t, size_t,
	struct kn_rx_event *);
static uint8_t payload_printable(const uint8_t *, size_t);

enum kn_rx_frame_kind
kn_rx_event_classify_control(uint8_t control)
{
	if ((control & 0xefU) == KN_AX25_CONTROL_UI)
		return KN_RX_FRAME_UI;
	if ((control & 0x01U) == 0)
		return KN_RX_FRAME_I;
	if ((control & 0x03U) == 0x01U)
		return KN_RX_FRAME_S;
	if ((control & 0x03U) == 0x03U)
		return KN_RX_FRAME_U;

	return KN_RX_FRAME_UNKNOWN;
}

void
kn_rx_event_clear(struct kn_rx_event *event)
{
	if (event == NULL)
		return;

	memset(event, 0, sizeof(*event));
}

const char *
kn_rx_event_kind_name(enum kn_rx_frame_kind kind)
{
	switch (kind) {
	case KN_RX_FRAME_UI:
		return "UI";
	case KN_RX_FRAME_I:
		return "I";
	case KN_RX_FRAME_S:
		return "S";
	case KN_RX_FRAME_U:
		return "U";
	case KN_RX_FRAME_UNKNOWN:
		return "unknown";
	case KN_RX_FRAME_MALFORMED:
		return "malformed";
	}

	return "unknown";
}

enum kn_rx_event_error
kn_rx_event_from_ax25(struct kn_rx_event *event, uint64_t id, uint64_t now,
	const char *port_name, uint8_t kiss_port, uint8_t kiss_command,
	const struct kn_ax25_frame *frame, size_t preview_bytes)
{
	if (event == NULL || port_name == NULL || frame == NULL)
		return KN_RX_EVENT_ERR_INVALID_ARGUMENT;
	if (frame->payload == NULL && frame->payload_len > 0)
		return KN_RX_EVENT_ERR_INVALID_ARGUMENT;

	kn_rx_event_clear(event);
	event->id = id;
	event->timestamp = now;
	event->kiss_port = kiss_port;
	event->kiss_command = kiss_command;
	event->source = frame->source.callsign;
	event->destination = frame->destination.callsign;
	event->control = frame->control;
	event->pid = frame->pid;
	event->has_pid = frame->has_pid;
	event->kind = kn_rx_event_classify_control(frame->control);
	event->payload_len = frame->payload_len;
	event->decode_status = KN_AX25_OK;

	if (copy_port(event->port_name, sizeof(event->port_name),
	    port_name) != KN_RX_EVENT_OK)
		return KN_RX_EVENT_ERR_INVALID_VALUE;
	if (format_path(frame, event->path, sizeof(event->path)) !=
	    KN_RX_EVENT_OK)
		return KN_RX_EVENT_ERR_BUFFER;
	return format_preview(frame->payload, frame->payload_len,
	    preview_bytes, event);
}

enum kn_rx_event_error
kn_rx_event_from_malformed(struct kn_rx_event *event, uint64_t id,
	uint64_t now, const char *port_name, uint8_t kiss_port,
	uint8_t kiss_command, int decode_status, size_t payload_len)
{
	if (event == NULL || port_name == NULL)
		return KN_RX_EVENT_ERR_INVALID_ARGUMENT;

	kn_rx_event_clear(event);
	event->id = id;
	event->timestamp = now;
	event->kiss_port = kiss_port;
	event->kiss_command = kiss_command;
	event->kind = KN_RX_FRAME_MALFORMED;
	event->payload_len = payload_len;
	event->decode_status = decode_status;
	event->malformed = 1;
	(void)snprintf(event->path, sizeof(event->path), "-");
	(void)snprintf(event->preview, sizeof(event->preview), "-");

	if (copy_port(event->port_name, sizeof(event->port_name),
	    port_name) != KN_RX_EVENT_OK)
		return KN_RX_EVENT_ERR_INVALID_VALUE;

	return KN_RX_EVENT_OK;
}

enum kn_rx_event_error
kn_rx_event_format_brief(const struct kn_rx_event *event, char *buf,
	size_t bufsiz)
{
	char source[KN_CALLSIGN_MAX + 4];
	char destination[KN_CALLSIGN_MAX + 4];
	int needed;

	if (event == NULL || buf == NULL || bufsiz == 0)
		return KN_RX_EVENT_ERR_INVALID_ARGUMENT;

	if (event->malformed != 0) {
		needed = snprintf(buf, bufsiz,
		    "RX EVENT id=%llu port=%s kiss_port=%u kind=%s "
		    "payload_len=%llu decode=%d",
		    (unsigned long long)event->id, event->port_name,
		    (unsigned int)event->kiss_port,
		    kn_rx_event_kind_name(event->kind),
		    (unsigned long long)event->payload_len,
		    event->decode_status);
		return needed >= 0 && (size_t)needed < bufsiz ?
		    KN_RX_EVENT_OK : KN_RX_EVENT_ERR_BUFFER;
	}

	if (callsign_text(&event->source, source, sizeof(source)) !=
	    KN_RX_EVENT_OK ||
	    callsign_text(&event->destination, destination,
	    sizeof(destination)) != KN_RX_EVENT_OK)
		return KN_RX_EVENT_ERR_INVALID_VALUE;

	needed = snprintf(buf, bufsiz,
	    "RX EVENT id=%llu port=%s kiss_port=%u kind=%s from=%s to=%s "
	    "payload_len=%llu %s=%s",
	    (unsigned long long)event->id, event->port_name,
	    (unsigned int)event->kiss_port,
	    kn_rx_event_kind_name(event->kind), source, destination,
	    (unsigned long long)event->payload_len,
	    event->preview_binary != 0 ? "preview_hex" : "preview",
	    event->preview);
	return needed >= 0 && (size_t)needed < bufsiz ?
	    KN_RX_EVENT_OK : KN_RX_EVENT_ERR_BUFFER;
}

enum kn_rx_event_error
kn_rx_event_format_full(const struct kn_rx_event *event, char *buf,
	size_t bufsiz)
{
	char source[KN_CALLSIGN_MAX + 4];
	char destination[KN_CALLSIGN_MAX + 4];
	char pid[16];
	int needed;

	if (event == NULL || buf == NULL || bufsiz == 0)
		return KN_RX_EVENT_ERR_INVALID_ARGUMENT;

	if (event->malformed != 0) {
		needed = snprintf(buf, bufsiz,
		    "RX EVENT id=%llu time=%llu port=%s kiss_port=%u "
		    "kiss_cmd=%u kind=%s payload_len=%llu decode=%d",
		    (unsigned long long)event->id,
		    (unsigned long long)event->timestamp, event->port_name,
		    (unsigned int)event->kiss_port,
		    (unsigned int)event->kiss_command,
		    kn_rx_event_kind_name(event->kind),
		    (unsigned long long)event->payload_len,
		    event->decode_status);
		return needed >= 0 && (size_t)needed < bufsiz ?
		    KN_RX_EVENT_OK : KN_RX_EVENT_ERR_BUFFER;
	}

	if (callsign_text(&event->source, source, sizeof(source)) !=
	    KN_RX_EVENT_OK ||
	    callsign_text(&event->destination, destination,
	    sizeof(destination)) != KN_RX_EVENT_OK)
		return KN_RX_EVENT_ERR_INVALID_VALUE;

	if (event->has_pid != 0)
		(void)snprintf(pid, sizeof(pid), "0x%02x",
		    (unsigned int)event->pid);
	else
		(void)snprintf(pid, sizeof(pid), "none");

	needed = snprintf(buf, bufsiz,
	    "RX EVENT id=%llu time=%llu port=%s kiss_port=%u kiss_cmd=%u "
	    "kind=%s from=%s to=%s via=%s control=0x%02x pid=%s "
	    "payload_len=%llu %s=%s",
	    (unsigned long long)event->id,
	    (unsigned long long)event->timestamp, event->port_name,
	    (unsigned int)event->kiss_port,
	    (unsigned int)event->kiss_command,
	    kn_rx_event_kind_name(event->kind), source, destination,
	    event->path[0] == '\0' ? "-" : event->path,
	    (unsigned int)event->control, pid,
	    (unsigned long long)event->payload_len,
	    event->preview_binary != 0 ? "preview_hex" : "preview",
	    event->preview);
	return needed >= 0 && (size_t)needed < bufsiz ?
	    KN_RX_EVENT_OK : KN_RX_EVENT_ERR_BUFFER;
}

static enum kn_rx_event_error
callsign_text(const struct kn_callsign *callsign, char *buf, size_t bufsiz)
{
	if (kn_callsign_format(callsign, buf, bufsiz) != 0)
		return KN_RX_EVENT_ERR_INVALID_VALUE;

	return KN_RX_EVENT_OK;
}

static enum kn_rx_event_error
copy_port(char *dst, size_t dst_len, const char *src)
{
	size_t len;

	len = strlen(src);
	if (len == 0 || len >= dst_len)
		return KN_RX_EVENT_ERR_INVALID_VALUE;

	memcpy(dst, src, len + 1);
	return KN_RX_EVENT_OK;
}

static enum kn_rx_event_error
format_path(const struct kn_ax25_frame *frame, char *buf, size_t bufsiz)
{
	char call[KN_CALLSIGN_MAX + 4];
	size_t i;
	size_t offset;
	int needed;

	if (buf == NULL || bufsiz == 0)
		return KN_RX_EVENT_ERR_INVALID_ARGUMENT;

	if (frame->digipeater_count == 0) {
		(void)snprintf(buf, bufsiz, "-");
		return KN_RX_EVENT_OK;
	}

	offset = 0;
	buf[0] = '\0';
	for (i = 0; i < frame->digipeater_count; i++) {
		if (kn_callsign_format(&frame->digipeaters[i].callsign, call,
		    sizeof(call)) != 0)
			return KN_RX_EVENT_ERR_INVALID_VALUE;
		needed = snprintf(buf + offset, bufsiz - offset, "%s%s%s",
		    i == 0 ? "" : ",", call,
		    frame->digipeaters[i].repeated != 0 ? "*" : "");
		if (needed < 0 || (size_t)needed >= bufsiz - offset)
			return KN_RX_EVENT_ERR_BUFFER;
		offset += (size_t)needed;
	}

	return KN_RX_EVENT_OK;
}

static enum kn_rx_event_error
format_preview(const uint8_t *payload, size_t payload_len,
	size_t preview_bytes, struct kn_rx_event *event)
{
	size_t limit;
	size_t i;
	size_t offset;
	int needed;

	if (payload == NULL && payload_len > 0)
		return KN_RX_EVENT_ERR_INVALID_ARGUMENT;

	if (preview_bytes == 0 || preview_bytes > KN_RX_EVENT_PREVIEW_MAX)
		preview_bytes = KN_RX_EVENT_PREVIEW_DEFAULT;

	limit = payload_len < preview_bytes ? payload_len : preview_bytes;
	event->preview_len = limit;
	event->preview_binary = payload_printable(payload, limit) == 0 ? 1 : 0;

	if (event->preview_binary == 0) {
		offset = 0;
		event->preview[offset++] = '"';
		for (i = 0; i < limit; i++) {
			if (payload[i] == '"' || payload[i] == '\\') {
				if (offset + 2 >= sizeof(event->preview))
					return KN_RX_EVENT_ERR_BUFFER;
				event->preview[offset++] = '\\';
				event->preview[offset++] = (char)payload[i];
			} else {
				if (offset + 1 >= sizeof(event->preview))
					return KN_RX_EVENT_ERR_BUFFER;
				event->preview[offset++] = (char)payload[i];
			}
		}
		if (offset + 1 >= sizeof(event->preview))
			return KN_RX_EVENT_ERR_BUFFER;
		event->preview[offset++] = '"';
		event->preview[offset] = '\0';
		return KN_RX_EVENT_OK;
	}

	offset = 0;
	for (i = 0; i < limit; i++) {
		needed = snprintf(event->preview + offset,
		    sizeof(event->preview) - offset, "%02x",
		    (unsigned int)payload[i]);
		if (needed < 0 || (size_t)needed >=
		    sizeof(event->preview) - offset)
			return KN_RX_EVENT_ERR_BUFFER;
		offset += (size_t)needed;
	}
	if (limit == 0)
		(void)snprintf(event->preview, sizeof(event->preview), "-");

	return KN_RX_EVENT_OK;
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
