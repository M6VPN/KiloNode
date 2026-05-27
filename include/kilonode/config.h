/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/config.h */

#ifndef KILONODE_CONFIG_H
#define KILONODE_CONFIG_H

#include <sys/types.h>

#include <stdint.h>

#include "kilonode/access_policy.h"
#include "kilonode/callsign.h"
#include "kilonode/tx_policy.h"
#include "kilonode/transport.h"

#define KN_CONFIG_ALIAS_MAX     16
#define KN_CONFIG_ERROR_MAX     160
#define KN_CONFIG_HOST_MAX      128
#define KN_CONFIG_HEARD_MAX     256
#define KN_CONFIG_HEARD_MIN     1
#define KN_CONFIG_LINE_MAX      512
#define KN_CONFIG_LOCATION_MAX  128
#define KN_CONFIG_BBS_BODY_MAX  65536
#define KN_CONFIG_BBS_BODY_MIN  1
#define KN_CONFIG_MAX_FRAME_MAX 65535
#define KN_CONFIG_MAX_FRAME_MIN 64
#define KN_CONFIG_PATH_MAX      256
#define KN_CONFIG_PORT_MAX      16
#define KN_CONFIG_PORT_NAME_MAX 32
#define KN_CONFIG_RECEIVE_EVENTS_MAX    256
#define KN_CONFIG_RECEIVE_EVENTS_MIN    1
#define KN_CONFIG_RECEIVE_PREVIEW_MAX   256
#define KN_CONFIG_RECEIVE_PREVIEW_MIN   1
#define KN_CONFIG_RECEIVE_SESSIONS_MAX  128
#define KN_CONFIG_RECEIVE_SESSIONS_MIN  1
#define KN_CONFIG_RF_COMMAND_BYTES_MAX  128
#define KN_CONFIG_RF_COMMAND_BYTES_MIN  1
#define KN_CONFIG_RF_COMMAND_DEST_MAX   16
#define KN_CONFIG_RF_COMMAND_DESTS_MAX  8
#define KN_CONFIG_RF_COMMAND_EVENTS_MAX 128
#define KN_CONFIG_RF_COMMAND_EVENTS_MIN 1
#define KN_CONFIG_RF_IGNORE_AFTER_MAX   1000
#define KN_CONFIG_RF_IGNORE_AFTER_MIN   1
#define KN_CONFIG_RF_IGNORE_SECONDS_MAX 86400
#define KN_CONFIG_RF_IGNORE_SECONDS_MIN 1
#define KN_CONFIG_RF_RATE_COMMANDS_MAX  1000
#define KN_CONFIG_RF_RATE_COMMANDS_MIN  1
#define KN_CONFIG_RF_RATE_WINDOW_MAX    86400
#define KN_CONFIG_RF_RATE_WINDOW_MIN    1
#define KN_CONFIG_RF_REPLY_BYTES_MAX    200
#define KN_CONFIG_RF_REPLY_BYTES_MIN    1
#define KN_CONFIG_SHELL_BANNER_MAX      128
#define KN_CONFIG_SHELL_MAX_CLIENTS     16
#define KN_CONFIG_SHELL_MAX_CLIENTS_MIN 1

struct kn_config_access {
	struct kn_access_policy policy;
	uint8_t has_block;
	uint8_t has_default_policy;
	uint8_t has_allow_localhost;
	uint8_t has_max_line_bytes;
	uint8_t has_max_command_bytes;
	uint8_t has_max_clients;
	uint8_t has_idle_timeout_seconds;
	uint8_t has_input_rate_lines;
	uint8_t has_input_rate_window_seconds;
	uint8_t has_bbs_max_body_bytes;
	uint8_t has_control_max_command_bytes;
	uint8_t has_control_max_response_lines;
};

enum kn_config_error {
	KN_CONFIG_OK = 0,
	KN_CONFIG_ERR_INVALID_ARGUMENT,
	KN_CONFIG_ERR_IO,
	KN_CONFIG_ERR_LINE_TOO_LONG,
	KN_CONFIG_ERR_PARSE,
	KN_CONFIG_ERR_UNKNOWN_BLOCK,
	KN_CONFIG_ERR_UNKNOWN_KEY,
	KN_CONFIG_ERR_DUPLICATE_KEY,
	KN_CONFIG_ERR_MISSING_REQUIRED,
	KN_CONFIG_ERR_INVALID_VALUE,
	KN_CONFIG_ERR_DUPLICATE_PORT,
	KN_CONFIG_ERR_TOO_MANY_PORTS
};

enum kn_config_port_type {
	KN_CONFIG_PORT_NONE = 0,
	KN_CONFIG_PORT_STDIO,
	KN_CONFIG_PORT_TCP_CONNECT,
	KN_CONFIG_PORT_TCP_LISTEN,
	KN_CONFIG_PORT_SERIAL,
	KN_CONFIG_PORT_PTY,
	KN_CONFIG_PORT_UNIX_CONNECT,
	KN_CONFIG_PORT_UNIX_LISTEN,
	KN_CONFIG_PORT_MEMORY_TEST
};

struct kn_config_node {
	struct kn_callsign callsign;
	char alias[KN_CONFIG_ALIAS_MAX];
	char location[KN_CONFIG_LOCATION_MAX];
	uint8_t has_callsign;
	uint8_t has_alias;
	uint8_t has_location;
};

struct kn_config_control {
	char path[KN_CONFIG_PATH_MAX];
	uint8_t enabled;
	uint8_t has_block;
	uint8_t has_enabled;
	uint8_t has_path;
};

struct kn_config_bbs {
	char store_path[KN_CONFIG_PATH_MAX];
	size_t max_body_bytes;
	uint8_t enabled;
	uint8_t has_block;
	uint8_t has_enabled;
	uint8_t has_store_path;
	uint8_t has_max_body_bytes;
};

struct kn_config_heard {
	size_t max_entries;
	uint8_t enabled;
	uint8_t has_block;
	uint8_t has_enabled;
	uint8_t has_max_entries;
};

struct kn_config_receive {
	size_t max_events;
	size_t max_sessions;
	size_t payload_preview_bytes;
	uint8_t events_enabled;
	uint8_t has_block;
	uint8_t has_events_enabled;
	uint8_t has_max_events;
	uint8_t has_max_sessions;
	uint8_t has_payload_preview_bytes;
};

struct kn_config_transmit {
	struct kn_tx_policy policy;
	uint8_t has_block;
	uint8_t has_enabled;
	uint8_t has_dry_run;
	uint8_t has_max_queued;
	uint8_t has_max_payload_bytes;
	uint8_t has_payload_preview_bytes;
	uint8_t has_allow_ui;
	uint8_t has_allow_control_enqueue;
	uint8_t has_allow_shell_enqueue;
	uint8_t has_dispatch_enabled;
	uint8_t has_dispatch_test_only;
	uint8_t has_dispatch_real_kiss;
	uint8_t has_require_explicit_port_tx;
	uint8_t has_dispatch_max_per_cycle;
};

struct kn_config_rf_command {
	char accept_destinations[KN_CONFIG_RF_COMMAND_DESTS_MAX]
	    [KN_CONFIG_RF_COMMAND_DEST_MAX];
	size_t accept_destination_count;
	size_t max_events;
	size_t max_command_bytes;
	size_t max_reply_bytes;
	size_t rate_limit_commands;
	uint64_t rate_limit_window_seconds;
	size_t reply_rate_limit_commands;
	uint64_t reply_rate_limit_window_seconds;
	size_t auto_ignore_after_rejects;
	uint64_t auto_ignore_seconds;
	char ignore_list_path[KN_CONFIG_PATH_MAX];
	uint8_t enabled;
	uint8_t reply_enabled;
	uint8_t require_node_destination;
	uint8_t rate_limit_enabled;
	uint8_t auto_ignore_enabled;
	uint8_t has_block;
	uint8_t has_enabled;
	uint8_t has_reply_enabled;
	uint8_t has_max_events;
	uint8_t has_max_command_bytes;
	uint8_t has_max_reply_bytes;
	uint8_t has_accept_destinations;
	uint8_t has_require_node_destination;
	uint8_t has_rate_limit_enabled;
	uint8_t has_rate_limit_commands;
	uint8_t has_rate_limit_window_seconds;
	uint8_t has_reply_rate_limit_commands;
	uint8_t has_reply_rate_limit_window_seconds;
	uint8_t has_auto_ignore_enabled;
	uint8_t has_auto_ignore_after_rejects;
	uint8_t has_auto_ignore_seconds;
	uint8_t has_ignore_list_path;
};

struct kn_config_shell {
	char host[KN_CONFIG_HOST_MAX];
	char port[KN_CONFIG_PORT_NAME_MAX];
	char banner[KN_CONFIG_SHELL_BANNER_MAX];
	size_t max_clients;
	uint8_t enabled;
	uint8_t has_block;
	uint8_t has_enabled;
	uint8_t has_host;
	uint8_t has_port;
	uint8_t has_max_clients;
	uint8_t has_banner;
};

struct kn_config_port {
	char name[KN_CONFIG_PORT_NAME_MAX];
	enum kn_config_port_type type;
	char host[KN_CONFIG_HOST_MAX];
	char port[KN_CONFIG_PORT_NAME_MAX];
	char device[KN_CONFIG_PATH_MAX];
	char path[KN_CONFIG_PATH_MAX];
	unsigned int baud;
	size_t max_frame;
	uint8_t enabled;
	uint8_t tx_enabled;
	uint8_t flow_control;
	uint8_t has_type;
	uint8_t has_host;
	uint8_t has_port;
	uint8_t has_device;
	uint8_t has_path;
	uint8_t has_baud;
	uint8_t has_max_frame;
	uint8_t has_enabled;
	uint8_t has_tx_enabled;
	uint8_t has_flow_control;
};

struct kn_config {
	struct kn_config_node node;
	struct kn_config_access access;
	struct kn_config_control control;
	struct kn_config_bbs bbs;
	struct kn_config_heard heard;
	struct kn_config_receive receive;
	struct kn_config_transmit transmit;
	struct kn_config_rf_command rf_command;
	struct kn_config_shell shell;
	struct kn_config_port ports[KN_CONFIG_PORT_MAX];
	size_t port_count;
	enum kn_config_error error;
	size_t error_line;
	char error_text[KN_CONFIG_ERROR_MAX];
};

const char *kn_config_error_name(enum kn_config_error);
void kn_config_free(struct kn_config *);
void kn_config_init(struct kn_config *);
enum kn_config_error kn_config_parse_file(const char *, struct kn_config *);
enum kn_config_error kn_config_parse_text(const char *, struct kn_config *);
enum kn_transport_kind kn_config_port_transport_kind(
	const struct kn_config_port *);

#endif
