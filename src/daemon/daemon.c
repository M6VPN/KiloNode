/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/daemon/daemon.c */

#define _POSIX_C_SOURCE 200809L

#include <sys/types.h>

#include <poll.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "kilonode/buffer.h"
#include "kilonode/config.h"
#include "kilonode/control.h"
#include "kilonode/daemon.h"
#include "kilonode/ax25.h"
#include "kilonode/kiss_stream.h"
#include "kilonode/monitor.h"
#include "kilonode/stats.h"
#include "kilonode/transport.h"
#include "kilonode/transport_pty.h"
#include "kilonode/transport_serial.h"
#include "kilonode/transport_stdio.h"
#include "kilonode/transport_tcp.h"
#include "kilonode/transport_unix.h"

#define DAEMON_LINE_BUFSIZ 512
#define DAEMON_READ_BUFSIZ 2048

struct daemon_port {
	const struct kn_config_port *config;
	struct kn_transport transport;
	struct kn_kiss_stream_parser parser;
	struct kn_buffer frame_buf;
	struct kn_port_stats stats;
	uint8_t active;
};

static volatile sig_atomic_t daemon_stop;

static void close_ports(struct daemon_port *, size_t);
static enum kn_daemon_error open_config_port(struct daemon_port *,
	const struct kn_config_port *);
static int control_handle(struct kn_control_socket *,
	const struct kn_daemon_stats *, const struct daemon_port *, size_t);
static int pop_frames(struct daemon_port *, struct kn_daemon_stats *);
static void signal_handler(int);

static void
close_ports(struct daemon_port *ports, size_t port_count)
{
	size_t i;

	for (i = 0; i < port_count; i++) {
		kn_transport_close(&ports[i].transport);
		kn_kiss_stream_free(&ports[i].parser);
		kn_buffer_free(&ports[i].frame_buf);
		ports[i].active = 0;
	}
}

enum kn_daemon_error
kn_daemon_run_foreground(const struct kn_config *config)
{
	struct daemon_port ports[KN_CONFIG_PORT_MAX];
	struct pollfd pollfds[KN_CONFIG_PORT_MAX + 1];
	struct kn_control_socket control;
	struct kn_daemon_stats daemon_stats;
	struct sigaction sa;
	uint8_t read_buf[DAEMON_READ_BUFSIZ];
	size_t i;
	size_t active_count;
	size_t port_count;
	size_t read_len;
	int poll_rc;
	enum kn_transport_error transport_rc;
	enum kn_kiss_stream_error stream_rc;
	uint8_t control_enabled;

	if (config == NULL)
		return KN_DAEMON_ERR_INVALID_ARGUMENT;

	memset(ports, 0, sizeof(ports));
	kn_control_socket_init(&control);
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = signal_handler;
	(void)sigaction(SIGINT, &sa, NULL);
	(void)sigaction(SIGTERM, &sa, NULL);
	daemon_stop = 0;

	kn_daemon_stats_init(&daemon_stats, config->port_count, 0);
	for (i = 0; i < config->port_count; i++) {
		if (config->ports[i].enabled != 0)
			daemon_stats.enabled_ports++;
	}

	port_count = 0;
	for (i = 0; i < config->port_count; i++) {
		kn_port_stats_init(&ports[port_count].stats, &config->ports[i]);
		ports[port_count].config = &config->ports[i];
		if (config->ports[i].enabled == 0) {
			port_count++;
			continue;
		}
		if (open_config_port(&ports[port_count],
		    &config->ports[i]) != KN_DAEMON_OK) {
			close_ports(ports, port_count);
			kn_control_socket_close(&control);
			return KN_DAEMON_ERR_TRANSPORT;
		}
		ports[port_count].stats.open = 1;
		daemon_stats.open_ports++;
		port_count++;
	}

	if (port_count == 0)
		return KN_DAEMON_ERR_CONFIG;

	control_enabled = config->control.has_block != 0 &&
	    config->control.enabled != 0;
	if (control_enabled != 0) {
		if (kn_control_socket_open(&control,
		    config->control.path) != KN_CONTROL_OK) {
			close_ports(ports, port_count);
			return KN_DAEMON_ERR_RUNTIME;
		}
	}

	while (daemon_stop == 0) {
		active_count = 0;
		for (i = 0; i < port_count; i++) {
			if (ports[i].active == 0)
				continue;
			pollfds[active_count].fd = kn_transport_fd(
			    &ports[i].transport);
			pollfds[active_count].events = POLLIN;
			pollfds[active_count].revents = 0;
			active_count++;
		}
		if (control_enabled != 0) {
			pollfds[active_count].fd = kn_control_socket_fd(&control);
			pollfds[active_count].events = POLLIN;
			pollfds[active_count].revents = 0;
			active_count++;
		}

		if (active_count == 0)
			break;

		poll_rc = poll(pollfds, active_count, -1);
		if (poll_rc < 0) {
			if (daemon_stop != 0)
				break;
			close_ports(ports, port_count);
			return KN_DAEMON_ERR_RUNTIME;
		}

		active_count = 0;
		for (i = 0; i < port_count; i++) {
			if (ports[i].active == 0)
				continue;

			if ((pollfds[active_count].revents & POLLIN) == 0) {
				active_count++;
				continue;
			}

			transport_rc = kn_transport_read(&ports[i].transport,
			    read_buf, sizeof(read_buf), &read_len);
			if (transport_rc == KN_TRANSPORT_ERR_EOF) {
				kn_transport_close(&ports[i].transport);
				ports[i].active = 0;
				ports[i].stats.open = 0;
				if (daemon_stats.open_ports > 0)
					daemon_stats.open_ports--;
				active_count++;
				continue;
			}
			if (transport_rc != KN_TRANSPORT_OK) {
				close_ports(ports, port_count);
				return KN_DAEMON_ERR_TRANSPORT;
			}

			stream_rc = kn_kiss_stream_feed(&ports[i].parser,
			    read_buf, read_len);
			kn_stats_add_bytes(&daemon_stats, &ports[i].stats,
			    read_len);
			if (stream_rc != KN_KISS_STREAM_OK) {
				kn_stats_add_malformed_kiss(&daemon_stats,
				    &ports[i].stats, "kiss");
				fprintf(stdout, "%s KISS stream error %d\n",
				    ports[i].config->name, (int)stream_rc);
			}

			if (pop_frames(&ports[i], &daemon_stats) != 0) {
				close_ports(ports, port_count);
				kn_control_socket_close(&control);
				return KN_DAEMON_ERR_RUNTIME;
			}

			active_count++;
		}
		if (control_enabled != 0 &&
		    (pollfds[active_count].revents & POLLIN) != 0) {
			if (control_handle(&control, &daemon_stats, ports,
			    port_count) != 0) {
				close_ports(ports, port_count);
				kn_control_socket_close(&control);
				return KN_DAEMON_ERR_RUNTIME;
			}
		}
	}

	close_ports(ports, port_count);
	kn_control_socket_close(&control);
	return KN_DAEMON_OK;
}

static enum kn_daemon_error
open_config_port(struct daemon_port *port, const struct kn_config_port *config)
{
	char slave_path[KN_TRANSPORT_PTY_PATH_MAX];
	enum kn_transport_error rc;

	port->config = config;
	kn_port_stats_init(&port->stats, config);
	kn_transport_reset(&port->transport);

	if (kn_kiss_stream_init(&port->parser,
	    config->max_frame) != KN_KISS_STREAM_OK)
		return KN_DAEMON_ERR_RUNTIME;
	if (kn_buffer_init(&port->frame_buf, 0) != 0) {
		kn_kiss_stream_free(&port->parser);
		return KN_DAEMON_ERR_RUNTIME;
	}

	switch (config->type) {
	case KN_CONFIG_PORT_STDIO:
		rc = kn_transport_stdio_open(&port->transport);
		break;
	case KN_CONFIG_PORT_TCP_CONNECT:
		rc = kn_transport_tcp_connect_open(&port->transport,
		    config->host, config->port);
		break;
	case KN_CONFIG_PORT_TCP_LISTEN:
		rc = kn_transport_tcp_listen_open(&port->transport,
		    config->host, config->port);
		break;
	case KN_CONFIG_PORT_SERIAL:
		rc = kn_transport_serial_open(&port->transport,
		    config->device, config->baud, config->flow_control);
		break;
	case KN_CONFIG_PORT_PTY:
		rc = kn_transport_pty_open(&port->transport, slave_path,
		    sizeof(slave_path));
		if (rc == KN_TRANSPORT_OK)
			fprintf(stderr, "%s PTY slave: %s\n", config->name,
			    slave_path);
		break;
	case KN_CONFIG_PORT_UNIX_CONNECT:
		rc = kn_transport_unix_connect_open(&port->transport,
		    config->path);
		break;
	case KN_CONFIG_PORT_UNIX_LISTEN:
		rc = kn_transport_unix_listen_open(&port->transport,
		    config->path);
		break;
	case KN_CONFIG_PORT_NONE:
		rc = KN_TRANSPORT_ERR_INVALID_CONFIG;
		break;
	}

	if (rc != KN_TRANSPORT_OK) {
		fprintf(stderr, "%s open failed: %s\n", config->name,
		    kn_transport_error_name(rc));
		kn_kiss_stream_free(&port->parser);
		kn_buffer_free(&port->frame_buf);
		return KN_DAEMON_ERR_TRANSPORT;
	}

	port->active = 1;
	return KN_DAEMON_OK;
}

static int
control_handle(struct kn_control_socket *control,
	const struct kn_daemon_stats *daemon_stats, const struct daemon_port *ports,
	size_t port_count)
{
	struct kn_port_stats port_stats[KN_CONFIG_PORT_MAX];
	struct kn_control_snapshot snapshot;
	char command[KN_CONTROL_COMMAND_MAX];
	char response[KN_CONTROL_QUEUE_MAX];
	size_t command_len;
	size_t i;
	enum kn_control_error control_rc;

	control_rc = kn_control_socket_read_command(control, command,
	    sizeof(command), &command_len);
	if (control_rc == KN_CONTROL_ERR_OVERLONG_COMMAND) {
		(void)kn_control_socket_write(control->client_fd,
		    "ERR overlong-command\n", 21);
		(void)close(control->client_fd);
		control->client_fd = -1;
		return 0;
	}
	if (control_rc != KN_CONTROL_OK)
		return 1;

	(void)command_len;
	for (i = 0; i < port_count; i++)
		port_stats[i] = ports[i].stats;

	snapshot.daemon = daemon_stats;
	snapshot.ports = port_stats;
	snapshot.port_count = port_count;
	control_rc = kn_control_protocol_handle(command, &snapshot, response,
	    sizeof(response));
	(void)control_rc;

	if (kn_control_socket_write(control->client_fd, response,
	    strlen(response)) != KN_CONTROL_OK)
		return 1;

	(void)close(control->client_fd);
	control->client_fd = -1;
	return 0;
}

static int
pop_frames(struct daemon_port *port, struct kn_daemon_stats *daemon_stats)
{
	struct kn_kiss_stream_frame frame;
	char line[DAEMON_LINE_BUFSIZ];
	struct kn_ax25_frame ax25;
	enum kn_kiss_stream_error stream_rc;
	enum kn_monitor_error monitor_rc;

	while (kn_kiss_stream_has_frame(&port->parser) != 0) {
		stream_rc = kn_kiss_stream_pop_frame(&port->parser, &frame,
		    &port->frame_buf);
		if (stream_rc != KN_KISS_STREAM_OK)
			return 1;

		kn_stats_add_kiss_frame(daemon_stats, &port->stats);
		if (frame.command == 0 &&
		    kn_ax25_frame_decode(frame.payload,
		    frame.payload_len, &ax25) == KN_AX25_OK)
			kn_stats_add_ax25_frame(daemon_stats, &port->stats);
		else if (frame.command == 0)
			kn_stats_add_malformed_ax25(daemon_stats, &port->stats,
			    "ax25");

		monitor_rc = kn_monitor_format_kiss(line, sizeof(line),
		    frame.port, frame.command, frame.payload, frame.payload_len);
		if (monitor_rc != KN_MONITOR_OK)
			printf("%s monitor format error %d\n",
			    port->config->name, (int)monitor_rc);
		else
			printf("%s %s\n", port->config->name, line);
	}

	return 0;
}

static void
signal_handler(int sig)
{
	(void)sig;
	daemon_stop = 1;
}
