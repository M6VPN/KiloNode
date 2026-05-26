/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_access_policy.c */

#include <sys/types.h>

#include <string.h>

#include "kilonode/access_policy.h"

static int test_body_growth(void);
static int test_command_length(void);
static int test_default_values(void);
static int test_line_length(void);
static int test_localhost_allowed(void);
static int test_max_clients(void);
static int test_nonlocal_denied(void);
static int test_parse_default(void);
static int test_validate_invalid(void);

int
main(void)
{
	if (test_default_values() != 0)
		return 1;
	if (test_parse_default() != 0)
		return 1;
	if (test_validate_invalid() != 0)
		return 1;
	if (test_localhost_allowed() != 0)
		return 1;
	if (test_nonlocal_denied() != 0)
		return 1;
	if (test_max_clients() != 0)
		return 1;
	if (test_command_length() != 0)
		return 1;
	if (test_line_length() != 0)
		return 1;
	if (test_body_growth() != 0)
		return 1;
	return 0;
}

static int
test_body_growth(void)
{
	struct kn_access_policy policy;

	kn_access_policy_defaults(&policy);
	policy.bbs_max_body_bytes = 10;
	if (kn_access_policy_body_growth(&policy, 5, 5) !=
	    KN_ACCESS_POLICY_OK)
		return 1;
	return kn_access_policy_body_growth(&policy, 5, 6) ==
	    KN_ACCESS_POLICY_ERR_LIMIT ? 0 : 1;
}

static int
test_command_length(void)
{
	struct kn_access_policy policy;

	kn_access_policy_defaults(&policy);
	policy.max_command_bytes = 4;
	if (kn_access_policy_check_command(&policy, 4) != KN_ACCESS_POLICY_OK)
		return 1;
	return kn_access_policy_check_command(&policy, 5) ==
	    KN_ACCESS_POLICY_ERR_LIMIT ? 0 : 1;
}

static int
test_default_values(void)
{
	struct kn_access_policy policy;

	kn_access_policy_defaults(&policy);
	if (policy.default_policy != KN_ACCESS_POLICY_ALLOW)
		return 1;
	if (policy.allow_localhost != 1)
		return 1;
	if (policy.max_line_bytes != KN_ACCESS_LINE_MAX)
		return 1;
	if (policy.control_max_response_lines != KN_ACCESS_CONTROL_LINES_MAX)
		return 1;
	return kn_access_policy_validate(&policy) == KN_ACCESS_POLICY_OK ?
	    0 : 1;
}

static int
test_line_length(void)
{
	struct kn_access_policy policy;

	kn_access_policy_defaults(&policy);
	policy.max_line_bytes = 3;
	if (kn_access_policy_check_line(&policy, 3) != KN_ACCESS_POLICY_OK)
		return 1;
	return kn_access_policy_check_line(&policy, 4) ==
	    KN_ACCESS_POLICY_ERR_LIMIT ? 0 : 1;
}

static int
test_localhost_allowed(void)
{
	struct kn_access_policy policy;

	kn_access_policy_defaults(&policy);
	if (kn_access_policy_check_remote(&policy, "127.0.0.1") !=
	    KN_ACCESS_POLICY_OK)
		return 1;
	if (kn_access_policy_is_localhost("localhost") == 0)
		return 1;
	return kn_access_policy_is_localhost("192.0.2.1") == 0 ? 0 : 1;
}

static int
test_max_clients(void)
{
	struct kn_access_policy policy;

	kn_access_policy_defaults(&policy);
	policy.max_clients = 2;
	if (kn_access_policy_check_clients(&policy, 1) != KN_ACCESS_POLICY_OK)
		return 1;
	return kn_access_policy_check_clients(&policy, 2) ==
	    KN_ACCESS_POLICY_ERR_LIMIT ? 0 : 1;
}

static int
test_nonlocal_denied(void)
{
	struct kn_access_policy policy;

	kn_access_policy_defaults(&policy);
	policy.default_policy = KN_ACCESS_POLICY_DENY;
	if (kn_access_policy_check_remote(&policy, "127.0.0.1") !=
	    KN_ACCESS_POLICY_OK)
		return 1;
	return kn_access_policy_check_remote(&policy, "192.0.2.1") ==
	    KN_ACCESS_POLICY_ERR_DENIED ? 0 : 1;
}

static int
test_parse_default(void)
{
	enum kn_access_policy_default value;

	if (kn_access_policy_parse_default("allow", &value) == 0 ||
	    value != KN_ACCESS_POLICY_ALLOW)
		return 1;
	if (kn_access_policy_parse_default("deny", &value) == 0 ||
	    value != KN_ACCESS_POLICY_DENY)
		return 1;
	return kn_access_policy_parse_default("bad", &value) == 0 ? 0 : 1;
}

static int
test_validate_invalid(void)
{
	struct kn_access_policy policy;

	kn_access_policy_defaults(&policy);
	policy.max_line_bytes = 0;
	return kn_access_policy_validate(&policy) ==
	    KN_ACCESS_POLICY_ERR_INVALID_VALUE ? 0 : 1;
}
