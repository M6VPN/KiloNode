/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_tx_policy.c */

#include <sys/types.h>

#include "kilonode/config.h"
#include "kilonode/tx_policy.h"

static int test_allow_ui(void);
static int test_config_valid_block(void);
static int test_defaults(void);
static int test_invalid_config(void);

int
main(void)
{
	if (test_defaults() != 0)
		return 1;
	if (test_allow_ui() != 0)
		return 1;
	if (test_config_valid_block() != 0)
		return 1;
	if (test_invalid_config() != 0)
		return 1;

	return 0;
}

static int
test_allow_ui(void)
{
	struct kn_tx_policy policy;

	kn_tx_policy_defaults(&policy);
	if (kn_tx_policy_allow_ui(&policy, 1) !=
	    KN_TX_POLICY_ERR_DISABLED)
		return 1;
	policy.enabled = 1;
	if (kn_tx_policy_allow_ui(&policy, 1) !=
	    KN_TX_POLICY_ERR_UI_NOT_ALLOWED)
		return 1;
	policy.allow_ui = 1;
	if (kn_tx_policy_allow_ui(&policy, policy.max_payload_bytes + 1) !=
	    KN_TX_POLICY_ERR_TOO_LARGE)
		return 1;

	return kn_tx_policy_allow_ui(&policy, 1) == KN_TX_POLICY_OK ? 0 : 1;
}

static int
test_config_valid_block(void)
{
	const char text[] =
	    "node {\n"
	    "\tcallsign M6VPN-1\n"
	    "}\n"
	    "transmit {\n"
	    "\tenabled true\n"
	    "\tdry-run true\n"
	    "\tmax-queued 8\n"
	    "\tmax-payload-bytes 128\n"
	    "\tpayload-preview-bytes 32\n"
	    "\tallow-ui true\n"
	    "}\n";
	struct kn_config config;

	if (kn_config_parse_text(text, &config) != KN_CONFIG_OK)
		return 1;
	if (config.transmit.policy.enabled == 0 ||
	    config.transmit.policy.allow_ui == 0)
		return 1;
	if (config.transmit.policy.max_queued != 8 ||
	    config.transmit.policy.max_payload_bytes != 128)
		return 1;

	return config.transmit.policy.payload_preview_bytes == 32 ? 0 : 1;
}

static int
test_defaults(void)
{
	struct kn_tx_policy policy;

	kn_tx_policy_defaults(&policy);
	if (kn_tx_policy_validate(&policy) != KN_TX_POLICY_OK)
		return 1;
	if (policy.enabled != 0 || policy.dry_run == 0 ||
	    policy.allow_ui != 0)
		return 1;

	return policy.max_queued == KN_TX_POLICY_MAX_QUEUED_DEFAULT ? 0 : 1;
}

static int
test_invalid_config(void)
{
	const char duplicate[] =
	    "node {\n"
	    "\tcallsign M6VPN-1\n"
	    "}\n"
	    "transmit {\n"
	    "\tenabled false\n"
	    "}\n"
	    "transmit {\n"
	    "\tenabled false\n"
	    "}\n";
	const char unknown[] =
	    "node {\n"
	    "\tcallsign M6VPN-1\n"
	    "}\n"
	    "transmit {\n"
	    "\tbogus true\n"
	    "}\n";
	const char bad_limit[] =
	    "node {\n"
	    "\tcallsign M6VPN-1\n"
	    "}\n"
	    "transmit {\n"
	    "\tmax-queued 0\n"
	    "}\n";
	struct kn_config config;

	if (kn_config_parse_text(duplicate, &config) !=
	    KN_CONFIG_ERR_DUPLICATE_KEY)
		return 1;
	if (kn_config_parse_text(unknown, &config) !=
	    KN_CONFIG_ERR_UNKNOWN_KEY)
		return 1;

	return kn_config_parse_text(bad_limit, &config) ==
	    KN_CONFIG_ERR_INVALID_VALUE ? 0 : 1;
}
