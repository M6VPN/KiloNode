/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_tx_dry_run.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "kilonode/tx_dry_run.h"

static void make_port(struct kn_port_stats *, const char *, uint8_t, uint8_t);
static void policy_allow(struct kn_tx_policy *);
static int test_control_disabled(void);
static int test_default_policy_rejects(void);
static int test_dry_run_required(void);
static int test_invalid_callsigns(void);
static int test_invalid_port(void);
static int test_invalid_via(void);
static int test_too_many_via(void);
static int test_queue_full(void);
static int test_ui_disabled(void);
static int test_valid_enqueue(void);

int
main(void)
{
	if (test_default_policy_rejects() != 0)
		return 1;
	if (test_control_disabled() != 0)
		return 1;
	if (test_dry_run_required() != 0)
		return 1;
	if (test_ui_disabled() != 0)
		return 1;
	if (test_invalid_port() != 0)
		return 1;
	if (test_invalid_callsigns() != 0)
		return 1;
	if (test_invalid_via() != 0)
		return 1;
	if (test_too_many_via() != 0)
		return 1;
	if (test_queue_full() != 0)
		return 1;
	if (test_valid_enqueue() != 0)
		return 1;

	return 0;
}

static void
make_port(struct kn_port_stats *port, const char *name, uint8_t enabled,
	uint8_t open)
{
	memset(port, 0, sizeof(*port));
	(void)snprintf(port->name, sizeof(port->name), "%s", name);
	port->enabled = enabled;
	port->open = open;
}

static void
policy_allow(struct kn_tx_policy *policy)
{
	kn_tx_policy_defaults(policy);
	policy->enabled = 1;
	policy->dry_run = 1;
	policy->allow_ui = 1;
	policy->allow_control_enqueue = 1;
}

static int
test_control_disabled(void)
{
	const uint8_t payload[] = "hello";
	struct kn_tx_policy policy;
	struct kn_tx_queue queue;
	struct kn_port_stats port;
	uint64_t id;

	policy_allow(&policy);
	policy.allow_control_enqueue = 0;
	make_port(&port, "kiss0", 1, 1);
	if (kn_tx_queue_init(&queue, &policy) != KN_TX_QUEUE_OK)
		return 1;

	return kn_tx_dry_run_enqueue_ui(&queue, &port, 1,
	    KN_TX_DRY_RUN_ORIGIN_CONTROL, 10, "kiss0", "M6VPN-1", "CQ",
	    NULL, KN_AX25_PID_NO_LAYER_3, payload, 5, &id) ==
	    KN_TX_DRY_RUN_ERR_CONTROL_DISABLED ? 0 : 1;
}

static int
test_default_policy_rejects(void)
{
	const uint8_t payload[] = "hello";
	struct kn_tx_policy policy;
	struct kn_tx_queue queue;
	struct kn_port_stats port;
	uint64_t id;

	kn_tx_policy_defaults(&policy);
	make_port(&port, "kiss0", 1, 1);
	if (kn_tx_queue_init(&queue, &policy) != KN_TX_QUEUE_OK)
		return 1;

	return kn_tx_dry_run_enqueue_ui(&queue, &port, 1,
	    KN_TX_DRY_RUN_ORIGIN_CONTROL, 10, "kiss0", "M6VPN-1", "CQ",
	    NULL, KN_AX25_PID_NO_LAYER_3, payload, 5, &id) ==
	    KN_TX_DRY_RUN_ERR_DISABLED ? 0 : 1;
}

static int
test_dry_run_required(void)
{
	const uint8_t payload[] = "hello";
	struct kn_tx_policy policy;
	struct kn_tx_queue queue;
	struct kn_port_stats port;
	uint64_t id;

	policy_allow(&policy);
	policy.dry_run = 0;
	make_port(&port, "kiss0", 1, 1);
	if (kn_tx_queue_init(&queue, &policy) != KN_TX_QUEUE_OK)
		return 1;

	return kn_tx_dry_run_enqueue_ui(&queue, &port, 1,
	    KN_TX_DRY_RUN_ORIGIN_CONTROL, 10, "kiss0", "M6VPN-1", "CQ",
	    NULL, KN_AX25_PID_NO_LAYER_3, payload, 5, &id) ==
	    KN_TX_DRY_RUN_ERR_DRY_RUN_REQUIRED ? 0 : 1;
}

static int
test_invalid_callsigns(void)
{
	const uint8_t payload[] = "hello";
	struct kn_tx_policy policy;
	struct kn_tx_queue queue;
	struct kn_port_stats port;
	uint64_t id;

	policy_allow(&policy);
	make_port(&port, "kiss0", 1, 1);
	if (kn_tx_queue_init(&queue, &policy) != KN_TX_QUEUE_OK)
		return 1;
	if (kn_tx_dry_run_enqueue_ui(&queue, &port, 1,
	    KN_TX_DRY_RUN_ORIGIN_CONTROL, 10, "kiss0", "bad*", "CQ",
	    NULL, KN_AX25_PID_NO_LAYER_3, payload, 5, &id) !=
	    KN_TX_DRY_RUN_ERR_INVALID_CALLSIGN)
		return 1;

	return kn_tx_dry_run_enqueue_ui(&queue, &port, 1,
	    KN_TX_DRY_RUN_ORIGIN_CONTROL, 10, "kiss0", "M6VPN-1", "bad*",
	    NULL, KN_AX25_PID_NO_LAYER_3, payload, 5, &id) ==
	    KN_TX_DRY_RUN_ERR_INVALID_CALLSIGN ? 0 : 1;
}

static int
test_invalid_port(void)
{
	const uint8_t payload[] = "hello";
	struct kn_tx_policy policy;
	struct kn_tx_queue queue;
	struct kn_port_stats port;
	uint64_t id;

	policy_allow(&policy);
	make_port(&port, "kiss0", 1, 0);
	if (kn_tx_queue_init(&queue, &policy) != KN_TX_QUEUE_OK)
		return 1;

	return kn_tx_dry_run_enqueue_ui(&queue, &port, 1,
	    KN_TX_DRY_RUN_ORIGIN_CONTROL, 10, "kiss0", "M6VPN-1", "CQ",
	    NULL, KN_AX25_PID_NO_LAYER_3, payload, 5, &id) ==
	    KN_TX_DRY_RUN_ERR_INVALID_PORT ? 0 : 1;
}

static int
test_invalid_via(void)
{
	const uint8_t payload[] = "hello";
	struct kn_tx_policy policy;
	struct kn_tx_queue queue;
	struct kn_port_stats port;
	uint64_t id;

	policy_allow(&policy);
	make_port(&port, "kiss0", 1, 1);
	if (kn_tx_queue_init(&queue, &policy) != KN_TX_QUEUE_OK)
		return 1;

	return kn_tx_dry_run_enqueue_ui(&queue, &port, 1,
	    KN_TX_DRY_RUN_ORIGIN_CONTROL, 10, "kiss0", "M6VPN-1", "CQ",
	    "WIDE1-1,bad*", KN_AX25_PID_NO_LAYER_3, payload, 5, &id) ==
	    KN_TX_DRY_RUN_ERR_INVALID_VIA ? 0 : 1;
}

static int
test_too_many_via(void)
{
	const uint8_t payload[] = "hello";
	const char via[] = "WIDE1-1,WIDE1-2,WIDE1-3,WIDE1-4,WIDE1-5,"
	    "WIDE1-6,WIDE1-7,WIDE1-8,WIDE1-9";
	struct kn_tx_policy policy;
	struct kn_tx_queue queue;
	struct kn_port_stats port;
	uint64_t id;

	policy_allow(&policy);
	make_port(&port, "kiss0", 1, 1);
	if (kn_tx_queue_init(&queue, &policy) != KN_TX_QUEUE_OK)
		return 1;

	return kn_tx_dry_run_enqueue_ui(&queue, &port, 1,
	    KN_TX_DRY_RUN_ORIGIN_CONTROL, 10, "kiss0", "M6VPN-1", "CQ",
	    via, KN_AX25_PID_NO_LAYER_3, payload, 5, &id) ==
	    KN_TX_DRY_RUN_ERR_INVALID_VIA ? 0 : 1;
}

static int
test_queue_full(void)
{
	const uint8_t payload[] = "hello";
	struct kn_tx_policy policy;
	struct kn_tx_queue queue;
	struct kn_tx_frame frame;
	struct kn_port_stats port;
	uint64_t id;

	policy_allow(&policy);
	policy.max_queued = 1;
	make_port(&port, "kiss0", 1, 1);
	if (kn_tx_queue_init(&queue, &policy) != KN_TX_QUEUE_OK)
		return 1;
	kn_tx_frame_clear(&frame);
	frame.id = kn_tx_queue_reserve_id(&queue);
	frame.payload_len = 1;
	if (kn_tx_queue_enqueue(&queue, &frame) != KN_TX_QUEUE_OK)
		return 1;

	return kn_tx_dry_run_enqueue_ui(&queue, &port, 1,
	    KN_TX_DRY_RUN_ORIGIN_CONTROL, 10, "kiss0", "M6VPN-1", "CQ",
	    NULL, KN_AX25_PID_NO_LAYER_3, payload, 5, &id) ==
	    KN_TX_DRY_RUN_ERR_QUEUE_FULL ? 0 : 1;
}

static int
test_ui_disabled(void)
{
	const uint8_t payload[] = "hello";
	struct kn_tx_policy policy;
	struct kn_tx_queue queue;
	struct kn_port_stats port;
	uint64_t id;

	policy_allow(&policy);
	policy.allow_ui = 0;
	make_port(&port, "kiss0", 1, 1);
	if (kn_tx_queue_init(&queue, &policy) != KN_TX_QUEUE_OK)
		return 1;

	return kn_tx_dry_run_enqueue_ui(&queue, &port, 1,
	    KN_TX_DRY_RUN_ORIGIN_CONTROL, 10, "kiss0", "M6VPN-1", "CQ",
	    NULL, KN_AX25_PID_NO_LAYER_3, payload, 5, &id) ==
	    KN_TX_DRY_RUN_ERR_UI_DISABLED ? 0 : 1;
}

static int
test_valid_enqueue(void)
{
	const uint8_t payload[] = "hello";
	struct kn_tx_policy policy;
	struct kn_tx_queue queue;
	struct kn_port_stats port;
	const struct kn_tx_frame *frame;
	uint64_t id;

	policy_allow(&policy);
	make_port(&port, "kiss0", 1, 1);
	if (kn_tx_queue_init(&queue, &policy) != KN_TX_QUEUE_OK)
		return 1;
	if (kn_tx_dry_run_enqueue_ui(&queue, &port, 1,
	    KN_TX_DRY_RUN_ORIGIN_CONTROL, 10, "kiss0", "M6VPN-1", "CQ",
	    "WIDE1-1,WIDE2-1", KN_AX25_PID_NO_LAYER_3, payload, 5,
	    &id) != KN_TX_DRY_RUN_OK)
		return 1;
	frame = kn_tx_queue_get(&queue, id);
	if (frame == NULL || frame->status != KN_TX_FRAME_DRY_RUN)
		return 1;
	if (frame->ax25_len == 0 || frame->kiss_len == 0)
		return 1;

	return strcmp(frame->preview, "\"hello\"") == 0 ? 0 : 1;
}
