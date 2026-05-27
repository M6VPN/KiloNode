/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_ax25_connection_event.c */

#include <sys/types.h>

#include <stddef.h>

#include "kilonode/ax25_connection_event.h"
#include "kilonode/ax25_control.h"

static void fill_frame(struct kn_ax25_frame *, const char *, const char *,
    uint8_t);
static int test_local_connect_request(void);
static int test_malformed_frame_rejected(void);
static int test_not_for_local_rejected(void);
static int test_rx_disc_event(void);
static int test_rx_dm_event(void);
static int test_rx_i_event(void);
static int test_rx_rej_event(void);
static int test_rx_rnr_event(void);
static int test_rx_rr_event(void);
static int test_rx_sabm_event(void);
static int test_rx_sabme_event(void);
static int test_rx_ua_event(void);
static int test_timeout_event(void);
static int test_ui_ignored(void);

int
main(void)
{
	if (test_rx_sabm_event() != 0)
		return 1;
	if (test_rx_sabme_event() != 0)
		return 1;
	if (test_rx_ua_event() != 0)
		return 1;
	if (test_rx_disc_event() != 0)
		return 1;
	if (test_rx_dm_event() != 0)
		return 1;
	if (test_rx_i_event() != 0)
		return 1;
	if (test_rx_rr_event() != 0)
		return 1;
	if (test_rx_rnr_event() != 0)
		return 1;
	if (test_rx_rej_event() != 0)
		return 1;
	if (test_ui_ignored() != 0)
		return 1;
	if (test_malformed_frame_rejected() != 0)
		return 1;
	if (test_not_for_local_rejected() != 0)
		return 1;
	if (test_local_connect_request() != 0)
		return 1;
	if (test_timeout_event() != 0)
		return 1;

	return 0;
}

static void
fill_frame(struct kn_ax25_frame *frame, const char *destination,
    const char *source, uint8_t control)
{
	kn_ax25_frame_reset(frame);
	(void)kn_callsign_parse(destination, &frame->destination.callsign);
	(void)kn_callsign_parse(source, &frame->source.callsign);
	frame->control = control;
}

static int
test_local_connect_request(void)
{
	struct kn_ax25_connection_event_record event;
	struct kn_ax25_connection_key key;

	(void)kn_ax25_connection_key_from_callsigns(&key, "kiss0",
	    "M6VPN-1", "N0CALL-1", NULL, 0);

	if (kn_ax25_connection_event_local_connect(&event, 42, &key) !=
	    KN_AX25_CONNECTION_EVENT_OK)
		return 1;

	return event.kind == KN_AX25_CONNECTION_EVENT_LOCAL_CONNECT_REQUEST &&
	    event.timestamp == 42 ? 0 : 1;
}

static int
test_malformed_frame_rejected(void)
{
	struct kn_ax25_connection_event_record event;
	struct kn_ax25_frame frame;
	struct kn_callsign local;

	kn_ax25_frame_reset(&frame);
	(void)kn_callsign_parse("M6VPN-1", &local);

	return kn_ax25_connection_event_from_frame(&event, 1, "kiss0",
	    &local, &frame) == KN_AX25_CONNECTION_EVENT_ERR_NOT_FOR_LOCAL ?
	    0 : 1;
}

static int
test_not_for_local_rejected(void)
{
	struct kn_ax25_connection_event_record event;
	struct kn_ax25_frame frame;
	struct kn_callsign local;

	fill_frame(&frame, "OTHER-1", "N0CALL-1", 0x2f);
	(void)kn_callsign_parse("M6VPN-1", &local);

	return kn_ax25_connection_event_from_frame(&event, 1, "kiss0",
	    &local, &frame) == KN_AX25_CONNECTION_EVENT_ERR_NOT_FOR_LOCAL ?
	    0 : 1;
}

static int
test_rx_disc_event(void)
{
	struct kn_ax25_connection_event_record event;
	struct kn_ax25_frame frame;
	struct kn_callsign local;

	fill_frame(&frame, "M6VPN-1", "N0CALL-1", 0x43);
	(void)kn_callsign_parse("M6VPN-1", &local);

	if (kn_ax25_connection_event_from_frame(&event, 2, "kiss0",
	    &local, &frame) != KN_AX25_CONNECTION_EVENT_OK)
		return 1;

	return event.kind == KN_AX25_CONNECTION_EVENT_RX_DISC ? 0 : 1;
}

static int
test_rx_dm_event(void)
{
	struct kn_ax25_connection_event_record event;
	struct kn_ax25_frame frame;
	struct kn_callsign local;

	fill_frame(&frame, "M6VPN-1", "N0CALL-1", 0x0f);
	(void)kn_callsign_parse("M6VPN-1", &local);

	if (kn_ax25_connection_event_from_frame(&event, 2, "kiss0",
	    &local, &frame) != KN_AX25_CONNECTION_EVENT_OK)
		return 1;

	return event.kind == KN_AX25_CONNECTION_EVENT_RX_DM ? 0 : 1;
}

static int
test_rx_i_event(void)
{
	struct kn_ax25_connection_event_record event;
	struct kn_ax25_frame frame;
	struct kn_callsign local;

	fill_frame(&frame, "M6VPN-1", "N0CALL-1", 0x00);
	frame.payload_len = 4;
	(void)kn_callsign_parse("M6VPN-1", &local);

	if (kn_ax25_connection_event_from_frame(&event, 2, "kiss0",
	    &local, &frame) != KN_AX25_CONNECTION_EVENT_OK)
		return 1;

	return event.kind == KN_AX25_CONNECTION_EVENT_RX_I &&
	    event.payload_len == 4 && event.ns == 0 && event.nr == 0 ?
	    0 : 1;
}

static int
test_rx_rej_event(void)
{
	struct kn_ax25_connection_event_record event;
	struct kn_ax25_frame frame;
	struct kn_callsign local;
	uint8_t control;

	(void)kn_ax25_control_encode_s(KN_AX25_S_SUBTYPE_REJ, 3, 0,
	    &control);
	fill_frame(&frame, "M6VPN-1", "N0CALL-1", control);
	(void)kn_callsign_parse("M6VPN-1", &local);

	if (kn_ax25_connection_event_from_frame(&event, 2, "kiss0",
	    &local, &frame) != KN_AX25_CONNECTION_EVENT_OK)
		return 1;

	return event.kind == KN_AX25_CONNECTION_EVENT_RX_REJ &&
	    event.nr == 3 ? 0 : 1;
}

static int
test_rx_rnr_event(void)
{
	struct kn_ax25_connection_event_record event;
	struct kn_ax25_frame frame;
	struct kn_callsign local;
	uint8_t control;

	(void)kn_ax25_control_encode_s(KN_AX25_S_SUBTYPE_RNR, 2, 1,
	    &control);
	fill_frame(&frame, "M6VPN-1", "N0CALL-1", control);
	(void)kn_callsign_parse("M6VPN-1", &local);

	if (kn_ax25_connection_event_from_frame(&event, 2, "kiss0",
	    &local, &frame) != KN_AX25_CONNECTION_EVENT_OK)
		return 1;

	return event.kind == KN_AX25_CONNECTION_EVENT_RX_RNR &&
	    event.nr == 2 && event.poll_final == 1 ? 0 : 1;
}

static int
test_rx_rr_event(void)
{
	struct kn_ax25_connection_event_record event;
	struct kn_ax25_frame frame;
	struct kn_callsign local;
	uint8_t control;

	(void)kn_ax25_control_encode_s(KN_AX25_S_SUBTYPE_RR, 1, 0,
	    &control);
	fill_frame(&frame, "M6VPN-1", "N0CALL-1", control);
	(void)kn_callsign_parse("M6VPN-1", &local);

	if (kn_ax25_connection_event_from_frame(&event, 2, "kiss0",
	    &local, &frame) != KN_AX25_CONNECTION_EVENT_OK)
		return 1;

	return event.kind == KN_AX25_CONNECTION_EVENT_RX_RR &&
	    event.nr == 1 ? 0 : 1;
}

static int
test_rx_sabm_event(void)
{
	struct kn_ax25_connection_event_record event;
	struct kn_ax25_frame frame;
	struct kn_callsign local;

	fill_frame(&frame, "M6VPN-1", "N0CALL-1", 0x2f);
	(void)kn_callsign_parse("M6VPN-1", &local);

	if (kn_ax25_connection_event_from_frame(&event, 2, "kiss0",
	    &local, &frame) != KN_AX25_CONNECTION_EVENT_OK)
		return 1;

	return event.kind == KN_AX25_CONNECTION_EVENT_RX_SABM &&
	    event.key.remote.ssid == 1 ? 0 : 1;
}

static int
test_rx_sabme_event(void)
{
	struct kn_ax25_connection_event_record event;
	struct kn_ax25_frame frame;
	struct kn_callsign local;

	fill_frame(&frame, "M6VPN-1", "N0CALL-1", 0x6f);
	(void)kn_callsign_parse("M6VPN-1", &local);

	if (kn_ax25_connection_event_from_frame(&event, 2, "kiss0",
	    &local, &frame) != KN_AX25_CONNECTION_EVENT_OK)
		return 1;

	return event.kind == KN_AX25_CONNECTION_EVENT_RX_SABME ? 0 : 1;
}

static int
test_rx_ua_event(void)
{
	struct kn_ax25_connection_event_record event;
	struct kn_ax25_frame frame;
	struct kn_callsign local;

	fill_frame(&frame, "M6VPN-1", "N0CALL-1", 0x63);
	(void)kn_callsign_parse("M6VPN-1", &local);

	if (kn_ax25_connection_event_from_frame(&event, 2, "kiss0",
	    &local, &frame) != KN_AX25_CONNECTION_EVENT_OK)
		return 1;

	return event.kind == KN_AX25_CONNECTION_EVENT_RX_UA ? 0 : 1;
}

static int
test_timeout_event(void)
{
	struct kn_ax25_connection_event_record event;
	struct kn_ax25_connection_key key;

	(void)kn_ax25_connection_key_from_callsigns(&key, "kiss0",
	    "M6VPN-1", "N0CALL-1", NULL, 0);

	if (kn_ax25_connection_event_timeout_t1(&event, 77, &key) !=
	    KN_AX25_CONNECTION_EVENT_OK)
		return 1;

	return event.kind == KN_AX25_CONNECTION_EVENT_TIMEOUT_T1 &&
	    event.timestamp == 77 ? 0 : 1;
}

static int
test_ui_ignored(void)
{
	struct kn_ax25_connection_event_record event;
	struct kn_ax25_frame frame;
	struct kn_callsign local;

	fill_frame(&frame, "M6VPN-1", "N0CALL-1", KN_AX25_CONTROL_UI);
	(void)kn_callsign_parse("M6VPN-1", &local);

	return kn_ax25_connection_event_from_frame(&event, 2, "kiss0",
	    &local, &frame) == KN_AX25_CONNECTION_EVENT_ERR_IGNORED ?
	    0 : 1;
}
