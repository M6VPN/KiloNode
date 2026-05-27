/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/compat/compat_kiss_capture.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "kilonode/ax25.h"
#include "kilonode/ax25_control.h"
#include "kilonode/buffer.h"
#include "kilonode/compat_kiss_capture.h"
#include "kilonode/kiss_stream.h"

static void add_mismatch(struct kn_compat_packet_decode *, const char *);
static void apply_expectations(const struct kn_compat_packet_capture *,
	struct kn_compat_packet_decode *, const struct kn_ax25_frame *);
static void format_payload(const uint8_t *, size_t, char *, size_t);
static const char *kind_from_frame(const struct kn_ax25_frame *);

void
kn_compat_packet_decode_clear(struct kn_compat_packet_decode *decode)
{
	if (decode == NULL)
		return;

	memset(decode, 0, sizeof(*decode));
}

const char *
kn_compat_kiss_capture_error_name(enum kn_compat_kiss_capture_error error)
{
	switch (error) {
	case KN_COMPAT_KISS_CAPTURE_OK:
		return "ok";
	case KN_COMPAT_KISS_CAPTURE_ERR_INVALID_ARGUMENT:
		return "invalid-argument";
	case KN_COMPAT_KISS_CAPTURE_ERR_UNSUPPORTED:
		return "unsupported";
	case KN_COMPAT_KISS_CAPTURE_ERR_DECODE:
		return "decode";
	case KN_COMPAT_KISS_CAPTURE_ERR_MISMATCH:
		return "mismatch";
	}

	return "unknown";
}

enum kn_compat_kiss_capture_error
kn_compat_kiss_capture_decode(const struct kn_compat_packet_capture *capture,
	struct kn_compat_packet_decode *decode)
{
	struct kn_kiss_stream_parser parser;
	struct kn_kiss_stream_frame kiss_frame;
	struct kn_ax25_frame ax25;
	struct kn_buffer out;
	enum kn_compat_kiss_capture_error rc;

	if (capture == NULL || decode == NULL)
		return KN_COMPAT_KISS_CAPTURE_ERR_INVALID_ARGUMENT;
	if (capture->method != KN_COMPAT_PACKET_METHOD_KISS)
		return KN_COMPAT_KISS_CAPTURE_ERR_UNSUPPORTED;

	kn_compat_packet_decode_clear(decode);
	(void)snprintf(decode->capture_name, sizeof(decode->capture_name),
	    "%s", capture->name);
	decode->method = capture->method;

	if (kn_kiss_stream_init(&parser, KN_COMPAT_CAPTURE_FRAME_MAX) !=
	    KN_KISS_STREAM_OK)
		return KN_COMPAT_KISS_CAPTURE_ERR_DECODE;
	if (kn_buffer_init(&out, 0) != 0) {
		kn_kiss_stream_free(&parser);
		return KN_COMPAT_KISS_CAPTURE_ERR_DECODE;
	}
	if (kn_kiss_stream_feed(&parser, capture->frame,
	    capture->frame_len) != KN_KISS_STREAM_OK ||
	    kn_kiss_stream_pop_frame(&parser, &kiss_frame, &out) !=
	    KN_KISS_STREAM_OK) {
		kn_buffer_free(&out);
		kn_kiss_stream_free(&parser);
		return KN_COMPAT_KISS_CAPTURE_ERR_DECODE;
	}
	decode->kiss_command = kiss_frame.command;
	decode->kiss_port = kiss_frame.port;
	if (kn_ax25_frame_decode(kiss_frame.payload, kiss_frame.payload_len,
	    &ax25) != KN_AX25_OK) {
		kn_buffer_free(&out);
		kn_kiss_stream_free(&parser);
		return KN_COMPAT_KISS_CAPTURE_ERR_DECODE;
	}

	(void)kn_callsign_format(&ax25.source.callsign, decode->source,
	    sizeof(decode->source));
	(void)kn_callsign_format(&ax25.destination.callsign,
	    decode->destination, sizeof(decode->destination));
	(void)snprintf(decode->kind, sizeof(decode->kind), "%s",
	    kind_from_frame(&ax25));
	decode->has_pid = ax25.has_pid;
	decode->pid = ax25.pid;
	decode->payload_len = ax25.payload_len;
	format_payload(ax25.payload, ax25.payload_len,
	    decode->payload_preview, sizeof(decode->payload_preview));

	apply_expectations(capture, decode, &ax25);
	decode->passed = decode->mismatch_count == 0 ? 1 : 0;
	rc = decode->passed != 0 ? KN_COMPAT_KISS_CAPTURE_OK :
	    KN_COMPAT_KISS_CAPTURE_ERR_MISMATCH;

	kn_buffer_free(&out);
	kn_kiss_stream_free(&parser);
	return rc;
}

static void
add_mismatch(struct kn_compat_packet_decode *decode, const char *text)
{
	if (decode->mismatch_count >= KN_COMPAT_PACKET_MISMATCH_MAX)
		return;
	(void)snprintf(decode->mismatches[decode->mismatch_count].text,
	    sizeof(decode->mismatches[decode->mismatch_count].text), "%s",
	    text);
	decode->mismatch_count++;
}

static void
apply_expectations(const struct kn_compat_packet_capture *capture,
	struct kn_compat_packet_decode *decode, const struct kn_ax25_frame *frame)
{
	char call[KN_CALLSIGN_MAX + 4];

	if (capture->expect_decode == KN_COMPAT_PACKET_EXPECT_AX25_UI &&
	    strcmp(decode->kind, "UI") != 0)
		add_mismatch(decode, "decode");
	if (capture->has_expect_source != 0) {
		(void)kn_callsign_format(&capture->expect_source, call,
		    sizeof(call));
		if (strcmp(call, decode->source) != 0)
			add_mismatch(decode, "source");
	}
	if (capture->has_expect_destination != 0) {
		(void)kn_callsign_format(&capture->expect_destination, call,
		    sizeof(call));
		if (strcmp(call, decode->destination) != 0)
			add_mismatch(decode, "destination");
	}
	if (capture->has_expect_pid != 0 &&
	    (frame->has_pid == 0 || frame->pid != capture->expect_pid))
		add_mismatch(decode, "pid");
	if (capture->has_expect_payload_text != 0 &&
	    (frame->payload_len != strlen(capture->expect_payload_text) ||
	    memcmp(frame->payload, capture->expect_payload_text,
	    frame->payload_len) != 0))
		add_mismatch(decode, "payload-text");
	if (capture->expect_payload_hex_len > 0 &&
	    (frame->payload_len != capture->expect_payload_hex_len ||
	    memcmp(frame->payload, capture->expect_payload_hex,
	    frame->payload_len) != 0))
		add_mismatch(decode, "payload-hex");
	if (capture->has_expect_kind != 0 &&
	    strcmp(capture->expect_kind, decode->kind) != 0)
		add_mismatch(decode, "kind");
}

static void
format_payload(const uint8_t *payload, size_t payload_len, char *buf,
	size_t bufsiz)
{
	size_t i;
	size_t off;
	int needed;
	uint8_t printable;

	if (buf == NULL || bufsiz == 0)
		return;
	buf[0] = '\0';
	printable = 1;
	for (i = 0; i < payload_len; i++) {
		if (payload[i] < 0x20 || payload[i] > 0x7e) {
			printable = 0;
			break;
		}
	}
	off = 0;
	for (i = 0; payload != NULL && i < payload_len && off + 1 < bufsiz;
	    i++) {
		if (printable != 0)
			needed = snprintf(buf + off, bufsiz - off, "%c",
			    payload[i]);
		else
			needed = snprintf(buf + off, bufsiz - off, "%02x",
			    payload[i]);
		if (needed < 0 || (size_t)needed >= bufsiz - off)
			return;
		off += (size_t)needed;
	}
}

static const char *
kind_from_frame(const struct kn_ax25_frame *frame)
{
	struct kn_ax25_control_info info;

	if (frame->control == KN_AX25_CONTROL_UI)
		return "UI";

	kn_ax25_control_decode(frame->control, &info);
	if (info.class == KN_AX25_CONTROL_CLASS_I)
		return "I";
	if (info.class == KN_AX25_CONTROL_CLASS_S)
		return kn_ax25_s_subtype_name(info.s_subtype);
	if (info.class == KN_AX25_CONTROL_CLASS_U)
		return kn_ax25_u_subtype_name(info.u_subtype);

	return "unknown";
}
