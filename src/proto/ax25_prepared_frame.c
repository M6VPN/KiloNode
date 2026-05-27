/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/proto/ax25_prepared_frame.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "kilonode/ax25_frame_builder.h"
#include "kilonode/ax25_prepared_frame.h"

static int port_name_valid(const char *);
static void set_reason(struct kn_ax25_prepared_frame *, const char *);

static int
port_name_valid(const char *port_name)
{
	size_t i;
	unsigned char ch;

	if (port_name == NULL || port_name[0] == '\0' ||
	    strlen(port_name) >= KN_AX25_PREPARED_FRAME_PORT_MAX)
		return 0;
	for (i = 0; port_name[i] != '\0'; i++) {
		ch = (unsigned char)port_name[i];
		if ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') ||
		    (ch >= '0' && ch <= '9') || ch == '_' || ch == '-')
			continue;
		return 0;
	}

	return 1;
}

static void
set_reason(struct kn_ax25_prepared_frame *frame, const char *reason)
{
	int needed;

	if (frame == NULL)
		return;
	if (reason == NULL)
		reason = "";
	needed = snprintf(frame->reason, sizeof(frame->reason), "%s",
	    reason);
	if (needed < 0 || (size_t)needed >= sizeof(frame->reason))
		frame->reason[sizeof(frame->reason) - 1] = '\0';
}

void
kn_ax25_prepared_frame_clear(struct kn_ax25_prepared_frame *frame)
{
	if (frame == NULL)
		return;

	memset(frame, 0, sizeof(*frame));
}

enum kn_ax25_prepared_frame_error
kn_ax25_prepared_frame_from_plan(struct kn_ax25_prepared_frame *frame,
	const struct kn_ax25_frame_plan *plan, uint32_t connection_id,
	const char *port_name, uint64_t now_ms, uint8_t build_raw)
{
	size_t i;
	int needed;
	enum kn_ax25_frame_builder_error builder_rc;

	if (frame == NULL || plan == NULL || port_name == NULL)
		return KN_AX25_PREPARED_FRAME_ERR_INVALID_ARGUMENT;
	if (build_raw > 1 || connection_id == 0 ||
	    port_name_valid(port_name) == 0)
		return KN_AX25_PREPARED_FRAME_ERR_INVALID_VALUE;

	kn_ax25_prepared_frame_clear(frame);
	frame->created_ms = now_ms;
	frame->connection_id = connection_id;
	needed = snprintf(frame->port_name, sizeof(frame->port_name), "%s",
	    port_name);
	if (needed < 0 || (size_t)needed >= sizeof(frame->port_name))
		return KN_AX25_PREPARED_FRAME_ERR_INVALID_VALUE;
	if (kn_ax25_frame_plan_validate(plan) != KN_AX25_FRAME_PLAN_OK) {
		frame->status = KN_AX25_PREPARED_FRAME_STATUS_BUILD_FAILED;
		set_reason(frame, "invalid-plan");
		return KN_AX25_PREPARED_FRAME_OK;
	}

	frame->local = plan->source;
	frame->remote = plan->destination;
	frame->digipeater_count = plan->digipeater_count;
	for (i = 0; i < plan->digipeater_count; i++)
		frame->digipeaters[i] = plan->digipeaters[i];
	frame->action_source = plan->action_source;
	frame->type = plan->type;
	frame->poll_final = plan->poll_final;
	frame->nr = plan->nr;
	frame->ns = plan->ns;
	frame->payload_len = plan->payload_len;
	frame->needs_fx25 = plan->needs_fx25;
	frame->needs_kiss = plan->needs_kiss;
	frame->status = KN_AX25_PREPARED_FRAME_STATUS_PREPARED;
	set_reason(frame, "prepared");

	if (build_raw == 0)
		return KN_AX25_PREPARED_FRAME_OK;

	builder_rc = kn_ax25_frame_builder_build_plan(plan, frame->raw,
	    sizeof(frame->raw), &frame->raw_len);
	if (builder_rc != KN_AX25_FRAME_BUILDER_OK) {
		frame->status = KN_AX25_PREPARED_FRAME_STATUS_BUILD_FAILED;
		frame->raw_len = 0;
		set_reason(frame, "build-failed");
	}

	return KN_AX25_PREPARED_FRAME_OK;
}

enum kn_ax25_prepared_frame_error
kn_ax25_prepared_frame_hex_preview(const struct kn_ax25_prepared_frame *frame,
	char *buf, size_t bufsiz)
{
	size_t i;
	size_t limit;
	size_t offset;
	int needed;

	if (frame == NULL || buf == NULL || bufsiz == 0)
		return KN_AX25_PREPARED_FRAME_ERR_INVALID_ARGUMENT;

	buf[0] = '\0';
	limit = frame->raw_len;
	if (limit > KN_AX25_PREPARED_FRAME_HEX_PREVIEW)
		limit = KN_AX25_PREPARED_FRAME_HEX_PREVIEW;
	offset = 0;
	for (i = 0; i < limit; i++) {
		needed = snprintf(buf + offset, bufsiz - offset, "%02x",
		    frame->raw[i]);
		if (needed < 0 || (size_t)needed >= bufsiz - offset)
			return KN_AX25_PREPARED_FRAME_ERR_BUFFER;
		offset += (size_t)needed;
	}

	return KN_AX25_PREPARED_FRAME_OK;
}

const char *
kn_ax25_prepared_frame_status_name(enum kn_ax25_prepared_frame_status status)
{
	switch (status) {
	case KN_AX25_PREPARED_FRAME_STATUS_PREPARED:
		return "prepared";
	case KN_AX25_PREPARED_FRAME_STATUS_BUILD_FAILED:
		return "build-failed";
	case KN_AX25_PREPARED_FRAME_STATUS_SUPPRESSED:
		return "suppressed";
	case KN_AX25_PREPARED_FRAME_STATUS_TX_BRIDGE_BLOCKED:
		return "tx-bridge-blocked";
	}

	return "unknown";
}

enum kn_ax25_prepared_frame_error
kn_ax25_prepared_frame_format(const struct kn_ax25_prepared_frame *frame,
	char *buf, size_t bufsiz)
{
	char local[KN_CALLSIGN_MAX + 4];
	char remote[KN_CALLSIGN_MAX + 4];
	int needed;

	if (frame == NULL || buf == NULL || bufsiz == 0)
		return KN_AX25_PREPARED_FRAME_ERR_INVALID_ARGUMENT;
	if (kn_callsign_format(&frame->local, local, sizeof(local)) != 0 ||
	    kn_callsign_format(&frame->remote, remote, sizeof(remote)) != 0)
		return KN_AX25_PREPARED_FRAME_ERR_INVALID_VALUE;

	needed = snprintf(buf, bufsiz,
	    "id=%llu port=%s local=%s remote=%s action=%s kind=%s "
	    "status=%s ax25_len=%llu",
	    (unsigned long long)frame->id, frame->port_name, local, remote,
	    kn_ax25_action_intent_name(frame->action_source),
	    kn_ax25_frame_plan_type_name(frame->type),
	    kn_ax25_prepared_frame_status_name(frame->status),
	    (unsigned long long)frame->raw_len);
	if (needed < 0 || (size_t)needed >= bufsiz)
		return KN_AX25_PREPARED_FRAME_ERR_BUFFER;

	return KN_AX25_PREPARED_FRAME_OK;
}

enum kn_ax25_prepared_frame_error
kn_ax25_prepared_frame_validate(const struct kn_ax25_prepared_frame *frame)
{
	char text[KN_CALLSIGN_MAX + 4];
	size_t i;

	if (frame == NULL)
		return KN_AX25_PREPARED_FRAME_ERR_INVALID_ARGUMENT;
	if (frame->connection_id == 0 || port_name_valid(frame->port_name) == 0)
		return KN_AX25_PREPARED_FRAME_ERR_INVALID_VALUE;
	if (kn_callsign_format(&frame->local, text, sizeof(text)) != 0 ||
	    kn_callsign_format(&frame->remote, text, sizeof(text)) != 0)
		return KN_AX25_PREPARED_FRAME_ERR_INVALID_VALUE;
	if (frame->digipeater_count > KN_AX25_MAX_DIGIS)
		return KN_AX25_PREPARED_FRAME_ERR_INVALID_VALUE;
	for (i = 0; i < frame->digipeater_count; i++) {
		if (kn_callsign_format(&frame->digipeaters[i], text,
		    sizeof(text)) != 0)
			return KN_AX25_PREPARED_FRAME_ERR_INVALID_VALUE;
	}
	if (frame->type >= KN_AX25_FRAME_PLAN_UNKNOWN ||
	    frame->poll_final > 1 || frame->nr > 7 || frame->ns > 7 ||
	    frame->needs_fx25 > 1 || frame->needs_kiss > 1 ||
	    frame->raw_len > sizeof(frame->raw))
		return KN_AX25_PREPARED_FRAME_ERR_INVALID_VALUE;

	return KN_AX25_PREPARED_FRAME_OK;
}
