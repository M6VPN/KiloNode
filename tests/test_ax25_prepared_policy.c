/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_ax25_prepared_policy.c */

#include <sys/types.h>

#include <string.h>

#include "kilonode/ax25_prepared_policy.h"

static int test_bridge_blocked(void);
static int test_default_policy(void);
static int test_format(void);
static int test_invalid_values(void);

int
main(void)
{
	if (test_default_policy() != 0)
		return 1;
	if (test_invalid_values() != 0)
		return 1;
	if (test_bridge_blocked() != 0)
		return 1;
	if (test_format() != 0)
		return 1;

	return 0;
}

static int
test_bridge_blocked(void)
{
	struct kn_ax25_prepared_frame frame;

	kn_ax25_prepared_frame_clear(&frame);

	return kn_ax25_prepared_bridge_to_tx(&frame) ==
	    KN_AX25_PREPARED_POLICY_ERR_BRIDGE_BLOCKED ? 0 : 1;
}

static int
test_default_policy(void)
{
	struct kn_ax25_prepared_policy policy;

	kn_ax25_prepared_policy_default(&policy);
	if (policy.enabled == 0 || policy.build_raw == 0)
		return 1;
	if (policy.bridge_to_tx != 0)
		return 1;

	return kn_ax25_prepared_policy_validate(&policy) ==
	    KN_AX25_PREPARED_POLICY_OK ? 0 : 1;
}

static int
test_format(void)
{
	struct kn_ax25_prepared_policy policy;
	char buf[128];

	kn_ax25_prepared_policy_default(&policy);
	if (kn_ax25_prepared_policy_format(&policy, buf, sizeof(buf)) !=
	    KN_AX25_PREPARED_POLICY_OK)
		return 1;

	return strstr(buf, "bridge_to_tx=false") != NULL ? 0 : 1;
}

static int
test_invalid_values(void)
{
	struct kn_ax25_prepared_policy policy;

	kn_ax25_prepared_policy_default(&policy);
	policy.bridge_to_tx = 1;
	if (kn_ax25_prepared_policy_validate(&policy) !=
	    KN_AX25_PREPARED_POLICY_ERR_INVALID_VALUE)
		return 1;
	kn_ax25_prepared_policy_default(&policy);
	policy.max_frames = 0;
	if (kn_ax25_prepared_policy_validate(&policy) !=
	    KN_AX25_PREPARED_POLICY_ERR_INVALID_VALUE)
		return 1;
	kn_ax25_prepared_policy_default(&policy);
	policy.build_raw = 2;

	return kn_ax25_prepared_policy_validate(&policy) ==
	    KN_AX25_PREPARED_POLICY_ERR_INVALID_VALUE ? 0 : 1;
}
