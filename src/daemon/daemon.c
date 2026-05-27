/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/daemon/daemon.c */

#define _POSIX_C_SOURCE 200809L

#include <sys/types.h>

#include <poll.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "kilonode/buffer.h"
#include "kilonode/bbs_read_state.h"
#include "kilonode/bbs_user.h"
#include "kilonode/config.h"
#include "kilonode/control.h"
#include "kilonode/daemon.h"
#include "kilonode/ax25.h"
#include "kilonode/heard.h"
#include "kilonode/kiss_stream.h"
#include "kilonode/message_store.h"
#include "kilonode/monitor.h"
#include "kilonode/node_shell.h"
#include "kilonode/rx_event.h"
#include "kilonode/rx_queue.h"
#include "kilonode/rx_session.h"
#include "kilonode/stats.h"
#include "kilonode/tx_dispatch.h"
#include "kilonode/tx_queue.h"
#include "kilonode/transport.h"
#include "kilonode/transport_memory.h"
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
	struct kn_transport_memory memory;
	uint8_t active;
};

static volatile sig_atomic_t daemon_stop;

static void close_ports(struct daemon_port *, size_t);
static enum kn_daemon_error open_config_port(struct daemon_port *,
	const struct kn_config_port *);
static int control_handle(struct kn_control_socket *,
	const struct kn_daemon_stats *, const struct daemon_port *, size_t,
	const struct kn_heard_table *, uint8_t, struct kn_message_store *,
	uint8_t, const struct kn_access_policy *, const struct kn_rx_queue *,
	const struct kn_rx_session_table *, uint8_t,
	struct kn_tx_queue *, struct kn_tx_dispatcher *);
static int pop_frames(struct daemon_port *, struct kn_daemon_stats *,
	struct kn_heard_table *, uint8_t, struct kn_rx_queue *,
	struct kn_rx_session_table *);
static void shell_snapshot_init(struct kn_node_shell_snapshot *,
	const struct kn_config *, const struct kn_daemon_stats *,
	const struct daemon_port *, size_t, const struct kn_heard_table *,
	uint8_t, const struct kn_node_shell_state *,
	struct kn_port_stats *, struct kn_node_shell_user *, size_t *,
	struct kn_message_store *, uint8_t);
static void signal_handler(int);

static void
close_ports(struct daemon_port *ports, size_t port_count)
{
	size_t i;

	for (i = 0; i < port_count; i++) {
		kn_transport_close(&ports[i].transport);
		kn_transport_memory_free(&ports[i].memory);
		kn_kiss_stream_free(&ports[i].parser);
		kn_buffer_free(&ports[i].frame_buf);
		ports[i].active = 0;
	}
}

enum kn_daemon_error
kn_daemon_run_foreground(const struct kn_config *config)
{
	struct daemon_port ports[KN_CONFIG_PORT_MAX];
	struct pollfd pollfds[KN_CONFIG_PORT_MAX + 2 +
	    KN_NODE_SHELL_MAX_CLIENTS];
	struct kn_node_shell_session *shell_sessions[KN_NODE_SHELL_MAX_CLIENTS];
	struct kn_control_socket control;
	struct kn_daemon_stats daemon_stats;
	struct kn_heard_table heard;
	struct kn_rx_queue rx_events;
	struct kn_rx_session_table rx_sessions;
	struct kn_tx_queue tx_queue;
	struct kn_tx_dispatcher tx_dispatch;
	struct kn_message_store bbs_store;
	struct kn_node_shell_state shell;
	struct sigaction sa;
	uint8_t read_buf[DAEMON_READ_BUFSIZ];
	struct kn_node_shell_snapshot shell_snapshot;
	struct kn_node_shell_user shell_users[KN_NODE_SHELL_MAX_CLIENTS];
	struct kn_port_stats shell_port_stats[KN_CONFIG_PORT_MAX];
	size_t i;
	size_t control_poll;
	size_t poll_count;
	size_t poll_index;
	size_t port_count;
	size_t read_len;
	size_t shell_poll;
	size_t shell_session_count;
	size_t shell_user_count;
	int poll_rc;
	enum kn_transport_error transport_rc;
	enum kn_kiss_stream_error stream_rc;
	uint8_t control_enabled;
	uint8_t bbs_enabled;
	uint8_t shell_enabled;

	if (config == NULL)
		return KN_DAEMON_ERR_INVALID_ARGUMENT;

	memset(ports, 0, sizeof(ports));
	kn_control_socket_init(&control);
	kn_message_store_init(&bbs_store);
	kn_node_shell_init(&shell);
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = signal_handler;
	(void)sigaction(SIGINT, &sa, NULL);
	(void)sigaction(SIGTERM, &sa, NULL);
	daemon_stop = 0;

	kn_daemon_stats_init(&daemon_stats, config->port_count, 0);
	kn_heard_init(&heard, config->heard.max_entries);
	kn_rx_queue_init(&rx_events, config->receive.max_events,
	    config->receive.payload_preview_bytes,
	    config->receive.events_enabled);
	kn_rx_session_init(&rx_sessions, config->receive.max_sessions);
	if (kn_tx_queue_init(&tx_queue, &config->transmit.policy) !=
	    KN_TX_QUEUE_OK)
		return KN_DAEMON_ERR_CONFIG;
	kn_tx_dispatch_clear(&tx_dispatch);
	for (i = 0; i < config->port_count; i++) {
		if (config->ports[i].enabled != 0)
			daemon_stats.enabled_ports++;
	}

	control_enabled = config->control.has_block != 0 &&
	    config->control.enabled != 0;
	bbs_enabled = config->bbs.has_block != 0 && config->bbs.enabled != 0;
	shell_enabled = config->shell.has_block != 0 &&
	    config->shell.enabled != 0;

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
			kn_message_store_close(&bbs_store);
			return KN_DAEMON_ERR_TRANSPORT;
		}
		ports[port_count].stats.open = 1;
		if (config->ports[i].type == KN_CONFIG_PORT_MEMORY_TEST) {
			if (kn_tx_dispatch_add_memory_target(&tx_dispatch,
			    config->ports[i].name, &ports[port_count].memory,
			    1, 1) != KN_TX_DISPATCH_OK) {
				close_ports(ports, port_count + 1);
				kn_control_socket_close(&control);
				kn_message_store_close(&bbs_store);
				return KN_DAEMON_ERR_RUNTIME;
			}
		}
		daemon_stats.open_ports++;
		port_count++;
	}

	if (port_count == 0 && control_enabled == 0 && shell_enabled == 0)
		return KN_DAEMON_ERR_CONFIG;

	if (bbs_enabled != 0) {
		if (kn_message_store_open(&bbs_store, config->bbs.store_path,
		    config->bbs.max_body_bytes) != KN_MESSAGE_STORE_OK) {
			close_ports(ports, port_count);
			kn_control_socket_close(&control);
			return KN_DAEMON_ERR_RUNTIME;
		}
		if (kn_bbs_user_init_store(&bbs_store) != KN_BBS_USER_OK ||
		    kn_bbs_read_state_init_store(&bbs_store) !=
		    KN_BBS_READ_STATE_OK) {
			close_ports(ports, port_count);
			kn_control_socket_close(&control);
			kn_message_store_close(&bbs_store);
			return KN_DAEMON_ERR_RUNTIME;
		}
	}

	if (control_enabled != 0) {
		if (kn_control_socket_open(&control,
		    config->control.path) != KN_CONTROL_OK) {
			close_ports(ports, port_count);
			kn_message_store_close(&bbs_store);
			return KN_DAEMON_ERR_RUNTIME;
		}
	}

	if (shell_enabled != 0) {
		if (kn_node_shell_open(&config->shell,
		    &shell) != KN_NODE_SHELL_OK) {
			close_ports(ports, port_count);
			kn_control_socket_close(&control);
			kn_message_store_close(&bbs_store);
			return KN_DAEMON_ERR_RUNTIME;
		}
		kn_node_shell_set_policy(&shell, &config->access.policy);
	}

	while (daemon_stop == 0) {
		if (shell_enabled != 0)
			kn_node_shell_prune_idle(&shell, (uint64_t)time(NULL));
		poll_count = 0;
		for (i = 0; i < port_count; i++) {
			if (ports[i].active == 0)
				continue;
			pollfds[poll_count].fd = kn_transport_fd(
			    &ports[i].transport);
			pollfds[poll_count].events = POLLIN;
			pollfds[poll_count].revents = 0;
			poll_count++;
		}
		control_poll = (size_t)-1;
		if (control_enabled != 0) {
			control_poll = poll_count;
			pollfds[poll_count].fd = kn_control_socket_fd(&control);
			pollfds[poll_count].events = POLLIN;
			pollfds[poll_count].revents = 0;
			poll_count++;
		}
		shell_poll = (size_t)-1;
		if (shell_enabled != 0) {
			shell_poll = poll_count;
			pollfds[poll_count].fd = kn_node_shell_fd(&shell);
			pollfds[poll_count].events = POLLIN;
			pollfds[poll_count].revents = 0;
			poll_count++;

			shell_session_count = 0;
			for (i = 0; i < shell.max_clients; i++) {
				if (shell.sessions[i].fd < 0 ||
				    shell.sessions[i].closed != 0)
					continue;
				shell_sessions[shell_session_count++] =
				    &shell.sessions[i];
				pollfds[poll_count].fd = shell.sessions[i].fd;
				pollfds[poll_count].events = POLLIN;
				pollfds[poll_count].revents = 0;
				poll_count++;
			}
		} else {
			shell_session_count = 0;
		}

		if (poll_count == 0)
			break;

		poll_rc = poll(pollfds, poll_count, -1);
		if (poll_rc < 0) {
			if (daemon_stop != 0)
				break;
			close_ports(ports, port_count);
			kn_node_shell_close(&shell);
			kn_message_store_close(&bbs_store);
			return KN_DAEMON_ERR_RUNTIME;
		}

		poll_index = 0;
		for (i = 0; i < port_count; i++) {
			if (ports[i].active == 0)
				continue;

			if ((pollfds[poll_index].revents & POLLIN) == 0) {
				poll_index++;
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
				poll_index++;
				continue;
			}
			if (transport_rc != KN_TRANSPORT_OK) {
				close_ports(ports, port_count);
				kn_node_shell_close(&shell);
				kn_message_store_close(&bbs_store);
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

			if (pop_frames(&ports[i], &daemon_stats, &heard,
			    config->heard.enabled, &rx_events,
			    &rx_sessions) != 0) {
				close_ports(ports, port_count);
				kn_control_socket_close(&control);
				kn_node_shell_close(&shell);
				kn_message_store_close(&bbs_store);
				return KN_DAEMON_ERR_RUNTIME;
			}

			poll_index++;
		}
		if (control_enabled != 0 && control_poll != (size_t)-1 &&
		    (pollfds[control_poll].revents & POLLIN) != 0) {
			if (control_handle(&control, &daemon_stats, ports,
			    port_count, &heard, config->heard.enabled,
			    &bbs_store, bbs_enabled,
			    &config->access.policy, &rx_events, &rx_sessions,
			    config->receive.events_enabled, &tx_queue,
			    &tx_dispatch) != 0) {
				close_ports(ports, port_count);
				kn_control_socket_close(&control);
				kn_node_shell_close(&shell);
				kn_message_store_close(&bbs_store);
				return KN_DAEMON_ERR_RUNTIME;
			}
		}
		if (shell_enabled != 0 && shell_poll != (size_t)-1 &&
		    (pollfds[shell_poll].revents & POLLIN) != 0) {
			if (kn_node_shell_accept(&shell,
			    config->shell.banner) != KN_NODE_SHELL_OK) {
				close_ports(ports, port_count);
				kn_control_socket_close(&control);
				kn_node_shell_close(&shell);
				kn_message_store_close(&bbs_store);
				return KN_DAEMON_ERR_RUNTIME;
			}
		}
		for (i = 0; i < shell_session_count; i++) {
			poll_index = shell_poll + 1 + i;
			if ((pollfds[poll_index].revents & POLLIN) == 0)
				continue;
			shell_snapshot_init(&shell_snapshot, config,
			    &daemon_stats, ports, port_count, &heard,
			    config->heard.enabled, &shell, shell_port_stats,
			    shell_users, &shell_user_count, &bbs_store,
			    bbs_enabled);
			if (kn_node_shell_process_session(shell_sessions[i],
			    &shell_snapshot) != KN_NODE_SHELL_OK) {
				close_ports(ports, port_count);
				kn_control_socket_close(&control);
				kn_node_shell_close(&shell);
				kn_message_store_close(&bbs_store);
				return KN_DAEMON_ERR_RUNTIME;
			}
		}
	}

	close_ports(ports, port_count);
	kn_control_socket_close(&control);
	kn_node_shell_close(&shell);
	kn_message_store_close(&bbs_store);
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
	rc = KN_TRANSPORT_ERR_INVALID_CONFIG;

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
	case KN_CONFIG_PORT_MEMORY_TEST:
		rc = kn_transport_memory_init(&port->memory,
		    KN_TRANSPORT_MEMORY_CAPACITY_DEFAULT) ==
		    KN_TRANSPORT_MEMORY_OK &&
		    kn_transport_memory_open(&port->memory) ==
		    KN_TRANSPORT_MEMORY_OK ? KN_TRANSPORT_OK :
		    KN_TRANSPORT_ERR_OPEN;
		kn_transport_reset(&port->transport);
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
	size_t port_count, const struct kn_heard_table *heard,
	uint8_t heard_enabled, struct kn_message_store *bbs_store,
	uint8_t bbs_enabled, const struct kn_access_policy *policy,
	const struct kn_rx_queue *rx_events,
	const struct kn_rx_session_table *rx_sessions, uint8_t rx_enabled,
	struct kn_tx_queue *tx_queue, struct kn_tx_dispatcher *tx_dispatch)
{
	struct kn_port_stats port_stats[KN_CONFIG_PORT_MAX];
	struct kn_control_snapshot snapshot;
	char command[KN_CONTROL_COMMAND_MAX];
	char response[KN_CONTROL_QUEUE_MAX];
	size_t command_max;
	size_t command_len;
	size_t i;
	enum kn_control_error control_rc;

	command_max = sizeof(command);
	if (policy != NULL && policy->control_max_command_bytes + 1 <
	    command_max)
		command_max = policy->control_max_command_bytes + 1;
	control_rc = kn_control_socket_read_command(control, command,
	    command_max, &command_len);
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
	if (heard_enabled != 0) {
		snapshot.heard = kn_heard_entries(heard);
		snapshot.heard_count = kn_heard_count(heard);
	} else {
		snapshot.heard = NULL;
		snapshot.heard_count = 0;
	}
	snapshot.bbs_enabled = bbs_enabled;
	snapshot.bbs_store = bbs_enabled != 0 ? bbs_store : NULL;
	snapshot.rx_enabled = rx_enabled;
	snapshot.rx_events = rx_enabled != 0 ? rx_events : NULL;
	snapshot.rx_sessions = rx_enabled != 0 ? rx_sessions : NULL;
	snapshot.tx_queue = tx_queue;
	snapshot.tx_dispatch = tx_dispatch;
	if (policy != NULL) {
		snapshot.control_max_command_bytes =
		    policy->control_max_command_bytes;
		snapshot.control_max_response_lines =
		    policy->control_max_response_lines;
	} else {
		snapshot.control_max_command_bytes = 0;
		snapshot.control_max_response_lines = 0;
	}
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
pop_frames(struct daemon_port *port, struct kn_daemon_stats *daemon_stats,
	struct kn_heard_table *heard, uint8_t heard_enabled,
	struct kn_rx_queue *rx_events, struct kn_rx_session_table *rx_sessions)
{
	struct kn_kiss_stream_frame frame;
	char line[DAEMON_LINE_BUFSIZ];
	struct kn_ax25_frame ax25;
	struct kn_rx_event event;
	enum kn_kiss_stream_error stream_rc;
	enum kn_monitor_error monitor_rc;
	enum kn_ax25_error ax25_rc;
	uint64_t now;

	while (kn_kiss_stream_has_frame(&port->parser) != 0) {
		stream_rc = kn_kiss_stream_pop_frame(&port->parser, &frame,
		    &port->frame_buf);
		if (stream_rc != KN_KISS_STREAM_OK)
			return 1;

		kn_stats_add_kiss_frame(daemon_stats, &port->stats);
		now = (uint64_t)time(NULL);
		if (frame.command == 0) {
			ax25_rc = kn_ax25_frame_decode(frame.payload,
			    frame.payload_len, &ax25);
			if (ax25_rc == KN_AX25_OK) {
				kn_stats_add_ax25_frame(daemon_stats,
				    &port->stats);
				if (heard_enabled != 0) {
					(void)kn_heard_update(heard,
					    port->config->name, &ax25, now);
				}
				if (rx_events != NULL &&
				    rx_events->enabled != 0) {
					if (kn_rx_event_from_ax25(&event,
					    kn_rx_queue_reserve_id(rx_events),
					    now, port->config->name, frame.port,
					    frame.command, &ax25,
					    rx_events->preview_bytes) ==
					    KN_RX_EVENT_OK) {
						(void)kn_rx_queue_push(rx_events,
						    &event);
						if (rx_sessions != NULL)
							(void)kn_rx_session_update(
							    rx_sessions,
							    &event);
					}
				}
			} else {
				kn_stats_add_malformed_ax25(daemon_stats,
				    &port->stats, "ax25");
				if (rx_events != NULL &&
				    rx_events->enabled != 0) {
					if (kn_rx_event_from_malformed(&event,
					    kn_rx_queue_reserve_id(rx_events),
					    now, port->config->name, frame.port,
					    frame.command, (int)ax25_rc,
					    frame.payload_len) ==
					    KN_RX_EVENT_OK)
						(void)kn_rx_queue_push(rx_events,
						    &event);
				}
			}
		}

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
shell_snapshot_init(struct kn_node_shell_snapshot *snapshot,
	const struct kn_config *config, const struct kn_daemon_stats *daemon_stats,
	const struct daemon_port *ports, size_t port_count,
	const struct kn_heard_table *heard, uint8_t heard_enabled,
	const struct kn_node_shell_state *shell,
	struct kn_port_stats *port_stats, struct kn_node_shell_user *users,
	size_t *user_count, struct kn_message_store *bbs_store,
	uint8_t bbs_enabled)
{
	size_t i;

	for (i = 0; i < port_count; i++)
		port_stats[i] = ports[i].stats;

	kn_node_shell_snapshot_users(shell, users, KN_NODE_SHELL_MAX_CLIENTS,
	    user_count);

	snapshot->node = &config->node;
	snapshot->daemon = daemon_stats;
	snapshot->ports = port_stats;
	snapshot->port_count = port_count;
	if (heard_enabled != 0) {
		snapshot->heard = kn_heard_entries(heard);
		snapshot->heard_count = kn_heard_count(heard);
	} else {
		snapshot->heard = NULL;
		snapshot->heard_count = 0;
	}
	snapshot->users = users;
	snapshot->user_count = *user_count;
	snapshot->policy = &config->access.policy;
	if (bbs_enabled != 0) {
		snapshot->bbs.store = bbs_store;
		snapshot->bbs.policy = &config->access.policy;
		snapshot->bbs.enabled = 1;
		snapshot->bbs.max_body_bytes = config->bbs.max_body_bytes <
		    config->access.policy.bbs_max_body_bytes ?
		    config->bbs.max_body_bytes :
		    config->access.policy.bbs_max_body_bytes;
	} else {
		snapshot->bbs.store = NULL;
		snapshot->bbs.policy = &config->access.policy;
		snapshot->bbs.enabled = 0;
		snapshot->bbs.max_body_bytes =
		    config->access.policy.bbs_max_body_bytes;
	}
}

static void
signal_handler(int sig)
{
	(void)sig;
	daemon_stop = 1;
}
