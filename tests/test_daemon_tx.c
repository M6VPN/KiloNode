/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_daemon_tx.c */

#include <sys/types.h>

#include "kilonode/config.h"
#include "kilonode/tx_frame.h"
#include "kilonode/tx_policy.h"
#include "kilonode/tx_queue.h"

static int test_disabled_policy_rejects(void);
static int test_queue_from_config(void);

int
main(void)
{
	if (test_queue_from_config() != 0)
		return 1;
	if (test_disabled_policy_rejects() != 0)
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
