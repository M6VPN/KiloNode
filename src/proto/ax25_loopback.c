/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/proto/ax25_loopback.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "kilonode/ax25_loopback.h"

static void collect_result(const struct kn_ax25_loopback *,
	struct kn_ax25_loopback_result *);
static struct kn_ax25_loopback_endpoint *endpoint(
	struct kn_ax25_loopback *, enum kn_ax25_loopback_script_endpoint);
static enum kn_ax25_loopback_error execute_command(
	struct kn_ax25_loopback *,
	const struct kn_ax25_loopback_script_command *);
static void mismatch(struct kn_ax25_loopback *,
	const struct kn_ax25_loopback_script_command *, const char *);
static enum kn_ax25_loopback_error run_until_idle(
	struct kn_ax25_loopback *, size_t);

static void
collect_result(const struct kn_ax25_loopback *loop,
	struct kn_ax25_loopback_result *result)
{
	enum kn_ax25_connection_state state;

	memset(result, 0, sizeof(*result));
	(void)snprintf(result->name, sizeof(result->name), "%s",
	    loop->name);
	if (kn_ax25_loopback_endpoint_state(&loop->a, &state) ==
	    KN_AX25_LOOPBACK_ENDPOINT_OK)
		result->endpoint_a_state = state;
	if (kn_ax25_loopback_endpoint_state(&loop->b, &state) ==
	    KN_AX25_LOOPBACK_ENDPOINT_OK)
		result->endpoint_b_state = state;
	result->endpoint_a_delivered = loop->a.delivered_payloads;
	result->endpoint_b_delivered = loop->b.delivered_payloads;
	result->prepared_frames_generated =
	    loop->a.outbound_prepared_frames + loop->b.outbound_prepared_frames;
	result->raw_ax25_frames_transferred =
	    loop->link.raw_ax25_frames_transferred;
	result->endpoint_a_rejected = loop->a.rejected_payloads;
	result->endpoint_b_rejected = loop->b.rejected_payloads;
	result->i_frames_sent = loop->a.i_frames_sent + loop->b.i_frames_sent;
	result->i_frames_received = loop->a.i_frames_received +
	    loop->b.i_frames_received;
	result->rr_frames_sent = loop->a.rr_frames_sent +
	    loop->b.rr_frames_sent;
	result->rr_frames_received = loop->a.rr_frames_received +
	    loop->b.rr_frames_received;
	result->real_tx_queue_writes =
	    loop->a.tx_queue_writes + loop->b.tx_queue_writes;
	result->dispatch_calls = loop->a.dispatch_calls + loop->b.dispatch_calls;
	result->fx25_frames_generated = loop->a.fx25_frames +
	    loop->b.fx25_frames + loop->link.fx25_frames_generated;
	result->mismatch_count = loop->mismatch_count;
	(void)snprintf(result->last_mismatch, sizeof(result->last_mismatch),
	    "%s", loop->last_mismatch);
	result->pass = result->mismatch_count == 0 &&
	    result->real_tx_queue_writes == 0 && result->dispatch_calls == 0 &&
	    result->fx25_frames_generated == 0 ? 1 : 0;
}

static struct kn_ax25_loopback_endpoint *
endpoint(struct kn_ax25_loopback *loop,
	enum kn_ax25_loopback_script_endpoint which)
{
	if (which == KN_AX25_LOOPBACK_SCRIPT_ENDPOINT_A)
		return &loop->a;
	if (which == KN_AX25_LOOPBACK_SCRIPT_ENDPOINT_B)
		return &loop->b;
	return NULL;
}

static enum kn_ax25_loopback_error
execute_command(struct kn_ax25_loopback *loop,
	const struct kn_ax25_loopback_script_command *command)
{
	struct kn_ax25_loopback_endpoint *ep;
	struct kn_ax25_loopback_endpoint *to;
	uint8_t frame[KN_AX25_LOOPBACK_LINK_FRAME_MAX];
	size_t frame_len;
	size_t moved;
	uint64_t actual;
	enum kn_ax25_connection_state state;

	switch (command->type) {
	case KN_AX25_LOOPBACK_SCRIPT_NOW:
		loop->now_ms = command->value;
		loop->a.now_ms = command->value;
		loop->b.now_ms = command->value;
		return KN_AX25_LOOPBACK_OK;
	case KN_AX25_LOOPBACK_SCRIPT_ADVANCE:
		loop->now_ms += command->value;
		loop->a.now_ms = loop->now_ms;
		loop->b.now_ms = loop->now_ms;
		return KN_AX25_LOOPBACK_OK;
	case KN_AX25_LOOPBACK_SCRIPT_PARAMS:
		return KN_AX25_LOOPBACK_OK;
	case KN_AX25_LOOPBACK_SCRIPT_EVENT:
		ep = endpoint(loop, command->endpoint);
		if (ep == NULL)
			return KN_AX25_LOOPBACK_ERR_INTERNAL;
		if (command->event ==
		    KN_AX25_LOOPBACK_SCRIPT_EVENT_LOCAL_CONNECT)
			return kn_ax25_loopback_endpoint_local_connect(ep) ==
			    KN_AX25_LOOPBACK_ENDPOINT_OK ?
			    KN_AX25_LOOPBACK_OK : KN_AX25_LOOPBACK_ERR_INTERNAL;
		if (command->event ==
		    KN_AX25_LOOPBACK_SCRIPT_EVENT_LOCAL_DISCONNECT)
			return kn_ax25_loopback_endpoint_local_disconnect(ep) ==
			    KN_AX25_LOOPBACK_ENDPOINT_OK ?
			    KN_AX25_LOOPBACK_OK : KN_AX25_LOOPBACK_ERR_INTERNAL;
		if (command->event == KN_AX25_LOOPBACK_SCRIPT_EVENT_SEND_I) {
			to = command->endpoint ==
			    KN_AX25_LOOPBACK_SCRIPT_ENDPOINT_A ? &loop->b :
			    &loop->a;
			if (kn_ax25_loopback_endpoint_send_i(ep,
			    command->payload, command->payload_len,
			    command->ns_override, command->use_ns_override,
			    frame, sizeof(frame),
			    &frame_len) != KN_AX25_LOOPBACK_ENDPOINT_OK)
				return KN_AX25_LOOPBACK_ERR_UNSUPPORTED;
			return kn_ax25_loopback_link_transfer_raw(&loop->link,
			    to, frame, frame_len) == KN_AX25_LOOPBACK_LINK_OK ?
			    KN_AX25_LOOPBACK_OK : KN_AX25_LOOPBACK_ERR_INTERNAL;
		}
		return KN_AX25_LOOPBACK_ERR_INTERNAL;
	case KN_AX25_LOOPBACK_SCRIPT_PROCESS_TIMERS:
		if (command->endpoint == KN_AX25_LOOPBACK_SCRIPT_ENDPOINT_A ||
		    command->endpoint == KN_AX25_LOOPBACK_SCRIPT_ENDPOINT_B) {
			ep = endpoint(loop, command->endpoint);
			return kn_ax25_loopback_endpoint_process_timers(ep,
			    KN_AX25_LOOPBACK_LINK_STEP_MAX, &moved) ==
			    KN_AX25_LOOPBACK_ENDPOINT_OK ?
			    KN_AX25_LOOPBACK_OK : KN_AX25_LOOPBACK_ERR_INTERNAL;
		}
		if (kn_ax25_loopback_endpoint_process_timers(&loop->a,
		    KN_AX25_LOOPBACK_LINK_STEP_MAX, &moved) !=
		    KN_AX25_LOOPBACK_ENDPOINT_OK ||
		    kn_ax25_loopback_endpoint_process_timers(&loop->b,
		    KN_AX25_LOOPBACK_LINK_STEP_MAX, &moved) !=
		    KN_AX25_LOOPBACK_ENDPOINT_OK)
			return KN_AX25_LOOPBACK_ERR_INTERNAL;
		return KN_AX25_LOOPBACK_OK;
	case KN_AX25_LOOPBACK_SCRIPT_TRANSFER:
		ep = endpoint(loop, command->endpoint);
		to = endpoint(loop, command->endpoint_to);
		if (ep == NULL || to == NULL)
			return KN_AX25_LOOPBACK_ERR_INTERNAL;
		return kn_ax25_loopback_link_transfer(&loop->link, ep, to,
		    &moved) == KN_AX25_LOOPBACK_LINK_OK ?
		    KN_AX25_LOOPBACK_OK : KN_AX25_LOOPBACK_ERR_INTERNAL;
	case KN_AX25_LOOPBACK_SCRIPT_RUN_UNTIL_IDLE:
		return run_until_idle(loop, (size_t)command->value);
	case KN_AX25_LOOPBACK_SCRIPT_EXPECT:
		actual = 0;
		if (command->expect == KN_AX25_LOOPBACK_SCRIPT_EXPECT_STATE) {
			ep = endpoint(loop, command->endpoint);
			if (ep == NULL ||
			    kn_ax25_loopback_endpoint_state(ep, &state) !=
			    KN_AX25_LOOPBACK_ENDPOINT_OK)
				return KN_AX25_LOOPBACK_ERR_INTERNAL;
			if (state != command->state) {
				mismatch(loop, command, "state");
				return KN_AX25_LOOPBACK_ERR_MISMATCH;
			}
			return KN_AX25_LOOPBACK_OK;
		}
		if (command->expect == KN_AX25_LOOPBACK_SCRIPT_EXPECT_DELIVERED) {
			ep = endpoint(loop, command->endpoint);
			actual = ep == NULL ? 0 : ep->delivered_payloads;
		} else if (command->expect ==
		    KN_AX25_LOOPBACK_SCRIPT_EXPECT_REJECTED) {
			ep = endpoint(loop, command->endpoint);
			actual = ep == NULL ? 0 : ep->rejected_payloads;
		} else if (command->expect ==
		    KN_AX25_LOOPBACK_SCRIPT_EXPECT_LAST_PAYLOAD_TEXT) {
			const struct kn_ax25_payload_delivery_record *delivery;

			ep = endpoint(loop, command->endpoint);
			delivery = ep == NULL ? NULL :
			    kn_ax25_payload_delivery_last_accepted(
			    &ep->deliveries);
			if (delivery == NULL || delivery->payload_is_text == 0 ||
			    delivery->preview_len != strlen(command->text) ||
			    memcmp(delivery->preview, command->text,
			    delivery->preview_len) != 0) {
				mismatch(loop, command, "last-payload-text");
				return KN_AX25_LOOPBACK_ERR_MISMATCH;
			}
			return KN_AX25_LOOPBACK_OK;
		} else if (command->expect ==
		    KN_AX25_LOOPBACK_SCRIPT_EXPECT_LAST_PAYLOAD_HEX) {
			const struct kn_ax25_payload_delivery_record *delivery;
			char hex[KN_AX25_PAYLOAD_DELIVERY_PREVIEW_MAX * 2 + 1];

			ep = endpoint(loop, command->endpoint);
			delivery = ep == NULL ? NULL :
			    kn_ax25_payload_delivery_last_accepted(
			    &ep->deliveries);
			if (delivery == NULL ||
			    kn_ax25_payload_delivery_preview_hex(delivery, hex,
			    sizeof(hex)) != KN_AX25_PAYLOAD_DELIVERY_OK ||
			    strcmp(hex, command->text) != 0) {
				mismatch(loop, command, "last-payload-hex");
				return KN_AX25_LOOPBACK_ERR_MISMATCH;
			}
			return KN_AX25_LOOPBACK_OK;
		} else if (command->expect ==
		    KN_AX25_LOOPBACK_SCRIPT_EXPECT_PREPARED_COUNT) {
			actual = loop->a.outbound_prepared_frames +
			    loop->b.outbound_prepared_frames;
		} else if (command->expect ==
		    KN_AX25_LOOPBACK_SCRIPT_EXPECT_TRANSFERRED) {
			actual = loop->link.raw_ax25_frames_transferred;
		} else if (command->expect ==
		    KN_AX25_LOOPBACK_SCRIPT_EXPECT_TX_WRITES) {
			actual = loop->a.tx_queue_writes + loop->b.tx_queue_writes;
		} else if (command->expect ==
		    KN_AX25_LOOPBACK_SCRIPT_EXPECT_DISPATCH_CALLS) {
			actual = loop->a.dispatch_calls + loop->b.dispatch_calls;
		} else if (command->expect ==
		    KN_AX25_LOOPBACK_SCRIPT_EXPECT_FX25_FRAMES) {
			actual = loop->a.fx25_frames + loop->b.fx25_frames +
			    loop->link.fx25_frames_generated;
		}
		if (actual != command->value) {
			mismatch(loop, command, "counter");
			return KN_AX25_LOOPBACK_ERR_MISMATCH;
		}
		return KN_AX25_LOOPBACK_OK;
	case KN_AX25_LOOPBACK_SCRIPT_NONE:
		break;
	}
	return KN_AX25_LOOPBACK_ERR_INTERNAL;
}

static void
mismatch(struct kn_ax25_loopback *loop,
	const struct kn_ax25_loopback_script_command *command,
	const char *text)
{
	loop->mismatch_count++;
	(void)snprintf(loop->last_mismatch, sizeof(loop->last_mismatch),
	    "line=%llu mismatch=%s", (unsigned long long)command->line,
	    text == NULL ? "unknown" : text);
}

static enum kn_ax25_loopback_error
run_until_idle(struct kn_ax25_loopback *loop, size_t max_steps)
{
	size_t step;
	size_t moved_ab;
	size_t moved_ba;

	for (step = 0; step < max_steps; step++) {
		if (kn_ax25_loopback_link_transfer(&loop->link, &loop->a,
		    &loop->b, &moved_ab) != KN_AX25_LOOPBACK_LINK_OK)
			return KN_AX25_LOOPBACK_ERR_INTERNAL;
		if (kn_ax25_loopback_link_transfer(&loop->link, &loop->b,
		    &loop->a, &moved_ba) != KN_AX25_LOOPBACK_LINK_OK)
			return KN_AX25_LOOPBACK_ERR_INTERNAL;
		if (moved_ab == 0 && moved_ba == 0)
			return KN_AX25_LOOPBACK_OK;
	}
	return KN_AX25_LOOPBACK_OK;
}

void
kn_ax25_loopback_init(struct kn_ax25_loopback *loop)
{
	if (loop == NULL)
		return;

	memset(loop, 0, sizeof(*loop));
	kn_ax25_loopback_endpoint_reset(&loop->a);
	kn_ax25_loopback_endpoint_reset(&loop->b);
	kn_ax25_loopback_link_init(&loop->link);
}

void
kn_ax25_loopback_reset(struct kn_ax25_loopback *loop)
{
	kn_ax25_loopback_init(loop);
}

enum kn_ax25_loopback_error
kn_ax25_loopback_run_file(const char *path,
	struct kn_ax25_loopback_result *result,
	struct kn_ax25_loopback_error_info *error)
{
	struct kn_ax25_loopback_script script;
	enum kn_ax25_loopback_error rc;

	rc = kn_ax25_loopback_script_parse_file(path, &script, error);
	if (rc != KN_AX25_LOOPBACK_OK)
		return rc;
	return kn_ax25_loopback_run_script(&script, result, error);
}

enum kn_ax25_loopback_error
kn_ax25_loopback_run_script(const struct kn_ax25_loopback_script *script,
	struct kn_ax25_loopback_result *result,
	struct kn_ax25_loopback_error_info *error)
{
	struct kn_ax25_loopback loop;
	size_t i;
	enum kn_ax25_loopback_error rc;

	if (script == NULL || result == NULL)
		return KN_AX25_LOOPBACK_ERR_INVALID_ARGUMENT;
	kn_ax25_loopback_init(&loop);
	(void)snprintf(loop.name, sizeof(loop.name), "%s", script->name);
	if (kn_ax25_loopback_endpoint_init(&loop.a, "A", script->endpoint_a,
	    script->endpoint_b, script->port, &script->params) !=
	    KN_AX25_LOOPBACK_ENDPOINT_OK ||
	    kn_ax25_loopback_endpoint_init(&loop.b, "B", script->endpoint_b,
	    script->endpoint_a, script->port, &script->params) !=
	    KN_AX25_LOOPBACK_ENDPOINT_OK) {
		if (error != NULL) {
			memset(error, 0, sizeof(*error));
			error->error = KN_AX25_LOOPBACK_ERR_INTERNAL;
			(void)snprintf(error->message, sizeof(error->message),
			    "endpoint-init");
		}
		return KN_AX25_LOOPBACK_ERR_INTERNAL;
	}
	for (i = 0; i < script->command_count; i++) {
		rc = execute_command(&loop, &script->commands[i]);
		if (rc != KN_AX25_LOOPBACK_OK &&
		    rc != KN_AX25_LOOPBACK_ERR_MISMATCH) {
			collect_result(&loop, result);
			if (error != NULL) {
				memset(error, 0, sizeof(*error));
				error->error = rc;
				error->line = script->commands[i].line;
				(void)snprintf(error->message,
				    sizeof(error->message), "execute");
			}
			return rc;
		}
	}
	collect_result(&loop, result);
	if (result->pass == 0) {
		if (error != NULL) {
			memset(error, 0, sizeof(*error));
			error->error = KN_AX25_LOOPBACK_ERR_MISMATCH;
			(void)snprintf(error->message, sizeof(error->message),
			    "%s", result->last_mismatch);
		}
		return KN_AX25_LOOPBACK_ERR_MISMATCH;
	}
	if (error != NULL) {
		memset(error, 0, sizeof(*error));
		error->error = KN_AX25_LOOPBACK_OK;
		(void)snprintf(error->message, sizeof(error->message), "ok");
	}
	return KN_AX25_LOOPBACK_OK;
}
