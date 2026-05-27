/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/proto/ax25_prepared_diag.c */

#include <sys/types.h>

#include <stdarg.h>
#include <stdio.h>

#include "kilonode/ax25_prepared_diag.h"

static enum kn_ax25_prepared_diag_error append_format(char *, size_t,
	size_t *, const char *, ...);
static enum kn_ax25_prepared_diag_error append_frame_line(char *, size_t,
	size_t *, const struct kn_ax25_prepared_frame *);

static enum kn_ax25_prepared_diag_error
append_format(char *buf, size_t bufsiz, size_t *offset, const char *fmt, ...)
{
	va_list args;
	int needed;

	if (buf == NULL || offset == NULL || fmt == NULL || bufsiz == 0)
		return KN_AX25_PREPARED_DIAG_ERR_INVALID_ARGUMENT;
	if (*offset >= bufsiz)
		return KN_AX25_PREPARED_DIAG_ERR_BUFFER;

	va_start(args, fmt);
	needed = vsnprintf(buf + *offset, bufsiz - *offset, fmt, args);
	va_end(args);
	if (needed < 0 || (size_t)needed >= bufsiz - *offset)
		return KN_AX25_PREPARED_DIAG_ERR_BUFFER;
	*offset += (size_t)needed;
	return KN_AX25_PREPARED_DIAG_OK;
}

static enum kn_ax25_prepared_diag_error
append_frame_line(char *buf, size_t bufsiz, size_t *offset,
	const struct kn_ax25_prepared_frame *frame)
{
	char local[KN_CALLSIGN_MAX + 4];
	char remote[KN_CALLSIGN_MAX + 4];

	if (buf == NULL || offset == NULL || frame == NULL)
		return KN_AX25_PREPARED_DIAG_ERR_INVALID_ARGUMENT;
	if (kn_callsign_format(&frame->local, local, sizeof(local)) != 0 ||
	    kn_callsign_format(&frame->remote, remote, sizeof(remote)) != 0)
		return KN_AX25_PREPARED_DIAG_ERR_BUFFER;

	return append_format(buf, bufsiz, offset,
	    "AX25 PREPARED id=%llu port=%s local=%s remote=%s "
	    "action=%s kind=%s status=%s ax25_len=%llu\n",
	    (unsigned long long)frame->id, frame->port_name, local, remote,
	    kn_ax25_action_intent_name(frame->action_source),
	    kn_ax25_frame_plan_type_name(frame->type),
	    kn_ax25_prepared_frame_status_name(frame->status),
	    (unsigned long long)frame->raw_len);
}

enum kn_ax25_prepared_diag_error
kn_ax25_prepared_diag_format_counters(const struct kn_ax25_runtime *runtime,
	char *buf, size_t bufsiz)
{
	struct kn_ax25_runtime fallback;
	const struct kn_ax25_runtime *rt;
	int needed;

	if (buf == NULL || bufsiz == 0)
		return KN_AX25_PREPARED_DIAG_ERR_INVALID_ARGUMENT;

	rt = runtime;
	if (rt == NULL) {
		kn_ax25_runtime_init(&fallback);
		rt = &fallback;
	}
	needed = snprintf(buf, bufsiz,
	    "OK AX25 PREPARED COUNTERS attempted=%llu stored=%llu "
	    "failed=%llu full=%llu bridge_blocked=%llu tx_writes=%llu\n"
	    "END\n",
	    (unsigned long long)rt->prepared_counters.frames_attempted,
	    (unsigned long long)rt->prepared_counters.frames_stored,
	    (unsigned long long)rt->prepared_counters.build_failures,
	    (unsigned long long)rt->prepared_counters.queue_full,
	    (unsigned long long)rt->prepared_counters.bridge_blocked,
	    (unsigned long long)
	    rt->prepared_counters.tx_queue_writes_attempted);
	if (needed < 0 || (size_t)needed >= bufsiz)
		return KN_AX25_PREPARED_DIAG_ERR_BUFFER;

	return KN_AX25_PREPARED_DIAG_OK;
}

enum kn_ax25_prepared_diag_error
kn_ax25_prepared_diag_format_frame(const struct kn_ax25_runtime *runtime,
	uint64_t id, char *buf, size_t bufsiz)
{
	struct kn_ax25_runtime fallback;
	const struct kn_ax25_runtime *rt;
	const struct kn_ax25_prepared_frame *frame;
	char local[KN_CALLSIGN_MAX + 4];
	char remote[KN_CALLSIGN_MAX + 4];
	char hex[(KN_AX25_PREPARED_FRAME_HEX_PREVIEW * 2U) + 1U];
	int needed;

	if (buf == NULL || bufsiz == 0 || id == 0)
		return KN_AX25_PREPARED_DIAG_ERR_INVALID_ARGUMENT;

	rt = runtime;
	if (rt == NULL) {
		kn_ax25_runtime_init(&fallback);
		rt = &fallback;
	}
	frame = kn_ax25_prepared_queue_get(&rt->prepared_queue, id);
	if (frame == NULL) {
		(void)snprintf(buf, bufsiz, "ERR prepared-frame-not-found\n");
		return KN_AX25_PREPARED_DIAG_ERR_NOT_FOUND;
	}
	if (kn_callsign_format(&frame->local, local, sizeof(local)) != 0 ||
	    kn_callsign_format(&frame->remote, remote, sizeof(remote)) != 0)
		return KN_AX25_PREPARED_DIAG_ERR_BUFFER;
	if (kn_ax25_prepared_frame_hex_preview(frame, hex, sizeof(hex)) !=
	    KN_AX25_PREPARED_FRAME_OK)
		return KN_AX25_PREPARED_DIAG_ERR_BUFFER;

	needed = snprintf(buf, bufsiz,
	    "OK AX25 PREPARED FRAME id=%llu\n"
	    "AX25 PREPARED id=%llu time=%llu port=%s local=%s remote=%s "
	    "action=%s kind=%s pf=%s nr=%u ns=%u status=%s "
	    "ax25_len=%llu payload_len=%llu needs_fx25=%s needs_kiss=%s "
	    "hex=%s\nEND\n",
	    (unsigned long long)frame->id,
	    (unsigned long long)frame->id,
	    (unsigned long long)frame->created_ms, frame->port_name,
	    local, remote, kn_ax25_action_intent_name(frame->action_source),
	    kn_ax25_frame_plan_type_name(frame->type),
	    frame->poll_final != 0 ? "true" : "false",
	    (unsigned int)frame->nr, (unsigned int)frame->ns,
	    kn_ax25_prepared_frame_status_name(frame->status),
	    (unsigned long long)frame->raw_len,
	    (unsigned long long)frame->payload_len,
	    frame->needs_fx25 != 0 ? "true" : "false",
	    frame->needs_kiss != 0 ? "true" : "false", hex);
	if (needed < 0 || (size_t)needed >= bufsiz)
		return KN_AX25_PREPARED_DIAG_ERR_BUFFER;

	return KN_AX25_PREPARED_DIAG_OK;
}

enum kn_ax25_prepared_diag_error
kn_ax25_prepared_diag_format_list(const struct kn_ax25_runtime *runtime,
	const char *port, uint32_t connection_id, char *buf, size_t bufsiz)
{
	struct kn_ax25_runtime fallback;
	const struct kn_ax25_runtime *rt;
	const struct kn_ax25_prepared_frame *frames[KN_AX25_PREPARED_QUEUE_MAX];
	size_t count;
	size_t i;
	size_t offset;
	enum kn_ax25_prepared_queue_error queue_rc;
	enum kn_ax25_prepared_diag_error diag_rc;

	if (buf == NULL || bufsiz == 0)
		return KN_AX25_PREPARED_DIAG_ERR_INVALID_ARGUMENT;

	rt = runtime;
	if (rt == NULL) {
		kn_ax25_runtime_init(&fallback);
		rt = &fallback;
	}
	count = 0;
	if (port != NULL)
		queue_rc = kn_ax25_prepared_queue_list_by_port(
		    &rt->prepared_queue, port, frames,
		    sizeof(frames) / sizeof(frames[0]), &count);
	else if (connection_id != 0)
		queue_rc = kn_ax25_prepared_queue_list_by_connection(
		    &rt->prepared_queue, connection_id, frames,
		    sizeof(frames) / sizeof(frames[0]), &count);
	else
		queue_rc = kn_ax25_prepared_queue_list(&rt->prepared_queue,
		    frames, sizeof(frames) / sizeof(frames[0]), &count);
	if (queue_rc != KN_AX25_PREPARED_QUEUE_OK)
		return KN_AX25_PREPARED_DIAG_ERR_BUFFER;

	offset = 0;
	diag_rc = append_format(buf, bufsiz, &offset,
	    "OK AX25 PREPARED count=%llu\n", (unsigned long long)count);
	if (diag_rc != KN_AX25_PREPARED_DIAG_OK)
		return diag_rc;
	for (i = 0; i < count; i++) {
		diag_rc = append_frame_line(buf, bufsiz, &offset, frames[i]);
		if (diag_rc != KN_AX25_PREPARED_DIAG_OK)
			return diag_rc;
	}

	return append_format(buf, bufsiz, &offset, "END\n");
}
