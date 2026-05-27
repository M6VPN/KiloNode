/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_control_protocol.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "kilonode/ax25.h"
#include "kilonode/ax25_control.h"
#include "kilonode/ax25_connection_event.h"
#include "kilonode/ax25_runtime.h"
#include "kilonode/callsign.h"
#include "kilonode/control.h"
#include "kilonode/heard.h"
#include "kilonode/rf_command_queue.h"
#include "kilonode/rx_event.h"
#include "kilonode/rx_queue.h"
#include "kilonode/rx_session.h"
#include "kilonode/stats.h"
#include "kilonode/transport_memory.h"
#include "kilonode/tx_dispatch.h"
#include "kilonode/tx_frame.h"
#include "kilonode/tx_policy.h"
#include "kilonode/tx_queue.h"

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
static int test_ax25_connection_detail(void);
static int test_ax25_connection_missing(void);
static int test_ax25_connections_empty(void);
static int test_ax25_connections_populated(void);
static int test_ax25_connections_port(void);
static int test_ax25_counters(void);
static int test_ax25_live(void);
static int test_ax25_malformed_command(void);
static int test_ax25_no_write_commands(void);
static int test_ax25_params(void);
static int test_ax25_scheduler(void);
static int test_ax25_scheduler_counters(void);
static int test_ax25_scheduler_malformed(void);
static int test_ax25_scheduler_timers(void);
static int test_ax25_status(void);
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
static int test_rf_command_existing(void);
static int test_rf_command_missing(void);
static int test_rf_abuse_source_existing(void);
static int test_rf_abuse_source_invalid(void);
static int test_rf_abuse_source_missing(void);
static int test_rf_abuse_sources_empty(void);
static int test_rf_abuse_sources_populated(void);
static int test_rf_abuse_status(void);
static int test_rf_commands_empty(void);
static int test_rf_commands_filters(void);
static int test_rf_commands_one(void);
static int test_rf_ignore_list_empty(void);
static int test_rf_ignore_list_populated(void);
static int test_rf_malformed_command(void);
static int test_rf_status_default(void);
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
static int test_tx_frame_existing(void);
static int test_tx_frame_missing(void);
static int test_tx_gates_default(void);
static int test_tx_gates_port_tx_disabled(void);
static int test_tx_gates_port_tx_enabled(void);
static int test_tx_dryrun_default_rejected(void);
static int test_tx_dryrun_invalid_callsign(void);
static int test_tx_dryrun_invalid_port(void);
static int test_tx_dryrun_invalid_via(void);
static int test_tx_dryrun_oversized(void);
static int test_tx_dryrun_queue_full(void);
static int test_tx_dryrun_valid(void);
static int test_tx_dryrun_via_valid(void);
static int test_tx_dispatch_run_default_rejected(void);
static int test_tx_dispatch_run_enabled(void);
static int test_tx_dispatch_run_real_enabled(void);
static int test_tx_dispatch_run_port(void);
static int test_tx_dispatch_run_port_invalid(void);
static int test_tx_dispatch_status_default(void);
static int test_tx_dispatch_status_enabled(void);
static int test_tx_malformed_command(void);
static int test_tx_queue_empty(void);
static int test_tx_queue_one(void);
static int test_tx_queue_port(void);
static int test_tx_status(void);
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
	if (test_ax25_status() != 0)
		return 1;
	if (test_ax25_params() != 0)
		return 1;
	if (test_ax25_connections_empty() != 0)
		return 1;
	if (test_ax25_connections_populated() != 0)
		return 1;
	if (test_ax25_connections_port() != 0)
		return 1;
	if (test_ax25_connection_detail() != 0)
		return 1;
	if (test_ax25_connection_missing() != 0)
		return 1;
	if (test_ax25_counters() != 0)
		return 1;
	if (test_ax25_live() != 0)
		return 1;
	if (test_ax25_scheduler() != 0)
		return 1;
	if (test_ax25_scheduler_timers() != 0)
		return 1;
	if (test_ax25_scheduler_counters() != 0)
		return 1;
	if (test_ax25_scheduler_malformed() != 0)
		return 1;
	if (test_ax25_malformed_command() != 0)
		return 1;
	if (test_ax25_no_write_commands() != 0)
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
	if (test_rf_status_default() != 0)
		return 1;
	if (test_rf_commands_empty() != 0)
		return 1;
	if (test_rf_commands_one() != 0)
		return 1;
	if (test_rf_commands_filters() != 0)
		return 1;
	if (test_rf_command_existing() != 0)
		return 1;
	if (test_rf_command_missing() != 0)
		return 1;
	if (test_rf_abuse_status() != 0)
		return 1;
	if (test_rf_abuse_sources_empty() != 0)
		return 1;
	if (test_rf_abuse_sources_populated() != 0)
		return 1;
	if (test_rf_abuse_source_existing() != 0)
		return 1;
	if (test_rf_abuse_source_missing() != 0)
		return 1;
	if (test_rf_abuse_source_invalid() != 0)
		return 1;
	if (test_rf_ignore_list_empty() != 0)
		return 1;
	if (test_rf_ignore_list_populated() != 0)
		return 1;
	if (test_rf_malformed_command() != 0)
		return 1;
	if (test_tx_status() != 0)
		return 1;
	if (test_tx_gates_default() != 0)
		return 1;
	if (test_tx_gates_port_tx_disabled() != 0)
		return 1;
	if (test_tx_gates_port_tx_enabled() != 0)
		return 1;
	if (test_tx_dryrun_default_rejected() != 0)
		return 1;
	if (test_tx_dryrun_valid() != 0)
		return 1;
	if (test_tx_dryrun_via_valid() != 0)
		return 1;
	if (test_tx_dryrun_invalid_callsign() != 0)
		return 1;
	if (test_tx_dryrun_invalid_port() != 0)
		return 1;
	if (test_tx_dryrun_invalid_via() != 0)
		return 1;
	if (test_tx_dryrun_oversized() != 0)
		return 1;
	if (test_tx_dryrun_queue_full() != 0)
		return 1;
	if (test_tx_dispatch_status_default() != 0)
		return 1;
	if (test_tx_dispatch_run_default_rejected() != 0)
		return 1;
	if (test_tx_dispatch_status_enabled() != 0)
		return 1;
	if (test_tx_dispatch_run_enabled() != 0)
		return 1;
	if (test_tx_dispatch_run_real_enabled() != 0)
		return 1;
	if (test_tx_dispatch_run_port() != 0)
		return 1;
	if (test_tx_dispatch_run_port_invalid() != 0)
		return 1;
	if (test_tx_queue_empty() != 0)
		return 1;
	if (test_tx_queue_one() != 0)
		return 1;
	if (test_tx_queue_port() != 0)
		return 1;
	if (test_tx_frame_existing() != 0)
		return 1;
	if (test_tx_frame_missing() != 0)
		return 1;
	if (test_tx_malformed_command() != 0)
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
	snapshot->tx_queue = NULL;
	snapshot->tx_dispatch = NULL;
	snapshot->rf_config = NULL;
	snapshot->rf_commands = NULL;
	snapshot->rf_abuse = NULL;
	snapshot->rf_ignore = NULL;
	snapshot->ax25_runtime = NULL;
	snapshot->control_max_command_bytes = 0;
	snapshot->control_max_response_lines = 0;
}

static int
ax25_runtime_populate(struct kn_ax25_runtime *runtime)
{
	struct kn_ax25_connection_key key;
	struct kn_ax25_connection_event_record event;
	struct kn_ax25_connection_table_result result;

	kn_ax25_runtime_init(runtime);
	(void)kn_ax25_runtime_set_enabled(runtime, 1, 1);
	if (kn_ax25_connection_key_from_callsigns(&key, "kiss0", "M6VPN-1",
	    "N0CALL", NULL, 0) != KN_AX25_CONNECTION_KEY_OK)
		return 1;
	if (kn_ax25_connection_event_local_connect(&event, 1710000000,
	    &key) != KN_AX25_CONNECTION_EVENT_OK)
		return 1;
	event.kind = KN_AX25_CONNECTION_EVENT_RX_SABM;
	event.control.class = KN_AX25_CONTROL_CLASS_U;
	event.control.u_subtype = KN_AX25_U_SUBTYPE_SABM;
	return kn_ax25_runtime_inject_event(runtime, &event, &result) ==
	    KN_AX25_RUNTIME_OK ? 0 : 1;
}

static int
test_ax25_connection_detail(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	struct kn_ax25_runtime runtime;
	char out[KN_CONTROL_QUEUE_MAX];

	snapshot_init(&snapshot, &daemon, NULL, 0);
	if (ax25_runtime_populate(&runtime) != 0)
		return 1;
	snapshot.ax25_runtime = &runtime;
	if (kn_control_protocol_handle("AX25 CONNECTION 1", &snapshot, out,
	    sizeof(out)) != KN_CONTROL_OK)
		return 1;
	if (strstr(out, "OK AX25 CONNECTION id=1\n") == NULL)
		return 1;

	return strstr(out, "AX25 PLAN index=0 kind=UA") != NULL ? 0 : 1;
}

static int
test_ax25_connection_missing(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	char out[KN_CONTROL_QUEUE_MAX];

	snapshot_init(&snapshot, &daemon, NULL, 0);
	if (kn_control_protocol_handle("AX25 CONNECTION 1", &snapshot, out,
	    sizeof(out)) != KN_CONTROL_ERR_UNKNOWN_COMMAND)
		return 1;

	return strcmp(out, "ERR connection-not-found\n") == 0 ? 0 : 1;
}

static int
test_ax25_connections_empty(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	char out[KN_CONTROL_QUEUE_MAX];

	snapshot_init(&snapshot, &daemon, NULL, 0);
	if (kn_control_protocol_handle("AX25 CONNECTIONS", &snapshot, out,
	    sizeof(out)) != KN_CONTROL_OK)
		return 1;

	return strcmp(out, "OK AX25 CONNECTIONS count=0\nEND\n") == 0 ?
	    0 : 1;
}

static int
test_ax25_connections_populated(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	struct kn_ax25_runtime runtime;
	char out[KN_CONTROL_QUEUE_MAX];

	snapshot_init(&snapshot, &daemon, NULL, 0);
	if (ax25_runtime_populate(&runtime) != 0)
		return 1;
	snapshot.ax25_runtime = &runtime;
	if (kn_control_protocol_handle("AX25 CONNECTIONS", &snapshot, out,
	    sizeof(out)) != KN_CONTROL_OK)
		return 1;
	if (strstr(out, "OK AX25 CONNECTIONS count=1\n") == NULL)
		return 1;

	return strstr(out, "state=connected") != NULL ? 0 : 1;
}

static int
test_ax25_connections_port(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	struct kn_ax25_runtime runtime;
	char out[KN_CONTROL_QUEUE_MAX];

	snapshot_init(&snapshot, &daemon, NULL, 0);
	if (ax25_runtime_populate(&runtime) != 0)
		return 1;
	snapshot.ax25_runtime = &runtime;
	if (kn_control_protocol_handle("AX25 CONNECTIONS PORT kiss0",
	    &snapshot, out, sizeof(out)) != KN_CONTROL_OK)
		return 1;

	return strstr(out, "OK AX25 CONNECTIONS count=1\n") != NULL ? 0 : 1;
}

static int
test_ax25_counters(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	struct kn_ax25_runtime runtime;
	char out[KN_CONTROL_QUEUE_MAX];

	snapshot_init(&snapshot, &daemon, NULL, 0);
	if (ax25_runtime_populate(&runtime) != 0)
		return 1;
	snapshot.ax25_runtime = &runtime;
	if (kn_control_protocol_handle("AX25 COUNTERS", &snapshot, out,
	    sizeof(out)) != KN_CONTROL_OK)
		return 1;

	return strstr(out, "OK AX25 COUNTERS events=1") != NULL ? 0 : 1;
}

static int
test_ax25_live(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	char out[KN_CONTROL_QUEUE_MAX];

	snapshot_init(&snapshot, &daemon, NULL, 0);
	if (kn_control_protocol_handle("AX25 LIVE", &snapshot, out,
	    sizeof(out)) != KN_CONTROL_OK)
		return 1;

	return strstr(out, "OK AX25 LIVE enabled=false feed=false") != NULL ?
	    0 : 1;
}

static int
test_ax25_malformed_command(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	char out[KN_CONTROL_QUEUE_MAX];

	snapshot_init(&snapshot, &daemon, NULL, 0);
	if (kn_control_protocol_handle("AX25", &snapshot, out,
	    sizeof(out)) != KN_CONTROL_ERR_UNKNOWN_COMMAND)
		return 1;

	return strcmp(out, "ERR invalid-ax25-command\n") == 0 ? 0 : 1;
}

static int
test_ax25_no_write_commands(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	char out[KN_CONTROL_QUEUE_MAX];

	snapshot_init(&snapshot, &daemon, NULL, 0);
	if (kn_control_protocol_handle("AX25 CONNECT N0CALL", &snapshot, out,
	    sizeof(out)) != KN_CONTROL_ERR_UNKNOWN_COMMAND)
		return 1;

	return strcmp(out, "ERR invalid-ax25-command\n") == 0 ? 0 : 1;
}

static int
test_ax25_params(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	char out[KN_CONTROL_QUEUE_MAX];

	snapshot_init(&snapshot, &daemon, NULL, 0);
	if (kn_control_protocol_handle("AX25 PARAMS", &snapshot, out,
	    sizeof(out)) != KN_CONTROL_OK)
		return 1;

	return strstr(out, "OK AX25 PARAMS modulo=8 window=1") != NULL ?
	    0 : 1;
}

static int
test_ax25_scheduler(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	char out[KN_CONTROL_QUEUE_MAX];

	snapshot_init(&snapshot, &daemon, NULL, 0);
	if (kn_control_protocol_handle("AX25 SCHEDULER", &snapshot, out,
	    sizeof(out)) != KN_CONTROL_OK)
		return 1;

	return strstr(out, "OK AX25 SCHEDULER enabled=false") != NULL ? 0 :
	    1;
}

static int
test_ax25_scheduler_counters(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	char out[KN_CONTROL_QUEUE_MAX];

	snapshot_init(&snapshot, &daemon, NULL, 0);
	if (kn_control_protocol_handle("AX25 SCHEDULER COUNTERS", &snapshot,
	    out, sizeof(out)) != KN_CONTROL_OK)
		return 1;

	return strstr(out, "cycles=0 expired=0") != NULL &&
	    strstr(out, "tx_writes=0") != NULL ? 0 : 1;
}

static int
test_ax25_scheduler_malformed(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	char out[KN_CONTROL_QUEUE_MAX];

	snapshot_init(&snapshot, &daemon, NULL, 0);
	if (kn_control_protocol_handle("AX25 SCHEDULER RUN", &snapshot, out,
	    sizeof(out)) != KN_CONTROL_ERR_UNKNOWN_COMMAND)
		return 1;

	return strcmp(out, "ERR invalid-ax25-command\n") == 0 ? 0 : 1;
}

static int
test_ax25_scheduler_timers(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	char out[KN_CONTROL_QUEUE_MAX];

	snapshot_init(&snapshot, &daemon, NULL, 0);
	if (kn_control_protocol_handle("AX25 SCHEDULER TIMERS", &snapshot,
	    out, sizeof(out)) != KN_CONTROL_OK)
		return 1;

	return strstr(out, "OK AX25 SCHEDULER TIMERS count=0") != NULL ?
	    0 : 1;
}

static int
test_ax25_status(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	char out[KN_CONTROL_QUEUE_MAX];

	snapshot_init(&snapshot, &daemon, NULL, 0);
	if (kn_control_protocol_handle("AX25 STATUS", &snapshot, out,
	    sizeof(out)) != KN_CONTROL_OK)
		return 1;

	return strcmp(out,
	    "OK AX25 STATUS enabled=false connected_mode=false "
	    "live_rx_feed=false live_rx_create_connections=false "
	    "live_scheduler=false live_scheduler_process_expired=false "
	    "connections=0 max_connections=32 diagnostics=true\nEND\n") == 0 ?
	    0 : 1;
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

static void
rf_snapshot_init(struct kn_control_snapshot *snapshot,
	struct kn_daemon_stats *daemon, struct kn_config_rf_command *config,
	struct kn_rf_command_queue *queue)
{
	snapshot_init(snapshot, daemon, NULL, 0);
	snapshot->rf_config = config;
	snapshot->rf_commands = queue;
}

static void
rf_abuse_snapshot_init(struct kn_control_snapshot *snapshot,
	struct kn_daemon_stats *daemon, struct kn_config_rf_command *config,
	struct kn_rf_command_queue *queue, struct kn_rf_abuse_state *abuse,
	struct kn_rf_ignore_list *ignore)
{
	rf_snapshot_init(snapshot, daemon, config, queue);
	snapshot->rf_abuse = abuse;
	snapshot->rf_ignore = ignore;
}

static void
rf_add_event(struct kn_rf_command_queue *queue)
{
	struct kn_rf_command_event event;

	kn_rf_command_event_clear(&event);
	event.id = kn_rf_command_queue_reserve_id(queue);
	event.timestamp = 10;
	event.rx_event_id = 3;
	(void)snprintf(event.port_name, sizeof(event.port_name), "%s",
	    "kiss0");
	(void)kn_callsign_parse("N0CALL", &event.source);
	(void)kn_callsign_parse("M6VPN-1", &event.destination);
	event.command = KN_RF_COMMAND_PING;
	event.status = KN_RF_COMMAND_STATUS_OK;
	(void)snprintf(event.raw, sizeof(event.raw), "%s", "PING");
	(void)kn_rf_command_queue_push(queue, &event);
}

static void
tx_snapshot_init(struct kn_control_snapshot *snapshot,
	struct kn_daemon_stats *daemon, struct kn_tx_queue *queue)
{
	snapshot_init(snapshot, daemon, NULL, 0);
	snapshot->tx_queue = queue;
}

static void
tx_snapshot_port_init(struct kn_control_snapshot *snapshot,
	struct kn_daemon_stats *daemon, struct kn_port_stats *port,
	struct kn_tx_queue *queue, uint8_t open)
{
	memset(port, 0, sizeof(*port));
	(void)snprintf(port->name, sizeof(port->name), "%s", "kiss0");
	port->enabled = 1;
	port->open = open;
	snapshot_init(snapshot, daemon, port, 1);
	snapshot->tx_queue = queue;
}

static void
tx_add_frame(struct kn_tx_queue *queue)
{
	struct kn_tx_frame frame;

	kn_tx_frame_clear(&frame);
	frame.id = kn_tx_queue_reserve_id(queue);
	frame.created = 10;
	(void)snprintf(frame.port_name, sizeof(frame.port_name), "%s", "kiss0");
	frame.kiss_port = 0;
	frame.kind = KN_TX_FRAME_UI;
	frame.status = KN_TX_FRAME_DRY_RUN;
	(void)kn_callsign_parse("M6VPN-1", &frame.source);
	(void)kn_callsign_parse("CQ", &frame.destination);
	frame.control = KN_AX25_CONTROL_UI;
	frame.pid = KN_AX25_PID_NO_LAYER_3;
	frame.has_pid = 1;
	frame.payload_len = 5;
	frame.ax25_len = 20;
	frame.kiss_len = 24;
	(void)snprintf(frame.preview, sizeof(frame.preview), "%s", "\"hello\"");
	(void)kn_tx_queue_enqueue(queue, &frame);
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
test_rf_command_existing(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	struct kn_config config;
	struct kn_rf_command_queue queue;
	char out[KN_CONTROL_QUEUE_MAX];

	kn_config_init(&config);
	(void)kn_rf_command_queue_init(&queue, 4);
	rf_add_event(&queue);
	rf_snapshot_init(&snapshot, &daemon, &config.rf_command, &queue);

	if (kn_control_protocol_handle("RF COMMAND 1", &snapshot, out,
	    sizeof(out)) != KN_CONTROL_OK)
		return 1;

	return strstr(out, "OK RF COMMAND id=1\n") != NULL &&
	    strstr(out, "raw=\"PING\"") != NULL ? 0 : 1;
}

static int
test_rf_command_missing(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	struct kn_config config;
	struct kn_rf_command_queue queue;
	char out[KN_CONTROL_QUEUE_MAX];

	kn_config_init(&config);
	(void)kn_rf_command_queue_init(&queue, 4);
	rf_snapshot_init(&snapshot, &daemon, &config.rf_command, &queue);

	if (kn_control_protocol_handle("RF COMMAND 9", &snapshot, out,
	    sizeof(out)) != KN_CONTROL_ERR_UNKNOWN_COMMAND)
		return 1;

	return strcmp(out, "ERR command-not-found\n") == 0 ? 0 : 1;
}

static int
test_rf_abuse_source_existing(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	struct kn_config config;
	struct kn_rf_command_queue queue;
	struct kn_rf_abuse_state abuse;
	struct kn_rf_ignore_list ignore;
	struct kn_callsign call;
	char out[KN_CONTROL_QUEUE_MAX];

	kn_config_init(&config);
	(void)kn_rf_command_queue_init(&queue, 4);
	kn_rf_abuse_init(&abuse, 4);
	kn_rf_ignore_init(&ignore);
	(void)kn_callsign_parse("N0CALL", &call);
	(void)kn_rf_abuse_record_accepted(&abuse, &call, 10);
	rf_abuse_snapshot_init(&snapshot, &daemon, &config.rf_command, &queue,
	    &abuse, &ignore);

	if (kn_control_protocol_handle("RF ABUSE SOURCE N0CALL", &snapshot,
	    out, sizeof(out)) != KN_CONTROL_OK)
		return 1;

	return strstr(out, "RF SOURCE call=N0CALL accepted=1") != NULL ? 0 : 1;
}

static int
test_rf_abuse_source_invalid(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	struct kn_config config;
	struct kn_rf_command_queue queue;
	struct kn_rf_abuse_state abuse;
	struct kn_rf_ignore_list ignore;
	char out[KN_CONTROL_QUEUE_MAX];

	kn_config_init(&config);
	(void)kn_rf_command_queue_init(&queue, 4);
	kn_rf_abuse_init(&abuse, 4);
	kn_rf_ignore_init(&ignore);
	rf_abuse_snapshot_init(&snapshot, &daemon, &config.rf_command, &queue,
	    &abuse, &ignore);

	if (kn_control_protocol_handle("RF ABUSE SOURCE BAD@", &snapshot, out,
	    sizeof(out)) != KN_CONTROL_ERR_UNKNOWN_COMMAND)
		return 1;

	return strcmp(out, "ERR invalid-callsign\n") == 0 ? 0 : 1;
}

static int
test_rf_abuse_source_missing(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	struct kn_config config;
	struct kn_rf_command_queue queue;
	struct kn_rf_abuse_state abuse;
	struct kn_rf_ignore_list ignore;
	char out[KN_CONTROL_QUEUE_MAX];

	kn_config_init(&config);
	(void)kn_rf_command_queue_init(&queue, 4);
	kn_rf_abuse_init(&abuse, 4);
	kn_rf_ignore_init(&ignore);
	rf_abuse_snapshot_init(&snapshot, &daemon, &config.rf_command, &queue,
	    &abuse, &ignore);

	if (kn_control_protocol_handle("RF ABUSE SOURCE N0CALL", &snapshot,
	    out, sizeof(out)) != KN_CONTROL_ERR_UNKNOWN_COMMAND)
		return 1;

	return strcmp(out, "ERR source-not-found\n") == 0 ? 0 : 1;
}

static int
test_rf_abuse_sources_empty(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	struct kn_config config;
	struct kn_rf_command_queue queue;
	struct kn_rf_abuse_state abuse;
	struct kn_rf_ignore_list ignore;
	char out[KN_CONTROL_QUEUE_MAX];

	kn_config_init(&config);
	(void)kn_rf_command_queue_init(&queue, 4);
	kn_rf_abuse_init(&abuse, 4);
	kn_rf_ignore_init(&ignore);
	rf_abuse_snapshot_init(&snapshot, &daemon, &config.rf_command, &queue,
	    &abuse, &ignore);

	if (kn_control_protocol_handle("RF ABUSE SOURCES", &snapshot, out,
	    sizeof(out)) != KN_CONTROL_OK)
		return 1;

	return strcmp(out, "OK RF ABUSE SOURCES count=0\nEND\n") == 0 ?
	    0 : 1;
}

static int
test_rf_abuse_sources_populated(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	struct kn_config config;
	struct kn_rf_command_queue queue;
	struct kn_rf_abuse_state abuse;
	struct kn_rf_ignore_list ignore;
	struct kn_callsign call;
	char out[KN_CONTROL_QUEUE_MAX];

	kn_config_init(&config);
	(void)kn_rf_command_queue_init(&queue, 4);
	kn_rf_abuse_init(&abuse, 4);
	kn_rf_ignore_init(&ignore);
	(void)kn_callsign_parse("N0CALL", &call);
	(void)kn_rf_abuse_record_rejected(&abuse, &config.rf_command, &call,
	    10, "rate-limited");
	rf_abuse_snapshot_init(&snapshot, &daemon, &config.rf_command, &queue,
	    &abuse, &ignore);

	if (kn_control_protocol_handle("RF ABUSE SOURCES", &snapshot, out,
	    sizeof(out)) != KN_CONTROL_OK)
		return 1;

	return strstr(out, "RF SOURCE call=N0CALL accepted=0 rejected=1") !=
	    NULL ? 0 : 1;
}

static int
test_rf_abuse_status(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	struct kn_config config;
	struct kn_rf_command_queue queue;
	struct kn_rf_abuse_state abuse;
	struct kn_rf_ignore_list ignore;
	char out[KN_CONTROL_QUEUE_MAX];

	kn_config_init(&config);
	(void)kn_rf_command_queue_init(&queue, 4);
	kn_rf_abuse_init(&abuse, 4);
	kn_rf_ignore_init(&ignore);
	rf_abuse_snapshot_init(&snapshot, &daemon, &config.rf_command, &queue,
	    &abuse, &ignore);

	if (kn_control_protocol_handle("RF ABUSE STATUS", &snapshot, out,
	    sizeof(out)) != KN_CONTROL_OK)
		return 1;

	return strstr(out, "OK RF ABUSE STATUS enabled=true") != NULL ? 0 : 1;
}

static int
test_rf_commands_empty(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	struct kn_config config;
	struct kn_rf_command_queue queue;
	char out[KN_CONTROL_QUEUE_MAX];

	kn_config_init(&config);
	(void)kn_rf_command_queue_init(&queue, 4);
	rf_snapshot_init(&snapshot, &daemon, &config.rf_command, &queue);

	if (kn_control_protocol_handle("RF COMMANDS", &snapshot, out,
	    sizeof(out)) != KN_CONTROL_OK)
		return 1;

	return strcmp(out, "OK RF COMMANDS count=0\nEND\n") == 0 ? 0 : 1;
}

static int
test_rf_commands_filters(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	struct kn_config config;
	struct kn_rf_command_queue queue;
	char out[KN_CONTROL_QUEUE_MAX];

	kn_config_init(&config);
	(void)kn_rf_command_queue_init(&queue, 4);
	rf_add_event(&queue);
	rf_snapshot_init(&snapshot, &daemon, &config.rf_command, &queue);

	if (kn_control_protocol_handle("RF COMMANDS FROM N0CALL", &snapshot,
	    out, sizeof(out)) != KN_CONTROL_OK)
		return 1;
	if (strstr(out, "count=1") == NULL)
		return 1;
	if (kn_control_protocol_handle("RF COMMANDS FROM BAD@", &snapshot,
	    out, sizeof(out)) != KN_CONTROL_ERR_UNKNOWN_COMMAND)
		return 1;

	return strcmp(out, "ERR invalid-callsign\n") == 0 ? 0 : 1;
}

static int
test_rf_commands_one(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	struct kn_config config;
	struct kn_rf_command_queue queue;
	char out[KN_CONTROL_QUEUE_MAX];

	kn_config_init(&config);
	(void)kn_rf_command_queue_init(&queue, 4);
	rf_add_event(&queue);
	rf_snapshot_init(&snapshot, &daemon, &config.rf_command, &queue);

	if (kn_control_protocol_handle("RF COMMANDS LIMIT 1", &snapshot, out,
	    sizeof(out)) != KN_CONTROL_OK)
		return 1;

	return strstr(out, "RF COMMAND id=1") != NULL ? 0 : 1;
}

static int
test_rf_ignore_list_empty(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	struct kn_config config;
	struct kn_rf_command_queue queue;
	struct kn_rf_abuse_state abuse;
	struct kn_rf_ignore_list ignore;
	char out[KN_CONTROL_QUEUE_MAX];

	kn_config_init(&config);
	(void)kn_rf_command_queue_init(&queue, 4);
	kn_rf_abuse_init(&abuse, 4);
	kn_rf_ignore_init(&ignore);
	rf_abuse_snapshot_init(&snapshot, &daemon, &config.rf_command, &queue,
	    &abuse, &ignore);

	if (kn_control_protocol_handle("RF IGNORE LIST", &snapshot, out,
	    sizeof(out)) != KN_CONTROL_OK)
		return 1;

	return strcmp(out, "OK RF IGNORE count=0\nEND\n") == 0 ? 0 : 1;
}

static int
test_rf_ignore_list_populated(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	struct kn_config config;
	struct kn_rf_command_queue queue;
	struct kn_rf_abuse_state abuse;
	struct kn_rf_ignore_list ignore;
	struct kn_callsign call;
	char out[KN_CONTROL_QUEUE_MAX];

	kn_config_init(&config);
	(void)kn_rf_command_queue_init(&queue, 4);
	kn_rf_abuse_init(&abuse, 4);
	kn_rf_ignore_init(&ignore);
	(void)kn_callsign_parse("N0CALL", &call);
	(void)kn_rf_ignore_add(&ignore, &call, "manual");
	rf_abuse_snapshot_init(&snapshot, &daemon, &config.rf_command, &queue,
	    &abuse, &ignore);

	if (kn_control_protocol_handle("RF IGNORE LIST", &snapshot, out,
	    sizeof(out)) != KN_CONTROL_OK)
		return 1;

	return strstr(out, "RF IGNORE call=N0CALL reason=\"manual\"") !=
	    NULL ? 0 : 1;
}

static int
test_rf_malformed_command(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	struct kn_config config;
	struct kn_rf_command_queue queue;
	char out[KN_CONTROL_QUEUE_MAX];

	kn_config_init(&config);
	(void)kn_rf_command_queue_init(&queue, 4);
	rf_snapshot_init(&snapshot, &daemon, &config.rf_command, &queue);

	if (kn_control_protocol_handle("RF NOPE", &snapshot, out,
	    sizeof(out)) != KN_CONTROL_ERR_UNKNOWN_COMMAND)
		return 1;

	return strcmp(out, "ERR invalid-rf-command\n") == 0 ? 0 : 1;
}

static int
test_rf_status_default(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	char out[KN_CONTROL_QUEUE_MAX];

	snapshot_init(&snapshot, &daemon, NULL, 0);
	if (kn_control_protocol_handle("RF STATUS", &snapshot, out,
	    sizeof(out)) != KN_CONTROL_OK)
		return 1;

	return strstr(out, "OK RF STATUS enabled=false") != NULL ? 0 : 1;
}

static int
test_tx_frame_existing(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	struct kn_tx_policy policy;
	struct kn_tx_queue queue;
	char out[KN_CONTROL_QUEUE_MAX];

	kn_tx_policy_defaults(&policy);
	policy.enabled = 1;
	if (kn_tx_queue_init(&queue, &policy) != KN_TX_QUEUE_OK)
		return 1;
	tx_add_frame(&queue);
	tx_snapshot_init(&snapshot, &daemon, &queue);

	if (kn_control_protocol_handle("TX FRAME 1", &snapshot, out,
	    sizeof(out)) != KN_CONTROL_OK)
		return 1;

	return strstr(out, "OK TX FRAME id=1\n") != NULL &&
	    strstr(out, "preview=\"hello\"") != NULL ? 0 : 1;
}

static int
test_tx_frame_missing(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	struct kn_tx_policy policy;
	struct kn_tx_queue queue;
	char out[KN_CONTROL_QUEUE_MAX];

	kn_tx_policy_defaults(&policy);
	policy.enabled = 1;
	if (kn_tx_queue_init(&queue, &policy) != KN_TX_QUEUE_OK)
		return 1;
	tx_snapshot_init(&snapshot, &daemon, &queue);

	if (kn_control_protocol_handle("TX FRAME 9", &snapshot, out,
	    sizeof(out)) != KN_CONTROL_ERR_UNKNOWN_COMMAND)
		return 1;

	return strcmp(out, "ERR frame-not-found\n") == 0 ? 0 : 1;
}

static int
test_tx_dryrun_default_rejected(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	struct kn_port_stats port;
	struct kn_tx_policy policy;
	struct kn_tx_queue queue;
	char out[KN_CONTROL_QUEUE_MAX];

	kn_tx_policy_defaults(&policy);
	if (kn_tx_queue_init(&queue, &policy) != KN_TX_QUEUE_OK)
		return 1;
	tx_snapshot_port_init(&snapshot, &daemon, &port, &queue, 1);

	if (kn_control_protocol_handle("TX DRYRUN UI PORT kiss0 FROM "
	    "M6VPN-1 TO CQ TEXT hello", &snapshot, out, sizeof(out)) !=
	    KN_CONTROL_ERR_UNKNOWN_COMMAND)
		return 1;

	return strcmp(out, "ERR tx-disabled\n") == 0 ? 0 : 1;
}

static int
test_tx_gates_default(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	struct kn_tx_policy policy;
	struct kn_tx_queue queue;
	char out[KN_CONTROL_QUEUE_MAX];

	kn_tx_policy_defaults(&policy);
	if (kn_tx_queue_init(&queue, &policy) != KN_TX_QUEUE_OK)
		return 1;
	tx_snapshot_init(&snapshot, &daemon, &queue);

	if (kn_control_protocol_handle("TX GATES", &snapshot, out,
	    sizeof(out)) != KN_CONTROL_OK)
		return 1;

	return strstr(out, "dispatch_real_kiss=false") != NULL &&
	    strstr(out, "require_explicit_port_tx=true") != NULL ? 0 : 1;
}

static int
test_tx_gates_port_tx_disabled(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	struct kn_tx_policy policy;
	struct kn_tx_queue queue;
	struct kn_tx_dispatcher dispatcher;
	struct kn_transport transport;
	char out[KN_CONTROL_QUEUE_MAX];

	kn_tx_policy_defaults(&policy);
	if (kn_tx_queue_init(&queue, &policy) != KN_TX_QUEUE_OK)
		return 1;
	tx_snapshot_init(&snapshot, &daemon, &queue);
	kn_transport_reset(&transport);
	transport.kind = KN_TRANSPORT_KIND_TCP_CLIENT;
	transport.write_fd = -1;
	transport.open = 1;
	kn_tx_dispatch_clear(&dispatcher);
	(void)kn_tx_dispatch_add_transport_target(&dispatcher, "kiss0",
	    &transport, KN_CONFIG_PORT_TCP_CONNECT,
	    KN_TRANSPORT_KIND_TCP_CLIENT, 1, 1, 0);
	snapshot.tx_dispatch = &dispatcher;

	if (kn_control_protocol_handle("TX GATES PORT kiss0", &snapshot, out,
	    sizeof(out)) != KN_CONTROL_OK)
		return 1;

	return strstr(out, "tx_enabled=false") != NULL &&
	    strstr(out, "reason=port-tx-disabled") != NULL ? 0 : 1;
}

static int
test_tx_gates_port_tx_enabled(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	struct kn_tx_policy policy;
	struct kn_tx_queue queue;
	struct kn_tx_dispatcher dispatcher;
	struct kn_transport transport;
	char out[KN_CONTROL_QUEUE_MAX];

	kn_tx_policy_defaults(&policy);
	policy.enabled = 1;
	policy.dry_run = 0;
	policy.dispatch_enabled = 1;
	policy.dispatch_test_only = 0;
	policy.dispatch_real_kiss = 1;
	if (kn_tx_queue_init(&queue, &policy) != KN_TX_QUEUE_OK)
		return 1;
	tx_snapshot_init(&snapshot, &daemon, &queue);
	kn_transport_reset(&transport);
	transport.kind = KN_TRANSPORT_KIND_TCP_CLIENT;
	transport.write_fd = 1;
	transport.open = 1;
	kn_tx_dispatch_clear(&dispatcher);
	(void)kn_tx_dispatch_add_transport_target(&dispatcher, "kiss0",
	    &transport, KN_CONFIG_PORT_TCP_CONNECT,
	    KN_TRANSPORT_KIND_TCP_CLIENT, 1, 1, 1);
	snapshot.tx_dispatch = &dispatcher;

	if (kn_control_protocol_handle("TX GATES PORT kiss0", &snapshot, out,
	    sizeof(out)) != KN_CONTROL_OK)
		return 1;

	transport.write_fd = -1;
	return strstr(out, "tx_enabled=true") != NULL &&
	    strstr(out, "allowed=true") != NULL &&
	    strstr(out, "reason=ok") != NULL ? 0 : 1;
}

static void
tx_policy_control_allowed(struct kn_tx_policy *policy)
{
	kn_tx_policy_defaults(policy);
	policy->enabled = 1;
	policy->dry_run = 1;
	policy->allow_ui = 1;
	policy->allow_control_enqueue = 1;
}

static void
tx_policy_dispatch_allowed(struct kn_tx_policy *policy)
{
	tx_policy_control_allowed(policy);
	policy->dispatch_enabled = 1;
	policy->dispatch_test_only = 1;
	policy->dispatch_max_per_cycle = 4;
}

static int
test_tx_dryrun_invalid_callsign(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	struct kn_port_stats port;
	struct kn_tx_policy policy;
	struct kn_tx_queue queue;
	char out[KN_CONTROL_QUEUE_MAX];

	tx_policy_control_allowed(&policy);
	if (kn_tx_queue_init(&queue, &policy) != KN_TX_QUEUE_OK)
		return 1;
	tx_snapshot_port_init(&snapshot, &daemon, &port, &queue, 1);

	if (kn_control_protocol_handle("TX DRYRUN UI PORT kiss0 FROM bad* "
	    "TO CQ TEXT hello", &snapshot, out, sizeof(out)) !=
	    KN_CONTROL_ERR_UNKNOWN_COMMAND)
		return 1;

	return strcmp(out, "ERR invalid-callsign\n") == 0 ? 0 : 1;
}

static int
test_tx_dryrun_invalid_port(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	struct kn_port_stats port;
	struct kn_tx_policy policy;
	struct kn_tx_queue queue;
	char out[KN_CONTROL_QUEUE_MAX];

	tx_policy_control_allowed(&policy);
	if (kn_tx_queue_init(&queue, &policy) != KN_TX_QUEUE_OK)
		return 1;
	tx_snapshot_port_init(&snapshot, &daemon, &port, &queue, 0);

	if (kn_control_protocol_handle("TX DRYRUN UI PORT kiss0 FROM "
	    "M6VPN-1 TO CQ TEXT hello", &snapshot, out, sizeof(out)) !=
	    KN_CONTROL_ERR_UNKNOWN_COMMAND)
		return 1;

	return strcmp(out, "ERR invalid-port\n") == 0 ? 0 : 1;
}

static int
test_tx_dryrun_invalid_via(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	struct kn_port_stats port;
	struct kn_tx_policy policy;
	struct kn_tx_queue queue;
	char out[KN_CONTROL_QUEUE_MAX];

	tx_policy_control_allowed(&policy);
	if (kn_tx_queue_init(&queue, &policy) != KN_TX_QUEUE_OK)
		return 1;
	tx_snapshot_port_init(&snapshot, &daemon, &port, &queue, 1);

	if (kn_control_protocol_handle("TX DRYRUN UI PORT kiss0 FROM "
	    "M6VPN-1 TO CQ VIA bad* TEXT hello", &snapshot, out,
	    sizeof(out)) != KN_CONTROL_ERR_UNKNOWN_COMMAND)
		return 1;

	return strcmp(out, "ERR invalid-via\n") == 0 ? 0 : 1;
}

static int
test_tx_dryrun_oversized(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	struct kn_port_stats port;
	struct kn_tx_policy policy;
	struct kn_tx_queue queue;
	char out[KN_CONTROL_QUEUE_MAX];

	tx_policy_control_allowed(&policy);
	policy.max_payload_bytes = 2;
	if (kn_tx_queue_init(&queue, &policy) != KN_TX_QUEUE_OK)
		return 1;
	tx_snapshot_port_init(&snapshot, &daemon, &port, &queue, 1);

	if (kn_control_protocol_handle("TX DRYRUN UI PORT kiss0 FROM "
	    "M6VPN-1 TO CQ TEXT hello", &snapshot, out, sizeof(out)) !=
	    KN_CONTROL_ERR_UNKNOWN_COMMAND)
		return 1;

	return strcmp(out, "ERR payload-too-large\n") == 0 ? 0 : 1;
}

static int
test_tx_dryrun_queue_full(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	struct kn_port_stats port;
	struct kn_tx_policy policy;
	struct kn_tx_queue queue;
	char out[KN_CONTROL_QUEUE_MAX];

	tx_policy_control_allowed(&policy);
	policy.max_queued = 1;
	if (kn_tx_queue_init(&queue, &policy) != KN_TX_QUEUE_OK)
		return 1;
	tx_add_frame(&queue);
	tx_snapshot_port_init(&snapshot, &daemon, &port, &queue, 1);

	if (kn_control_protocol_handle("TX DRYRUN UI PORT kiss0 FROM "
	    "M6VPN-1 TO CQ TEXT hello", &snapshot, out, sizeof(out)) !=
	    KN_CONTROL_ERR_UNKNOWN_COMMAND)
		return 1;

	return strcmp(out, "ERR tx-queue-full\n") == 0 ? 0 : 1;
}

static int
test_tx_dryrun_valid(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	struct kn_port_stats port;
	struct kn_tx_policy policy;
	struct kn_tx_queue queue;
	char out[KN_CONTROL_QUEUE_MAX];

	tx_policy_control_allowed(&policy);
	if (kn_tx_queue_init(&queue, &policy) != KN_TX_QUEUE_OK)
		return 1;
	tx_snapshot_port_init(&snapshot, &daemon, &port, &queue, 1);

	if (kn_control_protocol_handle("TX DRYRUN UI PORT kiss0 FROM "
	    "M6VPN-1 TO CQ TEXT hello", &snapshot, out, sizeof(out)) !=
	    KN_CONTROL_OK)
		return 1;
	if (strstr(out, "OK TX DRYRUN queued id=1 port=kiss0") == NULL)
		return 1;
	if (kn_control_protocol_handle("TX QUEUE", &snapshot, out,
	    sizeof(out)) != KN_CONTROL_OK)
		return 1;

	return strstr(out, "TX FRAME id=1 port=kiss0") != NULL ? 0 : 1;
}

static int
test_tx_dispatch_run_default_rejected(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	struct kn_tx_policy policy;
	struct kn_tx_queue queue;
	char out[KN_CONTROL_QUEUE_MAX];

	kn_tx_policy_defaults(&policy);
	if (kn_tx_queue_init(&queue, &policy) != KN_TX_QUEUE_OK)
		return 1;
	tx_snapshot_init(&snapshot, &daemon, &queue);

	if (kn_control_protocol_handle("TX DISPATCH RUN", &snapshot, out,
	    sizeof(out)) != KN_CONTROL_ERR_UNKNOWN_COMMAND)
		return 1;

	return strcmp(out, "ERR tx-dispatch-disabled\n") == 0 ? 0 : 1;
}

static int
test_tx_dispatch_run_enabled(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	struct kn_port_stats port;
	struct kn_tx_policy policy;
	struct kn_tx_queue queue;
	struct kn_tx_dispatcher dispatcher;
	struct kn_transport_memory memory;
	char out[KN_CONTROL_QUEUE_MAX];

	tx_policy_dispatch_allowed(&policy);
	if (kn_tx_queue_init(&queue, &policy) != KN_TX_QUEUE_OK)
		return 1;
	tx_snapshot_port_init(&snapshot, &daemon, &port, &queue, 1);
	if (kn_control_protocol_handle("TX DRYRUN UI PORT kiss0 FROM "
	    "M6VPN-1 TO CQ TEXT hello", &snapshot, out, sizeof(out)) !=
	    KN_CONTROL_OK)
		return 1;
	kn_tx_dispatch_clear(&dispatcher);
	if (kn_transport_memory_init(&memory, 256) !=
	    KN_TRANSPORT_MEMORY_OK)
		return 1;
	(void)kn_transport_memory_open(&memory);
	(void)kn_tx_dispatch_add_memory_target(&dispatcher, "kiss0",
	    &memory, 1, 1);
	snapshot.tx_dispatch = &dispatcher;

	if (kn_control_protocol_handle("TX DISPATCH RUN", &snapshot, out,
	    sizeof(out)) != KN_CONTROL_OK)
		return 1;
	if (strstr(out, "OK TX DISPATCH sent=1 failed=0") == NULL)
		return 1;

	return memory.len > 0 ? 0 : 1;
}

static int
test_tx_dispatch_run_port(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	struct kn_port_stats port;
	struct kn_tx_policy policy;
	struct kn_tx_queue queue;
	struct kn_tx_dispatcher dispatcher;
	struct kn_transport_memory memory;
	char out[KN_CONTROL_QUEUE_MAX];

	tx_policy_dispatch_allowed(&policy);
	if (kn_tx_queue_init(&queue, &policy) != KN_TX_QUEUE_OK)
		return 1;
	tx_snapshot_port_init(&snapshot, &daemon, &port, &queue, 1);
	if (kn_control_protocol_handle("TX DRYRUN UI PORT kiss0 FROM "
	    "M6VPN-1 TO CQ TEXT hello", &snapshot, out, sizeof(out)) !=
	    KN_CONTROL_OK)
		return 1;
	kn_tx_dispatch_clear(&dispatcher);
	if (kn_transport_memory_init(&memory, 256) !=
	    KN_TRANSPORT_MEMORY_OK)
		return 1;
	(void)kn_transport_memory_open(&memory);
	(void)kn_tx_dispatch_add_memory_target(&dispatcher, "kiss0",
	    &memory, 1, 1);
	snapshot.tx_dispatch = &dispatcher;

	if (kn_control_protocol_handle("TX DISPATCH RUN PORT kiss0",
	    &snapshot, out, sizeof(out)) != KN_CONTROL_OK)
		return 1;

	return strstr(out, "sent=1") != NULL ? 0 : 1;
}

static int
test_tx_dispatch_run_real_enabled(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	struct kn_tx_policy policy;
	struct kn_tx_queue queue;
	struct kn_tx_dispatcher dispatcher;
	struct kn_transport transport;
	char out[KN_CONTROL_QUEUE_MAX];
	int fds[2];

	kn_tx_policy_defaults(&policy);
	policy.enabled = 1;
	policy.dry_run = 0;
	policy.allow_ui = 1;
	policy.dispatch_enabled = 1;
	policy.dispatch_test_only = 0;
	policy.dispatch_real_kiss = 1;
	if (kn_tx_queue_init(&queue, &policy) != KN_TX_QUEUE_OK)
		return 1;
	tx_add_frame(&queue);
	tx_snapshot_init(&snapshot, &daemon, &queue);
	if (pipe(fds) != 0)
		return 1;
	kn_transport_reset(&transport);
	transport.kind = KN_TRANSPORT_KIND_TCP_CLIENT;
	transport.write_fd = fds[1];
	transport.open = 1;
	kn_tx_dispatch_clear(&dispatcher);
	(void)kn_tx_dispatch_add_transport_target(&dispatcher, "kiss0",
	    &transport, KN_CONFIG_PORT_TCP_CONNECT,
	    KN_TRANSPORT_KIND_TCP_CLIENT, 1, 1, 1);
	snapshot.tx_dispatch = &dispatcher;

	if (kn_control_protocol_handle("TX DISPATCH RUN", &snapshot, out,
	    sizeof(out)) != KN_CONTROL_OK) {
		(void)close(fds[0]);
		kn_transport_close(&transport);
		return 1;
	}
	(void)close(fds[0]);
	kn_transport_close(&transport);

	return strstr(out, "OK TX DISPATCH sent=1 failed=0") != NULL ? 0 : 1;
}

static int
test_tx_dispatch_run_port_invalid(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	struct kn_tx_policy policy;
	struct kn_tx_queue queue;
	struct kn_tx_dispatcher dispatcher;
	char out[KN_CONTROL_QUEUE_MAX];

	tx_policy_dispatch_allowed(&policy);
	if (kn_tx_queue_init(&queue, &policy) != KN_TX_QUEUE_OK)
		return 1;
	kn_tx_dispatch_clear(&dispatcher);
	tx_snapshot_init(&snapshot, &daemon, &queue);
	snapshot.tx_dispatch = &dispatcher;

	if (kn_control_protocol_handle("TX DISPATCH RUN PORT bad name",
	    &snapshot, out, sizeof(out)) != KN_CONTROL_ERR_UNKNOWN_COMMAND)
		return 1;

	return strcmp(out, "ERR invalid-port\n") == 0 ? 0 : 1;
}

static int
test_tx_dispatch_status_default(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	struct kn_tx_policy policy;
	struct kn_tx_queue queue;
	char out[KN_CONTROL_QUEUE_MAX];

	kn_tx_policy_defaults(&policy);
	if (kn_tx_queue_init(&queue, &policy) != KN_TX_QUEUE_OK)
		return 1;
	tx_snapshot_init(&snapshot, &daemon, &queue);

	if (kn_control_protocol_handle("TX DISPATCH STATUS", &snapshot, out,
	    sizeof(out)) != KN_CONTROL_OK)
		return 1;

	return strstr(out, "OK TX DISPATCH STATUS enabled=false") != NULL ?
	    0 : 1;
}

static int
test_tx_dispatch_status_enabled(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	struct kn_tx_policy policy;
	struct kn_tx_queue queue;
	struct kn_tx_dispatcher dispatcher;
	char out[KN_CONTROL_QUEUE_MAX];

	tx_policy_dispatch_allowed(&policy);
	if (kn_tx_queue_init(&queue, &policy) != KN_TX_QUEUE_OK)
		return 1;
	kn_tx_dispatch_clear(&dispatcher);
	tx_snapshot_init(&snapshot, &daemon, &queue);
	snapshot.tx_dispatch = &dispatcher;

	if (kn_control_protocol_handle("TX DISPATCH STATUS", &snapshot, out,
	    sizeof(out)) != KN_CONTROL_OK)
		return 1;

	return strstr(out, "enabled=true test_only=true") != NULL ? 0 : 1;
}

static int
test_tx_dryrun_via_valid(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	struct kn_port_stats port;
	struct kn_tx_policy policy;
	struct kn_tx_queue queue;
	char out[KN_CONTROL_QUEUE_MAX];

	tx_policy_control_allowed(&policy);
	if (kn_tx_queue_init(&queue, &policy) != KN_TX_QUEUE_OK)
		return 1;
	tx_snapshot_port_init(&snapshot, &daemon, &port, &queue, 1);

	if (kn_control_protocol_handle("TX DRYRUN UI PORT kiss0 FROM "
	    "M6VPN-1 TO CQ VIA WIDE1-1,WIDE2-1 TEXT hello", &snapshot,
	    out, sizeof(out)) != KN_CONTROL_OK)
		return 1;
	if (kn_control_protocol_handle("TX FRAME 1", &snapshot, out,
	    sizeof(out)) != KN_CONTROL_OK)
		return 1;

	return strstr(out, "via=WIDE1-1,WIDE2-1") != NULL ? 0 : 1;
}

static int
test_tx_malformed_command(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	struct kn_tx_policy policy;
	struct kn_tx_queue queue;
	char out[KN_CONTROL_QUEUE_MAX];

	kn_tx_policy_defaults(&policy);
	policy.enabled = 1;
	if (kn_tx_queue_init(&queue, &policy) != KN_TX_QUEUE_OK)
		return 1;
	tx_snapshot_init(&snapshot, &daemon, &queue);

	if (kn_control_protocol_handle("TX SEND", &snapshot, out,
	    sizeof(out)) != KN_CONTROL_ERR_UNKNOWN_COMMAND)
		return 1;

	return strcmp(out, "ERR invalid-tx-command\n") == 0 ? 0 : 1;
}

static int
test_tx_queue_empty(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	struct kn_tx_policy policy;
	struct kn_tx_queue queue;
	char out[KN_CONTROL_QUEUE_MAX];

	kn_tx_policy_defaults(&policy);
	policy.enabled = 1;
	if (kn_tx_queue_init(&queue, &policy) != KN_TX_QUEUE_OK)
		return 1;
	tx_snapshot_init(&snapshot, &daemon, &queue);

	if (kn_control_protocol_handle("TX QUEUE", &snapshot, out,
	    sizeof(out)) != KN_CONTROL_OK)
		return 1;

	return strcmp(out, "OK TX QUEUE count=0\nEND\n") == 0 ? 0 : 1;
}

static int
test_tx_queue_one(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	struct kn_tx_policy policy;
	struct kn_tx_queue queue;
	char out[KN_CONTROL_QUEUE_MAX];

	kn_tx_policy_defaults(&policy);
	policy.enabled = 1;
	if (kn_tx_queue_init(&queue, &policy) != KN_TX_QUEUE_OK)
		return 1;
	tx_add_frame(&queue);
	tx_snapshot_init(&snapshot, &daemon, &queue);

	if (kn_control_protocol_handle("TX QUEUE", &snapshot, out,
	    sizeof(out)) != KN_CONTROL_OK)
		return 1;

	return strstr(out, "TX FRAME id=1 port=kiss0") != NULL ? 0 : 1;
}

static int
test_tx_queue_port(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	struct kn_tx_policy policy;
	struct kn_tx_queue queue;
	char out[KN_CONTROL_QUEUE_MAX];

	kn_tx_policy_defaults(&policy);
	policy.enabled = 1;
	if (kn_tx_queue_init(&queue, &policy) != KN_TX_QUEUE_OK)
		return 1;
	tx_add_frame(&queue);
	tx_snapshot_init(&snapshot, &daemon, &queue);

	if (kn_control_protocol_handle("TX QUEUE PORT kiss0", &snapshot,
	    out, sizeof(out)) != KN_CONTROL_OK)
		return 1;
	if (strstr(out, "count=1") == NULL)
		return 1;
	if (kn_control_protocol_handle("TX QUEUE PORT bad name", &snapshot,
	    out, sizeof(out)) != KN_CONTROL_ERR_UNKNOWN_COMMAND)
		return 1;

	return strcmp(out, "ERR invalid-port\n") == 0 ? 0 : 1;
}

static int
test_tx_status(void)
{
	struct kn_control_snapshot snapshot;
	struct kn_daemon_stats daemon;
	struct kn_tx_policy policy;
	struct kn_tx_queue queue;
	char out[KN_CONTROL_QUEUE_MAX];

	kn_tx_policy_defaults(&policy);
	if (kn_tx_queue_init(&queue, &policy) != KN_TX_QUEUE_OK)
		return 1;
	tx_snapshot_init(&snapshot, &daemon, &queue);

	if (kn_control_protocol_handle("TX STATUS", &snapshot, out,
	    sizeof(out)) != KN_CONTROL_OK)
		return 1;

	return strstr(out, "OK TX STATUS enabled=false dry_run=true") !=
	    NULL ? 0 : 1;
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
