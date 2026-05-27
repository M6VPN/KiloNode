/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_ax25_scheduler_policy.c */

#include <sys/types.h>

#include <string.h>

#include "kilonode/ax25_scheduler_policy.h"

static int test_default_policy_disabled(void);
static int test_format(void);
static int test_invalid_max_expired_rejected(void);
static int test_process_requires_enabled(void);
static int test_tx_actions_rejected(void);
static int test_valid_enabled_diagnostic_only(void);

int
main(void)
{
	if (test_default_policy_disabled() != 0)
		return 1;
	if (test_valid_enabled_diagnostic_only() != 0)
		return 1;
	if (test_process_requires_enabled() != 0)
		return 1;
	if (test_tx_actions_rejected() != 0)
		return 1;
	if (test_invalid_max_expired_rejected() != 0)
		return 1;
	if (test_format() != 0)
		return 1;

	return 0;
}

static int
test_default_policy_disabled(void)
{
	struct kn_ax25_scheduler_policy policy;

	kn_ax25_scheduler_policy_default(&policy);
	if (policy.enabled != 0 || policy.process_expired != 0)
		return 1;
	if (policy.tx_actions_enabled != 0)
		return 1;

	return policy.max_expired_per_cycle ==
	    KN_AX25_LIVE_SCHEDULER_EXPIRED_DEFAULT ? 0 : 1;
}

static int
test_format(void)
{
	struct kn_ax25_scheduler_policy policy;
	char buf[128];

	kn_ax25_scheduler_policy_default(&policy);
	policy.enabled = 1;
	if (kn_ax25_scheduler_policy_format(&policy, buf, sizeof(buf)) !=
	    KN_AX25_SCHEDULER_POLICY_OK)
		return 1;

	return strstr(buf, "enabled=true process_expired=false") != NULL ?
	    0 : 1;
}

static int
test_invalid_max_expired_rejected(void)
{
	struct kn_ax25_scheduler_policy policy;

	kn_ax25_scheduler_policy_default(&policy);
	policy.max_expired_per_cycle = 0;

	return kn_ax25_scheduler_policy_validate(&policy) ==
	    KN_AX25_SCHEDULER_POLICY_ERR_INVALID_VALUE ? 0 : 1;
}

static int
test_process_requires_enabled(void)
{
	struct kn_ax25_scheduler_policy policy;

	kn_ax25_scheduler_policy_default(&policy);
	policy.process_expired = 1;

	return kn_ax25_scheduler_policy_validate(&policy) ==
	    KN_AX25_SCHEDULER_POLICY_ERR_INVALID_VALUE ? 0 : 1;
}

static int
test_tx_actions_rejected(void)
{
	struct kn_ax25_scheduler_policy policy;

	kn_ax25_scheduler_policy_default(&policy);
	policy.tx_actions_enabled = 1;

	return kn_ax25_scheduler_policy_validate(&policy) ==
	    KN_AX25_SCHEDULER_POLICY_ERR_INVALID_VALUE ? 0 : 1;
}

static int
test_valid_enabled_diagnostic_only(void)
{
	struct kn_ax25_scheduler_policy policy;

	kn_ax25_scheduler_policy_default(&policy);
	policy.enabled = 1;
	policy.process_expired = 1;
	policy.max_expired_per_cycle = 4;

	return kn_ax25_scheduler_policy_validate(&policy) ==
	    KN_AX25_SCHEDULER_POLICY_OK ? 0 : 1;
}
