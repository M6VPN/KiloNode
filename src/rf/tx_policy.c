/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/rf/tx_policy.c */

#include <sys/types.h>

#include <string.h>

#include "kilonode/tx_policy.h"

enum kn_tx_policy_error
kn_tx_policy_allow_dispatch(const struct kn_tx_policy *policy)
{
	enum kn_tx_policy_error rc;

	rc = kn_tx_policy_validate(policy);
	if (rc != KN_TX_POLICY_OK)
		return rc;
	if (policy->enabled == 0)
		return KN_TX_POLICY_ERR_DISABLED;
	if (policy->dry_run != 0)
		return KN_TX_POLICY_ERR_DISPATCH_DISABLED;

	return KN_TX_POLICY_OK;
}

enum kn_tx_policy_error
kn_tx_policy_allow_control_enqueue(const struct kn_tx_policy *policy,
	size_t payload_len)
{
	enum kn_tx_policy_error rc;

	rc = kn_tx_policy_allow_enqueue(policy, payload_len);
	if (rc != KN_TX_POLICY_OK)
		return rc;
	if (policy->dry_run == 0)
		return KN_TX_POLICY_ERR_DISPATCH_DISABLED;
	if (policy->allow_control_enqueue == 0)
		return KN_TX_POLICY_ERR_CONTROL_DISABLED;

	return KN_TX_POLICY_OK;
}

enum kn_tx_policy_error
kn_tx_policy_allow_enqueue(const struct kn_tx_policy *policy,
	size_t payload_len)
{
	enum kn_tx_policy_error rc;

	rc = kn_tx_policy_validate(policy);
	if (rc != KN_TX_POLICY_OK)
		return rc;
	if (policy->enabled == 0)
		return KN_TX_POLICY_ERR_DISABLED;
	if (payload_len > policy->max_payload_bytes)
		return KN_TX_POLICY_ERR_TOO_LARGE;

	return KN_TX_POLICY_OK;
}

enum kn_tx_policy_error
kn_tx_policy_allow_shell_enqueue(const struct kn_tx_policy *policy,
	size_t payload_len)
{
	enum kn_tx_policy_error rc;

	rc = kn_tx_policy_allow_enqueue(policy, payload_len);
	if (rc != KN_TX_POLICY_OK)
		return rc;
	if (policy->dry_run == 0)
		return KN_TX_POLICY_ERR_DISPATCH_DISABLED;
	if (policy->allow_shell_enqueue == 0)
		return KN_TX_POLICY_ERR_SHELL_DISABLED;

	return KN_TX_POLICY_OK;
}

enum kn_tx_policy_error
kn_tx_policy_allow_ui(const struct kn_tx_policy *policy, size_t payload_len)
{
	enum kn_tx_policy_error rc;

	rc = kn_tx_policy_allow_enqueue(policy, payload_len);
	if (rc != KN_TX_POLICY_OK)
		return rc;
	if (policy->allow_ui == 0)
		return KN_TX_POLICY_ERR_UI_NOT_ALLOWED;

	return KN_TX_POLICY_OK;
}

void
kn_tx_policy_defaults(struct kn_tx_policy *policy)
{
	if (policy == NULL)
		return;

	memset(policy, 0, sizeof(*policy));
	policy->max_queued = KN_TX_POLICY_MAX_QUEUED_DEFAULT;
	policy->max_payload_bytes = KN_TX_POLICY_PAYLOAD_DEFAULT;
	policy->payload_preview_bytes = KN_TX_POLICY_PREVIEW_DEFAULT;
	policy->dry_run = 1;
}

enum kn_tx_policy_error
kn_tx_policy_validate(const struct kn_tx_policy *policy)
{
	if (policy == NULL)
		return KN_TX_POLICY_ERR_INVALID_ARGUMENT;
	if (policy->max_queued < KN_TX_POLICY_MAX_QUEUED_MIN ||
	    policy->max_queued > KN_TX_POLICY_MAX_QUEUED_MAX)
		return KN_TX_POLICY_ERR_INVALID_ARGUMENT;
	if (policy->max_payload_bytes > KN_TX_POLICY_PAYLOAD_MAX)
		return KN_TX_POLICY_ERR_INVALID_ARGUMENT;
	if (policy->payload_preview_bytes < KN_TX_POLICY_PREVIEW_MIN ||
	    policy->payload_preview_bytes > KN_TX_POLICY_PREVIEW_MAX)
		return KN_TX_POLICY_ERR_INVALID_ARGUMENT;

	return KN_TX_POLICY_OK;
}
