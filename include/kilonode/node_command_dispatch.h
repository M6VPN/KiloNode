/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/node_command_dispatch.h */

#ifndef KILONODE_NODE_COMMAND_DISPATCH_H
#define KILONODE_NODE_COMMAND_DISPATCH_H

#include <sys/types.h>

#include <stdint.h>

#include "kilonode/node_command.h"
#include "kilonode/node_command_context.h"
#include "kilonode/node_command_profile.h"

#define KN_NODE_COMMAND_OUTPUT_MAX 32768

enum kn_node_command_dispatch_status {
	KN_NODE_COMMAND_DISPATCH_OK = 0,
	KN_NODE_COMMAND_DISPATCH_UNKNOWN,
	KN_NODE_COMMAND_DISPATCH_FORBIDDEN_CONTEXT,
	KN_NODE_COMMAND_DISPATCH_INVALID_ARGS,
	KN_NODE_COMMAND_DISPATCH_OVERLONG,
	KN_NODE_COMMAND_DISPATCH_CONTROL_CHARACTER,
	KN_NODE_COMMAND_DISPATCH_INTERNAL_ERROR
};

struct kn_node_command_dispatch_result {
	enum kn_node_command_dispatch_status status;
	enum kn_node_command_id command_id;
	char profile_name[KN_NODE_COMMAND_NAME_MAX];
	char output[KN_NODE_COMMAND_OUTPUT_MAX];
	size_t output_len;
	uint8_t close_session;
	uint8_t mode_transition;
	char reason[64];
};

void kn_node_command_dispatch_result_clear(
	struct kn_node_command_dispatch_result *);
const char *kn_node_command_dispatch_status_name(
	enum kn_node_command_dispatch_status);
enum kn_node_command_dispatch_status kn_node_command_dispatch(
	enum kn_node_command_context_kind,
	const struct kn_node_command_context *, const uint8_t *, size_t,
	size_t, struct kn_node_command_dispatch_result *);

#endif
