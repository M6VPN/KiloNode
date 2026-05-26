/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_rx_event.c */

#include <sys/types.h>

#include <string.h>

#include "kilonode/ax25.h"
#include "kilonode/callsign.h"
#include "kilonode/control.h"
#include "kilonode/rx_event.h"

static void make_frame(struct kn_ax25_frame *, uint8_t, const uint8_t *,
	size_t);
static int test_binary_preview(void);
static int test_classify(void);
static int test_digipeater_path(void);
static int test_malformed_event(void);
static int test_text_preview(void);
static int test_truncated_preview(void);

int
main(void)
{
	if (test_classify() != 0)
		return 1;
	if (test_text_preview() != 0)
		return 1;
	if (test_binary_preview() != 0)
		return 1;
	if (test_truncated_preview() != 0)
		return 1;
	if (test_digipeater_path() != 0)
		return 1;
	if (test_malformed_event() != 0)
		return 1;

	return 0;
}

static void
make_frame(struct kn_ax25_frame *frame, uint8_t control,
	const uint8_t *payload, size_t payload_len)
{
	kn_ax25_frame_reset(frame);
	(void)kn_callsign_parse("CQ", &frame->destination.callsign);
	(void)kn_callsign_parse("M6VPN-1", &frame->source.callsign);
	frame->control = control;
	frame->pid = KN_AX25_PID_NO_LAYER_3;
	frame->has_pid = 1;
	frame->payload = payload;
	frame->payload_len = payload_len;
}

static int
test_binary_preview(void)
{
	const uint8_t payload[] = { 0x00, 0x41, 0xff };
	struct kn_ax25_frame frame;
	struct kn_rx_event event;

	make_frame(&frame, KN_AX25_CONTROL_UI, payload, sizeof(payload));
	if (kn_rx_event_from_ax25(&event, 1, 10, "kiss0", 0, 0, &frame,
	    80) != KN_RX_EVENT_OK)
		return 1;
	if (event.preview_binary == 0)
		return 1;

	return strcmp(event.preview, "0041ff") == 0 ? 0 : 1;
}

static int
test_classify(void)
{
	if (kn_rx_event_classify_control(KN_AX25_CONTROL_UI) != KN_RX_FRAME_UI)
		return 1;
	if (kn_rx_event_classify_control(0x00) != KN_RX_FRAME_I)
		return 1;
	if (kn_rx_event_classify_control(0x01) != KN_RX_FRAME_S)
		return 1;
	if (kn_rx_event_classify_control(0x2f) != KN_RX_FRAME_U)
		return 1;

	return 0;
}

static int
test_digipeater_path(void)
{
	const uint8_t payload[] = "ok";
	struct kn_ax25_frame frame;
	struct kn_rx_event event;
	char line[KN_CONTROL_LINE_MAX];

	make_frame(&frame, KN_AX25_CONTROL_UI, payload, 2);
	frame.digipeater_count = 1;
	(void)kn_callsign_parse("WIDE1-1",
	    &frame.digipeaters[0].callsign);
	frame.digipeaters[0].repeated = 1;
	if (kn_rx_event_from_ax25(&event, 1, 10, "kiss0", 0, 0, &frame,
	    80) != KN_RX_EVENT_OK)
		return 1;
	if (strcmp(event.path, "WIDE1-1*") != 0)
		return 1;
	if (kn_rx_event_format_full(&event, line, sizeof(line)) !=
	    KN_RX_EVENT_OK)
		return 1;

	return strstr(line, "via=WIDE1-1*") != NULL ? 0 : 1;
}

static int
test_malformed_event(void)
{
	struct kn_rx_event event;

	if (kn_rx_event_from_malformed(&event, 2, 20, "kiss0", 0, 0,
	    KN_AX25_ERR_SHORT_FRAME, 4) != KN_RX_EVENT_OK)
		return 1;
	if (event.kind != KN_RX_FRAME_MALFORMED || event.malformed == 0)
		return 1;

	return event.payload_len == 4 ? 0 : 1;
}

static int
test_text_preview(void)
{
	const uint8_t payload[] = "hello";
	struct kn_ax25_frame frame;
	struct kn_rx_event event;
	char line[KN_CONTROL_LINE_MAX];

	make_frame(&frame, KN_AX25_CONTROL_UI, payload, 5);
	if (kn_rx_event_from_ax25(&event, 1, 10, "kiss0", 0, 0, &frame,
	    80) != KN_RX_EVENT_OK)
		return 1;
	if (strcmp(event.preview, "\"hello\"") != 0)
		return 1;
	if (kn_rx_event_format_brief(&event, line, sizeof(line)) !=
	    KN_RX_EVENT_OK)
		return 1;

	return strstr(line, "from=M6VPN-1 to=CQ") != NULL ? 0 : 1;
}

static int
test_truncated_preview(void)
{
	const uint8_t payload[] = "abcdef";
	struct kn_ax25_frame frame;
	struct kn_rx_event event;

	make_frame(&frame, KN_AX25_CONTROL_UI, payload, 6);
	if (kn_rx_event_from_ax25(&event, 1, 10, "kiss0", 0, 0, &frame,
	    3) != KN_RX_EVENT_OK)
		return 1;

	return strcmp(event.preview, "\"abc\"") == 0 ? 0 : 1;
}
