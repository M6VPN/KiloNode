/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/access_policy.h */

#ifndef KILONODE_ACCESS_POLICY_H
#define KILONODE_ACCESS_POLICY_H

#include <sys/types.h>

#include <stdint.h>

#include "kilonode/message.h"

#define KN_ACCESS_COMMAND_MAX          512
#define KN_ACCESS_CONTROL_LINES_MAX    200
#define KN_ACCESS_IDLE_TIMEOUT_DEFAULT 900
#define KN_ACCESS_LINE_MAX             512
#define KN_ACCESS_RATE_LINES_DEFAULT   30
#define KN_ACCESS_RATE_WINDOW_DEFAULT  60

enum kn_access_policy_default {
	KN_ACCESS_POLICY_DENY = 0,
	KN_ACCESS_POLICY_ALLOW
};

enum kn_access_policy_error {
	KN_ACCESS_POLICY_OK = 0,
	KN_ACCESS_POLICY_ERR_INVALID_ARGUMENT,
	KN_ACCESS_POLICY_ERR_INVALID_VALUE,
	KN_ACCESS_POLICY_ERR_DENIED,
	KN_ACCESS_POLICY_ERR_LIMIT
};

struct kn_access_policy {
	enum kn_access_policy_default default_policy;
	uint8_t allow_localhost;
	size_t max_line_bytes;
	size_t max_command_bytes;
	size_t max_clients;
	uint64_t idle_timeout_seconds;
	size_t input_rate_lines;
	uint64_t input_rate_window_seconds;
	size_t bbs_max_body_bytes;
	size_t control_max_command_bytes;
	size_t control_max_response_lines;
};

enum kn_access_policy_error kn_access_policy_body_growth(
	const struct kn_access_policy *, size_t, size_t);
enum kn_access_policy_error kn_access_policy_check_clients(
	const struct kn_access_policy *, size_t);
enum kn_access_policy_error kn_access_policy_check_command(
	const struct kn_access_policy *, size_t);
enum kn_access_policy_error kn_access_policy_check_control_command(
	const struct kn_access_policy *, size_t);
enum kn_access_policy_error kn_access_policy_check_line(
	const struct kn_access_policy *, size_t);
enum kn_access_policy_error kn_access_policy_check_remote(
	const struct kn_access_policy *, const char *);
void kn_access_policy_defaults(struct kn_access_policy *);
const char *kn_access_policy_error_name(enum kn_access_policy_error);
uint8_t kn_access_policy_is_localhost(const char *);
uint8_t kn_access_policy_parse_default(const char *,
	enum kn_access_policy_default *);
enum kn_access_policy_error kn_access_policy_validate(
	const struct kn_access_policy *);

#endif
