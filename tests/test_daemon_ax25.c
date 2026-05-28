/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_daemon_ax25.c */

#include <sys/types.h>

#include <string.h>

#include "kilonode/ax25_rx_feed.h"
#include "kilonode/ax25_runtime.h"
#include "kilonode/tx_policy.h"
#include "kilonode/tx_queue.h"

static int test_daemon_ax25_runtime_defaults(void);
static int test_daemon_ax25_runtime_no_live_processing_default(void);
static int test_daemon_scheduler_default_disabled(void);
static int test_daemon_scheduler_poll_no_tx(void);
static int test_daemon_scheduler_smoke_default_disabled(void);
static int test_daemon_scheduler_smoke_no_tx(void);
static int test_synthetic_sabm_reaches_runtime_when_enabled(void);
static int test_synthetic_ui_does_not_create_connection(void);
static int test_tx_queue_unchanged_by_feed(void);

int
main(void)
{
	if (test_daemon_ax25_runtime_defaults() != 0)
		return 1;
	if (test_daemon_ax25_runtime_no_live_processing_default() != 0)
		return 1;
	if (test_daemon_scheduler_default_disabled() != 0)
		return 1;
	if (test_daemon_scheduler_poll_no_tx() != 0)
		return 1;
	if (test_daemon_scheduler_smoke_default_disabled() != 0)
		return 1;
	if (test_daemon_scheduler_smoke_no_tx() != 0)
		return 1;
	if (test_synthetic_sabm_reaches_runtime_when_enabled() != 0)
		return 1;
	if (test_synthetic_ui_does_not_create_connection() != 0)
		return 1;
	if (test_tx_queue_unchanged_by_feed() != 0)
		return 1;

	return 0;
}

static int
test_daemon_ax25_runtime_defaults(void)
{
	struct kn_ax25_runtime runtime;

	kn_ax25_runtime_init(&runtime);
	if (runtime.enabled != 0)
		return 1;
	if (runtime.connected_mode_enabled != 0)
		return 1;

	return runtime.diagnostics_enabled == 1 ? 0 : 1;
}

static int
test_daemon_ax25_runtime_no_live_processing_default(void)
{
	struct kn_ax25_runtime runtime;

	kn_ax25_runtime_init(&runtime);

	return kn_ax25_runtime_connection_count(&runtime) == 0 ? 0 : 1;
}

static int
test_daemon_scheduler_default_disabled(void)
{
	struct kn_ax25_runtime runtime;

	kn_ax25_runtime_init(&runtime);

	return runtime.live_scheduler.policy.enabled == 0 &&
	    runtime.live_scheduler.policy.process_expired == 0 ? 0 : 1;
}

static int
test_daemon_scheduler_poll_no_tx(void)
{
	struct kn_ax25_runtime runtime;
	struct kn_ax25_scheduler_policy policy;

	kn_ax25_runtime_init(&runtime);
	(void)kn_ax25_runtime_set_enabled(&runtime, 1, 0);
	kn_ax25_scheduler_policy_default(&policy);
	policy.enabled = 1;
	if (kn_ax25_runtime_set_scheduler_policy(&runtime, &policy) !=
	    KN_AX25_RUNTIME_OK)
		return 1;
	if (kn_ax25_live_scheduler_poll(&runtime, 100) !=
	    KN_AX25_LIVE_SCHEDULER_OK)
		return 1;

	return runtime.live_scheduler.tx_writes_attempted == 0 ? 0 : 1;
}

static int
test_daemon_scheduler_smoke_default_disabled(void)
{
	struct kn_ax25_runtime runtime;

	kn_ax25_runtime_init(&runtime);

	return runtime.smoke_options.enabled == 0 &&
	    runtime.smoke_options.create_test_connection == 0 ? 0 : 1;
}

static int
test_daemon_scheduler_smoke_no_tx(void)
{
	struct kn_ax25_runtime runtime;
	struct kn_ax25_scheduler_policy policy;
	struct kn_ax25_scheduler_smoke_options smoke;

	kn_ax25_runtime_init(&runtime);
	(void)kn_ax25_runtime_set_enabled(&runtime, 1, 0);
	kn_ax25_scheduler_policy_default(&policy);
	policy.enabled = 1;
	policy.process_expired = 1;
	if (kn_ax25_runtime_set_scheduler_policy(&runtime, &policy) !=
	    KN_AX25_RUNTIME_OK)
		return 1;
	kn_ax25_scheduler_smoke_options_default(&smoke);
	smoke.enabled = 1;
	smoke.create_test_connection = 1;
	if (kn_ax25_runtime_set_scheduler_smoke_options(&runtime,
	    &smoke) != KN_AX25_RUNTIME_OK)
		return 1;
	if (kn_ax25_scheduler_smoke_poll(&runtime, 0) !=
	    KN_AX25_SCHEDULER_SMOKE_OK)
		return 1;

	return runtime.smoke_counters.tx_writes_attempted == 0 &&
	    runtime.smoke_counters.dispatch_calls_attempted == 0 ? 0 : 1;
}

static int
test_synthetic_sabm_reaches_runtime_when_enabled(void)
{
	struct kn_ax25_runtime runtime;
	struct kn_ax25_live_options live;
	struct kn_ax25_rx_feed_options options;
	struct kn_ax25_frame frame;
	struct kn_callsign local;

	kn_ax25_runtime_init(&runtime);
	(void)kn_ax25_runtime_set_enabled(&runtime, 1, 0);
	memset(&live, 0, sizeof(live));
	live.live_rx_feed = 1;
	live.live_rx_create_connections = 1;
	live.live_rx_retain_frame_plans = 1;
	(void)kn_ax25_runtime_set_live_options(&runtime, &live);
	kn_ax25_rx_feed_options_from_runtime(&runtime, &options);
	kn_ax25_frame_reset(&frame);
	(void)kn_callsign_parse("M6VPN-1", &frame.destination.callsign);
	(void)kn_callsign_parse("N0CALL", &frame.source.callsign);
	frame.control = 0x2f;
	(void)kn_callsign_parse("M6VPN-1", &local);
	if (kn_ax25_rx_feed_frame(&runtime, &options, "kiss0", &local,
	    &frame, 1, NULL) != KN_AX25_RX_FEED_OK)
		return 1;

	return kn_ax25_runtime_connection_count(&runtime) == 1 ? 0 : 1;
}

static int
test_synthetic_ui_does_not_create_connection(void)
{
	struct kn_ax25_runtime runtime;
	struct kn_ax25_live_options live;
	struct kn_ax25_rx_feed_options options;
	struct kn_ax25_frame frame;
	struct kn_callsign local;

	kn_ax25_runtime_init(&runtime);
	(void)kn_ax25_runtime_set_enabled(&runtime, 1, 0);
	memset(&live, 0, sizeof(live));
	live.live_rx_feed = 1;
	live.live_rx_create_connections = 1;
	live.live_rx_retain_frame_plans = 1;
	(void)kn_ax25_runtime_set_live_options(&runtime, &live);
	kn_ax25_rx_feed_options_from_runtime(&runtime, &options);
	kn_ax25_frame_reset(&frame);
	(void)kn_callsign_parse("M6VPN-1", &frame.destination.callsign);
	(void)kn_callsign_parse("N0CALL", &frame.source.callsign);
	frame.control = KN_AX25_CONTROL_UI;
	(void)kn_callsign_parse("M6VPN-1", &local);
	if (kn_ax25_rx_feed_frame(&runtime, &options, "kiss0", &local,
	    &frame, 1, NULL) != KN_AX25_RX_FEED_ERR_IGNORED)
		return 1;

	return kn_ax25_runtime_connection_count(&runtime) == 0 ? 0 : 1;
}

static int
test_tx_queue_unchanged_by_feed(void)
{
	struct kn_tx_policy policy;
	struct kn_tx_queue queue;
	struct kn_ax25_runtime runtime;
	struct kn_ax25_live_options live;
	struct kn_ax25_rx_feed_options options;
	struct kn_ax25_frame frame;
	struct kn_callsign local;

	kn_tx_policy_defaults(&policy);
	(void)kn_tx_queue_init(&queue, &policy);
	kn_ax25_runtime_init(&runtime);
	(void)kn_ax25_runtime_set_enabled(&runtime, 1, 0);
	memset(&live, 0, sizeof(live));
	live.live_rx_feed = 1;
	live.live_rx_create_connections = 1;
	live.live_rx_retain_frame_plans = 1;
	(void)kn_ax25_runtime_set_live_options(&runtime, &live);
	kn_ax25_rx_feed_options_from_runtime(&runtime, &options);
	kn_ax25_frame_reset(&frame);
	(void)kn_callsign_parse("M6VPN-1", &frame.destination.callsign);
	(void)kn_callsign_parse("N0CALL", &frame.source.callsign);
	frame.control = 0x2f;
	(void)kn_callsign_parse("M6VPN-1", &local);
	(void)kn_ax25_rx_feed_frame(&runtime, &options, "kiss0", &local,
	    &frame, 1, NULL);

	return kn_tx_queue_count(&queue) == 0 &&
	    runtime.live_counters.tx_queue_writes_attempted == 0 ? 0 : 1;
}
