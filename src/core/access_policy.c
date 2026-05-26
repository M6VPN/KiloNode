/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/core/access_policy.c */

#include <sys/types.h>

#include <stdint.h>
#include <string.h>

#include "kilonode/access_policy.h"

enum kn_access_policy_error
kn_access_policy_body_growth(const struct kn_access_policy *policy,
	size_t current, size_t add)
{
	if (policy == NULL)
		return KN_ACCESS_POLICY_ERR_INVALID_ARGUMENT;
	if (current + add < current)
		return KN_ACCESS_POLICY_ERR_LIMIT;
	if (current + add > policy->bbs_max_body_bytes)
		return KN_ACCESS_POLICY_ERR_LIMIT;
	return KN_ACCESS_POLICY_OK;
}

enum kn_access_policy_error
kn_access_policy_check_clients(const struct kn_access_policy *policy,
	size_t active_clients)
{
	if (policy == NULL)
		return KN_ACCESS_POLICY_ERR_INVALID_ARGUMENT;
	return active_clients < policy->max_clients ? KN_ACCESS_POLICY_OK :
	    KN_ACCESS_POLICY_ERR_LIMIT;
}

enum kn_access_policy_error
kn_access_policy_check_command(const struct kn_access_policy *policy,
	size_t len)
{
	if (policy == NULL)
		return KN_ACCESS_POLICY_ERR_INVALID_ARGUMENT;
	return len <= policy->max_command_bytes ? KN_ACCESS_POLICY_OK :
	    KN_ACCESS_POLICY_ERR_LIMIT;
}

enum kn_access_policy_error
kn_access_policy_check_control_command(const struct kn_access_policy *policy,
	size_t len)
{
	if (policy == NULL)
		return KN_ACCESS_POLICY_ERR_INVALID_ARGUMENT;
	return len <= policy->control_max_command_bytes ?
	    KN_ACCESS_POLICY_OK : KN_ACCESS_POLICY_ERR_LIMIT;
}

enum kn_access_policy_error
kn_access_policy_check_line(const struct kn_access_policy *policy, size_t len)
{
	if (policy == NULL)
		return KN_ACCESS_POLICY_ERR_INVALID_ARGUMENT;
	return len <= policy->max_line_bytes ? KN_ACCESS_POLICY_OK :
	    KN_ACCESS_POLICY_ERR_LIMIT;
}

enum kn_access_policy_error
kn_access_policy_check_remote(const struct kn_access_policy *policy,
	const char *remote)
{
	if (policy == NULL || remote == NULL)
		return KN_ACCESS_POLICY_ERR_INVALID_ARGUMENT;
	if (kn_access_policy_is_localhost(remote) != 0)
		return policy->allow_localhost != 0 ? KN_ACCESS_POLICY_OK :
		    KN_ACCESS_POLICY_ERR_DENIED;
	return policy->default_policy == KN_ACCESS_POLICY_ALLOW ?
	    KN_ACCESS_POLICY_OK : KN_ACCESS_POLICY_ERR_DENIED;
}

void
kn_access_policy_defaults(struct kn_access_policy *policy)
{
	if (policy == NULL)
		return;
	policy->default_policy = KN_ACCESS_POLICY_ALLOW;
	policy->allow_localhost = 1;
	policy->max_line_bytes = KN_ACCESS_LINE_MAX;
	policy->max_command_bytes = KN_ACCESS_COMMAND_MAX;
	policy->max_clients = 8;
	policy->idle_timeout_seconds = KN_ACCESS_IDLE_TIMEOUT_DEFAULT;
	policy->input_rate_lines = KN_ACCESS_RATE_LINES_DEFAULT;
	policy->input_rate_window_seconds = KN_ACCESS_RATE_WINDOW_DEFAULT;
	policy->bbs_max_body_bytes = KN_MESSAGE_BODY_MAX;
	policy->control_max_command_bytes = KN_ACCESS_COMMAND_MAX;
	policy->control_max_response_lines = KN_ACCESS_CONTROL_LINES_MAX;
}

const char *
kn_access_policy_error_name(enum kn_access_policy_error error)
{
	switch (error) {
	case KN_ACCESS_POLICY_OK:
		return "ok";
	case KN_ACCESS_POLICY_ERR_INVALID_ARGUMENT:
		return "invalid argument";
	case KN_ACCESS_POLICY_ERR_INVALID_VALUE:
		return "invalid value";
	case KN_ACCESS_POLICY_ERR_DENIED:
		return "denied";
	case KN_ACCESS_POLICY_ERR_LIMIT:
		return "limit";
	}

	return "unknown";
}

uint8_t
kn_access_policy_is_localhost(const char *remote)
{
	if (remote == NULL)
		return 0;
	return strcmp(remote, "127.0.0.1") == 0 ||
	    strcmp(remote, "::1") == 0 ||
	    strcmp(remote, "localhost") == 0 ? 1 : 0;
}

uint8_t
kn_access_policy_parse_default(const char *input,
	enum kn_access_policy_default *out)
{
	if (input == NULL || out == NULL)
		return 0;
	if (strcmp(input, "allow") == 0) {
		*out = KN_ACCESS_POLICY_ALLOW;
		return 1;
	}
	if (strcmp(input, "deny") == 0) {
		*out = KN_ACCESS_POLICY_DENY;
		return 1;
	}
	return 0;
}

enum kn_access_policy_error
kn_access_policy_validate(const struct kn_access_policy *policy)
{
	if (policy == NULL)
		return KN_ACCESS_POLICY_ERR_INVALID_ARGUMENT;
	if (policy->default_policy != KN_ACCESS_POLICY_ALLOW &&
	    policy->default_policy != KN_ACCESS_POLICY_DENY)
		return KN_ACCESS_POLICY_ERR_INVALID_VALUE;
	if (policy->max_line_bytes == 0 ||
	    policy->max_line_bytes > KN_ACCESS_LINE_MAX)
		return KN_ACCESS_POLICY_ERR_INVALID_VALUE;
	if (policy->max_command_bytes == 0 ||
	    policy->max_command_bytes > KN_ACCESS_COMMAND_MAX)
		return KN_ACCESS_POLICY_ERR_INVALID_VALUE;
	if (policy->max_clients == 0 || policy->max_clients > 16)
		return KN_ACCESS_POLICY_ERR_INVALID_VALUE;
	if (policy->idle_timeout_seconds == 0)
		return KN_ACCESS_POLICY_ERR_INVALID_VALUE;
	if (policy->input_rate_lines == 0 ||
	    policy->input_rate_window_seconds == 0)
		return KN_ACCESS_POLICY_ERR_INVALID_VALUE;
	if (policy->bbs_max_body_bytes == 0 ||
	    policy->bbs_max_body_bytes > KN_MESSAGE_BODY_MAX)
		return KN_ACCESS_POLICY_ERR_INVALID_VALUE;
	if (policy->control_max_command_bytes == 0 ||
	    policy->control_max_command_bytes > KN_ACCESS_COMMAND_MAX)
		return KN_ACCESS_POLICY_ERR_INVALID_VALUE;
	if (policy->control_max_response_lines == 0 ||
	    policy->control_max_response_lines > KN_ACCESS_CONTROL_LINES_MAX)
		return KN_ACCESS_POLICY_ERR_INVALID_VALUE;
	return KN_ACCESS_POLICY_OK;
}
