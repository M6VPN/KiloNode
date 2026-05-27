/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/proto/ax25_prepared_policy.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "kilonode/ax25_prepared_policy.h"

enum kn_ax25_prepared_policy_error
kn_ax25_prepared_bridge_to_tx(const struct kn_ax25_prepared_frame *frame)
{
	if (frame == NULL)
		return KN_AX25_PREPARED_POLICY_ERR_INVALID_ARGUMENT;

	return KN_AX25_PREPARED_POLICY_ERR_BRIDGE_BLOCKED;
}

void
kn_ax25_prepared_policy_default(struct kn_ax25_prepared_policy *policy)
{
	if (policy == NULL)
		return;

	memset(policy, 0, sizeof(*policy));
	policy->enabled = 1;
	policy->build_raw = 1;
	policy->max_frames = KN_AX25_PREPARED_QUEUE_DEFAULT_MAX;
}

enum kn_ax25_prepared_policy_error
kn_ax25_prepared_policy_format(const struct kn_ax25_prepared_policy *policy,
	char *buf, size_t bufsiz)
{
	int needed;

	if (policy == NULL || buf == NULL || bufsiz == 0)
		return KN_AX25_PREPARED_POLICY_ERR_INVALID_ARGUMENT;

	needed = snprintf(buf, bufsiz,
	    "enabled=%s build_raw=%s max_frames=%llu bridge_to_tx=%s",
	    policy->enabled != 0 ? "true" : "false",
	    policy->build_raw != 0 ? "true" : "false",
	    (unsigned long long)policy->max_frames,
	    policy->bridge_to_tx != 0 ? "true" : "false");
	if (needed < 0 || (size_t)needed >= bufsiz)
		return KN_AX25_PREPARED_POLICY_ERR_BUFFER;

	return KN_AX25_PREPARED_POLICY_OK;
}

enum kn_ax25_prepared_policy_error
kn_ax25_prepared_policy_validate(
	const struct kn_ax25_prepared_policy *policy)
{
	if (policy == NULL)
		return KN_AX25_PREPARED_POLICY_ERR_INVALID_ARGUMENT;
	if (policy->enabled > 1 || policy->build_raw > 1 ||
	    policy->bridge_to_tx > 1)
		return KN_AX25_PREPARED_POLICY_ERR_INVALID_VALUE;
	if (policy->bridge_to_tx != 0)
		return KN_AX25_PREPARED_POLICY_ERR_INVALID_VALUE;
	if (policy->max_frames == 0 ||
	    policy->max_frames > KN_AX25_PREPARED_QUEUE_MAX)
		return KN_AX25_PREPARED_POLICY_ERR_INVALID_VALUE;

	return KN_AX25_PREPARED_POLICY_OK;
}
