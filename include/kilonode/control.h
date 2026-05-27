/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/control.h */

#ifndef KILONODE_CONTROL_H
#define KILONODE_CONTROL_H

#include <sys/types.h>

#include <stdint.h>

#include "kilonode/ax25_runtime.h"
#include "kilonode/heard.h"
#include "kilonode/message_store.h"
#include "kilonode/rf_abuse.h"
#include "kilonode/rf_command_queue.h"
#include "kilonode/rf_ignore.h"
#include "kilonode/rx_queue.h"
#include "kilonode/rx_session.h"
#include "kilonode/stats.h"
#include "kilonode/tx_dispatch.h"
#include "kilonode/tx_queue.h"

#define KN_CONTROL_COMMAND_MAX 512
#define KN_CONTROL_LINE_MAX    256
#define KN_CONTROL_QUEUE_MAX   4096

enum kn_control_error {
	KN_CONTROL_OK = 0,
	KN_CONTROL_ERR_INVALID_ARGUMENT,
	KN_CONTROL_ERR_INVALID_PATH,
	KN_CONTROL_ERR_OPEN,
	KN_CONTROL_ERR_CONNECT,
	KN_CONTROL_ERR_IO,
	KN_CONTROL_ERR_OVERLONG_COMMAND,
	KN_CONTROL_ERR_UNKNOWN_COMMAND
};

struct kn_control_snapshot {
	const struct kn_daemon_stats *daemon;
	const struct kn_port_stats *ports;
	size_t port_count;
	const struct kn_heard_entry *heard;
	size_t heard_count;
	uint8_t bbs_enabled;
	struct kn_message_store *bbs_store;
	uint8_t rx_enabled;
	const struct kn_rx_queue *rx_events;
	const struct kn_rx_session_table *rx_sessions;
	struct kn_tx_queue *tx_queue;
	struct kn_tx_dispatcher *tx_dispatch;
	const struct kn_config_rf_command *rf_config;
	const struct kn_rf_command_queue *rf_commands;
	const struct kn_rf_abuse_state *rf_abuse;
	const struct kn_rf_ignore_list *rf_ignore;
	const struct kn_ax25_runtime *ax25_runtime;
	size_t control_max_command_bytes;
	size_t control_max_response_lines;
};

struct kn_control_socket {
	int listen_fd;
	int client_fd;
	char path[KN_CONFIG_PATH_MAX];
};

void kn_control_socket_close(struct kn_control_socket *);
enum kn_control_error kn_control_socket_connect(const char *, int *);
int kn_control_socket_fd(const struct kn_control_socket *);
void kn_control_socket_init(struct kn_control_socket *);
enum kn_control_error kn_control_socket_open(struct kn_control_socket *,
	const char *);
enum kn_control_error kn_control_socket_read_command(struct kn_control_socket *,
	char *, size_t, size_t *);
enum kn_control_error kn_control_socket_write(int, const char *, size_t);
enum kn_control_error kn_control_protocol_handle(const char *,
	const struct kn_control_snapshot *, char *, size_t);
uint8_t kn_control_socket_path_valid(const char *);

#endif
