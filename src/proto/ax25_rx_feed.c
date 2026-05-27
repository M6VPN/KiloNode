/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/proto/ax25_rx_feed.c */

#include <sys/types.h>

#include <string.h>

#include "kilonode/ax25_control.h"
#include "kilonode/ax25_rx_feed.h"

static uint8_t event_is_setup(enum kn_ax25_connection_event);
static enum kn_ax25_rx_feed_error feed_ignore(struct kn_ax25_runtime *,
	struct kn_ax25_rx_feed_result *, enum kn_ax25_rx_feed_error);
static void live_count_ignored(struct kn_ax25_runtime *,
	enum kn_ax25_rx_feed_error);

static uint8_t
event_is_setup(enum kn_ax25_connection_event event)
{
	return event == KN_AX25_CONNECTION_EVENT_RX_SABM ||
	    event == KN_AX25_CONNECTION_EVENT_RX_SABME ? 1 : 0;
}

static enum kn_ax25_rx_feed_error
feed_ignore(struct kn_ax25_runtime *runtime,
	struct kn_ax25_rx_feed_result *result, enum kn_ax25_rx_feed_error error)
{
	live_count_ignored(runtime, error);
	if (result != NULL)
		result->status = error;
	return error;
}

static void
live_count_ignored(struct kn_ax25_runtime *runtime,
	enum kn_ax25_rx_feed_error error)
{
	if (runtime == NULL)
		return;

	runtime->live_counters.frames_ignored++;
	if (error == KN_AX25_RX_FEED_ERR_MALFORMED)
		runtime->live_counters.frames_malformed++;
	if (error == KN_AX25_RX_FEED_ERR_NOT_RELEVANT)
		runtime->live_counters.frames_not_relevant++;
}

void
kn_ax25_rx_feed_options_from_runtime(const struct kn_ax25_runtime *runtime,
	struct kn_ax25_rx_feed_options *options)
{
	if (options == NULL)
		return;

	memset(options, 0, sizeof(*options));
	if (runtime == NULL)
		return;
	options->enabled = runtime->enabled != 0 &&
	    runtime->live.live_rx_feed != 0 ? 1 : 0;
	options->create_connections =
	    runtime->live.live_rx_create_connections;
	options->retain_frame_plans =
	    runtime->live.live_rx_retain_frame_plans;
}

void
kn_ax25_rx_feed_result_clear(struct kn_ax25_rx_feed_result *result)
{
	if (result == NULL)
		return;

	memset(result, 0, sizeof(*result));
	result->status = KN_AX25_RX_FEED_ERR_IGNORED;
}

enum kn_ax25_rx_feed_error
kn_ax25_rx_feed_frame(struct kn_ax25_runtime *runtime,
	const struct kn_ax25_rx_feed_options *options, const char *port_name,
	const struct kn_callsign *local, const struct kn_ax25_frame *frame,
	uint64_t timestamp, struct kn_ax25_rx_feed_result *result)
{
	struct kn_ax25_connection_event_record event;
	struct kn_ax25_connection_table_result table_result;
	struct kn_ax25_connection_record *record;
	struct kn_ax25_control_info control;
	size_t index;
	enum kn_ax25_connection_event_error event_rc;
	enum kn_ax25_connection_table_error find_rc;
	enum kn_ax25_runtime_error runtime_rc;

	if (runtime == NULL || options == NULL || port_name == NULL ||
	    local == NULL || frame == NULL)
		return KN_AX25_RX_FEED_ERR_INVALID_ARGUMENT;

	kn_ax25_rx_feed_result_clear(result);
	runtime->live_counters.frames_seen++;
	if (options->enabled == 0)
		return feed_ignore(runtime, result,
		    KN_AX25_RX_FEED_ERR_DISABLED);
	kn_ax25_control_decode(frame->control, &control);
	if (control.class == KN_AX25_CONTROL_CLASS_UI) {
		runtime->live_counters.ui_ignored++;
		return feed_ignore(runtime, result, KN_AX25_RX_FEED_ERR_IGNORED);
	}

	event_rc = kn_ax25_connection_event_from_frame_pair(&event, timestamp,
	    port_name, local, frame);
	if (event_rc == KN_AX25_CONNECTION_EVENT_ERR_IGNORED)
		return feed_ignore(runtime, result, KN_AX25_RX_FEED_ERR_IGNORED);
	if (event_rc == KN_AX25_CONNECTION_EVENT_ERR_NOT_FOR_LOCAL)
		return feed_ignore(runtime, result,
		    KN_AX25_RX_FEED_ERR_NOT_RELEVANT);
	if (event_rc != KN_AX25_CONNECTION_EVENT_OK)
		return feed_ignore(runtime, result,
		    KN_AX25_RX_FEED_ERR_MALFORMED);

	if (event_is_setup(event.kind) == 0 ||
	    options->create_connections == 0) {
		find_rc = kn_ax25_connection_table_find(&runtime->table,
		    &event.key, &index);
		if (find_rc != KN_AX25_CONNECTION_TABLE_OK)
			return feed_ignore(runtime, result,
			    KN_AX25_RX_FEED_ERR_NOT_RELEVANT);
	} else {
		index = 0;
	}

	kn_ax25_connection_table_result_clear(&table_result);
	runtime_rc = kn_ax25_runtime_inject_event(runtime, &event,
	    &table_result);
	if (runtime_rc != KN_AX25_RUNTIME_OK) {
		runtime->live_counters.events_rejected++;
		return feed_ignore(runtime, result,
		    runtime_rc == KN_AX25_RUNTIME_ERR_DISABLED ?
		    KN_AX25_RX_FEED_ERR_DISABLED :
		    KN_AX25_RX_FEED_ERR_RUNTIME);
	}

	runtime->live_counters.frames_accepted++;
	runtime->live_counters.events_generated++;
	runtime->live_counters.frame_plans_generated += table_result.plans.count;
	if (options->retain_frame_plans != 0) {
		runtime->live_counters.frame_plans_retained +=
		    table_result.plans.count;
	} else {
		record = kn_ax25_connection_table_get(&runtime->table,
		    table_result.record_index);
		if (record != NULL)
			kn_ax25_frame_plan_list_clear(&record->last_plans);
		kn_ax25_frame_plan_list_clear(&table_result.plans);
	}

	if (result != NULL) {
		result->status = KN_AX25_RX_FEED_OK;
		result->event_kind = event.kind;
		result->event_generated = 1;
		result->accepted = 1;
		result->created = table_result.created;
		result->record_index = table_result.record_index;
		result->action_count = table_result.actions.count;
		result->frame_plan_count = table_result.plans.count;
		result->payload_len = event.payload_len;
	}

	(void)index;
	return KN_AX25_RX_FEED_OK;
}

enum kn_ax25_rx_feed_error
kn_ax25_rx_feed_malformed(struct kn_ax25_runtime *runtime,
	const struct kn_ax25_rx_feed_options *options, size_t frame_len,
	struct kn_ax25_rx_feed_result *result)
{
	if (runtime == NULL || options == NULL)
		return KN_AX25_RX_FEED_ERR_INVALID_ARGUMENT;

	kn_ax25_rx_feed_result_clear(result);
	runtime->live_counters.frames_seen++;
	if (options->enabled == 0)
		return feed_ignore(runtime, result,
		    KN_AX25_RX_FEED_ERR_DISABLED);
	(void)frame_len;
	return feed_ignore(runtime, result, KN_AX25_RX_FEED_ERR_MALFORMED);
}
