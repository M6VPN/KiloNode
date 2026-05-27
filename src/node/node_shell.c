/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/node/node_shell.c */

#define _POSIX_C_SOURCE 200809L

#include <sys/socket.h>
#include <sys/types.h>

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "kilonode/bbs_user.h"
#include "kilonode/callsign.h"
#include "kilonode/config.h"
#include "kilonode/heard.h"
#include "kilonode/node_command.h"
#include "kilonode/node_command_context.h"
#include "kilonode/node_command_dispatch.h"
#include "kilonode/node_command_profile.h"
#include "kilonode/node_shell.h"
#include "kilonode/session_limits.h"
#include "kilonode/stats.h"

static enum kn_node_shell_error append_format(char *, size_t, size_t *,
	const char *, ...);
static enum kn_node_shell_error append_prompt(char *, size_t, size_t *);
static enum kn_node_shell_error append_safe(char *, size_t, size_t *,
	const char *);
static size_t active_count(const struct kn_node_shell_state *);
static void close_fd(int *);
static enum kn_node_shell_error command_bbs_enter(
	struct kn_node_shell_session *, const char *,
	const struct kn_node_shell_snapshot *, char *, size_t);
static enum kn_node_shell_error context_from_snapshot(
	const struct kn_node_shell_snapshot *, struct kn_node_command_context *,
	struct kn_node_command_user *, size_t);
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
	int needed;

	needed = snprintf(buf + *offset, bufsiz - *offset, "%s",
	    KN_NODE_SHELL_PROMPT);
	if (needed < 0 || (size_t)needed >= bufsiz - *offset)
		return KN_NODE_SHELL_ERR_IO;
	*offset += (size_t)needed;
	return KN_NODE_SHELL_OK;
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
command_bbs_enter(struct kn_node_shell_session *session, const char *args,
	const struct kn_node_shell_snapshot *snapshot, char *out, size_t out_len)
{
	struct kn_bbs_user user;
	char call[KN_CALLSIGN_MAX + 4];
	struct kn_callsign parsed;
	enum kn_bbs_user_error urc;
	int needed;

	if (snapshot->bbs.enabled == 0 || snapshot->bbs.store == NULL) {
		needed = snprintf(out, out_len, "ERR bbs-unavailable\r\n%s",
		    KN_NODE_SHELL_PROMPT);
		return needed < 0 || (size_t)needed >= out_len ?
		    KN_NODE_SHELL_ERR_IO : KN_NODE_SHELL_OK;
	}
	while (*args == ' ' || *args == '\t')
		args++;
	if (*args == '\0') {
		needed = snprintf(out, out_len,
		    "ERR callsign-required\r\n%s", KN_NODE_SHELL_PROMPT);
		return needed < 0 || (size_t)needed >= out_len ?
		    KN_NODE_SHELL_ERR_IO : KN_NODE_SHELL_OK;
	}
	if (kn_callsign_parse(args, &parsed) != 0 ||
	    kn_callsign_format(&parsed, call, sizeof(call)) != 0) {
		needed = snprintf(out, out_len,
		    "ERR invalid-callsign\r\n%s", KN_NODE_SHELL_PROMPT);
		return needed < 0 || (size_t)needed >= out_len ?
		    KN_NODE_SHELL_ERR_IO : KN_NODE_SHELL_OK;
	}
	urc = kn_bbs_user_init_store(snapshot->bbs.store);
	if (urc == KN_BBS_USER_OK)
		urc = kn_bbs_user_seen(snapshot->bbs.store, call,
		    (uint64_t)time(NULL), &user);
	if (urc != KN_BBS_USER_OK) {
		needed = snprintf(out, out_len, "ERR bbs-user code=%s\r\n%s",
		    kn_bbs_user_error_name(urc), KN_NODE_SHELL_PROMPT);
		return needed < 0 || (size_t)needed >= out_len ?
		    KN_NODE_SHELL_ERR_IO : KN_NODE_SHELL_OK;
	}
	kn_bbs_shell_reset(&session->bbs);
	session->bbs.active = 1;
	memcpy(session->bbs.identity, user.call, strlen(user.call) + 1);
	needed = snprintf(out, out_len, "OK BBS call=%s\r\nBBS %s> ",
	    user.call, user.call);
	return needed < 0 || (size_t)needed >= out_len ?
	    KN_NODE_SHELL_ERR_IO : KN_NODE_SHELL_OK;
}

static enum kn_node_shell_error
context_from_snapshot(const struct kn_node_shell_snapshot *snapshot,
	struct kn_node_command_context *context,
	struct kn_node_command_user *users, size_t user_cap)
{
	const struct kn_node_shell_user *user;
	size_t i;

	if (snapshot == NULL || context == NULL ||
	    (snapshot->user_count > 0 && users == NULL))
		return KN_NODE_SHELL_ERR_INVALID_ARGUMENT;
	kn_node_command_context_clear(context);
	context->node = snapshot->node;
	context->daemon = snapshot->daemon;
	context->ports = snapshot->ports;
	context->port_count = snapshot->port_count;
	context->heard = snapshot->heard;
	context->heard_count = snapshot->heard_count;
	context->output_limit = KN_NODE_SHELL_RESPONSE_MAX;
	context->bbs_enabled = snapshot->bbs.enabled != 0 &&
	    snapshot->bbs.store != NULL ? 1 : 0;
	context->user_count = snapshot->user_count;
	if (context->user_count > user_cap)
		context->user_count = user_cap;
	for (i = 0; i < context->user_count; i++) {
		user = &snapshot->users[i];
		(void)snprintf(users[i].remote, sizeof(users[i].remote),
		    "%s", user->remote);
		users[i].connected_at = user->connected_at;
		users[i].command_count = user->command_count;
	}
	context->users = users;

	return KN_NODE_SHELL_OK;
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
	struct kn_node_command_context context;
	struct kn_node_command_user users[KN_NODE_SHELL_MAX_CLIENTS];
	struct kn_node_command_dispatch_result result;
	size_t len;
	size_t offset;
	enum kn_node_command_dispatch_status dispatch_rc;

	if (line == NULL || snapshot == NULL || snapshot->node == NULL ||
	    snapshot->daemon == NULL || out == NULL || out_len == 0 ||
	    close_session == NULL)
		return KN_NODE_SHELL_ERR_INVALID_ARGUMENT;

	out[0] = '\0';
	*close_session = 0;
	len = strlen(line);
	if (snapshot->policy != NULL &&
	    kn_access_policy_check_command(snapshot->policy, len) !=
	    KN_ACCESS_POLICY_OK) {
		(void)snprintf(out, out_len,
		    "ERR command-too-long\r\n%s", KN_NODE_SHELL_PROMPT);
		return KN_NODE_SHELL_OK;
	}
	if (len >= KN_NODE_SHELL_LINE_MAX) {
		(void)snprintf(out, out_len,
		    "ERR line-too-long\r\n%s", KN_NODE_SHELL_PROMPT);
		return KN_NODE_SHELL_OK;
	}

	offset = 0;
	if (context_from_snapshot(snapshot, &context, users,
	    sizeof(users) / sizeof(users[0])) != KN_NODE_SHELL_OK)
		return KN_NODE_SHELL_ERR_INVALID_ARGUMENT;
	context.output_limit = out_len;
	dispatch_rc = kn_node_command_dispatch(KN_NODE_COMMAND_CONTEXT_LOCAL,
	    &context, (const uint8_t *)line, len, KN_NODE_SHELL_LINE_MAX - 1,
	    &result);
	if (dispatch_rc == KN_NODE_COMMAND_DISPATCH_OVERLONG) {
		(void)snprintf(out, out_len,
		    "ERR line-too-long\r\n%s", KN_NODE_SHELL_PROMPT);
		return KN_NODE_SHELL_OK;
	}
	if (dispatch_rc == KN_NODE_COMMAND_DISPATCH_CONTROL_CHARACTER) {
		(void)snprintf(out, out_len,
		    "ERR control-character\r\n%s", KN_NODE_SHELL_PROMPT);
		return KN_NODE_SHELL_OK;
	}
	if (dispatch_rc == KN_NODE_COMMAND_DISPATCH_INTERNAL_ERROR)
		return KN_NODE_SHELL_ERR_IO;
	if (result.output_len >= out_len)
		return KN_NODE_SHELL_ERR_IO;
	memcpy(out, result.output, result.output_len + 1);
	offset = result.output_len;
	*close_session = result.close_session;
	if (*close_session != 0)
		return KN_NODE_SHELL_OK;

	return append_prompt(out, out_len, &offset);
}

enum kn_node_shell_error
kn_node_shell_format_session_command(struct kn_node_shell_session *session,
	const char *line, const struct kn_node_shell_snapshot *snapshot,
	char *out, size_t out_len, uint8_t *close_session)
{
	struct kn_node_command_input parsed;
	size_t len;
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
	if (kn_node_command_parse((const uint8_t *)line, len,
	    KN_NODE_SHELL_LINE_MAX - 1, &parsed) == KN_NODE_COMMAND_OK &&
	    parsed.command_len != 0 &&
	    kn_node_command_id_from_name(parsed.command) ==
	    KN_NODE_COMMAND_ID_BBS) {
		*close_session = 0;
		return command_bbs_enter(session, parsed.args, snapshot, out,
		    out_len);
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
	kn_access_policy_defaults(&state->policy);
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
	kn_access_policy_defaults(&state->policy);
	if (state->policy.max_clients < state->max_clients)
		state->max_clients = state->policy.max_clients;
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
	session->last_activity = session->connected_at;
	kn_session_rate_init(&session->rate);
	remote_string(&addr, session->remote, sizeof(session->remote));
	if (kn_access_policy_check_remote(&state->policy,
	    session->remote) != KN_ACCESS_POLICY_OK) {
		(void)write_all(fd, "ERR access-denied\r\n", 19);
		kn_node_shell_session_close(session);
		return KN_NODE_SHELL_OK;
	}
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
	uint64_t now;

	if (session == NULL || snapshot == NULL || session->fd < 0)
		return KN_NODE_SHELL_ERR_INVALID_ARGUMENT;

	now = (uint64_t)time(NULL);
	if (snapshot->policy != NULL &&
	    kn_session_idle_expired(session->last_activity, now,
	    snapshot->policy->idle_timeout_seconds) != 0) {
		(void)write_all(session->fd, "ERR idle-timeout\r\n", 18);
		kn_node_shell_session_close(session);
		return KN_NODE_SHELL_OK;
	}

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
				kn_node_shell_session_close(session);
				return KN_NODE_SHELL_OK;
			}
			if (snapshot->policy != NULL &&
			    kn_session_rate_check(&session->rate,
			    snapshot->policy->input_rate_lines,
			    snapshot->policy->input_rate_window_seconds,
			    now) != KN_SESSION_LIMIT_OK) {
				(void)write_all(session->fd,
				    "ERR rate-limit\r\n", 16);
				kn_node_shell_session_close(session);
				return KN_NODE_SHELL_OK;
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
			session->last_activity = now;
			continue;
		}
		if (session->discard_line != 0)
			continue;
		if (session->line_len + 1 >= sizeof(session->line) ||
		    (snapshot->policy != NULL &&
		    kn_access_policy_check_line(snapshot->policy,
		    session->line_len + 1) != KN_ACCESS_POLICY_OK)) {
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
kn_node_shell_prune_idle(struct kn_node_shell_state *state, uint64_t now)
{
	size_t i;

	if (state == NULL)
		return;
	for (i = 0; i < state->max_clients; i++) {
		if (state->sessions[i].fd < 0 ||
		    state->sessions[i].closed != 0)
			continue;
		if (kn_session_idle_expired(state->sessions[i].last_activity,
		    now, state->policy.idle_timeout_seconds) != 0)
			kn_node_shell_session_close(&state->sessions[i]);
	}
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
kn_node_shell_set_policy(struct kn_node_shell_state *state,
	const struct kn_access_policy *policy)
{
	if (state == NULL || policy == NULL)
		return;
	state->policy = *policy;
	if (state->policy.max_clients < state->max_clients)
		state->max_clients = state->policy.max_clients;
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
