/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/node_command_profile.h */

#ifndef KILONODE_NODE_COMMAND_PROFILE_H
#define KILONODE_NODE_COMMAND_PROFILE_H

#include <sys/types.h>

#include <stdint.h>

#define KN_NODE_COMMAND_PROFILE_MAX 16

enum kn_node_command_id {
	KN_NODE_COMMAND_ID_HELP = 0,
	KN_NODE_COMMAND_ID_INFO,
	KN_NODE_COMMAND_ID_PORTS,
	KN_NODE_COMMAND_ID_HEARD,
	KN_NODE_COMMAND_ID_STATS,
	KN_NODE_COMMAND_ID_USERS,
	KN_NODE_COMMAND_ID_BBS,
	KN_NODE_COMMAND_ID_BYE,
	KN_NODE_COMMAND_ID_QUIT,
	KN_NODE_COMMAND_ID_PING,
	KN_NODE_COMMAND_ID_UNKNOWN
};

enum kn_node_command_context_kind {
	KN_NODE_COMMAND_CONTEXT_LOCAL = 0x01,
	KN_NODE_COMMAND_CONTEXT_RF_UI = 0x02,
	KN_NODE_COMMAND_CONTEXT_BBS = 0x04
};

enum kn_node_command_argument_policy {
	KN_NODE_COMMAND_ARGS_NONE = 0,
	KN_NODE_COMMAND_ARGS_OPTIONAL,
	KN_NODE_COMMAND_ARGS_REQUIRED,
	KN_NODE_COMMAND_ARGS_FREE_TEXT
};

enum kn_node_command_output_class {
	KN_NODE_COMMAND_OUTPUT_ONE_LINE = 0,
	KN_NODE_COMMAND_OUTPUT_MULTI_LINE,
	KN_NODE_COMMAND_OUTPUT_MODE_TRANSITION,
	KN_NODE_COMMAND_OUTPUT_CLOSE_SESSION,
	KN_NODE_COMMAND_OUTPUT_NO_REPLY
};

enum kn_node_command_safety_class {
	KN_NODE_COMMAND_SAFETY_READ_ONLY = 0,
	KN_NODE_COMMAND_SAFETY_LOCAL_ONLY,
	KN_NODE_COMMAND_SAFETY_DISABLED_OVER_RF,
	KN_NODE_COMMAND_SAFETY_FUTURE
};

struct kn_node_command_profile {
	enum kn_node_command_id id;
	const char *name;
	size_t min_abbrev;
	const char *aliases[2];
	uint8_t contexts;
	enum kn_node_command_argument_policy argument_policy;
	enum kn_node_command_output_class output_class;
	enum kn_node_command_safety_class safety_class;
};

const char *kn_node_command_argument_policy_name(
	enum kn_node_command_argument_policy);
enum kn_node_command_id kn_node_command_id_from_name(const char *);
const char *kn_node_command_id_name(enum kn_node_command_id);
const char *kn_node_command_output_class_name(
	enum kn_node_command_output_class);
const struct kn_node_command_profile *kn_node_command_profile_find(
	const char *);
const struct kn_node_command_profile *kn_node_command_profile_find_id(
	enum kn_node_command_id);
const struct kn_node_command_profile *kn_node_command_profiles(size_t *);
const char *kn_node_command_safety_class_name(
	enum kn_node_command_safety_class);

#endif
