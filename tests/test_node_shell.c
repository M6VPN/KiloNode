/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_node_shell.c */

#define _XOPEN_SOURCE 700

#include <sys/stat.h>
#include <sys/types.h>

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "kilonode/callsign.h"
#include "kilonode/config.h"
#include "kilonode/heard.h"
#include "kilonode/message_store.h"
#include "kilonode/node_shell.h"

static void snapshot_init(struct kn_node_shell_snapshot *,
	struct kn_config_node *, struct kn_daemon_stats *,
	struct kn_port_stats *, size_t, struct kn_heard_entry *, size_t,
	struct kn_node_shell_user *, size_t);
static void cleanup_store(const char *);
static int make_store(char *, size_t);
static int run_command(const char *, struct kn_node_shell_snapshot *, char *,
	size_t, uint8_t *);
static int run_session_command(struct kn_node_shell_session *, const char *,
	struct kn_node_shell_snapshot *, char *, size_t, uint8_t *);
static int test_bye_command(void);
static int test_bbs_command_changes_mode(void);
static int test_control_characters_sanitized(void);
static int test_empty_line(void);
static int test_exit_from_bbs_mode(void);
static int test_heard_empty(void);
static int test_heard_one_entry(void);
static int test_help_bbs_enabled(void);
static int test_help_command(void);
static int test_info_command(void);
static int test_lowercase_command(void);
static int test_policy_command_too_long(void);
static int test_overlong_line(void);
static int test_ports_zero_and_one(void);
static int test_quit_command(void);
static int test_stats_command(void);
static int test_unknown_command(void);
static int test_users_one_session(void);

int
main(void)
{
	if (test_help_command() != 0)
		return 1;
	if (test_help_bbs_enabled() != 0)
		return 1;
	if (test_bbs_command_changes_mode() != 0)
		return 1;
	if (test_exit_from_bbs_mode() != 0)
		return 1;
	if (test_info_command() != 0)
		return 1;
	if (test_ports_zero_and_one() != 0)
		return 1;
	if (test_heard_empty() != 0)
		return 1;
	if (test_heard_one_entry() != 0)
		return 1;
	if (test_users_one_session() != 0)
		return 1;
	if (test_stats_command() != 0)
		return 1;
	if (test_bye_command() != 0)
		return 1;
	if (test_quit_command() != 0)
		return 1;
	if (test_unknown_command() != 0)
		return 1;
	if (test_lowercase_command() != 0)
		return 1;
	if (test_empty_line() != 0)
		return 1;
	if (test_overlong_line() != 0)
		return 1;
	if (test_policy_command_too_long() != 0)
		return 1;
	if (test_control_characters_sanitized() != 0)
		return 1;

	return 0;
}

static void
cleanup_store(const char *dir)
{
	char path[512];

	(void)snprintf(path, sizeof(path), "%s/users/M6VPN-1.user", dir);
	(void)unlink(path);
	(void)snprintf(path, sizeof(path), "%s/users/M6VPN-1.user.tmp", dir);
	(void)unlink(path);
	(void)snprintf(path, sizeof(path), "%s/users", dir);
	(void)rmdir(path);
	(void)snprintf(path, sizeof(path), "%s/read", dir);
	(void)rmdir(path);
	(void)snprintf(path, sizeof(path), "%s/index/all.idx", dir);
	(void)unlink(path);
	(void)snprintf(path, sizeof(path), "%s/index/private.idx", dir);
	(void)unlink(path);
	(void)snprintf(path, sizeof(path), "%s/index/bulletin.idx", dir);
	(void)unlink(path);
	(void)snprintf(path, sizeof(path), "%s/index", dir);
	(void)rmdir(path);
	(void)snprintf(path, sizeof(path), "%s/meta/next-id", dir);
	(void)unlink(path);
	(void)snprintf(path, sizeof(path), "%s/meta/next-id.tmp", dir);
	(void)unlink(path);
	(void)snprintf(path, sizeof(path), "%s/meta", dir);
	(void)rmdir(path);
	(void)snprintf(path, sizeof(path), "%s/msg", dir);
	(void)rmdir(path);
	(void)rmdir(dir);
}

static int
make_store(char *buf, size_t bufsiz)
{
	unsigned int i;
	int needed;

	for (i = 0; i < 100; i++) {
		needed = snprintf(buf, bufsiz, "/tmp/kilonode-node-%ld-%u",
		    (long)getpid(), i);
		if (needed < 0 || (size_t)needed >= bufsiz)
			return 1;
		if (mkdir(buf, 0700) == 0)
			return 0;
		if (errno != EEXIST)
			return 1;
	}

	return 1;
}

static void
snapshot_init(struct kn_node_shell_snapshot *snapshot,
	struct kn_config_node *node, struct kn_daemon_stats *daemon,
	struct kn_port_stats *ports, size_t port_count,
	struct kn_heard_entry *heard, size_t heard_count,
	struct kn_node_shell_user *users, size_t user_count)
{
	memset(snapshot, 0, sizeof(*snapshot));
	memset(node, 0, sizeof(*node));
	memset(daemon, 0, sizeof(*daemon));
	(void)kn_callsign_parse("M6VPN-1", &node->callsign);
	memcpy(node->alias, "KILON", 6);
	memcpy(node->location, "Test node", 10);
	node->has_callsign = 1;
	node->has_alias = 1;
	node->has_location = 1;
	snapshot->node = node;
	snapshot->daemon = daemon;
	snapshot->ports = ports;
	snapshot->port_count = port_count;
	snapshot->heard = heard;
	snapshot->heard_count = heard_count;
	snapshot->users = users;
	snapshot->user_count = user_count;
	snapshot->bbs.enabled = 0;
	snapshot->bbs.store = NULL;
	snapshot->bbs.max_body_bytes = KN_MESSAGE_BODY_MAX;
}

static int
run_command(const char *command, struct kn_node_shell_snapshot *snapshot,
	char *out, size_t out_len, uint8_t *close_session)
{
	return kn_node_shell_format_command(command, snapshot, out, out_len,
	    close_session) == KN_NODE_SHELL_OK ? 0 : 1;
}

static int
run_session_command(struct kn_node_shell_session *session, const char *command,
	struct kn_node_shell_snapshot *snapshot, char *out, size_t out_len,
	uint8_t *close_session)
{
	return kn_node_shell_format_session_command(session, command, snapshot,
	    out, out_len, close_session) == KN_NODE_SHELL_OK ? 0 : 1;
}

static int
test_bye_command(void)
{
	struct kn_node_shell_snapshot snapshot;
	struct kn_config_node node;
	struct kn_daemon_stats daemon;
	char out[KN_NODE_SHELL_RESPONSE_MAX];
	uint8_t close_session;

	snapshot_init(&snapshot, &node, &daemon, NULL, 0, NULL, 0, NULL, 0);
	if (run_command("BYE", &snapshot, out, sizeof(out),
	    &close_session) != 0)
		return 1;
	if (close_session != 1)
		return 1;

	return strcmp(out, "BYE\r\n") == 0 ? 0 : 1;
}

static int
test_bbs_command_changes_mode(void)
{
	struct kn_node_shell_snapshot snapshot;
	struct kn_config_node node;
	struct kn_daemon_stats daemon;
	struct kn_message_store store;
	struct kn_node_shell_session session;
	char dir[256];
	char out[KN_NODE_SHELL_RESPONSE_MAX];
	uint8_t close_session;
	int rc;

	if (make_store(dir, sizeof(dir)) != 0)
		return 1;
	memset(&session, 0, sizeof(session));
	kn_message_store_init(&store);
	if (kn_message_store_open(&store, dir, KN_MESSAGE_BODY_MAX) !=
	    KN_MESSAGE_STORE_OK) {
		cleanup_store(dir);
		return 1;
	}
	snapshot_init(&snapshot, &node, &daemon, NULL, 0, NULL, 0, NULL, 0);
	snapshot.bbs.enabled = 1;
	snapshot.bbs.store = &store;
	rc = run_session_command(&session, "BBS M6VPN-1", &snapshot, out,
	    sizeof(out), &close_session) == 0 && session.bbs.active != 0 &&
	    strcmp(session.bbs.identity, "M6VPN-1") == 0 &&
	    strstr(out, "OK BBS call=M6VPN-1\r\nBBS M6VPN-1> ") != NULL ?
	    0 : 1;
	kn_message_store_close(&store);
	cleanup_store(dir);
	return rc;
}

static int
test_control_characters_sanitized(void)
{
	struct kn_node_shell_snapshot snapshot;
	struct kn_config_node node;
	struct kn_daemon_stats daemon;
	char out[KN_NODE_SHELL_RESPONSE_MAX];
	uint8_t close_session;

	snapshot_init(&snapshot, &node, &daemon, NULL, 0, NULL, 0, NULL, 0);
	memcpy(node.location, "Bad\001Place", 10);
	if (run_command("INFO", &snapshot, out, sizeof(out),
	    &close_session) != 0)
		return 1;

	return strstr(out, "Bad?Place") != NULL ? 0 : 1;
}

static int
test_empty_line(void)
{
	struct kn_node_shell_snapshot snapshot;
	struct kn_config_node node;
	struct kn_daemon_stats daemon;
	char out[KN_NODE_SHELL_RESPONSE_MAX];
	uint8_t close_session;

	snapshot_init(&snapshot, &node, &daemon, NULL, 0, NULL, 0, NULL, 0);
	if (run_command("", &snapshot, out, sizeof(out), &close_session) != 0)
		return 1;

	return strcmp(out, KN_NODE_SHELL_PROMPT) == 0 ? 0 : 1;
}

static int
test_exit_from_bbs_mode(void)
{
	struct kn_node_shell_snapshot snapshot;
	struct kn_config_node node;
	struct kn_daemon_stats daemon;
	struct kn_message_store store;
	struct kn_node_shell_session session;
	char dir[256];
	char out[KN_NODE_SHELL_RESPONSE_MAX];
	uint8_t close_session;
	int rc;

	if (make_store(dir, sizeof(dir)) != 0)
		return 1;
	memset(&session, 0, sizeof(session));
	kn_message_store_init(&store);
	if (kn_message_store_open(&store, dir, KN_MESSAGE_BODY_MAX) !=
	    KN_MESSAGE_STORE_OK) {
		cleanup_store(dir);
		return 1;
	}
	snapshot_init(&snapshot, &node, &daemon, NULL, 0, NULL, 0, NULL, 0);
	snapshot.bbs.enabled = 1;
	snapshot.bbs.store = &store;
	rc = run_session_command(&session, "BBS M6VPN-1", &snapshot, out,
	    sizeof(out), &close_session) == 0 &&
	    run_session_command(&session, "EXIT", &snapshot, out, sizeof(out),
	    &close_session) == 0 && session.bbs.active == 0 &&
	    strcmp(out, "OK EXIT\r\n" KN_NODE_SHELL_PROMPT) == 0 ? 0 : 1;
	kn_message_store_close(&store);
	cleanup_store(dir);
	return rc;
}

static int
test_heard_empty(void)
{
	struct kn_node_shell_snapshot snapshot;
	struct kn_config_node node;
	struct kn_daemon_stats daemon;
	char out[KN_NODE_SHELL_RESPONSE_MAX];
	uint8_t close_session;

	snapshot_init(&snapshot, &node, &daemon, NULL, 0, NULL, 0, NULL, 0);
	if (run_command("HEARD", &snapshot, out, sizeof(out),
	    &close_session) != 0)
		return 1;

	return strstr(out, "OK HEARD count=0\r\nEND\r\n") != NULL ? 0 : 1;
}

static int
test_heard_one_entry(void)
{
	struct kn_node_shell_snapshot snapshot;
	struct kn_config_node node;
	struct kn_daemon_stats daemon;
	struct kn_heard_entry heard;
	char out[KN_NODE_SHELL_RESPONSE_MAX];
	uint8_t close_session;

	memset(&heard, 0, sizeof(heard));
	(void)kn_callsign_parse("M6VPN-1", &heard.source);
	(void)kn_callsign_parse("CQ", &heard.last_destination);
	memcpy(heard.port_name, "kiss0", 6);
	heard.frame_count = 2;
	snapshot_init(&snapshot, &node, &daemon, NULL, 0, &heard, 1,
	    NULL, 0);
	if (run_command("HEARD PORT kiss0", &snapshot, out, sizeof(out),
	    &close_session) != 0)
		return 1;
	if (strstr(out, "OK HEARD count=1\r\n") == NULL)
		return 1;

	return strstr(out, "call=M6VPN-1") != NULL ? 0 : 1;
}

static int
test_help_bbs_enabled(void)
{
	struct kn_node_shell_snapshot snapshot;
	struct kn_config_node node;
	struct kn_daemon_stats daemon;
	struct kn_message_store store;
	char out[KN_NODE_SHELL_RESPONSE_MAX];
	uint8_t close_session;

	kn_message_store_init(&store);
	snapshot_init(&snapshot, &node, &daemon, NULL, 0, NULL, 0, NULL, 0);
	snapshot.bbs.enabled = 1;
	snapshot.bbs.store = &store;
	if (run_command("HELP", &snapshot, out, sizeof(out),
	    &close_session) != 0)
		return 1;

	return strstr(out, "STATS BBS BYE") != NULL ? 0 : 1;
}

static int
test_help_command(void)
{
	struct kn_node_shell_snapshot snapshot;
	struct kn_config_node node;
	struct kn_daemon_stats daemon;
	char out[KN_NODE_SHELL_RESPONSE_MAX];
	uint8_t close_session;

	snapshot_init(&snapshot, &node, &daemon, NULL, 0, NULL, 0, NULL, 0);
	if (run_command("HELP", &snapshot, out, sizeof(out),
	    &close_session) != 0)
		return 1;
	if (close_session != 0)
		return 1;
	if (strstr(out, "BBS(unavailable)") == NULL)
		return 1;

	return strstr(out, KN_NODE_SHELL_PROMPT) != NULL ? 0 : 1;
}

static int
test_info_command(void)
{
	struct kn_node_shell_snapshot snapshot;
	struct kn_config_node node;
	struct kn_daemon_stats daemon;
	char out[KN_NODE_SHELL_RESPONSE_MAX];
	uint8_t close_session;

	snapshot_init(&snapshot, &node, &daemon, NULL, 0, NULL, 0, NULL, 0);
	if (run_command("INFO", &snapshot, out, sizeof(out),
	    &close_session) != 0)
		return 1;
	if (strstr(out, "call=M6VPN-1 alias=KILON location=Test node") == NULL)
		return 1;

	return strstr(out, "version=") != NULL ? 0 : 1;
}

static int
test_lowercase_command(void)
{
	struct kn_node_shell_snapshot snapshot;
	struct kn_config_node node;
	struct kn_daemon_stats daemon;
	char out[KN_NODE_SHELL_RESPONSE_MAX];
	uint8_t close_session;

	snapshot_init(&snapshot, &node, &daemon, NULL, 0, NULL, 0, NULL, 0);
	if (run_command("help", &snapshot, out, sizeof(out),
	    &close_session) != 0)
		return 1;

	return strstr(out, "OK HELP") != NULL ? 0 : 1;
}

static int
test_overlong_line(void)
{
	struct kn_node_shell_snapshot snapshot;
	struct kn_config_node node;
	struct kn_daemon_stats daemon;
	char command[KN_NODE_SHELL_LINE_MAX + 1];
	char out[KN_NODE_SHELL_RESPONSE_MAX];
	uint8_t close_session;

	memset(command, 'A', sizeof(command));
	command[sizeof(command) - 1] = '\0';
	snapshot_init(&snapshot, &node, &daemon, NULL, 0, NULL, 0, NULL, 0);
	if (run_command(command, &snapshot, out, sizeof(out),
	    &close_session) != 0)
		return 1;

	return strstr(out, "ERR line-too-long") != NULL ? 0 : 1;
}

static int
test_policy_command_too_long(void)
{
	struct kn_node_shell_snapshot snapshot;
	struct kn_config_node node;
	struct kn_daemon_stats daemon;
	struct kn_access_policy policy;
	char out[KN_NODE_SHELL_RESPONSE_MAX];
	uint8_t close_session;

	snapshot_init(&snapshot, &node, &daemon, NULL, 0, NULL, 0, NULL, 0);
	kn_access_policy_defaults(&policy);
	policy.max_command_bytes = 3;
	snapshot.policy = &policy;
	if (run_command("HELP", &snapshot, out, sizeof(out),
	    &close_session) != 0)
		return 1;

	return strstr(out, "ERR command-too-long") != NULL ? 0 : 1;
}

static int
test_ports_zero_and_one(void)
{
	struct kn_node_shell_snapshot snapshot;
	struct kn_config_node node;
	struct kn_daemon_stats daemon;
	struct kn_port_stats port;
	char out[KN_NODE_SHELL_RESPONSE_MAX];
	uint8_t close_session;

	memset(&port, 0, sizeof(port));
	memcpy(port.name, "kiss0", 6);
	port.type = KN_CONFIG_PORT_TCP_LISTEN;
	port.enabled = 1;
	port.open = 1;
	snapshot_init(&snapshot, &node, &daemon, &port, 1, NULL, 0, NULL, 0);
	if (run_command("PORTS", &snapshot, out, sizeof(out),
	    &close_session) != 0)
		return 1;
	if (strstr(out, "OK PORTS count=1") == NULL)
		return 1;

	return strstr(out, "PORT name=kiss0 type=tcp-listen") != NULL ? 0 : 1;
}

static int
test_quit_command(void)
{
	struct kn_node_shell_snapshot snapshot;
	struct kn_config_node node;
	struct kn_daemon_stats daemon;
	char out[KN_NODE_SHELL_RESPONSE_MAX];
	uint8_t close_session;

	snapshot_init(&snapshot, &node, &daemon, NULL, 0, NULL, 0, NULL, 0);
	if (run_command("QUIT", &snapshot, out, sizeof(out),
	    &close_session) != 0)
		return 1;
	if (close_session != 1)
		return 1;

	return strcmp(out, "BYE\r\n") == 0 ? 0 : 1;
}

static int
test_stats_command(void)
{
	struct kn_node_shell_snapshot snapshot;
	struct kn_config_node node;
	struct kn_daemon_stats daemon;
	char out[KN_NODE_SHELL_RESPONSE_MAX];
	uint8_t close_session;

	snapshot_init(&snapshot, &node, &daemon, NULL, 0, NULL, 0, NULL, 0);
	daemon.bytes_received = 10;
	daemon.kiss_frames_received = 2;
	if (run_command("STATS", &snapshot, out, sizeof(out),
	    &close_session) != 0)
		return 1;

	return strstr(out, "OK STATS rx_bytes=10 kiss_frames=2") != NULL ?
	    0 : 1;
}

static int
test_unknown_command(void)
{
	struct kn_node_shell_snapshot snapshot;
	struct kn_config_node node;
	struct kn_daemon_stats daemon;
	char out[KN_NODE_SHELL_RESPONSE_MAX];
	uint8_t close_session;

	snapshot_init(&snapshot, &node, &daemon, NULL, 0, NULL, 0, NULL, 0);
	if (run_command("NOPE", &snapshot, out, sizeof(out),
	    &close_session) != 0)
		return 1;

	return strstr(out, "ERR unknown-command") != NULL ? 0 : 1;
}

static int
test_users_one_session(void)
{
	struct kn_node_shell_snapshot snapshot;
	struct kn_config_node node;
	struct kn_daemon_stats daemon;
	struct kn_node_shell_user user;
	char out[KN_NODE_SHELL_RESPONSE_MAX];
	uint8_t close_session;

	memset(&user, 0, sizeof(user));
	memcpy(user.remote, "127.0.0.1", 10);
	user.command_count = 3;
	user.connected_at = 100;
	snapshot_init(&snapshot, &node, &daemon, NULL, 0, NULL, 0, &user, 1);
	if (run_command("USERS", &snapshot, out, sizeof(out),
	    &close_session) != 0)
		return 1;
	if (strstr(out, "OK USERS count=1") == NULL)
		return 1;

	return strstr(out, "remote=127.0.0.1 commands=3") != NULL ? 0 : 1;
}
