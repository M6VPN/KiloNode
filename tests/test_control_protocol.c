/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_control_protocol.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "kilonode/ax25.h"
#include "kilonode/callsign.h"
#include "kilonode/control.h"
#include "kilonode/heard.h"
#include "kilonode/rx_event.h"
#include "kilonode/rx_queue.h"
#include "kilonode/rx_session.h"
#include "kilonode/stats.h"

static void snapshot_init(struct kn_control_snapshot *,
	struct kn_daemon_stats *, struct kn_port_stats *, size_t);
static int test_bbs_areas_disabled(void);
static int test_bbs_malformed_command(void);
static int test_bbs_message_disabled(void);
static int test_bbs_messages_disabled(void);
static int test_bbs_messages_filter_disabled(void);
static int test_bbs_stats_disabled(void);
static int test_bbs_status_disabled(void);
static int test_bbs_unknown_command(void);
static int test_bbs_users_disabled(void);
static int test_control_character_command(void);
static int test_control_policy_command_limit(void);
static int test_help_response(void);
static int test_heard_empty_response(void);
static int test_heard_malformed_command(void);
static int test_heard_multiple_entries(void);
static int test_heard_one_entry(void);
static int test_heard_port_filter(void);
static int test_heard_port_unknown(void);
static int test_heard_port_overlong(void);
static int test_overlong_command(void);
static int test_ping_response(void);
static int test_ports_one_port(void);
static int test_ports_zero_ports(void);
static int test_response_line_cap(void);
static int test_rx_event_existing(void);
static int test_rx_event_missing(void);
static int test_rx_events_empty(void);
static int test_rx_events_filters(void);
static int test_rx_events_one(void);
static int test_rx_malformed_command(void);
static int test_rx_sessions_empty(void);
static int test_rx_sessions_one(void);
static int test_rx_status(void);
static int test_stats_response(void);
static int test_status_response(void);
static int test_unknown_command(void);
static int test_version_response(void);

int
main(void)
{
	if (test_ping_response() != 0)
		return 1;
	if (test_version_response() != 0)
		return 1;
	if (test_status_response() != 0)
		return 1;
	if (test_ports_zero_ports() != 0)
		return 1;
	if (test_ports_one_port() != 0)
		return 1;
	if (test_stats_response() != 0)
		return 1;
	if (test_heard_empty_response() != 0)
		return 1;
	if (test_heard_one_entry() != 0)
		return 1;
	if (test_heard_multiple_entries() != 0)
		return 1;
	if (test_heard_port_filter() != 0)
		return 1;
	if (test_heard_port_unknown() != 0)
		return 1;
	if (test_heard_malformed_command() != 0)
		return 1;
	if (test_heard_port_overlong() != 0)
		return 1;
	if (test_bbs_status_disabled() != 0)
		return 1;
	if (test_bbs_stats_disabled() != 0)
		return 1;
	if (test_bbs_areas_disabled() != 0)
		return 1;
	if (test_bbs_users_disabled() != 0)
		return 1;
	if (test_bbs_messages_disabled() != 0)
		return 1;
	if (test_bbs_messages_filter_disabled() != 0)
		return 1;
	if (test_bbs_message_disabled() != 0)
		return 1;
	if (test_bbs_unknown_command() != 0)
		return 1;
	if (test_bbs_malformed_command() != 0)
		return 1;
	if (test_help_response() != 0)
		return 1;
	if (test_unknown_command() != 0)
		return 1;
	if (test_overlong_command() != 0)
		return 1;
	if (test_control_policy_command_limit() != 0)
		return 1;
	if (test_response_line_cap() != 0)
		return 1;
	if (test_control_character_command() != 0)
		return 1;
	if (test_rx_status() != 0)
		return 1;
	if (test_rx_events_empty() != 0)
		return 1;
	if (test_rx_events_one() != 0)
		return 1;
	if (test_rx_events_filters() != 0)
		return 1;
	if (test_rx_event_existing() != 0)
		return 1;
	if (test_rx_event_missing() != 0)
		return 1;
	if (test_rx_sessions_empty() != 0)
		return 1;
	if (test_rx_sessions_one() != 0)
		return 1;
	if (test_rx_malformed_command() != 0)
		return 1;

	return 0;
}

static void
snapshot_init(struct kn_control_snapshot *snapshot,
	struct kn_daemon_stats *daemon, struct kn_port_stats *ports,
	size_t port_count)
{
	memset(daemon, 0, sizeof(*daemon));
	daemon->configured_ports = port_count;
	daemon->enabled_ports = port_count;
	daemon->open_ports = port_count;
	snapshot->daemon = daemon;
	snapshot->ports = ports;
	snapshot->port_count = port_count;
	snapshot->heard = NULL;
	snapshot->heard_count = 0;
	snapshot->bbs_enabled = 0;
	snapshot->bbs_store = NULL;
	snapshot->rx_enabled = 0;
	snapshot->rx_events = NULL;
	snapshot->rx_sessions = NULL;
	snapshot->control_max_command_bytes = 0;
	snapshot->control_max_response_lines = 0;
}

static int
test_bbs_areas_disabled(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	char out[KN_CONTROL_QUEUE_MAX];

	snapshot_init(&snapshot, &daemon, NULL, 0);
	if (kn_control_protocol_handle("BBS AREAS", &snapshot, out,
	    sizeof(out)) != KN_CONTROL_ERR_UNKNOWN_COMMAND)
		return 1;

	return strcmp(out, "ERR bbs-disabled\n") == 0 ? 0 : 1;
}

static int
test_bbs_malformed_command(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	char out[KN_CONTROL_QUEUE_MAX];

	snapshot_init(&snapshot, &daemon, NULL, 0);
	if (kn_control_protocol_handle("BBS", &snapshot, out,
	    sizeof(out)) != KN_CONTROL_ERR_UNKNOWN_COMMAND)
		return 1;

	return strcmp(out, "ERR invalid-bbs-command\n") == 0 ? 0 : 1;
}

static int
test_bbs_message_disabled(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	char out[KN_CONTROL_QUEUE_MAX];

	snapshot_init(&snapshot, &daemon, NULL, 0);
	if (kn_control_protocol_handle("BBS MESSAGE 1", &snapshot, out,
	    sizeof(out)) != KN_CONTROL_ERR_UNKNOWN_COMMAND)
		return 1;

	return strcmp(out, "ERR bbs-disabled\n") == 0 ? 0 : 1;
}

static int
test_bbs_messages_disabled(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	char out[KN_CONTROL_QUEUE_MAX];

	snapshot_init(&snapshot, &daemon, NULL, 0);
	if (kn_control_protocol_handle("BBS MESSAGES", &snapshot, out,
	    sizeof(out)) != KN_CONTROL_ERR_UNKNOWN_COMMAND)
		return 1;

	return strcmp(out, "ERR bbs-disabled\n") == 0 ? 0 : 1;
}

static int
test_bbs_messages_filter_disabled(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	char out[KN_CONTROL_QUEUE_MAX];

	snapshot_init(&snapshot, &daemon, NULL, 0);
	if (kn_control_protocol_handle("BBS MESSAGES AREA GENERAL", &snapshot,
	    out, sizeof(out)) != KN_CONTROL_ERR_UNKNOWN_COMMAND)
		return 1;

	return strcmp(out, "ERR bbs-disabled\n") == 0 ? 0 : 1;
}

static int
test_bbs_stats_disabled(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	char out[KN_CONTROL_QUEUE_MAX];

	snapshot_init(&snapshot, &daemon, NULL, 0);
	if (kn_control_protocol_handle("BBS STATS", &snapshot, out,
	    sizeof(out)) != KN_CONTROL_ERR_UNKNOWN_COMMAND)
		return 1;

	return strcmp(out, "ERR bbs-disabled\n") == 0 ? 0 : 1;
}

static int
test_bbs_status_disabled(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	char out[KN_CONTROL_QUEUE_MAX];

	snapshot_init(&snapshot, &daemon, NULL, 0);
	if (kn_control_protocol_handle("BBS STATUS", &snapshot, out,
	    sizeof(out)) != KN_CONTROL_OK)
		return 1;

	return strcmp(out,
	    "OK BBS STATUS enabled=false open=false\nEND\n") == 0 ? 0 : 1;
}

static int
test_bbs_unknown_command(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	char out[KN_CONTROL_QUEUE_MAX];

	snapshot_init(&snapshot, &daemon, NULL, 0);
	if (kn_control_protocol_handle("BBS NOPE", &snapshot, out,
	    sizeof(out)) != KN_CONTROL_ERR_UNKNOWN_COMMAND)
		return 1;

	return strcmp(out, "ERR bbs-disabled\n") == 0 ? 0 : 1;
}

static int
test_bbs_users_disabled(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	char out[KN_CONTROL_QUEUE_MAX];

	snapshot_init(&snapshot, &daemon, NULL, 0);
	if (kn_control_protocol_handle("BBS USERS", &snapshot, out,
	    sizeof(out)) != KN_CONTROL_ERR_UNKNOWN_COMMAND)
		return 1;

	return strcmp(out, "ERR bbs-disabled\n") == 0 ? 0 : 1;
}

static int
test_control_character_command(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	char out[KN_CONTROL_QUEUE_MAX];

	snapshot_init(&snapshot, &daemon, NULL, 0);
	if (kn_control_protocol_handle("BAD\001", &snapshot, out,
	    sizeof(out)) != KN_CONTROL_ERR_UNKNOWN_COMMAND)
		return 1;

	return strcmp(out, "ERR invalid-command\n") == 0 ? 0 : 1;
}

static int
test_control_policy_command_limit(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	char out[KN_CONTROL_QUEUE_MAX];

	snapshot_init(&snapshot, &daemon, NULL, 0);
	snapshot.control_max_command_bytes = 3;
	if (kn_control_protocol_handle("PING", &snapshot, out,
	    sizeof(out)) != KN_CONTROL_ERR_OVERLONG_COMMAND)
		return 1;

	return strcmp(out, "ERR overlong-command\n") == 0 ? 0 : 1;
}

static int
test_help_response(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	char out[KN_CONTROL_QUEUE_MAX];

	snapshot_init(&snapshot, &daemon, NULL, 0);
	if (kn_control_protocol_handle("HELP", &snapshot, out,
	    sizeof(out)) != KN_CONTROL_OK)
		return 1;

	return strstr(out, "END\n") != NULL ? 0 : 1;
}

static int
test_heard_empty_response(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	char out[KN_CONTROL_QUEUE_MAX];

	snapshot_init(&snapshot, &daemon, NULL, 0);
	if (kn_control_protocol_handle("HEARD", &snapshot, out,
	    sizeof(out)) != KN_CONTROL_OK)
		return 1;

	return strcmp(out, "OK HEARD count=0\nEND\n") == 0 ? 0 : 1;
}

static int
test_heard_malformed_command(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	char out[KN_CONTROL_QUEUE_MAX];

	snapshot_init(&snapshot, &daemon, NULL, 0);
	if (kn_control_protocol_handle("HEARD BAD", &snapshot, out,
	    sizeof(out)) != KN_CONTROL_ERR_UNKNOWN_COMMAND)
		return 1;

	return strcmp(out, "ERR invalid-heard-command\n") == 0 ? 0 : 1;
}

static int
test_heard_multiple_entries(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	struct kn_heard_entry entries[2];
	char out[KN_CONTROL_QUEUE_MAX];

	memset(entries, 0, sizeof(entries));
	(void)kn_callsign_parse("M6VPN-1", &entries[0].source);
	(void)kn_callsign_parse("CQ", &entries[0].last_destination);
	memcpy(entries[0].port_name, "kiss0", 6);
	entries[0].frame_count = 12;
	entries[0].last_ui = 1;
	entries[0].has_pid = 1;
	entries[0].last_pid = 0xf0;
	(void)kn_callsign_parse("N0CALL", &entries[1].source);
	(void)kn_callsign_parse("APRS", &entries[1].last_destination);
	memcpy(entries[1].port_name, "kiss1", 6);
	entries[1].frame_count = 2;
	entries[1].last_ui = 1;
	entries[1].has_pid = 1;
	entries[1].last_pid = 0xf0;
	snapshot_init(&snapshot, &daemon, NULL, 0);
	snapshot.heard = entries;
	snapshot.heard_count = 2;

	if (kn_control_protocol_handle("HEARD", &snapshot, out,
	    sizeof(out)) != KN_CONTROL_OK)
		return 1;
	if (strstr(out, "OK HEARD count=2\n") == NULL)
		return 1;
	if (strstr(out, "call=N0CALL") == NULL)
		return 1;

	return strstr(out, "END\n") != NULL ? 0 : 1;
}

static int
test_heard_one_entry(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	struct kn_heard_entry entry;
	char out[KN_CONTROL_QUEUE_MAX];

	memset(&entry, 0, sizeof(entry));
	(void)kn_callsign_parse("M6VPN-1", &entry.source);
	(void)kn_callsign_parse("CQ", &entry.last_destination);
	memcpy(entry.port_name, "kiss0", 6);
	entry.frame_count = 12;
	entry.last_ui = 1;
	entry.has_pid = 1;
	entry.last_pid = 0xf0;
	entry.last_payload_len = 23;
	entry.first_heard = 1710000000;
	entry.last_heard = 1710000300;
	(void)kn_callsign_parse("WIDE1-1", &entry.digipeaters[0].callsign);
	entry.digipeater_count = 1;
	snapshot_init(&snapshot, &daemon, NULL, 0);
	snapshot.heard = &entry;
	snapshot.heard_count = 1;

	if (kn_control_protocol_handle("HEARD", &snapshot, out,
	    sizeof(out)) != KN_CONTROL_OK)
		return 1;
	if (strstr(out, "OK HEARD count=1\n") == NULL)
		return 1;
	if (strstr(out, "HEARD port=kiss0 call=M6VPN-1") == NULL)
		return 1;
	if (strstr(out, "via=WIDE1-1") == NULL)
		return 1;

	return strstr(out, "last_pid=0xf0") != NULL ? 0 : 1;
}

static int
test_heard_port_filter(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	struct kn_heard_entry entries[2];
	char out[KN_CONTROL_QUEUE_MAX];

	memset(entries, 0, sizeof(entries));
	(void)kn_callsign_parse("M6VPN-1", &entries[0].source);
	(void)kn_callsign_parse("CQ", &entries[0].last_destination);
	memcpy(entries[0].port_name, "kiss0", 6);
	(void)kn_callsign_parse("N0CALL", &entries[1].source);
	(void)kn_callsign_parse("CQ", &entries[1].last_destination);
	memcpy(entries[1].port_name, "kiss1", 6);
	snapshot_init(&snapshot, &daemon, NULL, 0);
	snapshot.heard = entries;
	snapshot.heard_count = 2;

	if (kn_control_protocol_handle("HEARD PORT kiss1", &snapshot, out,
	    sizeof(out)) != KN_CONTROL_OK)
		return 1;
	if (strstr(out, "OK HEARD count=1\n") == NULL)
		return 1;
	if (strstr(out, "call=N0CALL") == NULL)
		return 1;

	return strstr(out, "call=M6VPN") == NULL ? 0 : 1;
}

static int
test_heard_port_overlong(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	char command[KN_CONTROL_COMMAND_MAX];
	char out[KN_CONTROL_QUEUE_MAX];

	memset(command, 'A', sizeof(command));
	memcpy(command, "HEARD PORT ", 11);
	command[sizeof(command) - 1] = '\0';
	snapshot_init(&snapshot, &daemon, NULL, 0);
	if (kn_control_protocol_handle(command, &snapshot, out,
	    sizeof(out)) != KN_CONTROL_ERR_UNKNOWN_COMMAND)
		return 1;

	return strcmp(out, "ERR invalid-heard-command\n") == 0 ? 0 : 1;
}

static int
test_heard_port_unknown(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	struct kn_heard_entry entry;
	char out[KN_CONTROL_QUEUE_MAX];

	memset(&entry, 0, sizeof(entry));
	(void)kn_callsign_parse("M6VPN-1", &entry.source);
	(void)kn_callsign_parse("CQ", &entry.last_destination);
	memcpy(entry.port_name, "kiss0", 6);
	snapshot_init(&snapshot, &daemon, NULL, 0);
	snapshot.heard = &entry;
	snapshot.heard_count = 1;

	if (kn_control_protocol_handle("HEARD PORT kiss1", &snapshot, out,
	    sizeof(out)) != KN_CONTROL_OK)
		return 1;

	return strcmp(out, "OK HEARD count=0\nEND\n") == 0 ? 0 : 1;
}

static int
test_overlong_command(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	char command[KN_CONTROL_COMMAND_MAX + 4];
	char out[KN_CONTROL_QUEUE_MAX];

	memset(command, 'A', sizeof(command));
	command[sizeof(command) - 1] = '\0';
	snapshot_init(&snapshot, &daemon, NULL, 0);
	if (kn_control_protocol_handle(command, &snapshot, out,
	    sizeof(out)) != KN_CONTROL_ERR_OVERLONG_COMMAND)
		return 1;

	return strcmp(out, "ERR overlong-command\n") == 0 ? 0 : 1;
}

static int
test_ping_response(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	char out[KN_CONTROL_QUEUE_MAX];

	snapshot_init(&snapshot, &daemon, NULL, 0);
	if (kn_control_protocol_handle("PING", &snapshot, out,
	    sizeof(out)) != KN_CONTROL_OK)
		return 1;

	return strcmp(out, "OK PONG\n") == 0 ? 0 : 1;
}

static int
test_ports_one_port(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	struct kn_port_stats port;
	char out[KN_CONTROL_QUEUE_MAX];

	memset(&port, 0, sizeof(port));
	memcpy(port.name, "kiss0", 6);
	port.type = KN_CONFIG_PORT_TCP_LISTEN;
	port.enabled = 1;
	port.open = 1;
	port.bytes_received = 1234;
	port.kiss_frames_received = 12;
	snapshot_init(&snapshot, &daemon, &port, 1);

	if (kn_control_protocol_handle("PORTS", &snapshot, out,
	    sizeof(out)) != KN_CONTROL_OK)
		return 1;
	if (strstr(out, "OK PORT name=kiss0 type=tcp-listen enabled=true") == NULL)
		return 1;

	return strstr(out, "END\n") != NULL ? 0 : 1;
}

static int
test_ports_zero_ports(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	char out[KN_CONTROL_QUEUE_MAX];

	snapshot_init(&snapshot, &daemon, NULL, 0);
	if (kn_control_protocol_handle("PORTS", &snapshot, out,
	    sizeof(out)) != KN_CONTROL_OK)
		return 1;

	return strcmp(out, "END\n") == 0 ? 0 : 1;
}

static int
test_response_line_cap(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	struct kn_heard_entry entry;
	char out[KN_CONTROL_QUEUE_MAX];

	memset(&entry, 0, sizeof(entry));
	(void)kn_callsign_parse("M6VPN-1", &entry.source);
	(void)kn_callsign_parse("CQ", &entry.last_destination);
	memcpy(entry.port_name, "kiss0", 6);
	snapshot_init(&snapshot, &daemon, NULL, 0);
	snapshot.heard = &entry;
	snapshot.heard_count = 1;
	snapshot.control_max_response_lines = 1;
	if (kn_control_protocol_handle("HEARD", &snapshot, out,
	    sizeof(out)) != KN_CONTROL_ERR_UNKNOWN_COMMAND)
		return 1;

	return strcmp(out, "ERR response-line-limit\n") == 0 ? 0 : 1;
}

static void
rx_snapshot_init(struct kn_control_snapshot *snapshot,
	struct kn_daemon_stats *daemon, struct kn_rx_queue *queue,
	struct kn_rx_session_table *sessions)
{
	snapshot_init(snapshot, daemon, NULL, 0);
	snapshot->rx_enabled = 1;
	snapshot->rx_events = queue;
	snapshot->rx_sessions = sessions;
}

static void
rx_add_event(struct kn_rx_queue *queue, struct kn_rx_session_table *sessions)
{
	struct kn_rx_event event;

	kn_rx_event_clear(&event);
	event.id = kn_rx_queue_reserve_id(queue);
	event.timestamp = 10;
	(void)snprintf(event.port_name, sizeof(event.port_name), "%s", "kiss0");
	event.kiss_port = 0;
	event.kind = KN_RX_FRAME_UI;
	(void)kn_callsign_parse("M6VPN-1", &event.source);
	(void)kn_callsign_parse("CQ", &event.destination);
	event.control = KN_AX25_CONTROL_UI;
	event.pid = KN_AX25_PID_NO_LAYER_3;
	event.has_pid = 1;
	event.payload_len = 5;
	(void)snprintf(event.preview, sizeof(event.preview), "%s", "\"hello\"");
	(void)kn_rx_queue_push(queue, &event);
	(void)kn_rx_session_update(sessions, &event);
}

static int
test_rx_event_existing(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	struct kn_rx_queue queue;
	struct kn_rx_session_table sessions;
	char out[KN_CONTROL_QUEUE_MAX];

	kn_rx_queue_init(&queue, 4, 80, 1);
	kn_rx_session_init(&sessions, 4);
	rx_add_event(&queue, &sessions);
	rx_snapshot_init(&snapshot, &daemon, &queue, &sessions);

	if (kn_control_protocol_handle("RX EVENT 1", &snapshot, out,
	    sizeof(out)) != KN_CONTROL_OK)
		return 1;

	return strstr(out, "OK RX EVENT id=1\n") != NULL &&
	    strstr(out, "preview=\"hello\"") != NULL ? 0 : 1;
}

static int
test_rx_event_missing(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	struct kn_rx_queue queue;
	struct kn_rx_session_table sessions;
	char out[KN_CONTROL_QUEUE_MAX];

	kn_rx_queue_init(&queue, 4, 80, 1);
	kn_rx_session_init(&sessions, 4);
	rx_snapshot_init(&snapshot, &daemon, &queue, &sessions);

	if (kn_control_protocol_handle("RX EVENT 9", &snapshot, out,
	    sizeof(out)) != KN_CONTROL_ERR_UNKNOWN_COMMAND)
		return 1;

	return strcmp(out, "ERR event-not-found\n") == 0 ? 0 : 1;
}

static int
test_rx_events_empty(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	struct kn_rx_queue queue;
	struct kn_rx_session_table sessions;
	char out[KN_CONTROL_QUEUE_MAX];

	kn_rx_queue_init(&queue, 4, 80, 1);
	kn_rx_session_init(&sessions, 4);
	rx_snapshot_init(&snapshot, &daemon, &queue, &sessions);

	if (kn_control_protocol_handle("RX EVENTS", &snapshot, out,
	    sizeof(out)) != KN_CONTROL_OK)
		return 1;

	return strcmp(out, "OK RX EVENTS count=0\nEND\n") == 0 ? 0 : 1;
}

static int
test_rx_events_filters(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	struct kn_rx_queue queue;
	struct kn_rx_session_table sessions;
	char out[KN_CONTROL_QUEUE_MAX];

	kn_rx_queue_init(&queue, 4, 80, 1);
	kn_rx_session_init(&sessions, 4);
	rx_add_event(&queue, &sessions);
	rx_snapshot_init(&snapshot, &daemon, &queue, &sessions);

	if (kn_control_protocol_handle("RX EVENTS FROM M6VPN-1", &snapshot,
	    out, sizeof(out)) != KN_CONTROL_OK)
		return 1;
	if (strstr(out, "count=1") == NULL)
		return 1;
	if (kn_control_protocol_handle("RX EVENTS TO BAD@", &snapshot, out,
	    sizeof(out)) != KN_CONTROL_ERR_UNKNOWN_COMMAND)
		return 1;

	return strcmp(out, "ERR invalid-callsign\n") == 0 ? 0 : 1;
}

static int
test_rx_events_one(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	struct kn_rx_queue queue;
	struct kn_rx_session_table sessions;
	char out[KN_CONTROL_QUEUE_MAX];

	kn_rx_queue_init(&queue, 4, 80, 1);
	kn_rx_session_init(&sessions, 4);
	rx_add_event(&queue, &sessions);
	rx_snapshot_init(&snapshot, &daemon, &queue, &sessions);

	if (kn_control_protocol_handle("RX EVENTS LIMIT 1", &snapshot, out,
	    sizeof(out)) != KN_CONTROL_OK)
		return 1;

	return strstr(out, "RX EVENT id=1") != NULL ? 0 : 1;
}

static int
test_rx_malformed_command(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	struct kn_rx_queue queue;
	struct kn_rx_session_table sessions;
	char out[KN_CONTROL_QUEUE_MAX];

	kn_rx_queue_init(&queue, 4, 80, 1);
	kn_rx_session_init(&sessions, 4);
	rx_snapshot_init(&snapshot, &daemon, &queue, &sessions);

	if (kn_control_protocol_handle("RX NOPE", &snapshot, out,
	    sizeof(out)) != KN_CONTROL_ERR_UNKNOWN_COMMAND)
		return 1;

	return strcmp(out, "ERR invalid-rx-command\n") == 0 ? 0 : 1;
}

static int
test_rx_sessions_empty(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	struct kn_rx_queue queue;
	struct kn_rx_session_table sessions;
	char out[KN_CONTROL_QUEUE_MAX];

	kn_rx_queue_init(&queue, 4, 80, 1);
	kn_rx_session_init(&sessions, 4);
	rx_snapshot_init(&snapshot, &daemon, &queue, &sessions);

	if (kn_control_protocol_handle("RX SESSIONS", &snapshot, out,
	    sizeof(out)) != KN_CONTROL_OK)
		return 1;

	return strcmp(out, "OK RX SESSIONS count=0\nEND\n") == 0 ? 0 : 1;
}

static int
test_rx_sessions_one(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	struct kn_rx_queue queue;
	struct kn_rx_session_table sessions;
	char out[KN_CONTROL_QUEUE_MAX];

	kn_rx_queue_init(&queue, 4, 80, 1);
	kn_rx_session_init(&sessions, 4);
	rx_add_event(&queue, &sessions);
	rx_snapshot_init(&snapshot, &daemon, &queue, &sessions);

	if (kn_control_protocol_handle("RX SESSIONS PORT kiss0", &snapshot,
	    out, sizeof(out)) != KN_CONTROL_OK)
		return 1;

	return strstr(out, "RX SESSION port=kiss0 from=M6VPN-1") != NULL ?
	    0 : 1;
}

static int
test_rx_status(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	struct kn_rx_queue queue;
	struct kn_rx_session_table sessions;
	char out[KN_CONTROL_QUEUE_MAX];

	kn_rx_queue_init(&queue, 4, 80, 1);
	kn_rx_session_init(&sessions, 4);
	rx_snapshot_init(&snapshot, &daemon, &queue, &sessions);

	if (kn_control_protocol_handle("RX STATUS", &snapshot, out,
	    sizeof(out)) != KN_CONTROL_OK)
		return 1;

	return strstr(out, "OK RX STATUS events_enabled=true") != NULL ? 0 : 1;
}

static int
test_stats_response(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	char out[KN_CONTROL_QUEUE_MAX];

	snapshot_init(&snapshot, &daemon, NULL, 0);
	daemon.bytes_received = 1234;
	daemon.kiss_frames_received = 12;
	daemon.ax25_frames_decoded = 10;
	daemon.malformed_kiss_frames = 1;
	daemon.malformed_ax25_frames = 1;

	if (kn_control_protocol_handle("STATS", &snapshot, out,
	    sizeof(out)) != KN_CONTROL_OK)
		return 1;

	return strstr(out, "OK STATS rx_bytes=1234 kiss_frames=12") != NULL ?
	    0 : 1;
}

static int
test_status_response(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	char out[KN_CONTROL_QUEUE_MAX];

	snapshot_init(&snapshot, &daemon, NULL, 0);
	daemon.configured_ports = 1;
	daemon.open_ports = 1;
	daemon.kiss_frames_received = 12;

	if (kn_control_protocol_handle("STATUS", &snapshot, out,
	    sizeof(out)) != KN_CONTROL_OK)
		return 1;

	return strcmp(out,
	    "OK STATUS running ports=1 open=1 frames=12 malformed=0\n") == 0 ?
	    0 : 1;
}

static int
test_unknown_command(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	char out[KN_CONTROL_QUEUE_MAX];

	snapshot_init(&snapshot, &daemon, NULL, 0);
	if (kn_control_protocol_handle("NOPE", &snapshot, out,
	    sizeof(out)) != KN_CONTROL_ERR_UNKNOWN_COMMAND)
		return 1;

	return strcmp(out, "ERR unknown-command\n") == 0 ? 0 : 1;
}

static int
test_version_response(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	char out[KN_CONTROL_QUEUE_MAX];

	snapshot_init(&snapshot, &daemon, NULL, 0);
	if (kn_control_protocol_handle("VERSION", &snapshot, out,
	    sizeof(out)) != KN_CONTROL_OK)
		return 1;

	return strstr(out, "OK VERSION KiloNode ") == out ? 0 : 1;
}
