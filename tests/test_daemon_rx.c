/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_daemon_rx.c */

#include <sys/types.h>

#include "kilonode/ax25.h"
#include "kilonode/callsign.h"
#include "kilonode/heard.h"
#include "kilonode/rx_event.h"
#include "kilonode/rx_queue.h"
#include "kilonode/rx_session.h"
#include "kilonode/stats.h"

static void make_frame(struct kn_ax25_frame *);
static int test_decoded_frame_updates_heard_rx_and_session(void);
static int test_malformed_event_does_not_create_session(void);

int
main(void)
{
	if (test_decoded_frame_updates_heard_rx_and_session() != 0)
		return 1;
	if (test_malformed_event_does_not_create_session() != 0)
		return 1;

	return 0;
}

static void
make_frame(struct kn_ax25_frame *frame)
{
	static const uint8_t payload[] = "hello";

	kn_ax25_frame_reset(frame);
	(void)kn_callsign_parse("CQ", &frame->destination.callsign);
	(void)kn_callsign_parse("M6VPN-1", &frame->source.callsign);
	frame->control = KN_AX25_CONTROL_UI;
	frame->pid = KN_AX25_PID_NO_LAYER_3;
	frame->has_pid = 1;
	frame->payload = payload;
	frame->payload_len = 5;
}

static int
test_decoded_frame_updates_heard_rx_and_session(void)
{
	struct kn_ax25_frame frame;
	struct kn_heard_table heard;
	struct kn_rx_queue queue;
	struct kn_rx_session_table sessions;
	struct kn_rx_event event;

	make_frame(&frame);
	kn_heard_init(&heard, 4);
	kn_rx_queue_init(&queue, 4, 80, 1);
	kn_rx_session_init(&sessions, 4);

	if (kn_heard_update(&heard, "kiss0", &frame, 10) != KN_HEARD_OK)
		return 1;
	if (kn_rx_event_from_ax25(&event, kn_rx_queue_reserve_id(&queue), 10,
	    "kiss0", 0, 0, &frame, queue.preview_bytes) != KN_RX_EVENT_OK)
		return 1;
	if (kn_rx_queue_push(&queue, &event) != KN_RX_QUEUE_OK)
		return 1;
	if (kn_rx_session_update(&sessions, &event) != KN_RX_SESSION_OK)
		return 1;

	if (kn_heard_count(&heard) != 1 || kn_rx_queue_count(&queue) != 1)
		return 1;

	return kn_rx_session_count(&sessions) == 1 ? 0 : 1;
}

static int
test_malformed_event_does_not_create_session(void)
{
	struct kn_rx_queue queue;
	struct kn_rx_session_table sessions;
	struct kn_rx_event event;

	kn_rx_queue_init(&queue, 4, 80, 1);
	kn_rx_session_init(&sessions, 4);
	if (kn_rx_event_from_malformed(&event, kn_rx_queue_reserve_id(&queue),
	    10, "kiss0", 0, 0, KN_AX25_ERR_SHORT_FRAME, 1) !=
	    KN_RX_EVENT_OK)
		return 1;
	if (kn_rx_queue_push(&queue, &event) != KN_RX_QUEUE_OK)
		return 1;

	return kn_rx_session_count(&sessions) == 0 ? 0 : 1;
}
