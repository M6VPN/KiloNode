/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_ax25_prepared_tx_policy.c */

#include <sys/types.h>

#include <string.h>

#include "kilonode/ax25_prepared_tx_policy.h"

static int test_default_policy(void);
static int test_format(void);
static int test_invalid_values(void);
static int test_valid_test_policy(void);

int
main(void)
{
	if (test_default_policy() != 0)
		return 1;
	if (test_valid_test_policy() != 0)
		return 1;
	if (test_invalid_values() != 0)
		return 1;
	if (test_format() != 0)
		return 1;

	return 0;
}

static int
test_default_policy(void)
{
	struct kn_ax25_prepared_tx_policy policy;

	kn_ax25_prepared_tx_policy_default(&policy);
	if (policy.bridge_enabled != 0 || policy.test_only == 0)
		return 1;
	if (policy.allow_control_frames != 0 || policy.allow_i_frames != 0)
		return 1;
	if (policy.allow_fx25_wrapping != 0)
		return 1;

	return kn_ax25_prepared_tx_policy_validate(&policy) ==
	    KN_AX25_PREPARED_TX_POLICY_OK ? 0 : 1;
}

static int
test_format(void)
{
	struct kn_ax25_prepared_tx_policy policy;
	char buf[256];

	kn_ax25_prepared_tx_policy_default(&policy);
	if (kn_ax25_prepared_tx_policy_format(&policy, buf,
	    sizeof(buf)) != KN_AX25_PREPARED_TX_POLICY_OK)
		return 1;

	return strstr(buf, "enabled=false") != NULL &&
	    strstr(buf, "fx25=false") != NULL ? 0 : 1;
}

static int
test_invalid_values(void)
{
	struct kn_ax25_prepared_tx_policy policy;

	kn_ax25_prepared_tx_policy_default(&policy);
	policy.max_bridge_per_call = 0;
	if (kn_ax25_prepared_tx_policy_validate(&policy) !=
	    KN_AX25_PREPARED_TX_POLICY_ERR_INVALID_VALUE)
		return 1;
	kn_ax25_prepared_tx_policy_default(&policy);
	policy.bridge_enabled = 1;
	policy.test_only = 0;
	if (kn_ax25_prepared_tx_policy_validate(&policy) !=
	    KN_AX25_PREPARED_TX_POLICY_ERR_INVALID_VALUE)
		return 1;
	kn_ax25_prepared_tx_policy_default(&policy);
	policy.allow_i_frames = 1;
	if (kn_ax25_prepared_tx_policy_validate(&policy) !=
	    KN_AX25_PREPARED_TX_POLICY_ERR_INVALID_VALUE)
		return 1;
	kn_ax25_prepared_tx_policy_default(&policy);
	policy.allow_fx25_wrapping = 1;

	return kn_ax25_prepared_tx_policy_validate(&policy) ==
	    KN_AX25_PREPARED_TX_POLICY_ERR_INVALID_VALUE ? 0 : 1;
}

static int
test_valid_test_policy(void)
{
	struct kn_ax25_prepared_tx_policy policy;

	kn_ax25_prepared_tx_policy_default(&policy);
	policy.bridge_enabled = 1;
	policy.allow_control_frames = 1;

	return kn_ax25_prepared_tx_policy_validate(&policy) ==
	    KN_AX25_PREPARED_TX_POLICY_OK ? 0 : 1;
}
