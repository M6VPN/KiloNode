/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/node/node_command_dispatch.c */

#include <sys/types.h>

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "kilonode/callsign.h"
#include "kilonode/node_command.h"
#include "kilonode/node_command_dispatch.h"

#define KILONODE_VERSION "0.1.0"

static enum kn_node_command_dispatch_status append_format(char *, size_t,
	size_t *, const char *, ...);
static enum kn_node_command_dispatch_status append_safe(char *, size_t,
	size_t *, const char *);
static enum kn_node_command_dispatch_status format_heard(
	enum kn_node_command_context_kind, const struct kn_node_command_context *,
	const char *, char *, size_t, size_t *);
static enum kn_node_command_dispatch_status format_help(
	enum kn_node_command_context_kind, const struct kn_node_command_context *,
	char *, size_t, size_t *);
static enum kn_node_command_dispatch_status format_info(
	enum kn_node_command_context_kind, const struct kn_node_command_context *,
	char *, size_t, size_t *);
static enum kn_node_command_dispatch_status format_ports(
	enum kn_node_command_context_kind, const struct kn_node_command_context *,
	char *, size_t, size_t *);
static enum kn_node_command_dispatch_status format_stats(
	enum kn_node_command_context_kind, const struct kn_node_command_context *,
	char *, size_t, size_t *);
static enum kn_node_command_dispatch_status format_users(
	const struct kn_node_command_context *, char *, size_t, size_t *);
static enum kn_node_command_dispatch_status validate_args(
	const struct kn_node_command_profile *, const struct kn_node_command_input *);

void
kn_node_command_context_clear(struct kn_node_command_context *context)
{
	if (context == NULL)
		return;
	memset(context, 0, sizeof(*context));
}

void
kn_node_command_dispatch_result_clear(
	struct kn_node_command_dispatch_result *result)
{
	if (result == NULL)
		return;
	memset(result, 0, sizeof(*result));
	result->status = KN_NODE_COMMAND_DISPATCH_INTERNAL_ERROR;
	result->command_id = KN_NODE_COMMAND_ID_UNKNOWN;
}

const char *
kn_node_command_dispatch_status_name(
	enum kn_node_command_dispatch_status status)
{
	switch (status) {
	case KN_NODE_COMMAND_DISPATCH_OK:
		return "ok";
	case KN_NODE_COMMAND_DISPATCH_UNKNOWN:
		return "unknown";
	case KN_NODE_COMMAND_DISPATCH_FORBIDDEN_CONTEXT:
		return "forbidden-context";
	case KN_NODE_COMMAND_DISPATCH_INVALID_ARGS:
		return "invalid-args";
	case KN_NODE_COMMAND_DISPATCH_OVERLONG:
		return "overlong";
	case KN_NODE_COMMAND_DISPATCH_CONTROL_CHARACTER:
		return "control-character";
	case KN_NODE_COMMAND_DISPATCH_INTERNAL_ERROR:
		return "internal-error";
	}

	return "unknown";
}

enum kn_node_command_dispatch_status
kn_node_command_dispatch(enum kn_node_command_context_kind kind,
	const struct kn_node_command_context *context, const uint8_t *input,
	size_t input_len, size_t max_input_len,
	struct kn_node_command_dispatch_result *result)
{
	struct kn_node_command_input parsed;
	const struct kn_node_command_profile *profile;
	size_t offset;
	size_t output_len;
	enum kn_node_command_error parse_rc;
	enum kn_node_command_dispatch_status rc;

	if (context == NULL || result == NULL)
		return KN_NODE_COMMAND_DISPATCH_INTERNAL_ERROR;
	kn_node_command_dispatch_result_clear(result);
	parse_rc = kn_node_command_parse(input, input_len, max_input_len,
	    &parsed);
	if (parse_rc == KN_NODE_COMMAND_ERR_EMPTY) {
		result->status = KN_NODE_COMMAND_DISPATCH_OK;
		return KN_NODE_COMMAND_DISPATCH_OK;
	}
	if (parse_rc == KN_NODE_COMMAND_ERR_OVERLONG) {
		result->status = KN_NODE_COMMAND_DISPATCH_OVERLONG;
		(void)snprintf(result->reason, sizeof(result->reason),
		    "overlong");
		return result->status;
	}
	if (parse_rc == KN_NODE_COMMAND_ERR_CONTROL) {
		result->status = KN_NODE_COMMAND_DISPATCH_CONTROL_CHARACTER;
		(void)snprintf(result->reason, sizeof(result->reason),
		    "control-character");
		return result->status;
	}
	if (parse_rc != KN_NODE_COMMAND_OK) {
		result->status = KN_NODE_COMMAND_DISPATCH_INTERNAL_ERROR;
		return result->status;
	}

	profile = kn_node_command_profile_find(parsed.command);
	if (profile == NULL) {
		result->status = KN_NODE_COMMAND_DISPATCH_UNKNOWN;
		result->command_id = KN_NODE_COMMAND_ID_UNKNOWN;
		(void)snprintf(result->profile_name,
		    sizeof(result->profile_name), "UNKNOWN");
		(void)snprintf(result->output, sizeof(result->output),
		    kind == KN_NODE_COMMAND_CONTEXT_RF_UI ?
		    "ERR unknown command" : "ERR unknown-command\r\n");
		result->output_len = strlen(result->output);
		return result->status;
	}

	result->command_id = profile->id;
	(void)snprintf(result->profile_name, sizeof(result->profile_name),
	    "%s", profile->name);
	if ((profile->contexts & (uint8_t)kind) == 0) {
		result->status = KN_NODE_COMMAND_DISPATCH_FORBIDDEN_CONTEXT;
		(void)snprintf(result->reason, sizeof(result->reason),
		    "forbidden-context");
		if (kind == KN_NODE_COMMAND_CONTEXT_RF_UI)
			(void)snprintf(result->output, sizeof(result->output),
			    "ERR command unavailable");
		else
			(void)snprintf(result->output, sizeof(result->output),
			    "ERR forbidden-command\r\n");
		result->output_len = strlen(result->output);
		return result->status;
	}
	rc = validate_args(profile, &parsed);
	if (rc != KN_NODE_COMMAND_DISPATCH_OK) {
		result->status = rc;
		(void)snprintf(result->reason, sizeof(result->reason),
		    "invalid-args");
		(void)snprintf(result->output, sizeof(result->output),
		    kind == KN_NODE_COMMAND_CONTEXT_RF_UI ?
		    "ERR invalid arguments" : "ERR invalid-args\r\n");
		result->output_len = strlen(result->output);
		return result->status;
	}

	output_len = context->output_limit;
	if (output_len == 0 || output_len > sizeof(result->output))
		output_len = sizeof(result->output);
	offset = 0;
	switch (profile->id) {
	case KN_NODE_COMMAND_ID_HELP:
		rc = format_help(kind, context, result->output, output_len,
		    &offset);
		break;
	case KN_NODE_COMMAND_ID_INFO:
		rc = format_info(kind, context, result->output, output_len,
		    &offset);
		break;
	case KN_NODE_COMMAND_ID_PORTS:
		rc = format_ports(kind, context, result->output, output_len,
		    &offset);
		break;
	case KN_NODE_COMMAND_ID_HEARD:
		rc = format_heard(kind, context, parsed.args, result->output,
		    output_len, &offset);
		break;
	case KN_NODE_COMMAND_ID_STATS:
		rc = format_stats(kind, context, result->output, output_len,
		    &offset);
		break;
	case KN_NODE_COMMAND_ID_USERS:
		rc = format_users(context, result->output, output_len, &offset);
		break;
	case KN_NODE_COMMAND_ID_BBS:
		result->mode_transition = 1;
		rc = append_format(result->output, output_len, &offset,
		    "ERR use-session-command\r\n");
		break;
	case KN_NODE_COMMAND_ID_BYE:
	case KN_NODE_COMMAND_ID_QUIT:
		result->close_session = 1;
		rc = append_format(result->output, output_len, &offset,
		    "BYE\r\n");
		break;
	case KN_NODE_COMMAND_ID_PING:
		rc = append_format(result->output, output_len, &offset, "PONG");
		break;
	case KN_NODE_COMMAND_ID_UNKNOWN:
	default:
		rc = KN_NODE_COMMAND_DISPATCH_UNKNOWN;
		break;
	}
	result->status = rc;
	result->output_len = offset;
	return rc;
}

static enum kn_node_command_dispatch_status
append_format(char *buf, size_t bufsiz, size_t *offset, const char *fmt, ...)
{
	va_list args;
	int needed;

	if (*offset >= bufsiz)
		return KN_NODE_COMMAND_DISPATCH_INTERNAL_ERROR;
	va_start(args, fmt);
	needed = vsnprintf(buf + *offset, bufsiz - *offset, fmt, args);
	va_end(args);
	if (needed < 0 || (size_t)needed >= bufsiz - *offset)
		return KN_NODE_COMMAND_DISPATCH_INTERNAL_ERROR;
	*offset += (size_t)needed;
	return KN_NODE_COMMAND_DISPATCH_OK;
}

static enum kn_node_command_dispatch_status
append_safe(char *buf, size_t bufsiz, size_t *offset, const char *input)
{
	size_t i;

	for (i = 0; input != NULL && input[i] != '\0'; i++) {
		if (*offset + 1 >= bufsiz)
			return KN_NODE_COMMAND_DISPATCH_INTERNAL_ERROR;
		if ((unsigned char)input[i] < 0x20 ||
		    (unsigned char)input[i] > 0x7e)
			buf[(*offset)++] = '?';
		else
			buf[(*offset)++] = input[i];
		buf[*offset] = '\0';
	}

	return KN_NODE_COMMAND_DISPATCH_OK;
}

static enum kn_node_command_dispatch_status
format_heard(enum kn_node_command_context_kind kind,
	const struct kn_node_command_context *context, const char *args,
	char *buf, size_t bufsiz, size_t *offset)
{
	char call[KN_CALLSIGN_MAX + 4];
	char dest[KN_CALLSIGN_MAX + 4];
	const struct kn_heard_entry *entry;
	const char *port_filter;
	char key[6];
	size_t count;
	size_t i;
	size_t limit;
	enum kn_node_command_dispatch_status rc;

	port_filter = NULL;
	if (kind == KN_NODE_COMMAND_CONTEXT_LOCAL && args != NULL &&
	    args[0] != '\0') {
		for (i = 0; i < 5 && args[i] != '\0'; i++)
			key[i] = args[i] >= 'a' && args[i] <= 'z' ?
			    (char)(args[i] - 'a' + 'A') : args[i];
		key[i] = '\0';
		if (strcmp(key, "PORT ") != 0 || args[5] == '\0')
			return append_format(buf, bufsiz, offset,
			    "ERR invalid-heard-command\r\n");
		port_filter = args + 5;
	}
	if (kind == KN_NODE_COMMAND_CONTEXT_RF_UI && args != NULL &&
	    args[0] != '\0')
		return append_format(buf, bufsiz, offset,
		    "ERR invalid arguments");

	count = 0;
	for (i = 0; i < context->heard_count; i++) {
		entry = &context->heard[i];
		if (port_filter == NULL ||
		    strcmp(entry->port_name, port_filter) == 0)
			count++;
	}
	if (kind == KN_NODE_COMMAND_CONTEXT_RF_UI) {
		rc = append_format(buf, bufsiz, offset, "HEARD count=%llu",
		    (unsigned long long)context->heard_count);
		if (rc != KN_NODE_COMMAND_DISPATCH_OK)
			return rc;
		limit = context->heard_count < 4 ? context->heard_count : 4;
		for (i = 0; context->heard != NULL && i < limit; i++) {
			if (kn_heard_format_callsign(&context->heard[i].source,
			    call, sizeof(call)) != 0)
				continue;
			rc = append_format(buf, bufsiz, offset, " %s", call);
			if (rc != KN_NODE_COMMAND_DISPATCH_OK)
				return rc;
		}
		return KN_NODE_COMMAND_DISPATCH_OK;
	}

	rc = append_format(buf, bufsiz, offset, "OK HEARD count=%llu\r\n",
	    (unsigned long long)count);
	if (rc != KN_NODE_COMMAND_DISPATCH_OK)
		return rc;
	for (i = 0; i < context->heard_count; i++) {
		entry = &context->heard[i];
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
		if (rc != KN_NODE_COMMAND_DISPATCH_OK)
			return rc;
	}

	return append_format(buf, bufsiz, offset, "END\r\n");
}

static enum kn_node_command_dispatch_status
format_help(enum kn_node_command_context_kind kind,
	const struct kn_node_command_context *context, char *buf, size_t bufsiz,
	size_t *offset)
{
	if (kind == KN_NODE_COMMAND_CONTEXT_RF_UI)
		return append_format(buf, bufsiz, offset,
		    "KiloNode commands: HELP INFO PORTS HEARD STATS PING");
	if (context->bbs_enabled != 0)
		return append_format(buf, bufsiz, offset,
		    "OK HELP HELP INFO PORTS HEARD USERS STATS BBS BYE "
		    "QUIT\r\n");
	return append_format(buf, bufsiz, offset,
	    "OK HELP HELP INFO PORTS HEARD USERS STATS BBS(unavailable) "
	    "BYE QUIT\r\n");
}

static enum kn_node_command_dispatch_status
format_info(enum kn_node_command_context_kind kind,
	const struct kn_node_command_context *context, char *buf, size_t bufsiz,
	size_t *offset)
{
	char call[KN_CALLSIGN_MAX + 4];
	enum kn_node_command_dispatch_status rc;

	if (context->node == NULL ||
	    kn_callsign_format(&context->node->callsign, call,
	    sizeof(call)) != 0)
		(void)snprintf(call, sizeof(call), "-");
	if (kind == KN_NODE_COMMAND_CONTEXT_RF_UI)
		return append_format(buf, bufsiz, offset,
		    "KiloNode call=%s alias=%s", call,
		    context->node != NULL && context->node->alias[0] != '\0' ?
		    context->node->alias : "-");

	rc = append_format(buf, bufsiz, offset, "OK INFO call=%s alias=",
	    call);
	if (rc != KN_NODE_COMMAND_DISPATCH_OK)
		return rc;
	if (context->node != NULL && context->node->has_alias != 0)
		rc = append_safe(buf, bufsiz, offset, context->node->alias);
	else
		rc = append_format(buf, bufsiz, offset, "-");
	if (rc != KN_NODE_COMMAND_DISPATCH_OK)
		return rc;
	rc = append_format(buf, bufsiz, offset, " location=");
	if (rc != KN_NODE_COMMAND_DISPATCH_OK)
		return rc;
	if (context->node != NULL && context->node->has_location != 0)
		rc = append_safe(buf, bufsiz, offset, context->node->location);
	else
		rc = append_format(buf, bufsiz, offset, "-");
	if (rc != KN_NODE_COMMAND_DISPATCH_OK)
		return rc;
	return append_format(buf, bufsiz, offset, " version=%s\r\n",
	    KILONODE_VERSION);
}

static enum kn_node_command_dispatch_status
format_ports(enum kn_node_command_context_kind kind,
	const struct kn_node_command_context *context, char *buf, size_t bufsiz,
	size_t *offset)
{
	const struct kn_port_stats *port;
	size_t i;
	enum kn_node_command_dispatch_status rc;

	if (kind == KN_NODE_COMMAND_CONTEXT_RF_UI) {
		rc = append_format(buf, bufsiz, offset, "PORTS count=%llu",
		    (unsigned long long)context->port_count);
		if (rc != KN_NODE_COMMAND_DISPATCH_OK)
			return rc;
		for (i = 0; context->ports != NULL && i < context->port_count &&
		    i < 4; i++) {
			rc = append_format(buf, bufsiz, offset, " %s:%s",
			    context->ports[i].name,
			    context->ports[i].open != 0 ? "open" : "closed");
			if (rc != KN_NODE_COMMAND_DISPATCH_OK)
				return rc;
		}
		return KN_NODE_COMMAND_DISPATCH_OK;
	}

	rc = append_format(buf, bufsiz, offset, "OK PORTS count=%llu\r\n",
	    (unsigned long long)context->port_count);
	if (rc != KN_NODE_COMMAND_DISPATCH_OK)
		return rc;
	for (i = 0; i < context->port_count; i++) {
		port = &context->ports[i];
		rc = append_format(buf, bufsiz, offset,
		    "PORT name=%s type=%s enabled=%s open=%s\r\n",
		    port->name, kn_stats_port_type_name(port->type),
		    port->enabled != 0 ? "true" : "false",
		    port->open != 0 ? "true" : "false");
		if (rc != KN_NODE_COMMAND_DISPATCH_OK)
			return rc;
	}

	return append_format(buf, bufsiz, offset, "END\r\n");
}

static enum kn_node_command_dispatch_status
format_stats(enum kn_node_command_context_kind kind,
	const struct kn_node_command_context *context, char *buf, size_t bufsiz,
	size_t *offset)
{
	const struct kn_daemon_stats *stats;

	stats = context->daemon;
	if (stats == NULL)
		return KN_NODE_COMMAND_DISPATCH_INTERNAL_ERROR;
	if (kind == KN_NODE_COMMAND_CONTEXT_RF_UI)
		return append_format(buf, bufsiz, offset,
		    "STATS rx=%llu ax25=%llu malformed=%llu",
		    (unsigned long long)stats->kiss_frames_received,
		    (unsigned long long)stats->ax25_frames_decoded,
		    (unsigned long long)(stats->malformed_kiss_frames +
		    stats->malformed_ax25_frames));
	return append_format(buf, bufsiz, offset,
	    "OK STATS rx_bytes=%llu kiss_frames=%llu ax25_frames=%llu "
	    "malformed_kiss=%llu malformed_ax25=%llu\r\n",
	    (unsigned long long)stats->bytes_received,
	    (unsigned long long)stats->kiss_frames_received,
	    (unsigned long long)stats->ax25_frames_decoded,
	    (unsigned long long)stats->malformed_kiss_frames,
	    (unsigned long long)stats->malformed_ax25_frames);
}

static enum kn_node_command_dispatch_status
format_users(const struct kn_node_command_context *context, char *buf,
	size_t bufsiz, size_t *offset)
{
	const struct kn_node_command_user *user;
	size_t i;
	enum kn_node_command_dispatch_status rc;

	rc = append_format(buf, bufsiz, offset, "OK USERS count=%llu\r\n",
	    (unsigned long long)context->user_count);
	if (rc != KN_NODE_COMMAND_DISPATCH_OK)
		return rc;
	for (i = 0; i < context->user_count; i++) {
		user = &context->users[i];
		rc = append_format(buf, bufsiz, offset,
		    "USER remote=%s commands=%llu connected=%llu\r\n",
		    user->remote,
		    (unsigned long long)user->command_count,
		    (unsigned long long)user->connected_at);
		if (rc != KN_NODE_COMMAND_DISPATCH_OK)
			return rc;
	}

	return append_format(buf, bufsiz, offset, "END\r\n");
}

static enum kn_node_command_dispatch_status
validate_args(const struct kn_node_command_profile *profile,
	const struct kn_node_command_input *parsed)
{
	switch (profile->argument_policy) {
	case KN_NODE_COMMAND_ARGS_NONE:
		return parsed->args_len == 0 ? KN_NODE_COMMAND_DISPATCH_OK :
		    KN_NODE_COMMAND_DISPATCH_INVALID_ARGS;
	case KN_NODE_COMMAND_ARGS_OPTIONAL:
	case KN_NODE_COMMAND_ARGS_FREE_TEXT:
		return KN_NODE_COMMAND_DISPATCH_OK;
	case KN_NODE_COMMAND_ARGS_REQUIRED:
		return parsed->args_len == 0 ?
		    KN_NODE_COMMAND_DISPATCH_INVALID_ARGS :
		    KN_NODE_COMMAND_DISPATCH_OK;
	}

	return KN_NODE_COMMAND_DISPATCH_INTERNAL_ERROR;
}
