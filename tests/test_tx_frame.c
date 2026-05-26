/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_tx_frame.c */

#include <sys/types.h>

#include <stdint.h>
#include <string.h>

#include "kilonode/ax25.h"
#include "kilonode/kiss.h"
#include "kilonode/tx_frame.h"
#include "kilonode/tx_policy.h"

static void policy_allow(struct kn_tx_policy *);
static int test_binary_payload(void);
static int test_invalid_callsign(void);
static int test_kiss_escaping(void);
static int test_raw_builder(void);
static int test_ui_digipeaters(void);
static int test_ui_frame(void);

int
main(void)
{
	if (test_ui_frame() != 0)
		return 1;
	if (test_ui_digipeaters() != 0)
		return 1;
	if (test_binary_payload() != 0)
		return 1;
	if (test_invalid_callsign() != 0)
		return 1;
	if (test_kiss_escaping() != 0)
		return 1;
	if (test_raw_builder() != 0)
		return 1;

	return 0;
}

static void
policy_allow(struct kn_tx_policy *policy)
{
	kn_tx_policy_defaults(policy);
	policy->enabled = 1;
	policy->allow_ui = 1;
}

static int
test_binary_payload(void)
{
	const uint8_t payload[] = { 0x00, 0x41, 0xff };
	struct kn_tx_policy policy;
	struct kn_tx_frame frame;

	policy_allow(&policy);
	if (kn_tx_frame_build_ui(&frame, 1, 10, "kiss0", 0, "M6VPN-1",
	    "CQ", NULL, 0, KN_AX25_PID_NO_LAYER_3, payload,
	    sizeof(payload), &policy) != KN_TX_FRAME_OK)
		return 1;
	if (frame.preview_binary == 0)
		return 1;

	return strcmp(frame.preview, "0041ff") == 0 ? 0 : 1;
}

static int
test_invalid_callsign(void)
{
	const uint8_t payload[] = "hello";
	struct kn_tx_policy policy;
	struct kn_tx_frame frame;

	policy_allow(&policy);
	if (kn_tx_frame_build_ui(&frame, 1, 10, "kiss0", 0, "bad*",
	    "CQ", NULL, 0, KN_AX25_PID_NO_LAYER_3, payload, 5,
	    &policy) != KN_TX_FRAME_ERR_INVALID_VALUE)
		return 1;

	return kn_tx_frame_build_ui(&frame, 1, 10, "kiss0", 0, "M6VPN-1",
	    "bad*", NULL, 0, KN_AX25_PID_NO_LAYER_3, payload, 5,
	    &policy) == KN_TX_FRAME_ERR_INVALID_VALUE ? 0 : 1;
}

static int
test_kiss_escaping(void)
{
	const uint8_t payload[] = { KN_KISS_FEND, KN_KISS_FESC };
	struct kn_tx_policy policy;
	struct kn_tx_frame frame;

	policy_allow(&policy);
	if (kn_tx_frame_build_ui(&frame, 1, 10, "kiss0", 0, "M6VPN-1",
	    "CQ", NULL, 0, KN_AX25_PID_NO_LAYER_3, payload,
	    sizeof(payload), &policy) != KN_TX_FRAME_OK)
		return 1;
	if (frame.kiss[0] != KN_KISS_FEND ||
	    frame.kiss[frame.kiss_len - 1] != KN_KISS_FEND)
		return 1;

	return memchr(frame.kiss, KN_KISS_FESC, frame.kiss_len) != NULL ?
	    0 : 1;
}

static int
test_raw_builder(void)
{
	const uint8_t raw[] = { 0x82, 0xa0, 0x40, 0x40, 0x40, 0x40 };
	struct kn_tx_frame frame;

	if (kn_tx_frame_build_raw_ax25(&frame, 7, 20, "kiss0", 0, 0,
	    raw, sizeof(raw), 80) != KN_TX_FRAME_OK)
		return 1;
	if (frame.kind != KN_TX_FRAME_RAW_AX25 || frame.ax25_len !=
	    sizeof(raw))
		return 1;

	return frame.kiss_len > frame.ax25_len ? 0 : 1;
}

static int
test_ui_digipeaters(void)
{
	const uint8_t payload[] = "hello";
	const char *digis[] = { "WIDE1-1", "WIDE2-2" };
	struct kn_tx_policy policy;
	struct kn_tx_frame frame;
	char line[256];

	policy_allow(&policy);
	if (kn_tx_frame_build_ui(&frame, 1, 10, "kiss0", 0, "M6VPN-1",
	    "CQ", digis, 2, KN_AX25_PID_NO_LAYER_3, payload, 5,
	    &policy) != KN_TX_FRAME_OK)
		return 1;
	if (strcmp(frame.path, "WIDE1-1,WIDE2-2") != 0)
		return 1;
	if (kn_tx_frame_format_full(&frame, line, sizeof(line)) !=
	    KN_TX_FRAME_OK)
		return 1;

	return strstr(line, "via=WIDE1-1,WIDE2-2") != NULL ? 0 : 1;
}

static int
test_ui_frame(void)
{
	const uint8_t payload[] = "hello";
	struct kn_tx_policy policy;
	struct kn_tx_frame frame;
	char line[256];

	policy_allow(&policy);
	if (kn_tx_frame_build_ui(&frame, 1, 10, "kiss0", 0, "M6VPN-1",
	    "CQ", NULL, 0, KN_AX25_PID_NO_LAYER_3, payload, 5,
	    &policy) != KN_TX_FRAME_OK)
		return 1;
	if (frame.status != KN_TX_FRAME_DRY_RUN ||
	    frame.control != KN_AX25_CONTROL_UI ||
	    frame.pid != KN_AX25_PID_NO_LAYER_3)
		return 1;
	if (frame.ax25_len <= 16 || frame.kiss_len <= frame.ax25_len)
		return 1;
	if (strcmp(frame.preview, "\"hello\"") != 0)
		return 1;
	if (kn_tx_frame_format_brief(&frame, line, sizeof(line)) !=
	    KN_TX_FRAME_OK)
		return 1;

	return strstr(line, "from=M6VPN-1 to=CQ") != NULL ? 0 : 1;
}
