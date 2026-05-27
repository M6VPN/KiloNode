/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/proto/ax25_scheduler_policy.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "kilonode/ax25_scheduler_policy.h"

void
kn_ax25_scheduler_policy_default(struct kn_ax25_scheduler_policy *policy)
{
	if (policy == NULL)
		return;

	memset(policy, 0, sizeof(*policy));
	policy->max_expired_per_cycle =
	    KN_AX25_LIVE_SCHEDULER_EXPIRED_DEFAULT;
	policy->diagnostics_enabled = 1;
}

enum kn_ax25_scheduler_policy_error
kn_ax25_scheduler_policy_format(
	const struct kn_ax25_scheduler_policy *policy, char *buf, size_t bufsiz)
{
	int needed;

	if (policy == NULL || buf == NULL || bufsiz == 0)
		return KN_AX25_SCHEDULER_POLICY_ERR_INVALID_ARGUMENT;
	if (kn_ax25_scheduler_policy_validate(policy) !=
	    KN_AX25_SCHEDULER_POLICY_OK)
		return KN_AX25_SCHEDULER_POLICY_ERR_INVALID_VALUE;

	needed = snprintf(buf, bufsiz,
	    "enabled=%s process_expired=%s max_expired=%llu "
	    "tx_actions=%s diagnostics=%s",
	    policy->enabled != 0 ? "true" : "false",
	    policy->process_expired != 0 ? "true" : "false",
	    (unsigned long long)policy->max_expired_per_cycle,
	    policy->tx_actions_enabled != 0 ? "true" : "false",
	    policy->diagnostics_enabled != 0 ? "true" : "false");
	if (needed < 0 || (size_t)needed >= bufsiz)
		return KN_AX25_SCHEDULER_POLICY_ERR_BUFFER;

	return KN_AX25_SCHEDULER_POLICY_OK;
}

enum kn_ax25_scheduler_policy_error
kn_ax25_scheduler_policy_validate(
	const struct kn_ax25_scheduler_policy *policy)
{
	if (policy == NULL)
		return KN_AX25_SCHEDULER_POLICY_ERR_INVALID_ARGUMENT;
	if (policy->enabled > 1 || policy->process_expired > 1 ||
	    policy->tx_actions_enabled > 1 || policy->diagnostics_enabled > 1)
		return KN_AX25_SCHEDULER_POLICY_ERR_INVALID_VALUE;
	if (policy->max_expired_per_cycle == 0 ||
	    policy->max_expired_per_cycle >
	    KN_AX25_LIVE_SCHEDULER_EXPIRED_MAX)
		return KN_AX25_SCHEDULER_POLICY_ERR_INVALID_VALUE;
	if (policy->process_expired != 0 && policy->enabled == 0)
		return KN_AX25_SCHEDULER_POLICY_ERR_INVALID_VALUE;
	if (policy->tx_actions_enabled != 0)
		return KN_AX25_SCHEDULER_POLICY_ERR_INVALID_VALUE;

	return KN_AX25_SCHEDULER_POLICY_OK;
}
