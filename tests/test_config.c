/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_config.c */

#include <sys/types.h>

#include <string.h>

#include "kilonode/config.h"

static int expect_error(const char *, enum kn_config_error);
static int test_access_block(void);
static int test_access_duplicate_block(void);
static int test_access_omitted_defaults(void);
static int test_access_unknown_key(void);
static int test_ax25_duplicate_block(void);
static int test_ax25_invalid_bool(void);
static int test_ax25_invalid_live_dependency(void);
static int test_ax25_invalid_max_connections(void);
static int test_ax25_invalid_modulo(void);
static int test_ax25_omitted_defaults(void);
static int test_ax25_unknown_key(void);
static int test_ax25_valid_block(void);
static int test_bbs_disabled_block(void);
static int test_bbs_duplicate_block(void);
static int test_bbs_enabled_block(void);
static int test_bbs_omitted_disabled(void);
static int test_bbs_quoted_path(void);
static int test_bbs_unknown_key(void);
static int test_comment_handling(void);
static int test_control_block(void);
static int test_duplicate_control_block(void);
static int test_duplicate_heard_block(void);
static int test_heard_block(void);
static int test_invalid_heard_max_entries(void);
static int test_duplicate_receive_block(void);
static int test_invalid_receive_max_events(void);
static int test_invalid_receive_max_sessions(void);
static int test_invalid_receive_preview(void);
static int test_duplicate_port_name(void);
static int test_invalid_baud(void);
static int test_invalid_callsign(void);
static int test_invalid_max_frame(void);
static int test_invalid_bbs_max_body(void);
static int test_invalid_shell_max_clients(void);
static int test_invalid_shell_port(void);
static int test_invalid_access_default_policy(void);
static int test_invalid_access_line_bytes(void);
static int test_invalid_access_rate_window(void);
static int test_line_number_error(void);
static int test_minimal_valid_config(void);
static int test_missing_control_path(void);
static int test_missing_bbs_store_path(void);
static int test_missing_node_callsign(void);
static int test_missing_port_type(void);
static int test_missing_shell_host(void);
static int test_missing_shell_port(void);
static int test_multiple_ports_disabled(void);
static int test_one_tcp_listen_port(void);
static int test_quoted_string(void);
static int test_receive_block(void);
static int test_receive_omitted_defaults(void);
static int test_receive_unknown_key(void);
static int test_rf_command_block(void);
static int test_rf_command_duplicate_block(void);
static int test_rf_command_invalid_destination(void);
static int test_rf_command_invalid_ignore_path(void);
static int test_rf_command_invalid_max(void);
static int test_rf_command_invalid_rate_limit(void);
static int test_rf_command_omitted_defaults(void);
static int test_rf_command_unknown_key(void);
static int test_shell_block(void);
static int test_shell_duplicate_block(void);
static int test_shell_omitted_disabled(void);
static int test_shell_unknown_key(void);
static int test_unknown_block(void);
static int test_unknown_key(void);
static int test_transmit_allow_ui_false_control_config(void);
static int test_transmit_dry_run_false_control_rejected(void);
static int test_transmit_invalid_allow_control(void);
static int test_transmit_invalid_allow_shell(void);
static int test_transmit_invalid_dispatch_max(void);
static int test_transmit_omitted_defaults(void);
static int test_transmit_real_dispatch_config(void);
static int test_transmit_test_dispatch_config(void);
static int test_transmit_dispatch_without_enabled_rejected(void);
static int test_transmit_dispatch_not_test_only_rejected(void);
static int test_transmit_valid_control_dry_run(void);

int
main(void)
{
	if (test_minimal_valid_config() != 0)
		return 1;
	if (test_one_tcp_listen_port() != 0)
		return 1;
	if (test_multiple_ports_disabled() != 0)
		return 1;
	if (test_quoted_string() != 0)
		return 1;
	if (test_comment_handling() != 0)
		return 1;
	if (test_access_omitted_defaults() != 0)
		return 1;
	if (test_access_block() != 0)
		return 1;
	if (test_access_duplicate_block() != 0)
		return 1;
	if (test_access_unknown_key() != 0)
		return 1;
	if (test_ax25_omitted_defaults() != 0)
		return 1;
	if (test_ax25_valid_block() != 0)
		return 1;
	if (test_ax25_duplicate_block() != 0)
		return 1;
	if (test_ax25_unknown_key() != 0)
		return 1;
	if (test_ax25_invalid_bool() != 0)
		return 1;
	if (test_ax25_invalid_live_dependency() != 0)
		return 1;
	if (test_ax25_invalid_max_connections() != 0)
		return 1;
	if (test_ax25_invalid_modulo() != 0)
		return 1;
	if (test_invalid_access_default_policy() != 0)
		return 1;
	if (test_invalid_access_line_bytes() != 0)
		return 1;
	if (test_invalid_access_rate_window() != 0)
		return 1;
	if (test_control_block() != 0)
		return 1;
	if (test_duplicate_control_block() != 0)
		return 1;
	if (test_heard_block() != 0)
		return 1;
	if (test_duplicate_heard_block() != 0)
		return 1;
	if (test_invalid_heard_max_entries() != 0)
		return 1;
	if (test_receive_omitted_defaults() != 0)
		return 1;
	if (test_receive_block() != 0)
		return 1;
	if (test_duplicate_receive_block() != 0)
		return 1;
	if (test_receive_unknown_key() != 0)
		return 1;
	if (test_invalid_receive_max_events() != 0)
		return 1;
	if (test_invalid_receive_max_sessions() != 0)
		return 1;
	if (test_invalid_receive_preview() != 0)
		return 1;
	if (test_rf_command_omitted_defaults() != 0)
		return 1;
	if (test_rf_command_block() != 0)
		return 1;
	if (test_rf_command_duplicate_block() != 0)
		return 1;
	if (test_rf_command_unknown_key() != 0)
		return 1;
	if (test_rf_command_invalid_max() != 0)
		return 1;
	if (test_rf_command_invalid_destination() != 0)
		return 1;
	if (test_rf_command_invalid_rate_limit() != 0)
		return 1;
	if (test_rf_command_invalid_ignore_path() != 0)
		return 1;
	if (test_transmit_omitted_defaults() != 0)
		return 1;
	if (test_transmit_valid_control_dry_run() != 0)
		return 1;
	if (test_transmit_dry_run_false_control_rejected() != 0)
		return 1;
	if (test_transmit_invalid_allow_control() != 0)
		return 1;
	if (test_transmit_invalid_allow_shell() != 0)
		return 1;
	if (test_transmit_allow_ui_false_control_config() != 0)
		return 1;
	if (test_transmit_test_dispatch_config() != 0)
		return 1;
	if (test_transmit_real_dispatch_config() != 0)
		return 1;
	if (test_transmit_dispatch_without_enabled_rejected() != 0)
		return 1;
	if (test_transmit_dispatch_not_test_only_rejected() != 0)
		return 1;
	if (test_transmit_invalid_dispatch_max() != 0)
		return 1;
	if (test_duplicate_port_name() != 0)
		return 1;
	if (test_unknown_block() != 0)
		return 1;
	if (test_unknown_key() != 0)
		return 1;
	if (test_missing_node_callsign() != 0)
		return 1;
	if (test_missing_port_type() != 0)
		return 1;
	if (test_invalid_callsign() != 0)
		return 1;
	if (test_invalid_baud() != 0)
		return 1;
	if (test_invalid_max_frame() != 0)
		return 1;
	if (test_bbs_omitted_disabled() != 0)
		return 1;
	if (test_bbs_disabled_block() != 0)
		return 1;
	if (test_bbs_enabled_block() != 0)
		return 1;
	if (test_missing_bbs_store_path() != 0)
		return 1;
	if (test_bbs_duplicate_block() != 0)
		return 1;
	if (test_bbs_unknown_key() != 0)
		return 1;
	if (test_invalid_bbs_max_body() != 0)
		return 1;
	if (test_bbs_quoted_path() != 0)
		return 1;
	if (test_shell_omitted_disabled() != 0)
		return 1;
	if (test_shell_block() != 0)
		return 1;
	if (test_invalid_shell_port() != 0)
		return 1;
	if (test_invalid_shell_max_clients() != 0)
		return 1;
	if (test_missing_shell_host() != 0)
		return 1;
	if (test_missing_shell_port() != 0)
		return 1;
	if (test_shell_duplicate_block() != 0)
		return 1;
	if (test_shell_unknown_key() != 0)
		return 1;
	if (test_line_number_error() != 0)
		return 1;
	if (test_missing_control_path() != 0)
		return 1;

	return 0;
}

static int
test_access_block(void)
{
	struct kn_config config;
	const char text[] =
		"node {\n"
		"callsign M6VPN-1\n"
		"}\n"
		"access {\n"
		"default-policy deny\n"
		"allow-localhost true\n"
		"max-line-bytes 128\n"
		"max-command-bytes 128\n"
		"max-clients 2\n"
		"idle-timeout-seconds 60\n"
		"input-rate-lines 5\n"
		"input-rate-window-seconds 10\n"
		"bbs-max-body-bytes 1024\n"
		"control-max-command-bytes 128\n"
		"control-max-response-lines 20\n"
		"}\n";

	if (kn_config_parse_text(text, &config) != KN_CONFIG_OK)
		return 1;
	if (config.access.policy.default_policy != KN_ACCESS_POLICY_DENY)
		return 1;
	if (config.access.policy.max_line_bytes != 128)
		return 1;
	if (config.access.policy.input_rate_window_seconds != 10)
		return 1;
	return config.access.policy.control_max_response_lines == 20 ? 0 : 1;
}

static int
test_access_duplicate_block(void)
{
	const char text[] =
		"node {\n"
		"callsign M6VPN-1\n"
		"}\n"
		"access {\n"
		"max-line-bytes 128\n"
		"}\n"
		"access {\n"
		"max-line-bytes 128\n"
		"}\n";

	return expect_error(text, KN_CONFIG_ERR_DUPLICATE_KEY);
}

static int
test_access_omitted_defaults(void)
{
	struct kn_config config;
	const char text[] =
		"node {\n"
		"callsign M6VPN-1\n"
		"}\n";

	if (kn_config_parse_text(text, &config) != KN_CONFIG_OK)
		return 1;
	if (config.access.has_block != 0)
		return 1;
	if (config.access.policy.max_line_bytes != KN_ACCESS_LINE_MAX)
		return 1;
	return config.access.policy.allow_localhost == 1 ? 0 : 1;
}

static int
test_access_unknown_key(void)
{
	const char text[] =
		"node {\n"
		"callsign M6VPN-1\n"
		"}\n"
		"access {\n"
		"bad value\n"
		"}\n";

	return expect_error(text, KN_CONFIG_ERR_UNKNOWN_KEY);
}

static int
expect_error(const char *text, enum kn_config_error expected)
{
	struct kn_config config;

	return kn_config_parse_text(text, &config) == expected ? 0 : 1;
}

static int
test_ax25_duplicate_block(void)
{
	const char text[] =
		"node {\n"
		"callsign M6VPN-1\n"
		"}\n"
		"ax25 {\n"
		"enabled false\n"
		"}\n"
		"ax25 {\n"
		"enabled false\n"
		"}\n";

	return expect_error(text, KN_CONFIG_ERR_DUPLICATE_KEY);
}

static int
test_ax25_invalid_bool(void)
{
	const char text[] =
		"node {\n"
		"callsign M6VPN-1\n"
		"}\n"
		"ax25 {\n"
		"enabled maybe\n"
		"}\n";

	return expect_error(text, KN_CONFIG_ERR_INVALID_VALUE);
}

static int
test_ax25_invalid_live_dependency(void)
{
	const char text[] =
		"node {\n"
		"callsign M6VPN-1\n"
		"}\n"
		"ax25 {\n"
		"enabled true\n"
		"live-rx-create-connections true\n"
		"}\n";

	return expect_error(text, KN_CONFIG_ERR_INVALID_VALUE);
}

static int
test_ax25_invalid_max_connections(void)
{
	const char text[] =
		"node {\n"
		"callsign M6VPN-1\n"
		"}\n"
		"ax25 {\n"
		"max-connections 0\n"
		"}\n";

	return expect_error(text, KN_CONFIG_ERR_INVALID_VALUE);
}

static int
test_ax25_invalid_modulo(void)
{
	const char text[] =
		"node {\n"
		"callsign M6VPN-1\n"
		"}\n"
		"ax25 {\n"
		"modulo 16\n"
		"}\n";

	return expect_error(text, KN_CONFIG_ERR_INVALID_VALUE);
}

static int
test_ax25_omitted_defaults(void)
{
	struct kn_config config;
	const char text[] =
		"node {\n"
		"callsign M6VPN-1\n"
		"}\n";

	if (kn_config_parse_text(text, &config) != KN_CONFIG_OK)
		return 1;
	if (config.ax25.enabled != 0 || config.ax25.connected_mode != 0)
		return 1;
	if (config.ax25.live_rx_feed != 0 ||
	    config.ax25.live_rx_create_connections != 0)
		return 1;

	return config.ax25.diagnostics == 1 &&
	    config.ax25.live_rx_retain_frame_plans == 1 ? 0 : 1;
}

static int
test_ax25_unknown_key(void)
{
	const char text[] =
		"node {\n"
		"callsign M6VPN-1\n"
		"}\n"
		"ax25 {\n"
		"bad value\n"
		"}\n";

	return expect_error(text, KN_CONFIG_ERR_UNKNOWN_KEY);
}

static int
test_ax25_valid_block(void)
{
	struct kn_config config;
	const char text[] =
		"node {\n"
		"callsign M6VPN-1\n"
		"}\n"
		"ax25 {\n"
		"enabled true\n"
		"connected-mode false\n"
		"diagnostics true\n"
		"live-rx-feed true\n"
		"live-rx-create-connections true\n"
		"live-rx-retain-frame-plans true\n"
		"max-connections 8\n"
		"modulo 8\n"
		"window-size 1\n"
		"t1-ms 3000\n"
		"t2-ms 1000\n"
		"t3-ms 300000\n"
		"n2-retries 10\n"
		"}\n";

	if (kn_config_parse_text(text, &config) != KN_CONFIG_OK)
		return 1;
	if (config.ax25.enabled != 1 || config.ax25.live_rx_feed != 1)
		return 1;
	if (config.ax25.max_connections != 8)
		return 1;

	return config.ax25.params.t3_ms == 300000 ? 0 : 1;
}

static int
test_bbs_disabled_block(void)
{
	struct kn_config config;
	const char text[] =
		"node {\n"
		"callsign M6VPN-1\n"
		"}\n"
		"bbs {\n"
		"enabled false\n"
		"store-path ./var/messages\n"
		"max-body-bytes 65536\n"
		"}\n";

	if (kn_config_parse_text(text, &config) != KN_CONFIG_OK)
		return 1;
	if (config.bbs.enabled != 0)
		return 1;
	return strcmp(config.bbs.store_path, "./var/messages") == 0 ? 0 : 1;
}

static int
test_bbs_duplicate_block(void)
{
	const char text[] =
		"node {\n"
		"callsign M6VPN-1\n"
		"}\n"
		"bbs {\n"
		"enabled false\n"
		"}\n"
		"bbs {\n"
		"enabled false\n"
		"}\n";

	return expect_error(text, KN_CONFIG_ERR_DUPLICATE_KEY);
}

static int
test_bbs_enabled_block(void)
{
	struct kn_config config;
	const char text[] =
		"node {\n"
		"callsign M6VPN-1\n"
		"}\n"
		"bbs {\n"
		"enabled true\n"
		"store-path ./var/messages\n"
		"max-body-bytes 4096\n"
		"}\n";

	if (kn_config_parse_text(text, &config) != KN_CONFIG_OK)
		return 1;
	if (config.bbs.enabled != 1)
		return 1;
	if (strcmp(config.bbs.store_path, "./var/messages") != 0)
		return 1;

	return config.bbs.max_body_bytes == 4096 ? 0 : 1;
}

static int
test_bbs_omitted_disabled(void)
{
	struct kn_config config;
	const char text[] =
		"node {\n"
		"callsign M6VPN-1\n"
		"}\n";

	if (kn_config_parse_text(text, &config) != KN_CONFIG_OK)
		return 1;
	if (config.bbs.enabled != 0)
		return 1;

	return config.bbs.max_body_bytes == KN_CONFIG_BBS_BODY_MAX ? 0 : 1;
}

static int
test_bbs_quoted_path(void)
{
	struct kn_config config;
	const char text[] =
		"node {\n"
		"callsign M6VPN-1\n"
		"}\n"
		"bbs {\n"
		"enabled false\n"
		"store-path \"./var/messages test\"\n"
		"}\n";

	if (kn_config_parse_text(text, &config) != KN_CONFIG_OK)
		return 1;

	return strcmp(config.bbs.store_path, "./var/messages test") == 0 ?
	    0 : 1;
}

static int
test_bbs_unknown_key(void)
{
	const char text[] =
		"node {\n"
		"callsign M6VPN-1\n"
		"}\n"
		"bbs {\n"
		"bad value\n"
		"}\n";

	return expect_error(text, KN_CONFIG_ERR_UNKNOWN_KEY);
}

static int
test_comment_handling(void)
{
	struct kn_config config;
	const char text[] =
		"# comment\n"
		"node { # node block\n"
		"callsign M6VPN-1 # inline\n"
		"}\n";

	if (kn_config_parse_text(text, &config) != KN_CONFIG_OK)
		return 1;

	return config.node.callsign.ssid == 1 ? 0 : 1;
}

static int
test_control_block(void)
{
	struct kn_config config;
	const char text[] =
		"node {\n"
		"callsign M6VPN-1\n"
		"}\n"
		"control {\n"
		"enabled true\n"
		"path /tmp/kilonode/control.sock\n"
		"}\n";

	if (kn_config_parse_text(text, &config) != KN_CONFIG_OK)
		return 1;
	if (config.control.enabled != 1)
		return 1;

	return strcmp(config.control.path, "/tmp/kilonode/control.sock") == 0 ?
	    0 : 1;
}

static int
test_duplicate_control_block(void)
{
	const char text[] =
		"node {\n"
		"callsign M6VPN-1\n"
		"}\n"
		"control {\n"
		"enabled false\n"
		"}\n"
		"control {\n"
		"enabled false\n"
		"}\n";

	return expect_error(text, KN_CONFIG_ERR_DUPLICATE_KEY);
}

static int
test_duplicate_heard_block(void)
{
	const char text[] =
		"node {\n"
		"callsign M6VPN-1\n"
		"}\n"
		"heard {\n"
		"enabled true\n"
		"}\n"
		"heard {\n"
		"enabled true\n"
		"}\n";

	return expect_error(text, KN_CONFIG_ERR_DUPLICATE_KEY);
}

static int
test_heard_block(void)
{
	struct kn_config config;
	const char text[] =
		"node {\n"
		"callsign M6VPN-1\n"
		"}\n"
		"heard {\n"
		"enabled true\n"
		"max-entries 64\n"
		"}\n";

	if (kn_config_parse_text(text, &config) != KN_CONFIG_OK)
		return 1;
	if (config.heard.enabled != 1)
		return 1;

	return config.heard.max_entries == 64 ? 0 : 1;
}

static int
test_duplicate_receive_block(void)
{
	const char text[] =
		"node {\n"
		"callsign M6VPN-1\n"
		"}\n"
		"receive {\n"
		"max-events 10\n"
		"}\n"
		"receive {\n"
		"max-events 10\n"
		"}\n";

	return expect_error(text, KN_CONFIG_ERR_DUPLICATE_KEY);
}

static int
test_receive_block(void)
{
	struct kn_config config;
	const char text[] =
		"node {\n"
		"callsign M6VPN-1\n"
		"}\n"
		"receive {\n"
		"events-enabled false\n"
		"max-events 32\n"
		"max-sessions 16\n"
		"payload-preview-bytes 40\n"
		"}\n";

	if (kn_config_parse_text(text, &config) != KN_CONFIG_OK)
		return 1;
	if (config.receive.events_enabled != 0)
		return 1;
	if (config.receive.max_events != 32)
		return 1;
	if (config.receive.max_sessions != 16)
		return 1;

	return config.receive.payload_preview_bytes == 40 ? 0 : 1;
}

static int
test_receive_omitted_defaults(void)
{
	struct kn_config config;
	const char text[] =
		"node {\n"
		"callsign M6VPN-1\n"
		"}\n";

	if (kn_config_parse_text(text, &config) != KN_CONFIG_OK)
		return 1;
	if (config.receive.has_block != 0)
		return 1;
	if (config.receive.events_enabled != 1)
		return 1;
	return config.receive.max_events == KN_CONFIG_RECEIVE_EVENTS_MAX ? 0 : 1;
}

static int
test_receive_unknown_key(void)
{
	const char text[] =
		"node {\n"
		"callsign M6VPN-1\n"
		"}\n"
		"receive {\n"
		"unknown true\n"
		"}\n";

	return expect_error(text, KN_CONFIG_ERR_UNKNOWN_KEY);
}

static int
test_rf_command_block(void)
{
	struct kn_config config;
	const char text[] =
		"node {\n"
		"callsign M6VPN-1\n"
		"}\n"
		"rf-command {\n"
		"enabled true\n"
		"reply-enabled true\n"
		"max-events 32\n"
		"max-command-bytes 64\n"
		"max-reply-bytes 120\n"
		"accept-destinations node,cq\n"
		"require-node-destination true\n"
		"rate-limit-enabled true\n"
		"rate-limit-commands 6\n"
		"rate-limit-window-seconds 60\n"
		"reply-rate-limit-commands 3\n"
		"reply-rate-limit-window-seconds 60\n"
		"auto-ignore-enabled false\n"
		"auto-ignore-after-rejects 10\n"
		"auto-ignore-seconds 900\n"
		"ignore-list-path ./var/rf-ignore.txt\n"
		"}\n";

	if (kn_config_parse_text(text, &config) != KN_CONFIG_OK)
		return 1;
	if (config.rf_command.enabled != 1 ||
	    config.rf_command.reply_enabled != 1)
		return 1;
	if (config.rf_command.max_events != 32 ||
	    config.rf_command.max_command_bytes != 64 ||
	    config.rf_command.max_reply_bytes != 120)
		return 1;
	if (config.rf_command.rate_limit_commands != 6 ||
	    config.rf_command.reply_rate_limit_commands != 3 ||
	    config.rf_command.auto_ignore_after_rejects != 10)
		return 1;
	if (strcmp(config.rf_command.ignore_list_path,
	    "./var/rf-ignore.txt") != 0)
		return 1;
	if (config.rf_command.accept_destination_count != 2)
		return 1;

	return strcmp(config.rf_command.accept_destinations[0], "NODE") == 0 ?
	    0 : 1;
}

static int
test_rf_command_duplicate_block(void)
{
	const char text[] =
		"node {\n"
		"callsign M6VPN-1\n"
		"}\n"
		"rf-command {\n"
		"enabled false\n"
		"}\n"
		"rf-command {\n"
		"enabled false\n"
		"}\n";

	return expect_error(text, KN_CONFIG_ERR_DUPLICATE_KEY);
}

static int
test_rf_command_invalid_destination(void)
{
	const char text[] =
		"node {\n"
		"callsign M6VPN-1\n"
		"}\n"
		"rf-command {\n"
		"accept-destinations NODE,bad*\n"
		"}\n";

	return expect_error(text, KN_CONFIG_ERR_INVALID_VALUE);
}

static int
test_rf_command_invalid_ignore_path(void)
{
	const char text[] =
		"node {\n"
		"callsign M6VPN-1\n"
		"}\n"
		"rf-command {\n"
		"ignore-list-path ../bad\n"
		"}\n";

	return expect_error(text, KN_CONFIG_ERR_INVALID_VALUE);
}

static int
test_rf_command_invalid_max(void)
{
	const char text[] =
		"node {\n"
		"callsign M6VPN-1\n"
		"}\n"
		"rf-command {\n"
		"max-events 0\n"
		"}\n";

	return expect_error(text, KN_CONFIG_ERR_INVALID_VALUE);
}

static int
test_rf_command_invalid_rate_limit(void)
{
	const char text[] =
		"node {\n"
		"callsign M6VPN-1\n"
		"}\n"
		"rf-command {\n"
		"rate-limit-commands 0\n"
		"}\n";

	return expect_error(text, KN_CONFIG_ERR_INVALID_VALUE);
}

static int
test_rf_command_omitted_defaults(void)
{
	struct kn_config config;
	const char text[] =
		"node {\n"
		"callsign M6VPN-1\n"
		"}\n";

	if (kn_config_parse_text(text, &config) != KN_CONFIG_OK)
		return 1;
	if (config.rf_command.enabled != 0 ||
	    config.rf_command.reply_enabled != 0)
		return 1;
	if (config.rf_command.require_node_destination != 1)
		return 1;
	if (config.rf_command.rate_limit_enabled != 1 ||
	    config.rf_command.rate_limit_commands != 6 ||
	    config.rf_command.rate_limit_window_seconds != 60)
		return 1;
	if (config.rf_command.reply_rate_limit_commands != 3 ||
	    config.rf_command.reply_rate_limit_window_seconds != 60)
		return 1;
	if (config.rf_command.auto_ignore_enabled != 0 ||
	    config.rf_command.auto_ignore_after_rejects != 10 ||
	    config.rf_command.auto_ignore_seconds != 900)
		return 1;

	return config.rf_command.accept_destination_count == 2 ? 0 : 1;
}

static int
test_rf_command_unknown_key(void)
{
	const char text[] =
		"node {\n"
		"callsign M6VPN-1\n"
		"}\n"
		"rf-command {\n"
		"unknown true\n"
		"}\n";

	return expect_error(text, KN_CONFIG_ERR_UNKNOWN_KEY);
}

static int
test_duplicate_port_name(void)
{
	const char text[] =
		"node {\n"
		"callsign M6VPN-1\n"
		"}\n"
		"port kiss0 {\n"
		"type stdio\n"
		"}\n"
		"port kiss0 {\n"
		"type pty\n"
		"}\n";

	return expect_error(text, KN_CONFIG_ERR_DUPLICATE_PORT);
}

static int
test_invalid_baud(void)
{
	const char text[] =
		"node {\n"
		"callsign M6VPN-1\n"
		"}\n"
		"port kiss0 {\n"
		"type serial\n"
		"device /dev/null\n"
		"baud 12345\n"
		"}\n";

	return expect_error(text, KN_CONFIG_ERR_INVALID_VALUE);
}

static int
test_invalid_bbs_max_body(void)
{
	const char text[] =
		"node {\n"
		"callsign M6VPN-1\n"
		"}\n"
		"bbs {\n"
		"enabled false\n"
		"max-body-bytes 999999\n"
		"}\n";

	return expect_error(text, KN_CONFIG_ERR_INVALID_VALUE);
}

static int
test_invalid_callsign(void)
{
	const char text[] =
		"node {\n"
		"callsign bad\n"
		"}\n";

	return expect_error(text, KN_CONFIG_ERR_INVALID_VALUE);
}

static int
test_invalid_heard_max_entries(void)
{
	const char text[] =
		"node {\n"
		"callsign M6VPN-1\n"
		"}\n"
		"heard {\n"
		"max-entries 9999\n"
		"}\n";

	return expect_error(text, KN_CONFIG_ERR_INVALID_VALUE);
}

static int
test_invalid_receive_max_events(void)
{
	const char text[] =
		"node {\n"
		"callsign M6VPN-1\n"
		"}\n"
		"receive {\n"
		"max-events 9999\n"
		"}\n";

	return expect_error(text, KN_CONFIG_ERR_INVALID_VALUE);
}

static int
test_invalid_receive_max_sessions(void)
{
	const char text[] =
		"node {\n"
		"callsign M6VPN-1\n"
		"}\n"
		"receive {\n"
		"max-sessions 9999\n"
		"}\n";

	return expect_error(text, KN_CONFIG_ERR_INVALID_VALUE);
}

static int
test_invalid_receive_preview(void)
{
	const char text[] =
		"node {\n"
		"callsign M6VPN-1\n"
		"}\n"
		"receive {\n"
		"payload-preview-bytes 9999\n"
		"}\n";

	return expect_error(text, KN_CONFIG_ERR_INVALID_VALUE);
}

static int
test_invalid_max_frame(void)
{
	const char text[] =
		"node {\n"
		"callsign M6VPN-1\n"
		"}\n"
		"port kiss0 {\n"
		"type stdio\n"
		"max-frame 10\n"
		"}\n";

	return expect_error(text, KN_CONFIG_ERR_INVALID_VALUE);
}

static int
test_invalid_access_default_policy(void)
{
	const char text[] =
		"node {\n"
		"callsign M6VPN-1\n"
		"}\n"
		"access {\n"
		"default-policy maybe\n"
		"}\n";

	return expect_error(text, KN_CONFIG_ERR_INVALID_VALUE);
}

static int
test_invalid_access_line_bytes(void)
{
	const char text[] =
		"node {\n"
		"callsign M6VPN-1\n"
		"}\n"
		"access {\n"
		"max-line-bytes 0\n"
		"}\n";

	return expect_error(text, KN_CONFIG_ERR_INVALID_VALUE);
}

static int
test_invalid_access_rate_window(void)
{
	const char text[] =
		"node {\n"
		"callsign M6VPN-1\n"
		"}\n"
		"access {\n"
		"input-rate-window-seconds 0\n"
		"}\n";

	return expect_error(text, KN_CONFIG_ERR_INVALID_VALUE);
}

static int
test_invalid_shell_max_clients(void)
{
	const char text[] =
		"node {\n"
		"callsign M6VPN-1\n"
		"}\n"
		"shell {\n"
		"enabled true\n"
		"host 127.0.0.1\n"
		"port 8010\n"
		"max-clients 99\n"
		"}\n";

	return expect_error(text, KN_CONFIG_ERR_INVALID_VALUE);
}

static int
test_invalid_shell_port(void)
{
	const char text[] =
		"node {\n"
		"callsign M6VPN-1\n"
		"}\n"
		"shell {\n"
		"enabled true\n"
		"host 127.0.0.1\n"
		"port 0\n"
		"}\n";

	return expect_error(text, KN_CONFIG_ERR_INVALID_VALUE);
}

static int
test_line_number_error(void)
{
	struct kn_config config;
	const char text[] =
		"node {\n"
		"callsign M6VPN-1\n"
		"}\n"
		"badblock {\n"
		"}\n";

	if (kn_config_parse_text(text, &config) != KN_CONFIG_ERR_UNKNOWN_BLOCK)
		return 1;

	return config.error_line == 4 ? 0 : 1;
}

static int
test_minimal_valid_config(void)
{
	struct kn_config config;
	const char text[] =
		"node {\n"
		"callsign M6VPN-1\n"
		"}\n";

	if (kn_config_parse_text(text, &config) != KN_CONFIG_OK)
		return 1;
	if (strcmp(config.node.callsign.call, "M6VPN") != 0)
		return 1;
	if (config.node.callsign.ssid != 1)
		return 1;

	return config.port_count == 0 ? 0 : 1;
}

static int
test_missing_control_path(void)
{
	const char text[] =
		"node {\n"
		"callsign M6VPN-1\n"
		"}\n"
		"control {\n"
		"enabled true\n"
		"}\n";

	return expect_error(text, KN_CONFIG_ERR_MISSING_REQUIRED);
}

static int
test_missing_bbs_store_path(void)
{
	const char text[] =
		"node {\n"
		"callsign M6VPN-1\n"
		"}\n"
		"bbs {\n"
		"enabled true\n"
		"}\n";

	return expect_error(text, KN_CONFIG_ERR_MISSING_REQUIRED);
}

static int
test_missing_node_callsign(void)
{
	const char text[] =
		"node {\n"
		"alias KILON\n"
		"}\n";

	return expect_error(text, KN_CONFIG_ERR_MISSING_REQUIRED);
}

static int
test_missing_port_type(void)
{
	const char text[] =
		"node {\n"
		"callsign M6VPN-1\n"
		"}\n"
		"port kiss0 {\n"
		"enabled true\n"
		"}\n";

	return expect_error(text, KN_CONFIG_ERR_MISSING_REQUIRED);
}

static int
test_missing_shell_host(void)
{
	const char text[] =
		"node {\n"
		"callsign M6VPN-1\n"
		"}\n"
		"shell {\n"
		"enabled true\n"
		"port 8010\n"
		"}\n";

	return expect_error(text, KN_CONFIG_ERR_MISSING_REQUIRED);
}

static int
test_missing_shell_port(void)
{
	const char text[] =
		"node {\n"
		"callsign M6VPN-1\n"
		"}\n"
		"shell {\n"
		"enabled true\n"
		"host 127.0.0.1\n"
		"}\n";

	return expect_error(text, KN_CONFIG_ERR_MISSING_REQUIRED);
}

static int
test_multiple_ports_disabled(void)
{
	struct kn_config config;
	const char text[] =
		"node {\n"
		"callsign M6VPN-1\n"
		"}\n"
		"port kiss0 {\n"
		"type stdio\n"
		"enabled true\n"
		"}\n"
		"port kiss1 {\n"
		"type pty\n"
		"enabled false\n"
		"}\n";

	if (kn_config_parse_text(text, &config) != KN_CONFIG_OK)
		return 1;
	if (config.port_count != 2)
		return 1;
	if (config.ports[0].enabled != 1 || config.ports[1].enabled != 0)
		return 1;

	return 0;
}

static int
test_one_tcp_listen_port(void)
{
	struct kn_config config;
	const char text[] =
		"node {\n"
		"callsign M6VPN-1\n"
		"}\n"
		"port kiss0 {\n"
		"type tcp-listen\n"
		"host 127.0.0.1\n"
		"port 8001\n"
		"max-frame 2048\n"
		"enabled true\n"
		"}\n";

	if (kn_config_parse_text(text, &config) != KN_CONFIG_OK)
		return 1;
	if (config.port_count != 1)
		return 1;
	if (config.ports[0].type != KN_CONFIG_PORT_TCP_LISTEN)
		return 1;
	if (strcmp(config.ports[0].host, "127.0.0.1") != 0)
		return 1;
	if (strcmp(config.ports[0].port, "8001") != 0)
		return 1;

	return config.ports[0].max_frame == 2048 ? 0 : 1;
}

static int
test_quoted_string(void)
{
	struct kn_config config;
	const char text[] =
		"node {\n"
		"callsign M6VPN-1\n"
		"location \"Test node\"\n"
		"}\n";

	if (kn_config_parse_text(text, &config) != KN_CONFIG_OK)
		return 1;

	return strcmp(config.node.location, "Test node") == 0 ? 0 : 1;
}

static int
test_shell_block(void)
{
	struct kn_config config;
	const char text[] =
		"node {\n"
		"callsign M6VPN-1\n"
		"}\n"
		"shell {\n"
		"enabled true\n"
		"host 127.0.0.1\n"
		"port 8010\n"
		"max-clients 4\n"
		"banner \"KiloNode test shell\"\n"
		"}\n";

	if (kn_config_parse_text(text, &config) != KN_CONFIG_OK)
		return 1;
	if (config.shell.enabled != 1)
		return 1;
	if (strcmp(config.shell.host, "127.0.0.1") != 0)
		return 1;
	if (strcmp(config.shell.port, "8010") != 0)
		return 1;
	if (strcmp(config.shell.banner, "KiloNode test shell") != 0)
		return 1;

	return config.shell.max_clients == 4 ? 0 : 1;
}

static int
test_shell_duplicate_block(void)
{
	const char text[] =
		"node {\n"
		"callsign M6VPN-1\n"
		"}\n"
		"shell {\n"
		"enabled false\n"
		"}\n"
		"shell {\n"
		"enabled false\n"
		"}\n";

	return expect_error(text, KN_CONFIG_ERR_DUPLICATE_KEY);
}

static int
test_shell_omitted_disabled(void)
{
	struct kn_config config;
	const char text[] =
		"node {\n"
		"callsign M6VPN-1\n"
		"}\n";

	if (kn_config_parse_text(text, &config) != KN_CONFIG_OK)
		return 1;
	if (config.shell.enabled != 0)
		return 1;

	return config.shell.max_clients == 4 ? 0 : 1;
}

static int
test_shell_unknown_key(void)
{
	const char text[] =
		"node {\n"
		"callsign M6VPN-1\n"
		"}\n"
		"shell {\n"
		"unknown value\n"
		"}\n";

	return expect_error(text, KN_CONFIG_ERR_UNKNOWN_KEY);
}

static int
test_transmit_allow_ui_false_control_config(void)
{
	const char text[] =
		"node {\n"
		"callsign M6VPN-1\n"
		"}\n"
		"transmit {\n"
		"enabled true\n"
		"dry-run true\n"
		"allow-ui false\n"
		"allow-control-enqueue true\n"
		"}\n";
	struct kn_config config;

	if (kn_config_parse_text(text, &config) != KN_CONFIG_OK)
		return 1;

	return config.transmit.policy.allow_control_enqueue == 1 &&
	    config.transmit.policy.allow_ui == 0 ? 0 : 1;
}

static int
test_transmit_dry_run_false_control_rejected(void)
{
	const char text[] =
		"node {\n"
		"callsign M6VPN-1\n"
		"}\n"
		"transmit {\n"
		"enabled true\n"
		"dry-run false\n"
		"allow-control-enqueue true\n"
		"}\n";

	return expect_error(text, KN_CONFIG_ERR_INVALID_VALUE);
}

static int
test_transmit_invalid_allow_control(void)
{
	const char text[] =
		"node {\n"
		"callsign M6VPN-1\n"
		"}\n"
		"transmit {\n"
		"allow-control-enqueue maybe\n"
		"}\n";

	return expect_error(text, KN_CONFIG_ERR_INVALID_VALUE);
}

static int
test_transmit_invalid_allow_shell(void)
{
	const char text[] =
		"node {\n"
		"callsign M6VPN-1\n"
		"}\n"
		"transmit {\n"
		"allow-shell-enqueue maybe\n"
		"}\n";

	return expect_error(text, KN_CONFIG_ERR_INVALID_VALUE);
}

static int
test_transmit_invalid_dispatch_max(void)
{
	const char text[] =
		"node {\n"
		"callsign M6VPN-1\n"
		"}\n"
		"transmit {\n"
		"dispatch-max-per-cycle 0\n"
		"}\n";

	return expect_error(text, KN_CONFIG_ERR_INVALID_VALUE);
}

static int
test_transmit_omitted_defaults(void)
{
	struct kn_config config;
	const char text[] =
		"node {\n"
		"callsign M6VPN-1\n"
		"}\n";

	if (kn_config_parse_text(text, &config) != KN_CONFIG_OK)
		return 1;
	if (config.transmit.policy.enabled != 0)
		return 1;
	if (config.transmit.policy.dry_run == 0)
		return 1;
	if (config.transmit.policy.allow_control_enqueue != 0)
		return 1;
	if (config.transmit.policy.dispatch_enabled != 0)
		return 1;
	if (config.transmit.policy.dispatch_test_only == 0)
		return 1;
	if (config.transmit.policy.dispatch_real_kiss != 0)
		return 1;
	if (config.transmit.policy.require_explicit_port_tx == 0)
		return 1;

	return config.transmit.policy.allow_shell_enqueue == 0 &&
	    config.transmit.policy.dispatch_max_per_cycle == 4 ? 0 : 1;
}

static int
test_transmit_real_dispatch_config(void)
{
	struct kn_config config;
	const char text[] =
		"node {\n"
		"callsign M6VPN-1\n"
		"}\n"
		"transmit {\n"
		"enabled true\n"
		"dry-run false\n"
		"allow-ui true\n"
		"allow-control-enqueue true\n"
		"dispatch-enabled true\n"
		"dispatch-test-only false\n"
		"dispatch-real-kiss true\n"
		"require-explicit-port-tx true\n"
		"}\n"
		"port kiss0 {\n"
		"type tcp-connect\n"
		"host 127.0.0.1\n"
		"port 8001\n"
		"tx-enabled true\n"
		"}\n";

	if (kn_config_parse_text(text, &config) != KN_CONFIG_OK)
		return 1;
	if (config.transmit.policy.dispatch_real_kiss != 1 ||
	    config.transmit.policy.dispatch_test_only != 0)
		return 1;

	return config.ports[0].tx_enabled == 1 ? 0 : 1;
}

static int
test_transmit_test_dispatch_config(void)
{
	struct kn_config config;
	const char text[] =
		"node {\n"
		"callsign M6VPN-1\n"
		"}\n"
		"transmit {\n"
		"enabled true\n"
		"dry-run true\n"
		"allow-ui true\n"
		"allow-control-enqueue true\n"
		"dispatch-enabled true\n"
		"dispatch-test-only true\n"
		"dispatch-max-per-cycle 2\n"
		"}\n"
		"port mem0 {\n"
		"type memory-test\n"
		"enabled true\n"
		"}\n";

	if (kn_config_parse_text(text, &config) != KN_CONFIG_OK)
		return 1;
	if (config.ports[0].type != KN_CONFIG_PORT_MEMORY_TEST)
		return 1;

	return config.transmit.policy.dispatch_enabled == 1 &&
	    config.transmit.policy.dispatch_max_per_cycle == 2 ? 0 : 1;
}

static int
test_transmit_dispatch_without_enabled_rejected(void)
{
	const char text[] =
		"node {\n"
		"callsign M6VPN-1\n"
		"}\n"
		"transmit {\n"
		"enabled false\n"
		"dispatch-enabled true\n"
		"}\n";

	return expect_error(text, KN_CONFIG_ERR_INVALID_VALUE);
}

static int
test_transmit_dispatch_not_test_only_rejected(void)
{
	const char text[] =
		"node {\n"
		"callsign M6VPN-1\n"
		"}\n"
		"transmit {\n"
		"enabled true\n"
		"dispatch-enabled true\n"
		"dispatch-test-only false\n"
		"}\n";

	return expect_error(text, KN_CONFIG_ERR_INVALID_VALUE);
}

static int
test_transmit_valid_control_dry_run(void)
{
	struct kn_config config;
	const char text[] =
		"node {\n"
		"callsign M6VPN-1\n"
		"}\n"
		"transmit {\n"
		"enabled true\n"
		"dry-run true\n"
		"max-queued 16\n"
		"max-payload-bytes 128\n"
		"payload-preview-bytes 32\n"
		"allow-ui true\n"
		"allow-control-enqueue true\n"
		"allow-shell-enqueue false\n"
		"}\n";

	if (kn_config_parse_text(text, &config) != KN_CONFIG_OK)
		return 1;
	if (config.transmit.policy.enabled != 1 ||
	    config.transmit.policy.allow_ui != 1 ||
	    config.transmit.policy.allow_control_enqueue != 1)
		return 1;
	if (config.transmit.policy.max_queued != 16 ||
	    config.transmit.policy.max_payload_bytes != 128)
		return 1;

	return config.transmit.policy.payload_preview_bytes == 32 ? 0 : 1;
}

static int
test_unknown_block(void)
{
	const char text[] =
		"node {\n"
		"callsign M6VPN-1\n"
		"}\n"
		"other {\n"
		"}\n";

	return expect_error(text, KN_CONFIG_ERR_UNKNOWN_BLOCK);
}

static int
test_unknown_key(void)
{
	const char text[] =
		"node {\n"
		"callsign M6VPN-1\n"
		"badkey value\n"
		"}\n";

	return expect_error(text, KN_CONFIG_ERR_UNKNOWN_KEY);
}
