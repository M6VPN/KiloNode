/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_ax25_live_diag.c */

#include <sys/types.h>

#include <string.h>

#include "kilonode/ax25_live_diag.h"
#include "kilonode/ax25_rx_feed.h"
#include "kilonode/control.h"

static void fill_sabm(struct kn_ax25_frame *);
static void runtime_enable(struct kn_ax25_runtime *);
static int test_counters_increment_accepted(void);
static int test_counters_increment_ui(void);
static int test_default_live_disabled(void);
static int test_format_live_status(void);
static int test_output_truncation(void);

int
main(void)
{
	if (test_default_live_disabled() != 0)
		return 1;
	if (test_counters_increment_accepted() != 0)
		return 1;
	if (test_counters_increment_ui() != 0)
		return 1;
	if (test_format_live_status() != 0)
		return 1;
	if (test_output_truncation() != 0)
		return 1;

	return 0;
}

static void
fill_sabm(struct kn_ax25_frame *frame)
{
	kn_ax25_frame_reset(frame);
	(void)kn_callsign_parse("M6VPN-1", &frame->destination.callsign);
	(void)kn_callsign_parse("N0CALL", &frame->source.callsign);
	frame->control = 0x2f;
}

static void
runtime_enable(struct kn_ax25_runtime *runtime)
{
	struct kn_ax25_live_options live;

	kn_ax25_runtime_init(runtime);
	(void)kn_ax25_runtime_set_enabled(runtime, 1, 0);
	memset(&live, 0, sizeof(live));
	live.live_rx_feed = 1;
	live.live_rx_create_connections = 1;
	live.live_rx_retain_frame_plans = 1;
	(void)kn_ax25_runtime_set_live_options(runtime, &live);
}

static int
test_counters_increment_accepted(void)
{
	struct kn_ax25_runtime runtime;
	struct kn_ax25_rx_feed_options options;
	struct kn_ax25_frame frame;
	struct kn_callsign local;

	runtime_enable(&runtime);
	kn_ax25_rx_feed_options_from_runtime(&runtime, &options);
	fill_sabm(&frame);
	(void)kn_callsign_parse("M6VPN-1", &local);
	if (kn_ax25_rx_feed_frame(&runtime, &options, "kiss0", &local,
	    &frame, 1, NULL) != KN_AX25_RX_FEED_OK)
		return 1;

	return runtime.live_counters.frames_accepted == 1 &&
	    runtime.live_counters.frame_plans_retained > 0 ? 0 : 1;
}

static int
test_counters_increment_ui(void)
{
	struct kn_ax25_runtime runtime;
	struct kn_ax25_rx_feed_options options;
	struct kn_ax25_frame frame;
	struct kn_callsign local;

	runtime_enable(&runtime);
	kn_ax25_rx_feed_options_from_runtime(&runtime, &options);
	fill_sabm(&frame);
	frame.control = KN_AX25_CONTROL_UI;
	(void)kn_callsign_parse("M6VPN-1", &local);
	if (kn_ax25_rx_feed_frame(&runtime, &options, "kiss0", &local,
	    &frame, 1, NULL) != KN_AX25_RX_FEED_ERR_IGNORED)
		return 1;

	return runtime.live_counters.ui_ignored == 1 &&
	    runtime.live_counters.frames_ignored == 1 ? 0 : 1;
}

static int
test_default_live_disabled(void)
{
	struct kn_ax25_runtime runtime;

	kn_ax25_runtime_init(&runtime);
	if (runtime.live.live_rx_feed != 0)
		return 1;
	if (runtime.live.live_rx_create_connections != 0)
		return 1;
	if (runtime.live.live_rx_retain_frame_plans == 0)
		return 1;

	return runtime.live_counters.frames_seen == 0 ? 0 : 1;
}

static int
test_format_live_status(void)
{
	struct kn_ax25_runtime runtime;
	char out[KN_CONTROL_QUEUE_MAX];

	runtime_enable(&runtime);
	if (kn_ax25_live_diag_format(&runtime, out, sizeof(out)) !=
	    KN_AX25_LIVE_DIAG_OK)
		return 1;
	if (strstr(out, "OK AX25 LIVE enabled=true feed=true") == NULL)
		return 1;

	return strstr(out, "tx_writes=0\nEND\n") != NULL ? 0 : 1;
}

static int
test_output_truncation(void)
{
	struct kn_ax25_runtime runtime;
	char out[8];

	kn_ax25_runtime_init(&runtime);
	return kn_ax25_live_diag_format(&runtime, out, sizeof(out)) ==
	    KN_AX25_LIVE_DIAG_ERR_TRUNCATED ? 0 : 1;
}
