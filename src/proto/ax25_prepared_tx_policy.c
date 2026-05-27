/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/proto/ax25_prepared_tx_policy.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "kilonode/ax25_prepared_tx_policy.h"

void
kn_ax25_prepared_tx_policy_default(
	struct kn_ax25_prepared_tx_policy *policy)
{
	if (policy == NULL)
		return;

	memset(policy, 0, sizeof(*policy));
	policy->test_only = 1;
	policy->require_tx_policy_enabled = 1;
	policy->require_port_tx_enabled = 1;
	policy->require_dispatch_disabled = 1;
	policy->require_no_auto_dispatch = 1;
	policy->max_bridge_per_call =
	    KN_AX25_PREPARED_TX_MAX_PER_CALL_DEFAULT;
}

enum kn_ax25_prepared_tx_policy_error
kn_ax25_prepared_tx_policy_format(
	const struct kn_ax25_prepared_tx_policy *policy, char *buf,
	size_t bufsiz)
{
	int needed;

	if (policy == NULL || buf == NULL || bufsiz == 0)
		return KN_AX25_PREPARED_TX_POLICY_ERR_INVALID_ARGUMENT;

	needed = snprintf(buf, bufsiz,
	    "enabled=%s test_only=%s control_frames=%s i_frames=%s "
	    "require_tx_policy=%s require_port_tx=%s "
	    "require_dispatch_disabled=%s require_no_auto_dispatch=%s "
	    "max_per_call=%llu fx25=%s",
	    policy->bridge_enabled != 0 ? "true" : "false",
	    policy->test_only != 0 ? "true" : "false",
	    policy->allow_control_frames != 0 ? "true" : "false",
	    policy->allow_i_frames != 0 ? "true" : "false",
	    policy->require_tx_policy_enabled != 0 ? "true" : "false",
	    policy->require_port_tx_enabled != 0 ? "true" : "false",
	    policy->require_dispatch_disabled != 0 ? "true" : "false",
	    policy->require_no_auto_dispatch != 0 ? "true" : "false",
	    (unsigned long long)policy->max_bridge_per_call,
	    policy->allow_fx25_wrapping != 0 ? "true" : "false");
	if (needed < 0 || (size_t)needed >= bufsiz)
		return KN_AX25_PREPARED_TX_POLICY_ERR_BUFFER;

	return KN_AX25_PREPARED_TX_POLICY_OK;
}

enum kn_ax25_prepared_tx_policy_error
kn_ax25_prepared_tx_policy_validate(
	const struct kn_ax25_prepared_tx_policy *policy)
{
	if (policy == NULL)
		return KN_AX25_PREPARED_TX_POLICY_ERR_INVALID_ARGUMENT;
	if (policy->bridge_enabled > 1 || policy->test_only > 1 ||
	    policy->allow_control_frames > 1 || policy->allow_i_frames > 1 ||
	    policy->require_tx_policy_enabled > 1 ||
	    policy->require_port_tx_enabled > 1 ||
	    policy->require_dispatch_disabled > 1 ||
	    policy->require_no_auto_dispatch > 1 ||
	    policy->allow_fx25_wrapping > 1)
		return KN_AX25_PREPARED_TX_POLICY_ERR_INVALID_VALUE;
	if (policy->max_bridge_per_call == 0 ||
	    policy->max_bridge_per_call >
	    KN_AX25_PREPARED_TX_MAX_PER_CALL_MAX)
		return KN_AX25_PREPARED_TX_POLICY_ERR_INVALID_VALUE;
	if (policy->bridge_enabled != 0 && policy->test_only == 0)
		return KN_AX25_PREPARED_TX_POLICY_ERR_INVALID_VALUE;
	if (policy->allow_i_frames != 0 ||
	    policy->allow_fx25_wrapping != 0)
		return KN_AX25_PREPARED_TX_POLICY_ERR_INVALID_VALUE;

	return KN_AX25_PREPARED_TX_POLICY_OK;
}
