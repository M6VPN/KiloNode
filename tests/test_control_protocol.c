/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_control_protocol.c */

#include <sys/types.h>

#include <string.h>

#include "kilonode/control.h"
#include "kilonode/stats.h"

static void snapshot_init(struct kn_control_snapshot *,
	struct kn_daemon_stats *, struct kn_port_stats *, size_t);
static int test_control_character_command(void);
static int test_help_response(void);
static int test_overlong_command(void);
static int test_ping_response(void);
static int test_ports_one_port(void);
static int test_ports_zero_ports(void);
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
	if (test_help_response() != 0)
		return 1;
	if (test_unknown_command() != 0)
		return 1;
	if (test_overlong_command() != 0)
		return 1;
	if (test_control_character_command() != 0)
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
