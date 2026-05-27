/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_node_command_dispatch.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "kilonode/callsign.h"
#include "kilonode/node_command_dispatch.h"

static void context_init(struct kn_node_command_context *,
	struct kn_config_node *, struct kn_daemon_stats *,
	struct kn_port_stats *, struct kn_heard_entry *,
	struct kn_node_command_user *);
static int dispatch_local(const char *, struct kn_node_command_context *,
	struct kn_node_command_dispatch_result *);
static int dispatch_rf(const char *, struct kn_node_command_context *,
	struct kn_node_command_dispatch_result *);
static int test_bbs_local(void);
static int test_bye_local(void);
static int test_forbidden_rf(void);
static int test_heard_local(void);
static int test_help_info_local(void);
static int test_ping_rf(void);
static int test_ports_stats_users(void);
static int test_unknown_rf(void);

int
main(void)
{
	if (test_help_info_local() != 0)
		return 1;
	if (test_ports_stats_users() != 0)
		return 1;
	if (test_heard_local() != 0)
		return 1;
	if (test_bbs_local() != 0)
		return 1;
	if (test_bye_local() != 0)
		return 1;
	if (test_ping_rf() != 0)
		return 1;
	if (test_forbidden_rf() != 0)
		return 1;
	if (test_unknown_rf() != 0)
		return 1;
	return 0;
}

static void
context_init(struct kn_node_command_context *context,
	struct kn_config_node *node, struct kn_daemon_stats *stats,
	struct kn_port_stats *port, struct kn_heard_entry *heard,
	struct kn_node_command_user *user)
{
	kn_node_command_context_clear(context);
	memset(node, 0, sizeof(*node));
	memset(stats, 0, sizeof(*stats));
	(void)kn_callsign_parse("M6VPN-1", &node->callsign);
	(void)snprintf(node->alias, sizeof(node->alias), "KILON");
	(void)snprintf(node->location, sizeof(node->location), "Test");
	node->has_callsign = 1;
	node->has_alias = 1;
	node->has_location = 1;
	context->node = node;
	context->daemon = stats;
	context->output_limit = KN_NODE_COMMAND_OUTPUT_MAX;
	if (port != NULL) {
		memset(port, 0, sizeof(*port));
		(void)snprintf(port->name, sizeof(port->name), "kiss0");
		port->type = KN_CONFIG_PORT_TCP_LISTEN;
		port->enabled = 1;
		port->open = 1;
		context->ports = port;
		context->port_count = 1;
	}
	if (heard != NULL) {
		memset(heard, 0, sizeof(*heard));
		(void)kn_callsign_parse("N0CALL", &heard->source);
		(void)kn_callsign_parse("CQ", &heard->last_destination);
		(void)snprintf(heard->port_name, sizeof(heard->port_name),
		    "kiss0");
		heard->frame_count = 1;
		context->heard = heard;
		context->heard_count = 1;
	}
	if (user != NULL) {
		memset(user, 0, sizeof(*user));
		(void)snprintf(user->remote, sizeof(user->remote),
		    "127.0.0.1");
		user->command_count = 3;
		context->users = user;
		context->user_count = 1;
	}
}

static int
dispatch_local(const char *line, struct kn_node_command_context *context,
	struct kn_node_command_dispatch_result *result)
{
	return kn_node_command_dispatch(KN_NODE_COMMAND_CONTEXT_LOCAL, context,
	    (const uint8_t *)line, strlen(line), 512, result) ==
	    KN_NODE_COMMAND_DISPATCH_OK ? 0 : 1;
}

static int
dispatch_rf(const char *line, struct kn_node_command_context *context,
	struct kn_node_command_dispatch_result *result)
{
	enum kn_node_command_dispatch_status rc;

	rc = kn_node_command_dispatch(KN_NODE_COMMAND_CONTEXT_RF_UI, context,
	    (const uint8_t *)line, strlen(line), 128, result);
	return rc == KN_NODE_COMMAND_DISPATCH_OK ||
	    rc == KN_NODE_COMMAND_DISPATCH_UNKNOWN ||
	    rc == KN_NODE_COMMAND_DISPATCH_FORBIDDEN_CONTEXT ? 0 : 1;
}

static int
test_bbs_local(void)
{
	struct kn_node_command_context context;
	struct kn_config_node node;
	struct kn_daemon_stats stats;
	struct kn_node_command_dispatch_result result;

	context_init(&context, &node, &stats, NULL, NULL, NULL);
	context.bbs_enabled = 1;
	if (dispatch_local("BBS M6VPN-1", &context, &result) != 0)
		return 1;
	if (result.mode_transition == 0)
		return 1;
	return strstr(result.output, "use-session-command") != NULL ? 0 : 1;
}

static int
test_bye_local(void)
{
	struct kn_node_command_context context;
	struct kn_config_node node;
	struct kn_daemon_stats stats;
	struct kn_node_command_dispatch_result result;

	context_init(&context, &node, &stats, NULL, NULL, NULL);
	if (dispatch_local("BYE", &context, &result) != 0)
		return 1;
	if (result.close_session == 0)
		return 1;
	return strcmp(result.output, "BYE\r\n") == 0 ? 0 : 1;
}

static int
test_forbidden_rf(void)
{
	struct kn_node_command_context context;
	struct kn_config_node node;
	struct kn_daemon_stats stats;
	struct kn_node_command_dispatch_result result;

	context_init(&context, &node, &stats, NULL, NULL, NULL);
	if (dispatch_rf("BBS M6VPN-1", &context, &result) != 0)
		return 1;
	return result.status == KN_NODE_COMMAND_DISPATCH_FORBIDDEN_CONTEXT ?
	    0 : 1;
}

static int
test_heard_local(void)
{
	struct kn_node_command_context context;
	struct kn_config_node node;
	struct kn_daemon_stats stats;
	struct kn_heard_entry heard;
	struct kn_node_command_dispatch_result result;

	context_init(&context, &node, &stats, NULL, &heard, NULL);
	if (dispatch_local("HEARD", &context, &result) != 0)
		return 1;
	return strstr(result.output, "call=N0CALL") != NULL ? 0 : 1;
}

static int
test_help_info_local(void)
{
	struct kn_node_command_context context;
	struct kn_config_node node;
	struct kn_daemon_stats stats;
	struct kn_node_command_dispatch_result result;

	context_init(&context, &node, &stats, NULL, NULL, NULL);
	if (dispatch_local("HELP", &context, &result) != 0)
		return 1;
	if (strstr(result.output, "OK HELP") == NULL)
		return 1;
	if (dispatch_local("INFO", &context, &result) != 0)
		return 1;
	return strstr(result.output, "call=M6VPN-1 alias=KILON") != NULL ?
	    0 : 1;
}

static int
test_ping_rf(void)
{
	struct kn_node_command_context context;
	struct kn_config_node node;
	struct kn_daemon_stats stats;
	struct kn_node_command_dispatch_result result;

	context_init(&context, &node, &stats, NULL, NULL, NULL);
	if (dispatch_rf("PING", &context, &result) != 0)
		return 1;
	return strcmp(result.output, "PONG") == 0 ? 0 : 1;
}

static int
test_ports_stats_users(void)
{
	struct kn_node_command_context context;
	struct kn_config_node node;
	struct kn_daemon_stats stats;
	struct kn_port_stats port;
	struct kn_node_command_user user;
	struct kn_node_command_dispatch_result result;

	context_init(&context, &node, &stats, &port, NULL, &user);
	stats.bytes_received = 5;
	stats.kiss_frames_received = 2;
	if (dispatch_local("PORTS", &context, &result) != 0)
		return 1;
	if (strstr(result.output, "PORT name=kiss0") == NULL)
		return 1;
	if (dispatch_local("STATS", &context, &result) != 0)
		return 1;
	if (strstr(result.output, "rx_bytes=5") == NULL)
		return 1;
	if (dispatch_local("USERS", &context, &result) != 0)
		return 1;
	return strstr(result.output, "remote=127.0.0.1") != NULL ? 0 : 1;
}

static int
test_unknown_rf(void)
{
	struct kn_node_command_context context;
	struct kn_config_node node;
	struct kn_daemon_stats stats;
	struct kn_node_command_dispatch_result result;

	context_init(&context, &node, &stats, NULL, NULL, NULL);
	if (dispatch_rf("NOPE", &context, &result) != 0)
		return 1;
	if (result.status != KN_NODE_COMMAND_DISPATCH_UNKNOWN)
		return 1;
	return strcmp(result.output, "ERR unknown command") == 0 ? 0 : 1;
}
