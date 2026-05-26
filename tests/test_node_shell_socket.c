/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_node_shell_socket.c */

#define _POSIX_C_SOURCE 200809L

#include <sys/socket.h>
#include <sys/types.h>

#include <netdb.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "kilonode/callsign.h"
#include "kilonode/config.h"
#include "kilonode/node_shell.h"

static int connect_loopback(uint16_t);
static void snapshot_init(struct kn_node_shell_snapshot *,
	struct kn_config_node *, struct kn_daemon_stats *);
static int test_listener_help_bye(void);

int
main(void)
{
	if (test_listener_help_bye() != 0)
		return 1;

	return 0;
}

static int
connect_loopback(uint16_t port)
{
	struct addrinfo hints;
	struct addrinfo *result;
	char port_text[16];
	int fd;
	int rc;

	(void)snprintf(port_text, sizeof(port_text), "%u",
	    (unsigned int)port);
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	if (getaddrinfo("127.0.0.1", port_text, &hints, &result) != 0)
		return -1;

	fd = socket(result->ai_family, result->ai_socktype,
	    result->ai_protocol);
	if (fd < 0) {
		freeaddrinfo(result);
		return -1;
	}
	rc = connect(fd, result->ai_addr, result->ai_addrlen);
	freeaddrinfo(result);
	if (rc != 0) {
		(void)close(fd);
		return -1;
	}

	return fd;
}

static void
snapshot_init(struct kn_node_shell_snapshot *snapshot,
	struct kn_config_node *node, struct kn_daemon_stats *daemon)
{
	memset(node, 0, sizeof(*node));
	memset(daemon, 0, sizeof(*daemon));
	(void)kn_callsign_parse("M6VPN-1", &node->callsign);
	node->has_callsign = 1;
	snapshot->node = node;
	snapshot->daemon = daemon;
	snapshot->ports = NULL;
	snapshot->port_count = 0;
	snapshot->heard = NULL;
	snapshot->heard_count = 0;
	snapshot->users = NULL;
	snapshot->user_count = 0;
}

static int
test_listener_help_bye(void)
{
	struct kn_config_shell config;
	struct kn_node_shell_state state;
	struct kn_node_shell_snapshot snapshot;
	struct kn_config_node node;
	struct kn_daemon_stats daemon;
	char buf[KN_NODE_SHELL_RESPONSE_MAX];
	ssize_t nread;
	int client_fd;
	uint16_t port;

	memset(&config, 0, sizeof(config));
	memcpy(config.host, "127.0.0.1", 10);
	memcpy(config.port, "0", 2);
	memcpy(config.banner, "KiloNode test shell", 20);
	config.max_clients = 1;

	kn_node_shell_init(&state);
	if (kn_node_shell_open(&config, &state) != KN_NODE_SHELL_OK)
		return 1;
	port = kn_node_shell_port(&state);
	if (port == 0) {
		kn_node_shell_close(&state);
		return 1;
	}
	client_fd = connect_loopback(port);
	if (client_fd < 0) {
		kn_node_shell_close(&state);
		return 1;
	}
	if (kn_node_shell_accept(&state, config.banner) != KN_NODE_SHELL_OK)
		return 1;
	nread = read(client_fd, buf, sizeof(buf) - 1);
	if (nread <= 0)
		return 1;
	buf[nread] = '\0';
	if (strstr(buf, "KiloNode test shell") == NULL)
		return 1;
	if (strstr(buf, KN_NODE_SHELL_PROMPT) == NULL)
		return 1;

	snapshot_init(&snapshot, &node, &daemon);
	if (write(client_fd, "HELP\n", 5) != 5)
		return 1;
	if (kn_node_shell_process_session(&state.sessions[0],
	    &snapshot) != KN_NODE_SHELL_OK)
		return 1;
	nread = read(client_fd, buf, sizeof(buf) - 1);
	if (nread <= 0)
		return 1;
	buf[nread] = '\0';
	if (strstr(buf, "OK HELP") == NULL)
		return 1;

	if (write(client_fd, "BYE\n", 4) != 4)
		return 1;
	if (kn_node_shell_process_session(&state.sessions[0],
	    &snapshot) != KN_NODE_SHELL_OK)
		return 1;
	nread = read(client_fd, buf, sizeof(buf) - 1);
	if (nread <= 0)
		return 1;
	buf[nread] = '\0';
	if (strcmp(buf, "BYE\r\n") != 0)
		return 1;

	(void)close(client_fd);
	kn_node_shell_close(&state);
	return 0;
}
