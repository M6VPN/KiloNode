/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/node_shell.h */

#ifndef KILONODE_NODE_SHELL_H
#define KILONODE_NODE_SHELL_H

#include <sys/types.h>

#include <stdint.h>

#include "kilonode/config.h"
#include "kilonode/heard.h"
#include "kilonode/stats.h"

#define KN_NODE_SHELL_BANNER_MAX   128
#define KN_NODE_SHELL_LINE_MAX     512
#define KN_NODE_SHELL_MAX_CLIENTS  16
#define KN_NODE_SHELL_PROMPT       "KILONODE> "
#define KN_NODE_SHELL_REMOTE_MAX   64
#define KN_NODE_SHELL_RESPONSE_MAX 32768

enum kn_node_shell_error {
	KN_NODE_SHELL_OK = 0,
	KN_NODE_SHELL_ERR_INVALID_ARGUMENT,
	KN_NODE_SHELL_ERR_INVALID_CONFIG,
	KN_NODE_SHELL_ERR_RESOLVE,
	KN_NODE_SHELL_ERR_LISTEN,
	KN_NODE_SHELL_ERR_ACCEPT,
	KN_NODE_SHELL_ERR_IO
};

struct kn_node_shell_user {
	char remote[KN_NODE_SHELL_REMOTE_MAX];
	uint64_t connected_at;
	uint64_t command_count;
};

struct kn_node_shell_session {
	int fd;
	char line[KN_NODE_SHELL_LINE_MAX];
	size_t line_len;
	uint64_t connected_at;
	uint64_t command_count;
	char remote[KN_NODE_SHELL_REMOTE_MAX];
	uint8_t closed;
	uint8_t discard_line;
};

struct kn_node_shell_state {
	int listen_fd;
	struct kn_node_shell_session sessions[KN_NODE_SHELL_MAX_CLIENTS];
	size_t max_clients;
};

struct kn_node_shell_snapshot {
	const struct kn_config_node *node;
	const struct kn_daemon_stats *daemon;
	const struct kn_port_stats *ports;
	size_t port_count;
	const struct kn_heard_entry *heard;
	size_t heard_count;
	const struct kn_node_shell_user *users;
	size_t user_count;
};

void kn_node_shell_close(struct kn_node_shell_state *);
enum kn_node_shell_error kn_node_shell_format_command(const char *,
	const struct kn_node_shell_snapshot *, char *, size_t, uint8_t *);
int kn_node_shell_fd(const struct kn_node_shell_state *);
void kn_node_shell_init(struct kn_node_shell_state *);
enum kn_node_shell_error kn_node_shell_open(const struct kn_config_shell *,
	struct kn_node_shell_state *);
uint16_t kn_node_shell_port(const struct kn_node_shell_state *);
enum kn_node_shell_error kn_node_shell_accept(struct kn_node_shell_state *,
	const char *);
enum kn_node_shell_error kn_node_shell_process_session(
	struct kn_node_shell_session *, const struct kn_node_shell_snapshot *);
void kn_node_shell_session_close(struct kn_node_shell_session *);
void kn_node_shell_snapshot_users(const struct kn_node_shell_state *,
	struct kn_node_shell_user *, size_t, size_t *);

#endif
