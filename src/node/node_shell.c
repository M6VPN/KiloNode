/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/node/node_shell.c */

#define _POSIX_C_SOURCE 200809L

#include <sys/socket.h>
#include <sys/types.h>

#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "kilonode/callsign.h"
#include "kilonode/config.h"
#include "kilonode/heard.h"
#include "kilonode/node_shell.h"
#include "kilonode/stats.h"

#define KILONODE_VERSION "0.1.0"

static enum kn_node_shell_error append_format(char *, size_t, size_t *,
	const char *, ...);
static enum kn_node_shell_error append_prompt(char *, size_t, size_t *);
static enum kn_node_shell_error append_safe(char *, size_t, size_t *,
	const char *);
static size_t active_count(const struct kn_node_shell_state *);
static void close_fd(int *);
static enum kn_node_shell_error command_heard(const char *,
	const struct kn_node_shell_snapshot *, char *, size_t, size_t *);
static enum kn_node_shell_error command_info(
	const struct kn_node_shell_snapshot *, char *, size_t, size_t *);
static enum kn_node_shell_error command_ports(
	const struct kn_node_shell_snapshot *, char *, size_t, size_t *);
static enum kn_node_shell_error command_stats(
	const struct kn_node_shell_snapshot *, char *, size_t, size_t *);
static enum kn_node_shell_error command_users(
	const struct kn_node_shell_snapshot *, char *, size_t, size_t *);
static void remote_string(const struct sockaddr_storage *, char *, size_t);
static enum kn_node_shell_error send_greeting(int, const char *);
static enum kn_node_shell_error socket_open(const struct kn_config_shell *,
	int *);
static enum kn_node_shell_error write_all(int, const char *, size_t);

static enum kn_node_shell_error
append_format(char *buf, size_t bufsiz, size_t *offset, const char *fmt, ...)
{
	va_list args;
	int needed;

	if (*offset >= bufsiz)
		return KN_NODE_SHELL_ERR_IO;

	va_start(args, fmt);
	needed = vsnprintf(buf + *offset, bufsiz - *offset, fmt, args);
	va_end(args);
	if (needed < 0 || (size_t)needed >= bufsiz - *offset)
		return KN_NODE_SHELL_ERR_IO;

	*offset += (size_t)needed;
	return KN_NODE_SHELL_OK;
}

static enum kn_node_shell_error
append_prompt(char *buf, size_t bufsiz, size_t *offset)
{
	return append_format(buf, bufsiz, offset, "%s", KN_NODE_SHELL_PROMPT);
}

static enum kn_node_shell_error
append_safe(char *buf, size_t bufsiz, size_t *offset, const char *input)
{
	size_t i;

	for (i = 0; input != NULL && input[i] != '\0'; i++) {
		if (*offset + 1 >= bufsiz)
			return KN_NODE_SHELL_ERR_IO;
		if ((unsigned char)input[i] < 0x20 ||
		    (unsigned char)input[i] > 0x7e)
			buf[(*offset)++] = '?';
		else
			buf[(*offset)++] = input[i];
		buf[*offset] = '\0';
	}

	return KN_NODE_SHELL_OK;
}

static size_t
active_count(const struct kn_node_shell_state *state)
{
	size_t i;
	size_t count;

	count = 0;
	for (i = 0; i < state->max_clients; i++) {
		if (state->sessions[i].fd >= 0 &&
		    state->sessions[i].closed == 0)
			count++;
	}

	return count;
}

static void
close_fd(int *fd)
{
	if (*fd >= 0) {
		(void)close(*fd);
		*fd = -1;
	}
}

static enum kn_node_shell_error
command_heard(const char *args, const struct kn_node_shell_snapshot *snapshot,
	char *buf, size_t bufsiz, size_t *offset)
{
	char call[KN_CALLSIGN_MAX + 4];
	char dest[KN_CALLSIGN_MAX + 4];
	const struct kn_heard_entry *entry;
	const char *port_filter;
	char key[6];
	size_t count;
	size_t i;
	enum kn_node_shell_error rc;

	port_filter = NULL;
	if (args != NULL && args[0] != '\0') {
		for (i = 0; i < 5 && args[i] != '\0'; i++)
			key[i] = (char)toupper((unsigned char)args[i]);
		key[i] = '\0';
		if (strcmp(key, "PORT ") != 0 || args[5] == '\0')
			return append_format(buf, bufsiz, offset,
			    "ERR invalid-heard-command\r\n");
		port_filter = args + 5;
	}

	count = 0;
	for (i = 0; i < snapshot->heard_count; i++) {
		entry = &snapshot->heard[i];
		if (port_filter == NULL ||
		    strcmp(entry->port_name, port_filter) == 0)
			count++;
	}

	rc = append_format(buf, bufsiz, offset, "OK HEARD count=%llu\r\n",
	    (unsigned long long)count);
	if (rc != KN_NODE_SHELL_OK)
		return rc;

	for (i = 0; i < snapshot->heard_count; i++) {
		entry = &snapshot->heard[i];
		if (port_filter != NULL &&
		    strcmp(entry->port_name, port_filter) != 0)
			continue;
		if (kn_heard_format_callsign(&entry->source, call,
		    sizeof(call)) != 0)
			(void)snprintf(call, sizeof(call), "-");
		if (kn_heard_format_callsign(&entry->last_destination, dest,
		    sizeof(dest)) != 0)
			(void)snprintf(dest, sizeof(dest), "-");
		rc = append_format(buf, bufsiz, offset,
		    "HEARD port=%s call=%s last_dest=%s frames=%llu\r\n",
		    entry->port_name, call, dest,
		    (unsigned long long)entry->frame_count);
		if (rc != KN_NODE_SHELL_OK)
			return rc;
	}

	return append_format(buf, bufsiz, offset, "END\r\n");
}

static enum kn_node_shell_error
command_info(const struct kn_node_shell_snapshot *snapshot, char *buf,
	size_t bufsiz, size_t *offset)
{
	char call[KN_CALLSIGN_MAX + 4];
	enum kn_node_shell_error rc;

	if (kn_callsign_format(&snapshot->node->callsign, call,
	    sizeof(call)) != 0)
		(void)snprintf(call, sizeof(call), "-");

	rc = append_format(buf, bufsiz, offset, "OK INFO call=%s alias=", call);
	if (rc != KN_NODE_SHELL_OK)
		return rc;
	if (snapshot->node->has_alias != 0)
		rc = append_safe(buf, bufsiz, offset, snapshot->node->alias);
	else
		rc = append_format(buf, bufsiz, offset, "-");
	if (rc != KN_NODE_SHELL_OK)
		return rc;
	rc = append_format(buf, bufsiz, offset, " location=");
	if (rc != KN_NODE_SHELL_OK)
		return rc;
	if (snapshot->node->has_location != 0)
		rc = append_safe(buf, bufsiz, offset, snapshot->node->location);
	else
		rc = append_format(buf, bufsiz, offset, "-");
	if (rc != KN_NODE_SHELL_OK)
		return rc;

	return append_format(buf, bufsiz, offset, " version=%s\r\n",
	    KILONODE_VERSION);
}

static enum kn_node_shell_error
command_ports(const struct kn_node_shell_snapshot *snapshot, char *buf,
	size_t bufsiz, size_t *offset)
{
	const struct kn_port_stats *port;
	size_t i;
	enum kn_node_shell_error rc;

	rc = append_format(buf, bufsiz, offset, "OK PORTS count=%llu\r\n",
	    (unsigned long long)snapshot->port_count);
	if (rc != KN_NODE_SHELL_OK)
		return rc;
	for (i = 0; i < snapshot->port_count; i++) {
		port = &snapshot->ports[i];
		rc = append_format(buf, bufsiz, offset,
		    "PORT name=%s type=%s enabled=%s open=%s\r\n",
		    port->name, kn_stats_port_type_name(port->type),
		    port->enabled != 0 ? "true" : "false",
		    port->open != 0 ? "true" : "false");
		if (rc != KN_NODE_SHELL_OK)
			return rc;
	}

	return append_format(buf, bufsiz, offset, "END\r\n");
}

static enum kn_node_shell_error
command_stats(const struct kn_node_shell_snapshot *snapshot, char *buf,
	size_t bufsiz, size_t *offset)
{
	const struct kn_daemon_stats *stats;

	stats = snapshot->daemon;
	return append_format(buf, bufsiz, offset,
	    "OK STATS rx_bytes=%llu kiss_frames=%llu ax25_frames=%llu "
	    "malformed_kiss=%llu malformed_ax25=%llu\r\n",
	    (unsigned long long)stats->bytes_received,
	    (unsigned long long)stats->kiss_frames_received,
	    (unsigned long long)stats->ax25_frames_decoded,
	    (unsigned long long)stats->malformed_kiss_frames,
	    (unsigned long long)stats->malformed_ax25_frames);
}

static enum kn_node_shell_error
command_users(const struct kn_node_shell_snapshot *snapshot, char *buf,
	size_t bufsiz, size_t *offset)
{
	const struct kn_node_shell_user *user;
	size_t i;
	enum kn_node_shell_error rc;

	rc = append_format(buf, bufsiz, offset, "OK USERS count=%llu\r\n",
	    (unsigned long long)snapshot->user_count);
	if (rc != KN_NODE_SHELL_OK)
		return rc;
	for (i = 0; i < snapshot->user_count; i++) {
		user = &snapshot->users[i];
		rc = append_format(buf, bufsiz, offset,
		    "USER remote=%s commands=%llu connected=%llu\r\n",
		    user->remote,
		    (unsigned long long)user->command_count,
		    (unsigned long long)user->connected_at);
		if (rc != KN_NODE_SHELL_OK)
			return rc;
	}

	return append_format(buf, bufsiz, offset, "END\r\n");
}

void
kn_node_shell_close(struct kn_node_shell_state *state)
{
	size_t i;

	if (state == NULL)
		return;

	close_fd(&state->listen_fd);
	for (i = 0; i < KN_NODE_SHELL_MAX_CLIENTS; i++)
		kn_node_shell_session_close(&state->sessions[i]);
	state->max_clients = 0;
}

enum kn_node_shell_error
kn_node_shell_format_command(const char *line,
	const struct kn_node_shell_snapshot *snapshot, char *out, size_t out_len,
	uint8_t *close_session)
{
	char command[KN_NODE_SHELL_LINE_MAX];
	char word[16];
	const char *args;
	size_t i;
	size_t len;
	size_t offset;
	enum kn_node_shell_error rc;

	if (line == NULL || snapshot == NULL || snapshot->node == NULL ||
	    snapshot->daemon == NULL || out == NULL || out_len == 0 ||
	    close_session == NULL)
		return KN_NODE_SHELL_ERR_INVALID_ARGUMENT;

	out[0] = '\0';
	*close_session = 0;
	len = strlen(line);
	if (len >= sizeof(command)) {
		(void)snprintf(out, out_len,
		    "ERR line-too-long\r\n%s", KN_NODE_SHELL_PROMPT);
		return KN_NODE_SHELL_OK;
	}

	while (*line == ' ' || *line == '\t')
		line++;
	len = strlen(line);
	while (len > 0 && (line[len - 1] == ' ' || line[len - 1] == '\t'))
		len--;
	if (len == 0) {
		(void)snprintf(out, out_len, "%s", KN_NODE_SHELL_PROMPT);
		return KN_NODE_SHELL_OK;
	}

	memcpy(command, line, len);
	command[len] = '\0';
	for (i = 0; command[i] != '\0'; i++) {
		if ((unsigned char)command[i] < 0x20 ||
		    (unsigned char)command[i] > 0x7e)
			command[i] = '?';
	}

	i = 0;
	while (command[i] != '\0' && command[i] != ' ' &&
	    command[i] != '\t' && i + 1 < sizeof(word)) {
		word[i] = (char)toupper((unsigned char)command[i]);
		i++;
	}
	word[i] = '\0';
	args = command + i;
	while (*args == ' ' || *args == '\t')
		args++;

	offset = 0;
	if (strcmp(word, "HELP") == 0) {
		if (snapshot->bbs.enabled != 0 && snapshot->bbs.store != NULL)
			rc = append_format(out, out_len, &offset,
			    "OK HELP HELP INFO PORTS HEARD USERS STATS BBS "
			    "BYE QUIT\r\n");
		else
			rc = append_format(out, out_len, &offset,
			    "OK HELP HELP INFO PORTS HEARD USERS STATS "
			    "BBS(unavailable) BYE QUIT\r\n");
	} else if (strcmp(word, "INFO") == 0)
		rc = command_info(snapshot, out, out_len, &offset);
	else if (strcmp(word, "PORTS") == 0)
		rc = command_ports(snapshot, out, out_len, &offset);
	else if (strcmp(word, "USERS") == 0)
		rc = command_users(snapshot, out, out_len, &offset);
	else if (strcmp(word, "STATS") == 0)
		rc = command_stats(snapshot, out, out_len, &offset);
	else if (strcmp(word, "HEARD") == 0) {
		rc = command_heard(args, snapshot, out, out_len, &offset);
	} else if (strcmp(word, "BBS") == 0) {
		if (snapshot->bbs.enabled != 0 && snapshot->bbs.store != NULL)
			rc = append_format(out, out_len, &offset,
			    "OK BBS use-session-state\r\n");
		else
			rc = append_format(out, out_len, &offset,
			    "ERR bbs-unavailable\r\n");
	} else if (strcmp(word, "BYE") == 0 || strcmp(word, "QUIT") == 0) {
		*close_session = 1;
		return append_format(out, out_len, &offset, "BYE\r\n");
	} else {
		rc = append_format(out, out_len, &offset,
		    "ERR unknown-command\r\n");
	}
	if (rc != KN_NODE_SHELL_OK)
		return rc;

	return append_prompt(out, out_len, &offset);
}

enum kn_node_shell_error
kn_node_shell_format_session_command(struct kn_node_shell_session *session,
	const char *line, const struct kn_node_shell_snapshot *snapshot,
	char *out, size_t out_len, uint8_t *close_session)
{
	char command[KN_NODE_SHELL_LINE_MAX];
	size_t len;
	size_t i;
	uint8_t exit_bbs;

	if (session == NULL || line == NULL || snapshot == NULL ||
	    out == NULL || out_len == 0 || close_session == NULL)
		return KN_NODE_SHELL_ERR_INVALID_ARGUMENT;

	if (session->bbs.active != 0) {
		if (kn_bbs_shell_format(&session->bbs, line, &snapshot->bbs,
		    out, out_len, close_session, &exit_bbs) !=
		    KN_BBS_SHELL_OK)
			return KN_NODE_SHELL_ERR_IO;
		if (exit_bbs != 0)
			session->bbs.active = 0;
		return KN_NODE_SHELL_OK;
	}

	len = strlen(line);
	if (len < sizeof(command)) {
		while (*line == ' ' || *line == '\t')
			line++;
		len = strlen(line);
		while (len > 0 && (line[len - 1] == ' ' ||
		    line[len - 1] == '\t'))
			len--;
		if (len == 3) {
			for (i = 0; i < len; i++)
				command[i] = (char)toupper(
				    (unsigned char)line[i]);
			command[len] = '\0';
			if (strcmp(command, "BBS") == 0) {
				if (snapshot->bbs.enabled == 0 ||
				    snapshot->bbs.store == NULL) {
					(void)snprintf(out, out_len,
					    "ERR bbs-unavailable\r\n%s",
					    KN_NODE_SHELL_PROMPT);
					*close_session = 0;
					return KN_NODE_SHELL_OK;
				}
				kn_bbs_shell_reset(&session->bbs);
				session->bbs.active = 1;
				(void)snprintf(out, out_len, "OK BBS\r\n%s",
				    KN_BBS_SHELL_PROMPT);
				*close_session = 0;
				return KN_NODE_SHELL_OK;
			}
		}
	}

	return kn_node_shell_format_command(line, snapshot, out, out_len,
	    close_session);
}

int
kn_node_shell_fd(const struct kn_node_shell_state *state)
{
	if (state == NULL)
		return -1;

	return state->listen_fd;
}

void
kn_node_shell_init(struct kn_node_shell_state *state)
{
	size_t i;

	if (state == NULL)
		return;

	memset(state, 0, sizeof(*state));
	state->listen_fd = -1;
	state->max_clients = 0;
	for (i = 0; i < KN_NODE_SHELL_MAX_CLIENTS; i++)
		state->sessions[i].fd = -1;
}

enum kn_node_shell_error
kn_node_shell_open(const struct kn_config_shell *config,
	struct kn_node_shell_state *state)
{
	int fd;
	enum kn_node_shell_error rc;

	if (config == NULL || state == NULL)
		return KN_NODE_SHELL_ERR_INVALID_ARGUMENT;
	if (config->host[0] == '\0' || config->port[0] == '\0' ||
	    config->max_clients < KN_CONFIG_SHELL_MAX_CLIENTS_MIN ||
	    config->max_clients > KN_NODE_SHELL_MAX_CLIENTS)
		return KN_NODE_SHELL_ERR_INVALID_CONFIG;

	rc = socket_open(config, &fd);
	if (rc != KN_NODE_SHELL_OK)
		return rc;

	kn_node_shell_init(state);
	state->listen_fd = fd;
	state->max_clients = config->max_clients;
	return KN_NODE_SHELL_OK;
}

uint16_t
kn_node_shell_port(const struct kn_node_shell_state *state)
{
	struct sockaddr_in addr;
	socklen_t len;

	if (state == NULL || state->listen_fd < 0)
		return 0;

	memset(&addr, 0, sizeof(addr));
	len = sizeof(addr);
	if (getsockname(state->listen_fd, (struct sockaddr *)&addr, &len) != 0)
		return 0;

	return ntohs(addr.sin_port);
}

enum kn_node_shell_error
kn_node_shell_accept(struct kn_node_shell_state *state, const char *banner)
{
	struct sockaddr_storage addr;
	struct kn_node_shell_session *session;
	socklen_t addr_len;
	size_t i;
	int fd;

	if (state == NULL || state->listen_fd < 0)
		return KN_NODE_SHELL_ERR_INVALID_ARGUMENT;

	addr_len = sizeof(addr);
	for (;;) {
		fd = accept(state->listen_fd, (struct sockaddr *)&addr,
		    &addr_len);
		if (fd >= 0)
			break;
		if (errno == EINTR)
			continue;
		return KN_NODE_SHELL_ERR_ACCEPT;
	}

	if (active_count(state) >= state->max_clients) {
		(void)write_all(fd, "ERR max-clients\r\n", 17);
		(void)close(fd);
		return KN_NODE_SHELL_OK;
	}

	session = NULL;
	for (i = 0; i < state->max_clients; i++) {
		if (state->sessions[i].fd < 0 || state->sessions[i].closed != 0) {
			session = &state->sessions[i];
			break;
		}
	}
	if (session == NULL) {
		(void)close(fd);
		return KN_NODE_SHELL_ERR_ACCEPT;
	}

	memset(session, 0, sizeof(*session));
	session->fd = fd;
	session->connected_at = (uint64_t)time(NULL);
	remote_string(&addr, session->remote, sizeof(session->remote));
	if (send_greeting(fd, banner) != KN_NODE_SHELL_OK) {
		kn_node_shell_session_close(session);
		return KN_NODE_SHELL_ERR_IO;
	}

	return KN_NODE_SHELL_OK;
}

enum kn_node_shell_error
kn_node_shell_process_session(struct kn_node_shell_session *session,
	const struct kn_node_shell_snapshot *snapshot)
{
	char response[KN_NODE_SHELL_RESPONSE_MAX];
	const char overlong[] = "ERR line-too-long\r\n" KN_NODE_SHELL_PROMPT;
	uint8_t buf[256];
	size_t i;
	ssize_t nread;
	uint8_t close_session;
	enum kn_node_shell_error rc;

	if (session == NULL || snapshot == NULL || session->fd < 0)
		return KN_NODE_SHELL_ERR_INVALID_ARGUMENT;

	for (;;) {
		nread = read(session->fd, buf, sizeof(buf));
		if (nread >= 0)
			break;
		if (errno == EINTR)
			continue;
		kn_node_shell_session_close(session);
		return KN_NODE_SHELL_ERR_IO;
	}
	if (nread == 0) {
		kn_node_shell_session_close(session);
		return KN_NODE_SHELL_OK;
	}

	for (i = 0; i < (size_t)nread; i++) {
		if (buf[i] == '\r')
			continue;
		if (buf[i] == '\n') {
			if (session->discard_line != 0) {
				rc = write_all(session->fd, overlong,
				    strlen(overlong));
				session->line_len = 0;
				session->discard_line = 0;
				if (rc != KN_NODE_SHELL_OK)
					return rc;
				continue;
			}
			session->line[session->line_len] = '\0';
			if (session->line_len != 0)
				session->command_count++;
			rc = kn_node_shell_format_session_command(session,
			    session->line, snapshot, response, sizeof(response),
			    &close_session);
			session->line_len = 0;
			if (rc != KN_NODE_SHELL_OK)
				return rc;
			rc = write_all(session->fd, response, strlen(response));
			if (rc != KN_NODE_SHELL_OK)
				return rc;
			if (close_session != 0) {
				kn_node_shell_session_close(session);
				return KN_NODE_SHELL_OK;
			}
			continue;
		}
		if (session->discard_line != 0)
			continue;
		if (session->line_len + 1 >= sizeof(session->line)) {
			session->line_len = 0;
			session->discard_line = 1;
			continue;
		}
		if (buf[i] < 0x20 || buf[i] > 0x7e)
			session->line[session->line_len++] = '?';
		else
			session->line[session->line_len++] = (char)buf[i];
	}

	return KN_NODE_SHELL_OK;
}

void
kn_node_shell_session_close(struct kn_node_shell_session *session)
{
	if (session == NULL)
		return;

	close_fd(&session->fd);
	session->closed = 1;
	session->line_len = 0;
	session->discard_line = 0;
	kn_bbs_shell_reset(&session->bbs);
}

void
kn_node_shell_snapshot_users(const struct kn_node_shell_state *state,
	struct kn_node_shell_user *users, size_t max_users, size_t *user_count)
{
	size_t i;
	size_t count;

	if (user_count == NULL)
		return;

	*user_count = 0;
	if (state == NULL || users == NULL || max_users == 0)
		return;

	count = 0;
	for (i = 0; i < state->max_clients && count < max_users; i++) {
		if (state->sessions[i].fd < 0 ||
		    state->sessions[i].closed != 0)
			continue;
		memcpy(users[count].remote, state->sessions[i].remote,
		    sizeof(users[count].remote));
		users[count].connected_at = state->sessions[i].connected_at;
		users[count].command_count = state->sessions[i].command_count;
		count++;
	}
	*user_count = count;
}

static void
remote_string(const struct sockaddr_storage *addr, char *buf, size_t bufsiz)
{
	const struct sockaddr_in *in;

	if (bufsiz == 0)
		return;

	if (addr->ss_family != AF_INET) {
		(void)snprintf(buf, bufsiz, "unknown");
		return;
	}

	in = (const struct sockaddr_in *)addr;
	if (inet_ntop(AF_INET, &in->sin_addr, buf,
	    (socklen_t)bufsiz) == NULL)
		(void)snprintf(buf, bufsiz, "unknown");
}

static enum kn_node_shell_error
send_greeting(int fd, const char *banner)
{
	char out[KN_NODE_SHELL_RESPONSE_MAX];
	size_t offset;
	enum kn_node_shell_error rc;

	offset = 0;
	if (banner != NULL && banner[0] != '\0') {
		rc = append_safe(out, sizeof(out), &offset, banner);
		if (rc != KN_NODE_SHELL_OK)
			return rc;
		rc = append_format(out, sizeof(out), &offset, "\r\n");
		if (rc != KN_NODE_SHELL_OK)
			return rc;
	}
	rc = append_format(out, sizeof(out), &offset,
	    "Type HELP for commands.\r\n%s", KN_NODE_SHELL_PROMPT);
	if (rc != KN_NODE_SHELL_OK)
		return rc;

	return write_all(fd, out, offset);
}

static enum kn_node_shell_error
socket_open(const struct kn_config_shell *config, int *out_fd)
{
	struct addrinfo hints;
	struct addrinfo *result;
	struct addrinfo *rp;
	int fd;
	int gai_rc;
	int yes;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	gai_rc = getaddrinfo(config->host, config->port, &hints, &result);
	if (gai_rc != 0)
		return KN_NODE_SHELL_ERR_RESOLVE;

	for (rp = result; rp != NULL; rp = rp->ai_next) {
		fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (fd < 0)
			continue;
		yes = 1;
		(void)setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes,
		    sizeof(yes));
		if (bind(fd, rp->ai_addr, rp->ai_addrlen) == 0 &&
		    listen(fd, (int)config->max_clients) == 0) {
			freeaddrinfo(result);
			*out_fd = fd;
			return KN_NODE_SHELL_OK;
		}
		(void)close(fd);
	}

	freeaddrinfo(result);
	return KN_NODE_SHELL_ERR_LISTEN;
}

static enum kn_node_shell_error
write_all(int fd, const char *buf, size_t len)
{
	size_t done;
	ssize_t nwritten;

	done = 0;
	while (done < len) {
		nwritten = write(fd, buf + done, len - done);
		if (nwritten > 0) {
			done += (size_t)nwritten;
			continue;
		}
		if (nwritten < 0 && errno == EINTR)
			continue;
		return KN_NODE_SHELL_ERR_IO;
	}

	return KN_NODE_SHELL_OK;
}
