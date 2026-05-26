/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/core/stats.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>
#include <time.h>

#include "kilonode/stats.h"

static void set_last_error(struct kn_port_stats *, const char *);

void
kn_daemon_stats_init(struct kn_daemon_stats *stats, size_t configured_ports,
	size_t enabled_ports)
{
	if (stats == NULL)
		return;

	memset(stats, 0, sizeof(*stats));
	stats->start_time = (uint64_t)time(NULL);
	stats->configured_ports = (uint64_t)configured_ports;
	stats->enabled_ports = (uint64_t)enabled_ports;
}

void
kn_port_stats_init(struct kn_port_stats *stats, const struct kn_config_port *port)
{
	int needed;

	if (stats == NULL || port == NULL)
		return;

	memset(stats, 0, sizeof(*stats));
	needed = snprintf(stats->name, sizeof(stats->name), "%s", port->name);
	if (needed < 0 || (size_t)needed >= sizeof(stats->name))
		stats->name[0] = '\0';
	stats->type = port->type;
	stats->enabled = port->enabled;
}

void
kn_stats_add_ax25_frame(struct kn_daemon_stats *daemon,
	struct kn_port_stats *port)
{
	if (daemon != NULL)
		daemon->ax25_frames_decoded++;
	if (port != NULL)
		port->ax25_frames_decoded++;
}

void
kn_stats_add_bytes(struct kn_daemon_stats *daemon, struct kn_port_stats *port,
	size_t bytes)
{
	if (daemon != NULL)
		daemon->bytes_received += (uint64_t)bytes;
	if (port != NULL)
		port->bytes_received += (uint64_t)bytes;
}

void
kn_stats_add_kiss_frame(struct kn_daemon_stats *daemon,
	struct kn_port_stats *port)
{
	uint64_t now;

	now = (uint64_t)time(NULL);
	if (daemon != NULL)
		daemon->kiss_frames_received++;
	if (port != NULL) {
		port->kiss_frames_received++;
		port->last_frame_time = now;
	}
}

void
kn_stats_add_malformed_ax25(struct kn_daemon_stats *daemon,
	struct kn_port_stats *port, const char *error)
{
	if (daemon != NULL)
		daemon->malformed_ax25_frames++;
	if (port != NULL) {
		port->malformed_frames++;
		set_last_error(port, error);
	}
}

void
kn_stats_add_malformed_kiss(struct kn_daemon_stats *daemon,
	struct kn_port_stats *port, const char *error)
{
	if (daemon != NULL)
		daemon->malformed_kiss_frames++;
	if (port != NULL) {
		port->malformed_frames++;
		set_last_error(port, error);
	}
}

const char *
kn_stats_port_type_name(enum kn_config_port_type type)
{
	switch (type) {
	case KN_CONFIG_PORT_STDIO:
		return "stdio";
	case KN_CONFIG_PORT_TCP_CONNECT:
		return "tcp-connect";
	case KN_CONFIG_PORT_TCP_LISTEN:
		return "tcp-listen";
	case KN_CONFIG_PORT_SERIAL:
		return "serial";
	case KN_CONFIG_PORT_PTY:
		return "pty";
	case KN_CONFIG_PORT_UNIX_CONNECT:
		return "unix-connect";
	case KN_CONFIG_PORT_UNIX_LISTEN:
		return "unix-listen";
	case KN_CONFIG_PORT_NONE:
		return "none";
	}

	return "unknown";
}

static void
set_last_error(struct kn_port_stats *port, const char *error)
{
	int needed;

	needed = snprintf(port->last_error, sizeof(port->last_error), "%s",
	    error == NULL ? "" : error);
	if (needed < 0 || (size_t)needed >= sizeof(port->last_error))
		port->last_error[0] = '\0';
}
