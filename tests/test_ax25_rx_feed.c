/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_ax25_rx_feed.c */

#include <sys/types.h>

#include <string.h>

#include "kilonode/ax25_control.h"
#include "kilonode/ax25_rx_feed.h"

static void fill_frame(struct kn_ax25_frame *, const char *, const char *,
	uint8_t);
static void runtime_enable(struct kn_ax25_runtime *, uint8_t);
static int test_disabled_feed_ignores_frame(void);
static int test_disc_updates_existing(void);
static int test_i_frame_no_connection_ignored(void);
static int test_i_frame_updates_existing(void);
static int test_malformed_counted(void);
static int test_rej_updates_existing(void);
static int test_rnr_updates_existing(void);
static int test_rr_updates_existing(void);
static int test_sabm_create_disabled_ignored(void);
static int test_sabm_creates_connection(void);
static int test_sabme_creates_connection(void);
static int test_tx_write_counter_zero(void);
static int test_ua_updates_existing(void);
static int test_ui_ignored(void);

int
main(void)
{
	if (test_disabled_feed_ignores_frame() != 0)
		return 1;
	if (test_ui_ignored() != 0)
		return 1;
	if (test_malformed_counted() != 0)
		return 1;
	if (test_sabm_creates_connection() != 0)
		return 1;
	if (test_sabm_create_disabled_ignored() != 0)
		return 1;
	if (test_sabme_creates_connection() != 0)
		return 1;
	if (test_disc_updates_existing() != 0)
		return 1;
	if (test_ua_updates_existing() != 0)
		return 1;
	if (test_rr_updates_existing() != 0)
		return 1;
	if (test_rnr_updates_existing() != 0)
		return 1;
	if (test_rej_updates_existing() != 0)
		return 1;
	if (test_i_frame_updates_existing() != 0)
		return 1;
	if (test_i_frame_no_connection_ignored() != 0)
		return 1;
	if (test_tx_write_counter_zero() != 0)
		return 1;

	return 0;
}

static void
fill_frame(struct kn_ax25_frame *frame, const char *destination,
	const char *source, uint8_t control)
{
	static const uint8_t payload[] = { 'T', 'E', 'S', 'T' };

	kn_ax25_frame_reset(frame);
	(void)kn_callsign_parse(destination, &frame->destination.callsign);
	(void)kn_callsign_parse(source, &frame->source.callsign);
	frame->control = control;
	frame->payload = payload;
	frame->payload_len = sizeof(payload);
}

static void
runtime_enable(struct kn_ax25_runtime *runtime, uint8_t create)
{
	struct kn_ax25_live_options live;

	kn_ax25_runtime_init(runtime);
	(void)kn_ax25_runtime_set_enabled(runtime, 1, 0);
	memset(&live, 0, sizeof(live));
	live.live_rx_feed = 1;
	live.live_rx_create_connections = create;
	live.live_rx_retain_frame_plans = 1;
	(void)kn_ax25_runtime_set_live_options(runtime, &live);
}

static int
test_disabled_feed_ignores_frame(void)
{
	struct kn_ax25_runtime runtime;
	struct kn_ax25_rx_feed_options options;
	struct kn_ax25_rx_feed_result result;
	struct kn_ax25_frame frame;
	struct kn_callsign local;

	kn_ax25_runtime_init(&runtime);
	kn_ax25_rx_feed_options_from_runtime(&runtime, &options);
	fill_frame(&frame, "M6VPN-1", "N0CALL", 0x2f);
	(void)kn_callsign_parse("M6VPN-1", &local);
	if (kn_ax25_rx_feed_frame(&runtime, &options, "kiss0", &local,
	    &frame, 1, &result) != KN_AX25_RX_FEED_ERR_DISABLED)
		return 1;

	return runtime.live_counters.frames_ignored == 1 &&
	    kn_ax25_runtime_connection_count(&runtime) == 0 ? 0 : 1;
}

static int
test_disc_updates_existing(void)
{
	struct kn_ax25_runtime runtime;
	struct kn_ax25_rx_feed_options options;
	struct kn_ax25_rx_feed_result result;
	struct kn_ax25_frame frame;
	struct kn_callsign local;

	runtime_enable(&runtime, 1);
	kn_ax25_rx_feed_options_from_runtime(&runtime, &options);
	(void)kn_callsign_parse("M6VPN-1", &local);
	fill_frame(&frame, "M6VPN-1", "N0CALL", 0x2f);
	(void)kn_ax25_rx_feed_frame(&runtime, &options, "kiss0", &local,
	    &frame, 1, &result);
	fill_frame(&frame, "M6VPN-1", "N0CALL", 0x43);

	return kn_ax25_rx_feed_frame(&runtime, &options, "kiss0", &local,
	    &frame, 2, &result) == KN_AX25_RX_FEED_OK &&
	    result.frame_plan_count > 0 ? 0 : 1;
}

static int
test_i_frame_no_connection_ignored(void)
{
	struct kn_ax25_runtime runtime;
	struct kn_ax25_rx_feed_options options;
	struct kn_ax25_rx_feed_result result;
	struct kn_ax25_frame frame;
	struct kn_callsign local;

	runtime_enable(&runtime, 1);
	kn_ax25_rx_feed_options_from_runtime(&runtime, &options);
	fill_frame(&frame, "M6VPN-1", "N0CALL", 0x00);
	(void)kn_callsign_parse("M6VPN-1", &local);

	return kn_ax25_rx_feed_frame(&runtime, &options, "kiss0", &local,
	    &frame, 2, &result) == KN_AX25_RX_FEED_ERR_NOT_RELEVANT ? 0 : 1;
}

static int
test_i_frame_updates_existing(void)
{
	struct kn_ax25_runtime runtime;
	struct kn_ax25_rx_feed_options options;
	struct kn_ax25_rx_feed_result result;
	struct kn_ax25_frame frame;
	struct kn_callsign local;

	runtime_enable(&runtime, 1);
	kn_ax25_rx_feed_options_from_runtime(&runtime, &options);
	(void)kn_callsign_parse("M6VPN-1", &local);
	fill_frame(&frame, "M6VPN-1", "N0CALL", 0x2f);
	(void)kn_ax25_rx_feed_frame(&runtime, &options, "kiss0", &local,
	    &frame, 1, &result);
	fill_frame(&frame, "M6VPN-1", "N0CALL", 0x00);
	if (kn_ax25_rx_feed_frame(&runtime, &options, "kiss0", &local,
	    &frame, 2, &result) != KN_AX25_RX_FEED_OK)
		return 1;

	return result.payload_len == 4 &&
	    runtime.live_counters.frames_accepted == 2 ? 0 : 1;
}

static int
test_malformed_counted(void)
{
	struct kn_ax25_runtime runtime;
	struct kn_ax25_rx_feed_options options;
	struct kn_ax25_rx_feed_result result;

	runtime_enable(&runtime, 0);
	kn_ax25_rx_feed_options_from_runtime(&runtime, &options);
	if (kn_ax25_rx_feed_malformed(&runtime, &options, 2, &result) !=
	    KN_AX25_RX_FEED_ERR_MALFORMED)
		return 1;

	return runtime.live_counters.frames_malformed == 1 ? 0 : 1;
}

static int
test_rej_updates_existing(void)
{
	struct kn_ax25_runtime runtime;
	struct kn_ax25_rx_feed_options options;
	struct kn_ax25_rx_feed_result result;
	struct kn_ax25_frame frame;
	struct kn_callsign local;
	uint8_t control;

	runtime_enable(&runtime, 1);
	kn_ax25_rx_feed_options_from_runtime(&runtime, &options);
	(void)kn_callsign_parse("M6VPN-1", &local);
	fill_frame(&frame, "M6VPN-1", "N0CALL", 0x2f);
	(void)kn_ax25_rx_feed_frame(&runtime, &options, "kiss0", &local,
	    &frame, 1, &result);
	(void)kn_ax25_control_encode_s(KN_AX25_S_SUBTYPE_REJ, 0, 0,
	    &control);
	fill_frame(&frame, "M6VPN-1", "N0CALL", control);

	return kn_ax25_rx_feed_frame(&runtime, &options, "kiss0", &local,
	    &frame, 2, &result) == KN_AX25_RX_FEED_OK ? 0 : 1;
}

static int
test_rnr_updates_existing(void)
{
	struct kn_ax25_runtime runtime;
	struct kn_ax25_rx_feed_options options;
	struct kn_ax25_rx_feed_result result;
	struct kn_ax25_frame frame;
	struct kn_callsign local;
	uint8_t control;

	runtime_enable(&runtime, 1);
	kn_ax25_rx_feed_options_from_runtime(&runtime, &options);
	(void)kn_callsign_parse("M6VPN-1", &local);
	fill_frame(&frame, "M6VPN-1", "N0CALL", 0x2f);
	(void)kn_ax25_rx_feed_frame(&runtime, &options, "kiss0", &local,
	    &frame, 1, &result);
	(void)kn_ax25_control_encode_s(KN_AX25_S_SUBTYPE_RNR, 0, 0,
	    &control);
	fill_frame(&frame, "M6VPN-1", "N0CALL", control);

	return kn_ax25_rx_feed_frame(&runtime, &options, "kiss0", &local,
	    &frame, 2, &result) == KN_AX25_RX_FEED_OK ? 0 : 1;
}

static int
test_rr_updates_existing(void)
{
	struct kn_ax25_runtime runtime;
	struct kn_ax25_rx_feed_options options;
	struct kn_ax25_rx_feed_result result;
	struct kn_ax25_frame frame;
	struct kn_callsign local;
	uint8_t control;

	runtime_enable(&runtime, 1);
	kn_ax25_rx_feed_options_from_runtime(&runtime, &options);
	(void)kn_callsign_parse("M6VPN-1", &local);
	fill_frame(&frame, "M6VPN-1", "N0CALL", 0x2f);
	(void)kn_ax25_rx_feed_frame(&runtime, &options, "kiss0", &local,
	    &frame, 1, &result);
	(void)kn_ax25_control_encode_s(KN_AX25_S_SUBTYPE_RR, 0, 0,
	    &control);
	fill_frame(&frame, "M6VPN-1", "N0CALL", control);

	return kn_ax25_rx_feed_frame(&runtime, &options, "kiss0", &local,
	    &frame, 2, &result) == KN_AX25_RX_FEED_OK ? 0 : 1;
}

static int
test_sabm_create_disabled_ignored(void)
{
	struct kn_ax25_runtime runtime;
	struct kn_ax25_rx_feed_options options;
	struct kn_ax25_rx_feed_result result;
	struct kn_ax25_frame frame;
	struct kn_callsign local;

	runtime_enable(&runtime, 0);
	kn_ax25_rx_feed_options_from_runtime(&runtime, &options);
	fill_frame(&frame, "M6VPN-1", "N0CALL", 0x2f);
	(void)kn_callsign_parse("M6VPN-1", &local);

	return kn_ax25_rx_feed_frame(&runtime, &options, "kiss0", &local,
	    &frame, 1, &result) == KN_AX25_RX_FEED_ERR_NOT_RELEVANT &&
	    kn_ax25_runtime_connection_count(&runtime) == 0 ? 0 : 1;
}

static int
test_sabm_creates_connection(void)
{
	struct kn_ax25_runtime runtime;
	struct kn_ax25_rx_feed_options options;
	struct kn_ax25_rx_feed_result result;
	struct kn_ax25_frame frame;
	struct kn_callsign local;

	runtime_enable(&runtime, 1);
	kn_ax25_rx_feed_options_from_runtime(&runtime, &options);
	fill_frame(&frame, "M6VPN-1", "N0CALL", 0x2f);
	(void)kn_callsign_parse("M6VPN-1", &local);
	if (kn_ax25_rx_feed_frame(&runtime, &options, "kiss0", &local,
	    &frame, 1, &result) != KN_AX25_RX_FEED_OK)
		return 1;

	return result.created != 0 && result.frame_plan_count > 0 &&
	    kn_ax25_runtime_connection_count(&runtime) == 1 ? 0 : 1;
}

static int
test_sabme_creates_connection(void)
{
	struct kn_ax25_runtime runtime;
	struct kn_ax25_rx_feed_options options;
	struct kn_ax25_rx_feed_result result;
	struct kn_ax25_frame frame;
	struct kn_callsign local;

	runtime_enable(&runtime, 1);
	kn_ax25_rx_feed_options_from_runtime(&runtime, &options);
	fill_frame(&frame, "M6VPN-1", "N0CALL", 0x6f);
	(void)kn_callsign_parse("M6VPN-1", &local);

	return kn_ax25_rx_feed_frame(&runtime, &options, "kiss0", &local,
	    &frame, 1, &result) == KN_AX25_RX_FEED_OK &&
	    result.event_kind == KN_AX25_CONNECTION_EVENT_RX_SABME ? 0 : 1;
}

static int
test_tx_write_counter_zero(void)
{
	struct kn_ax25_runtime runtime;
	struct kn_ax25_rx_feed_options options;
	struct kn_ax25_rx_feed_result result;
	struct kn_ax25_frame frame;
	struct kn_callsign local;

	runtime_enable(&runtime, 1);
	kn_ax25_rx_feed_options_from_runtime(&runtime, &options);
	fill_frame(&frame, "M6VPN-1", "N0CALL", 0x2f);
	(void)kn_callsign_parse("M6VPN-1", &local);
	(void)kn_ax25_rx_feed_frame(&runtime, &options, "kiss0", &local,
	    &frame, 1, &result);

	return runtime.live_counters.tx_queue_writes_attempted == 0 ? 0 : 1;
}

static int
test_ua_updates_existing(void)
{
	struct kn_ax25_runtime runtime;
	struct kn_ax25_rx_feed_options options;
	struct kn_ax25_rx_feed_result result;
	struct kn_ax25_frame frame;
	struct kn_callsign local;

	runtime_enable(&runtime, 1);
	kn_ax25_rx_feed_options_from_runtime(&runtime, &options);
	(void)kn_callsign_parse("M6VPN-1", &local);
	fill_frame(&frame, "M6VPN-1", "N0CALL", 0x2f);
	(void)kn_ax25_rx_feed_frame(&runtime, &options, "kiss0", &local,
	    &frame, 1, &result);
	fill_frame(&frame, "M6VPN-1", "N0CALL", 0x63);

	return kn_ax25_rx_feed_frame(&runtime, &options, "kiss0", &local,
	    &frame, 2, &result) == KN_AX25_RX_FEED_OK ? 0 : 1;
}

static int
test_ui_ignored(void)
{
	struct kn_ax25_runtime runtime;
	struct kn_ax25_rx_feed_options options;
	struct kn_ax25_rx_feed_result result;
	struct kn_ax25_frame frame;
	struct kn_callsign local;

	runtime_enable(&runtime, 1);
	kn_ax25_rx_feed_options_from_runtime(&runtime, &options);
	fill_frame(&frame, "M6VPN-1", "N0CALL", KN_AX25_CONTROL_UI);
	(void)kn_callsign_parse("M6VPN-1", &local);
	if (kn_ax25_rx_feed_frame(&runtime, &options, "kiss0", &local,
	    &frame, 1, &result) != KN_AX25_RX_FEED_ERR_IGNORED)
		return 1;

	return runtime.live_counters.ui_ignored == 1 &&
	    kn_ax25_runtime_connection_count(&runtime) == 0 ? 0 : 1;
}
