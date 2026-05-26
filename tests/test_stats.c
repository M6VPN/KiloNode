/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_stats.c */

#include <sys/types.h>

#include <string.h>

#include "kilonode/config.h"
#include "kilonode/stats.h"

static void port_config_init(struct kn_config_port *);
static int test_counter_updates(void);
static int test_initial_stats(void);
static int test_port_type_names(void);

int
main(void)
{
	if (test_initial_stats() != 0)
		return 1;
	if (test_counter_updates() != 0)
		return 1;
	if (test_port_type_names() != 0)
		return 1;

	return 0;
}

static void
port_config_init(struct kn_config_port *port)
{
	memset(port, 0, sizeof(*port));
	memcpy(port->name, "kiss0", 6);
	port->type = KN_CONFIG_PORT_TCP_LISTEN;
	port->enabled = 1;
}

static int
test_counter_updates(void)
{
	struct kn_daemon_stats daemon;
	struct kn_port_stats port_stats;
	struct kn_config_port port_config;

	port_config_init(&port_config);
	kn_daemon_stats_init(&daemon, 1, 1);
	kn_port_stats_init(&port_stats, &port_config);

	kn_stats_add_bytes(&daemon, &port_stats, 42);
	kn_stats_add_kiss_frame(&daemon, &port_stats);
	kn_stats_add_ax25_frame(&daemon, &port_stats);
	kn_stats_add_malformed_kiss(&daemon, &port_stats, "kiss");
	kn_stats_add_malformed_ax25(&daemon, &port_stats, "ax25");

	if (daemon.bytes_received != 42 || port_stats.bytes_received != 42)
		return 1;
	if (daemon.kiss_frames_received != 1 ||
	    port_stats.kiss_frames_received != 1)
		return 1;
	if (daemon.ax25_frames_decoded != 1 ||
	    port_stats.ax25_frames_decoded != 1)
		return 1;
	if (daemon.malformed_kiss_frames != 1 ||
	    daemon.malformed_ax25_frames != 1)
		return 1;
	if (port_stats.malformed_frames != 2)
		return 1;

	return strcmp(port_stats.last_error, "ax25") == 0 ? 0 : 1;
}

static int
test_initial_stats(void)
{
	struct kn_daemon_stats daemon;
	struct kn_port_stats port_stats;
	struct kn_config_port port_config;

	port_config_init(&port_config);
	kn_daemon_stats_init(&daemon, 3, 2);
	kn_port_stats_init(&port_stats, &port_config);

	if (daemon.configured_ports != 3 || daemon.enabled_ports != 2)
		return 1;
	if (daemon.bytes_received != 0 || daemon.open_ports != 0)
		return 1;
	if (strcmp(port_stats.name, "kiss0") != 0)
		return 1;
	if (port_stats.enabled != 1 || port_stats.open != 0)
		return 1;

	return 0;
}

static int
test_port_type_names(void)
{
	if (strcmp(kn_stats_port_type_name(KN_CONFIG_PORT_TCP_LISTEN),
	    "tcp-listen") != 0)
		return 1;
	if (strcmp(kn_stats_port_type_name(KN_CONFIG_PORT_SERIAL),
	    "serial") != 0)
		return 1;

	return 0;
}
