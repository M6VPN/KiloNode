/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_tx_dispatch.c */

#include <sys/types.h>

#include <string.h>
#include <unistd.h>

#include "kilonode/ax25.h"
#include "kilonode/tx_dispatch.h"
#include "kilonode/tx_dry_run.h"

static void make_frame(struct kn_tx_queue *, const char *);
static void make_policy(struct kn_tx_policy *);
static void make_real_policy(struct kn_tx_policy *);
static int test_by_port(void);
static int test_disabled_rejects(void);
static int test_empty_queue(void);
static int test_memory_dispatch(void);
static int test_no_safe_target(void);
static int test_per_cycle_limit(void);
static int test_real_dispatch(void);
static int test_real_gate_blocks(void);
static int test_write_failure(void);

int
main(void)
{
	if (test_disabled_rejects() != 0)
		return 1;
	if (test_empty_queue() != 0)
		return 1;
	if (test_memory_dispatch() != 0)
		return 1;
	if (test_write_failure() != 0)
		return 1;
	if (test_by_port() != 0)
		return 1;
	if (test_per_cycle_limit() != 0)
		return 1;
	if (test_no_safe_target() != 0)
		return 1;
	if (test_real_gate_blocks() != 0)
		return 1;
	if (test_real_dispatch() != 0)
		return 1;

	return 0;
}

static void
make_frame(struct kn_tx_queue *queue, const char *port)
{
	const uint8_t payload[] = "hello";
	struct kn_tx_frame frame;

	(void)kn_tx_frame_build_ui(&frame, kn_tx_queue_reserve_id(queue), 10,
	    port, 0, "M6VPN-1", "CQ", NULL, 0, KN_AX25_PID_NO_LAYER_3,
	    payload, 5, &queue->policy);
	frame.status = KN_TX_FRAME_DRY_RUN;
	(void)kn_tx_queue_enqueue(queue, &frame);
}

static void
make_policy(struct kn_tx_policy *policy)
{
	kn_tx_policy_defaults(policy);
	policy->enabled = 1;
	policy->dry_run = 1;
	policy->allow_ui = 1;
	policy->allow_control_enqueue = 1;
	policy->dispatch_enabled = 1;
	policy->dispatch_test_only = 1;
	policy->dispatch_max_per_cycle = 4;
}

static void
make_real_policy(struct kn_tx_policy *policy)
{
	kn_tx_policy_defaults(policy);
	policy->enabled = 1;
	policy->dry_run = 0;
	policy->allow_ui = 1;
	policy->allow_control_enqueue = 1;
	policy->dispatch_enabled = 1;
	policy->dispatch_test_only = 0;
	policy->dispatch_real_kiss = 1;
	policy->require_explicit_port_tx = 1;
	policy->dispatch_max_per_cycle = 4;
}

static int
test_by_port(void)
{
	struct kn_tx_policy policy;
	struct kn_tx_queue queue;
	struct kn_tx_dispatcher dispatcher;
	struct kn_transport_memory mem0;
	struct kn_transport_memory mem1;
	struct kn_tx_dispatch_result result;

	make_policy(&policy);
	if (kn_tx_queue_init(&queue, &policy) != KN_TX_QUEUE_OK)
		return 1;
	make_frame(&queue, "mem0");
	make_frame(&queue, "mem1");
	kn_tx_dispatch_clear(&dispatcher);
	if (kn_transport_memory_init(&mem0, 128) != KN_TRANSPORT_MEMORY_OK ||
	    kn_transport_memory_init(&mem1, 128) != KN_TRANSPORT_MEMORY_OK)
		return 1;
	(void)kn_transport_memory_open(&mem0);
	(void)kn_transport_memory_open(&mem1);
	(void)kn_tx_dispatch_add_memory_target(&dispatcher, "mem0", &mem0,
	    1, 1);
	(void)kn_tx_dispatch_add_memory_target(&dispatcher, "mem1", &mem1,
	    1, 1);
	if (kn_tx_dispatch_run(&queue, &dispatcher, "mem1", &result) !=
	    KN_TX_DISPATCH_OK)
		return 1;

	return result.sent == 1 && mem0.len == 0 && mem1.len > 0 ? 0 : 1;
}

static int
test_disabled_rejects(void)
{
	struct kn_tx_policy policy;
	struct kn_tx_queue queue;
	struct kn_tx_dispatcher dispatcher;
	struct kn_tx_dispatch_result result;

	kn_tx_policy_defaults(&policy);
	if (kn_tx_queue_init(&queue, &policy) != KN_TX_QUEUE_OK)
		return 1;
	kn_tx_dispatch_clear(&dispatcher);

	return kn_tx_dispatch_run(&queue, &dispatcher, NULL, &result) ==
	    KN_TX_DISPATCH_ERR_DISABLED ? 0 : 1;
}

static int
test_empty_queue(void)
{
	struct kn_tx_policy policy;
	struct kn_tx_queue queue;
	struct kn_tx_dispatcher dispatcher;
	struct kn_transport_memory memory;
	struct kn_tx_dispatch_result result;

	make_policy(&policy);
	if (kn_tx_queue_init(&queue, &policy) != KN_TX_QUEUE_OK)
		return 1;
	kn_tx_dispatch_clear(&dispatcher);
	if (kn_transport_memory_init(&memory, 128) != KN_TRANSPORT_MEMORY_OK)
		return 1;
	(void)kn_transport_memory_open(&memory);
	(void)kn_tx_dispatch_add_memory_target(&dispatcher, "mem0", &memory,
	    1, 1);
	if (kn_tx_dispatch_run(&queue, &dispatcher, NULL, &result) !=
	    KN_TX_DISPATCH_OK)
		return 1;

	return result.sent == 0 && result.failed == 0 ? 0 : 1;
}

static int
test_memory_dispatch(void)
{
	struct kn_tx_policy policy;
	struct kn_tx_queue queue;
	struct kn_tx_dispatcher dispatcher;
	struct kn_transport_memory memory;
	struct kn_tx_dispatch_result result;
	const struct kn_tx_frame *frame;

	make_policy(&policy);
	if (kn_tx_queue_init(&queue, &policy) != KN_TX_QUEUE_OK)
		return 1;
	make_frame(&queue, "mem0");
	frame = kn_tx_queue_get(&queue, 1);
	kn_tx_dispatch_clear(&dispatcher);
	if (kn_transport_memory_init(&memory, 128) != KN_TRANSPORT_MEMORY_OK)
		return 1;
	(void)kn_transport_memory_open(&memory);
	(void)kn_tx_dispatch_add_memory_target(&dispatcher, "mem0", &memory,
	    1, 1);
	if (kn_tx_dispatch_run(&queue, &dispatcher, NULL, &result) !=
	    KN_TX_DISPATCH_OK)
		return 1;
	if (frame == NULL || memory.len != frame->kiss_len ||
	    memcmp(memory.data, frame->kiss, frame->kiss_len) != 0)
		return 1;
	frame = kn_tx_queue_get(&queue, 1);

	return result.sent == 1 && frame != NULL &&
	    frame->status == KN_TX_FRAME_SENT &&
	    dispatcher.stats.bytes_written == memory.len ? 0 : 1;
}

static int
test_no_safe_target(void)
{
	struct kn_tx_policy policy;
	struct kn_tx_queue queue;
	struct kn_tx_dispatcher dispatcher;
	struct kn_tx_dispatch_result result;

	make_policy(&policy);
	if (kn_tx_queue_init(&queue, &policy) != KN_TX_QUEUE_OK)
		return 1;
	make_frame(&queue, "mem0");
	kn_tx_dispatch_clear(&dispatcher);

	return kn_tx_dispatch_run(&queue, &dispatcher, NULL, &result) ==
	    KN_TX_DISPATCH_ERR_NO_SAFE_TARGET ? 0 : 1;
}

static int
test_per_cycle_limit(void)
{
	struct kn_tx_policy policy;
	struct kn_tx_queue queue;
	struct kn_tx_dispatcher dispatcher;
	struct kn_transport_memory memory;
	struct kn_tx_dispatch_result result;

	make_policy(&policy);
	policy.dispatch_max_per_cycle = 1;
	if (kn_tx_queue_init(&queue, &policy) != KN_TX_QUEUE_OK)
		return 1;
	make_frame(&queue, "mem0");
	make_frame(&queue, "mem0");
	kn_tx_dispatch_clear(&dispatcher);
	if (kn_transport_memory_init(&memory, 256) != KN_TRANSPORT_MEMORY_OK)
		return 1;
	(void)kn_transport_memory_open(&memory);
	(void)kn_tx_dispatch_add_memory_target(&dispatcher, "mem0", &memory,
	    1, 1);
	if (kn_tx_dispatch_run(&queue, &dispatcher, NULL, &result) !=
	    KN_TX_DISPATCH_OK)
		return 1;

	return result.sent == 1 && result.remaining == 1 ? 0 : 1;
}

static int
test_real_dispatch(void)
{
	struct kn_tx_policy policy;
	struct kn_tx_queue queue;
	struct kn_tx_dispatcher dispatcher;
	struct kn_tx_dispatch_result result;
	struct kn_transport transport;
	const struct kn_tx_frame *frame;
	uint8_t written[512];
	int fds[2];
	ssize_t nread;

	make_real_policy(&policy);
	if (kn_tx_queue_init(&queue, &policy) != KN_TX_QUEUE_OK)
		return 1;
	make_frame(&queue, "kiss0");
	frame = kn_tx_queue_get(&queue, 1);
	if (frame == NULL)
		return 1;
	if (pipe(fds) != 0)
		return 1;
	kn_transport_reset(&transport);
	transport.kind = KN_TRANSPORT_KIND_TCP_CLIENT;
	transport.write_fd = fds[1];
	transport.open = 1;
	kn_tx_dispatch_clear(&dispatcher);
	(void)kn_tx_dispatch_add_transport_target(&dispatcher, "kiss0",
	    &transport, KN_CONFIG_PORT_TCP_CONNECT,
	    KN_TRANSPORT_KIND_TCP_CLIENT, 1, 1, 1);
	if (kn_tx_dispatch_run(&queue, &dispatcher, NULL, &result) !=
	    KN_TX_DISPATCH_OK) {
		(void)close(fds[0]);
		kn_transport_close(&transport);
		return 1;
	}
	nread = read(fds[0], written, sizeof(written));
	(void)close(fds[0]);
	kn_transport_close(&transport);
	if (nread < 0 || (size_t)nread != frame->kiss_len)
		return 1;
	if (memcmp(written, frame->kiss, frame->kiss_len) != 0)
		return 1;
	frame = kn_tx_queue_get(&queue, 1);

	return result.sent == 1 && frame != NULL &&
	    frame->status == KN_TX_FRAME_SENT ? 0 : 1;
}

static int
test_real_gate_blocks(void)
{
	struct kn_tx_policy policy;
	struct kn_tx_queue queue;
	struct kn_tx_dispatcher dispatcher;
	struct kn_tx_dispatch_result result;
	struct kn_transport transport;

	make_real_policy(&policy);
	if (kn_tx_queue_init(&queue, &policy) != KN_TX_QUEUE_OK)
		return 1;
	make_frame(&queue, "kiss0");
	kn_transport_reset(&transport);
	transport.kind = KN_TRANSPORT_KIND_TCP_CLIENT;
	transport.write_fd = -1;
	transport.open = 1;
	kn_tx_dispatch_clear(&dispatcher);
	(void)kn_tx_dispatch_add_transport_target(&dispatcher, "kiss0",
	    &transport, KN_CONFIG_PORT_TCP_CONNECT,
	    KN_TRANSPORT_KIND_TCP_CLIENT, 1, 1, 0);

	return kn_tx_dispatch_run(&queue, &dispatcher, NULL, &result) ==
	    KN_TX_DISPATCH_ERR_GATE_BLOCKED &&
	    result.gate_error == KN_TX_TRANSPORT_GATE_ERR_PORT_TX_DISABLED ?
	    0 : 1;
}

static int
test_write_failure(void)
{
	struct kn_tx_policy policy;
	struct kn_tx_queue queue;
	struct kn_tx_dispatcher dispatcher;
	struct kn_transport_memory memory;
	struct kn_tx_dispatch_result result;
	const struct kn_tx_frame *frame;

	make_policy(&policy);
	if (kn_tx_queue_init(&queue, &policy) != KN_TX_QUEUE_OK)
		return 1;
	make_frame(&queue, "mem0");
	kn_tx_dispatch_clear(&dispatcher);
	if (kn_transport_memory_init(&memory, 128) != KN_TRANSPORT_MEMORY_OK)
		return 1;
	(void)kn_transport_memory_open(&memory);
	memory.force_failure = 1;
	(void)kn_tx_dispatch_add_memory_target(&dispatcher, "mem0", &memory,
	    1, 1);
	if (kn_tx_dispatch_run(&queue, &dispatcher, NULL, &result) !=
	    KN_TX_DISPATCH_OK)
		return 1;
	frame = kn_tx_queue_get(&queue, 1);

	return result.failed == 1 && frame != NULL &&
	    frame->status == KN_TX_FRAME_FAILED ? 0 : 1;
}
