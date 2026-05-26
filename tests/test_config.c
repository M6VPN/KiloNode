/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_config.c */

#include <sys/types.h>

#include <string.h>

#include "kilonode/config.h"

static int expect_error(const char *, enum kn_config_error);
static int test_comment_handling(void);
static int test_control_block(void);
static int test_duplicate_control_block(void);
static int test_duplicate_heard_block(void);
static int test_heard_block(void);
static int test_invalid_heard_max_entries(void);
static int test_duplicate_port_name(void);
static int test_invalid_baud(void);
static int test_invalid_callsign(void);
static int test_invalid_max_frame(void);
static int test_invalid_shell_max_clients(void);
static int test_invalid_shell_port(void);
static int test_line_number_error(void);
static int test_minimal_valid_config(void);
static int test_missing_control_path(void);
static int test_missing_node_callsign(void);
static int test_missing_port_type(void);
static int test_missing_shell_host(void);
static int test_missing_shell_port(void);
static int test_multiple_ports_disabled(void);
static int test_one_tcp_listen_port(void);
static int test_quoted_string(void);
static int test_shell_block(void);
static int test_shell_duplicate_block(void);
static int test_shell_omitted_disabled(void);
static int test_shell_unknown_key(void);
static int test_unknown_block(void);
static int test_unknown_key(void);

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
expect_error(const char *text, enum kn_config_error expected)
{
	struct kn_config config;

	return kn_config_parse_text(text, &config) == expected ? 0 : 1;
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
