/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_daemon_tx.c */

#include <sys/types.h>

#include "kilonode/config.h"
#include "kilonode/ax25.h"
#include "kilonode/stats.h"
#include "kilonode/tx_dry_run.h"
#include "kilonode/tx_frame.h"
#include "kilonode/tx_dispatch.h"
#include "kilonode/tx_policy.h"
#include "kilonode/tx_queue.h"
#include "kilonode/transport_memory.h"

static int test_disabled_policy_rejects(void);
static int test_dispatch_config(void);
static int test_dispatch_memory_control_path(void);
static int test_queue_from_config(void);
static int test_real_dispatch_lab_config(void);

int
main(void)
{
	if (test_queue_from_config() != 0)
		return 1;
	if (test_disabled_policy_rejects() != 0)
		return 1;
	if (test_dispatch_config() != 0)
		return 1;
	if (test_dispatch_memory_control_path() != 0)
		return 1;
	if (test_real_dispatch_lab_config() != 0)
		return 1;

	return 0;
}

static int
test_disabled_policy_rejects(void)
{
	struct kn_config config;
	struct kn_tx_queue queue;
	struct kn_tx_frame frame;

	kn_config_init(&config);
	if (kn_tx_queue_init(&queue, &config.transmit.policy) !=
	    KN_TX_QUEUE_OK)
		return 1;
	kn_tx_frame_clear(&frame);
	frame.id = kn_tx_queue_reserve_id(&queue);
	frame.payload_len = 1;

	return kn_tx_queue_enqueue(&queue, &frame) == KN_TX_QUEUE_ERR_POLICY ?
	    0 : 1;
}

static int
test_dispatch_config(void)
{
	const char text[] =
	    "node {\n"
	    "\tcallsign M6VPN-1\n"
	    "}\n"
	    "transmit {\n"
	    "\tenabled true\n"
	    "\tdry-run true\n"
	    "\tallow-ui true\n"
	    "\tallow-control-enqueue true\n"
	    "\tdispatch-enabled true\n"
	    "\tdispatch-test-only true\n"
	    "\tdispatch-max-per-cycle 3\n"
	    "}\n"
	    "port mem0 {\n"
	    "\ttype memory-test\n"
	    "\tenabled true\n"
	    "}\n";
	struct kn_config config;
	struct kn_tx_queue queue;

	if (kn_config_parse_text(text, &config) != KN_CONFIG_OK)
		return 1;
	if (kn_tx_queue_init(&queue, &config.transmit.policy) !=
	    KN_TX_QUEUE_OK)
		return 1;
	if (config.ports[0].type != KN_CONFIG_PORT_MEMORY_TEST)
		return 1;

	return queue.policy.dispatch_enabled != 0 &&
	    queue.policy.dispatch_max_per_cycle == 3 ? 0 : 1;
}

static int
test_dispatch_memory_control_path(void)
{
	const uint8_t payload[] = "hello";
	struct kn_tx_policy policy;
	struct kn_tx_queue queue;
	struct kn_tx_dispatcher dispatcher;
	struct kn_transport_memory memory;
	struct kn_port_stats port;
	struct kn_tx_dispatch_result result;
	const struct kn_tx_frame *frame;
	uint64_t id;

	kn_tx_policy_defaults(&policy);
	policy.enabled = 1;
	policy.dry_run = 1;
	policy.allow_ui = 1;
	policy.allow_control_enqueue = 1;
	policy.dispatch_enabled = 1;
	policy.dispatch_test_only = 1;
	if (kn_tx_queue_init(&queue, &policy) != KN_TX_QUEUE_OK)
		return 1;
	kn_port_stats_init(&port, &(struct kn_config_port){
		.name = "mem0",
		.type = KN_CONFIG_PORT_MEMORY_TEST,
		.enabled = 1
	});
	port.open = 1;
	if (kn_tx_dry_run_enqueue_ui(&queue, &port, 1,
	    KN_TX_DRY_RUN_ORIGIN_CONTROL, 10, "mem0", "M6VPN-1", "CQ",
	    NULL, KN_AX25_PID_NO_LAYER_3, payload, 5, &id) !=
	    KN_TX_DRY_RUN_OK)
		return 1;
	kn_tx_dispatch_clear(&dispatcher);
	if (kn_transport_memory_init(&memory, 256) !=
	    KN_TRANSPORT_MEMORY_OK)
		return 1;
	(void)kn_transport_memory_open(&memory);
	(void)kn_tx_dispatch_add_memory_target(&dispatcher, "mem0",
	    &memory, 1, 1);
	if (kn_tx_dispatch_run(&queue, &dispatcher, NULL, &result) !=
	    KN_TX_DISPATCH_OK)
		return 1;
	frame = kn_tx_queue_get(&queue, id);

	return result.sent == 1 && frame != NULL &&
	    frame->status == KN_TX_FRAME_SENT && memory.len == frame->kiss_len ?
	    0 : 1;
}

static int
test_real_dispatch_lab_config(void)
{
	const char text[] =
	    "node {\n"
	    "\tcallsign M6VPN-1\n"
	    "}\n"
	    "transmit {\n"
	    "\tenabled true\n"
	    "\tdry-run true\n"
	    "\tallow-ui true\n"
	    "\tallow-control-enqueue true\n"
	    "\tdispatch-enabled true\n"
	    "\tdispatch-test-only false\n"
	    "\tdispatch-real-kiss true\n"
	    "\trequire-explicit-port-tx true\n"
	    "}\n"
	    "port kiss0 {\n"
	    "\ttype tcp-connect\n"
	    "\thost 127.0.0.1\n"
	    "\tport 8001\n"
	    "\tenabled true\n"
	    "\ttx-enabled false\n"
	    "}\n";
	struct kn_config config;

	if (kn_config_parse_text(text, &config) != KN_CONFIG_OK)
		return 1;
	if (config.transmit.policy.dispatch_real_kiss == 0 ||
	    config.transmit.policy.dispatch_test_only != 0)
		return 1;

	return config.ports[0].tx_enabled == 0 ? 0 : 1;
}

static int
test_queue_from_config(void)
{
	const char text[] =
	    "node {\n"
	    "\tcallsign M6VPN-1\n"
	    "}\n"
	    "transmit {\n"
	    "\tenabled true\n"
	    "\tdry-run true\n"
	    "\tmax-queued 2\n"
	    "\tmax-payload-bytes 32\n"
	    "\tpayload-preview-bytes 16\n"
	    "\tallow-ui true\n"
	    "}\n";
	struct kn_config config;
	struct kn_tx_queue queue;

	if (kn_config_parse_text(text, &config) != KN_CONFIG_OK)
		return 1;
	if (kn_tx_queue_init(&queue, &config.transmit.policy) !=
	    KN_TX_QUEUE_OK)
		return 1;
	if (queue.max_frames != 2 || queue.policy.max_payload_bytes != 32)
		return 1;

	return queue.policy.dry_run != 0 && queue.policy.allow_ui != 0 ?
	    0 : 1;
}
