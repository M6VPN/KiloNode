/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/node_command_context.h */

#ifndef KILONODE_NODE_COMMAND_CONTEXT_H
#define KILONODE_NODE_COMMAND_CONTEXT_H

#include <sys/types.h>

#include <stdint.h>

#include "kilonode/config.h"
#include "kilonode/heard.h"
#include "kilonode/stats.h"

#define KN_NODE_COMMAND_REMOTE_MAX 64

struct kn_node_command_user {
	char remote[KN_NODE_COMMAND_REMOTE_MAX];
	uint64_t connected_at;
	uint64_t command_count;
};

struct kn_node_command_context {
	const struct kn_config_node *node;
	const struct kn_daemon_stats *daemon;
	const struct kn_port_stats *ports;
	size_t port_count;
	const struct kn_heard_entry *heard;
	size_t heard_count;
	const struct kn_node_command_user *users;
	size_t user_count;
	const char *rf_source;
	size_t output_limit;
	uint8_t bbs_enabled;
};

void kn_node_command_context_clear(struct kn_node_command_context *);

#endif
