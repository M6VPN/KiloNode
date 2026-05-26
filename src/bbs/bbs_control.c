/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/bbs/bbs_control.c */

#include <sys/types.h>

#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kilonode/bbs_area.h"
#include "kilonode/bbs_control.h"
#include "kilonode/bbs_store_maintenance.h"
#include "kilonode/bbs_user.h"
#include "kilonode/callsign.h"
#include "kilonode/message_index.h"

static enum kn_bbs_control_error append_format(char *, size_t, size_t *,
	const char *, ...);
static enum kn_bbs_control_error append_hex_preview(char *, size_t, size_t *,
	const uint8_t *, size_t);
static enum kn_bbs_control_error append_message_summary(char *, size_t,
	size_t *, const struct kn_message *, uint8_t);
static enum kn_bbs_control_error append_quoted(char *, size_t, size_t *,
	const char *);
static enum kn_bbs_control_error append_safe_path(char *, size_t, size_t *,
	const char *);
static enum kn_bbs_control_error append_text_preview(char *, size_t, size_t *,
	const uint8_t *, size_t);
static uint8_t body_is_text(const uint8_t *, size_t);
static enum kn_bbs_control_error callsign_string(const struct kn_callsign *,
	char *, size_t);
static enum kn_bbs_control_error command_areas(struct kn_message_store *,
	char *, size_t);
static enum kn_bbs_control_error command_message(struct kn_message_store *,
	const char *, char *, size_t);
static enum kn_bbs_control_error command_messages(struct kn_message_store *,
	const char *, char *, size_t);
static enum kn_bbs_control_error command_stats(struct kn_message_store *,
	char *, size_t);
static enum kn_bbs_control_error command_status(uint8_t,
	struct kn_message_store *, char *, size_t);
static enum kn_bbs_control_error command_users(struct kn_message_store *,
	char *, size_t);
static enum kn_bbs_control_error parse_filter(const char *,
	enum kn_message_index_filter *, char *, size_t,
	enum kn_bbs_control_error *);
static uint8_t parse_id(const char *, uint64_t *);
static enum kn_bbs_control_error store_error_response(
	enum kn_message_store_error, char *, size_t);

static enum kn_bbs_control_error
append_format(char *buf, size_t bufsiz, size_t *offset, const char *fmt, ...)
{
	va_list args;
	int needed;

	if (*offset >= bufsiz)
		return KN_BBS_CONTROL_ERR_BUFFER;

	va_start(args, fmt);
	needed = vsnprintf(buf + *offset, bufsiz - *offset, fmt, args);
	va_end(args);
	if (needed < 0 || (size_t)needed >= bufsiz - *offset)
		return KN_BBS_CONTROL_ERR_BUFFER;

	*offset += (size_t)needed;
	return KN_BBS_CONTROL_OK;
}

static enum kn_bbs_control_error
append_hex_preview(char *buf, size_t bufsiz, size_t *offset,
	const uint8_t *body, size_t body_len)
{
	static const char hex[] = "0123456789abcdef";
	size_t i;
	size_t limit;

	limit = body_len > 32 ? 32 : body_len;
	for (i = 0; i < limit; i++) {
		if (*offset + 2 >= bufsiz)
			return KN_BBS_CONTROL_ERR_BUFFER;
		buf[(*offset)++] = hex[(body[i] >> 4) & 0x0f];
		buf[(*offset)++] = hex[body[i] & 0x0f];
		buf[*offset] = '\0';
	}

	return KN_BBS_CONTROL_OK;
}

static enum kn_bbs_control_error
append_message_summary(char *buf, size_t bufsiz, size_t *offset,
	const struct kn_message *message, uint8_t include_body_len)
{
	char from[KN_CALLSIGN_MAX + 4];
	char to[KN_CALLSIGN_MAX + 4];
	enum kn_bbs_control_error rc;

	rc = callsign_string(&message->from, from, sizeof(from));
	if (rc != KN_BBS_CONTROL_OK)
		return rc;
	rc = append_format(buf, bufsiz, offset,
	    "BBS MSG id=%llu type=%s from=%s ",
	    (unsigned long long)message->id,
	    kn_message_type_name(message->type), from);
	if (rc != KN_BBS_CONTROL_OK)
		return rc;

	if (message->type == KN_MESSAGE_TYPE_PRIVATE) {
		rc = callsign_string(&message->to, to, sizeof(to));
		if (rc != KN_BBS_CONTROL_OK)
			return rc;
		rc = append_format(buf, bufsiz, offset, "to=%s ", to);
	} else {
		rc = append_format(buf, bufsiz, offset, "area=%s ",
		    message->area);
	}
	if (rc != KN_BBS_CONTROL_OK)
		return rc;

	rc = append_format(buf, bufsiz, offset,
	    "created=%llu deleted=%s subject=",
	    (unsigned long long)message->created,
	    message->deleted != 0 ? "true" : "false");
	if (rc != KN_BBS_CONTROL_OK)
		return rc;
	rc = append_quoted(buf, bufsiz, offset, message->subject);
	if (rc != KN_BBS_CONTROL_OK)
		return rc;
	if (include_body_len != 0) {
		rc = append_format(buf, bufsiz, offset, " body_bytes=%llu",
		    (unsigned long long)message->body_len);
		if (rc != KN_BBS_CONTROL_OK)
			return rc;
	}

	return append_format(buf, bufsiz, offset, "\n");
}

static enum kn_bbs_control_error
append_quoted(char *buf, size_t bufsiz, size_t *offset, const char *input)
{
	size_t i;
	unsigned char ch;

	if (*offset + 1 >= bufsiz)
		return KN_BBS_CONTROL_ERR_BUFFER;
	buf[(*offset)++] = '"';
	buf[*offset] = '\0';

	for (i = 0; input != NULL && input[i] != '\0'; i++) {
		ch = (unsigned char)input[i];
		if (ch == '"' || ch == '\\') {
			if (*offset + 2 >= bufsiz)
				return KN_BBS_CONTROL_ERR_BUFFER;
			buf[(*offset)++] = '\\';
			buf[(*offset)++] = (char)ch;
		} else {
			if (*offset + 1 >= bufsiz)
				return KN_BBS_CONTROL_ERR_BUFFER;
			buf[(*offset)++] = (ch < 0x20 || ch > 0x7e) ? '?' :
			    (char)ch;
		}
		buf[*offset] = '\0';
	}

	if (*offset + 1 >= bufsiz)
		return KN_BBS_CONTROL_ERR_BUFFER;
	buf[(*offset)++] = '"';
	buf[*offset] = '\0';
	return KN_BBS_CONTROL_OK;
}

static enum kn_bbs_control_error
append_safe_path(char *buf, size_t bufsiz, size_t *offset, const char *path)
{
	size_t i;
	unsigned char ch;

	for (i = 0; path != NULL && path[i] != '\0'; i++) {
		if (*offset + 1 >= bufsiz)
			return KN_BBS_CONTROL_ERR_BUFFER;
		ch = (unsigned char)path[i];
		buf[(*offset)++] = (ch < 0x20 || ch > 0x7e) ? '?' :
		    (char)ch;
		buf[*offset] = '\0';
	}

	return KN_BBS_CONTROL_OK;
}

static enum kn_bbs_control_error
append_text_preview(char *buf, size_t bufsiz, size_t *offset,
	const uint8_t *body, size_t body_len)
{
	size_t i;
	size_t limit;
	uint8_t tmp[KN_BBS_CONTROL_PREVIEW_MAX * 2 + 1];
	size_t tmp_len;

	limit = body_len > KN_BBS_CONTROL_PREVIEW_MAX ?
	    KN_BBS_CONTROL_PREVIEW_MAX : body_len;
	tmp_len = 0;
	for (i = 0; i < limit; i++) {
		if (tmp_len + 2 >= sizeof(tmp))
			return KN_BBS_CONTROL_ERR_BUFFER;
		if (body[i] == '\n') {
			tmp[tmp_len++] = '\\';
			tmp[tmp_len++] = 'n';
		} else if (body[i] == '\r') {
			tmp[tmp_len++] = '\\';
			tmp[tmp_len++] = 'r';
		} else if (body[i] == '\t') {
			tmp[tmp_len++] = '\\';
			tmp[tmp_len++] = 't';
		} else if (body[i] == '"' || body[i] == '\\') {
			tmp[tmp_len++] = '\\';
			tmp[tmp_len++] = body[i];
		} else {
			tmp[tmp_len++] = body[i];
		}
	}
	tmp[tmp_len] = '\0';

	return append_quoted(buf, bufsiz, offset, (const char *)tmp);
}

static uint8_t
body_is_text(const uint8_t *body, size_t body_len)
{
	size_t i;

	for (i = 0; i < body_len; i++) {
		if (body[i] == '\n' || body[i] == '\r' || body[i] == '\t')
			continue;
		if (body[i] < 0x20 || body[i] > 0x7e)
			return 0;
	}

	return 1;
}

static enum kn_bbs_control_error
callsign_string(const struct kn_callsign *callsign, char *buf, size_t bufsiz)
{
	if (kn_callsign_format(callsign, buf, bufsiz) != 0)
		return KN_BBS_CONTROL_ERR_STORE;

	return KN_BBS_CONTROL_OK;
}

static enum kn_bbs_control_error
command_areas(struct kn_message_store *store, char *buf, size_t bufsiz)
{
	struct kn_message_index_area areas[KN_BBS_CONTROL_LIST_MAX];
	size_t count;
	size_t i;
	size_t offset;
	uint8_t truncated;
	enum kn_message_index_error irc;
	enum kn_bbs_control_error rc;

	irc = kn_message_index_areas(store, areas, KN_BBS_CONTROL_LIST_MAX,
	    &count);
	if (irc != KN_MESSAGE_INDEX_OK)
		return KN_BBS_CONTROL_ERR_STORE;
	truncated = count > KN_BBS_CONTROL_LIST_MAX ? 1 : 0;
	offset = 0;
	rc = append_format(buf, bufsiz, &offset,
	    "OK BBS AREAS count=%llu truncated=%s\n",
	    (unsigned long long)count, truncated != 0 ? "true" : "false");
	if (rc != KN_BBS_CONTROL_OK)
		return rc;
	for (i = 0; i < count && i < KN_BBS_CONTROL_LIST_MAX; i++) {
		rc = append_format(buf, bufsiz, &offset,
		    "BBS AREA name=%s messages=%llu newest=%llu\n",
		    areas[i].name, (unsigned long long)areas[i].count,
		    (unsigned long long)areas[i].newest_id);
		if (rc != KN_BBS_CONTROL_OK)
			return rc;
	}

	return append_format(buf, bufsiz, &offset, "END\n");
}

static enum kn_bbs_control_error
command_message(struct kn_message_store *store, const char *args, char *buf,
	size_t bufsiz)
{
	struct kn_message message;
	uint8_t *body;
	size_t body_len;
	size_t offset;
	uint64_t id;
	enum kn_message_store_error src;
	enum kn_bbs_control_error rc;

	if (parse_id(args, &id) == 0)
		return store_error_response(KN_MESSAGE_STORE_ERR_INVALID_ARGUMENT,
		    buf, bufsiz);
	src = kn_message_store_read_metadata(store, id, &message);
	if (src != KN_MESSAGE_STORE_OK)
		return store_error_response(src, buf, bufsiz);

	body = malloc(message.body_len == 0 ? 1 : message.body_len);
	if (body == NULL)
		return KN_BBS_CONTROL_ERR_STORE;
	src = kn_message_store_read_body(store, id, body, message.body_len,
	    &body_len);
	if (src != KN_MESSAGE_STORE_OK) {
		free(body);
		return store_error_response(src, buf, bufsiz);
	}

	offset = 0;
	rc = append_format(buf, bufsiz, &offset, "OK BBS MESSAGE id=%llu\n",
	    (unsigned long long)id);
	if (rc == KN_BBS_CONTROL_OK)
		rc = append_message_summary(buf, bufsiz, &offset, &message, 1);
	if (rc == KN_BBS_CONTROL_OK) {
		rc = append_format(buf, bufsiz, &offset,
		    "BBS PREVIEW binary=%s bytes=%llu truncated=%s ",
		    body_is_text(body, body_len) != 0 ? "false" : "true",
		    (unsigned long long)body_len,
		    body_len > KN_BBS_CONTROL_PREVIEW_MAX ? "true" : "false");
	}
	if (rc == KN_BBS_CONTROL_OK && body_is_text(body, body_len) != 0) {
		rc = append_format(buf, bufsiz, &offset, "text=");
		if (rc == KN_BBS_CONTROL_OK)
			rc = append_text_preview(buf, bufsiz, &offset, body,
			    body_len);
	} else if (rc == KN_BBS_CONTROL_OK) {
		rc = append_format(buf, bufsiz, &offset, "hex=");
		if (rc == KN_BBS_CONTROL_OK)
			rc = append_hex_preview(buf, bufsiz, &offset, body,
			    body_len);
	}
	free(body);
	if (rc != KN_BBS_CONTROL_OK)
		return rc;
	rc = append_format(buf, bufsiz, &offset, "\nEND\n");
	if (rc != KN_BBS_CONTROL_OK)
		return rc;

	return KN_BBS_CONTROL_OK;
}

static enum kn_bbs_control_error
command_messages(struct kn_message_store *store, const char *args, char *buf,
	size_t bufsiz)
{
	struct kn_message messages[KN_BBS_CONTROL_LIST_MAX];
	char value[KN_MESSAGE_AREA_MAX + 1];
	size_t count;
	size_t i;
	size_t offset;
	uint8_t truncated;
	enum kn_message_index_filter filter;
	enum kn_message_index_error irc;
	enum kn_bbs_control_error filter_rc;
	enum kn_bbs_control_error rc;

	rc = parse_filter(args, &filter, value, sizeof(value), &filter_rc);
	if (rc != KN_BBS_CONTROL_OK)
		return filter_rc;

	irc = kn_message_index_list(store, filter, value[0] == '\0' ? NULL :
	    value, messages, KN_BBS_CONTROL_LIST_MAX, &count);
	if (irc != KN_MESSAGE_INDEX_OK)
		return KN_BBS_CONTROL_ERR_STORE;

	truncated = count > KN_BBS_CONTROL_LIST_MAX ? 1 : 0;
	offset = 0;
	rc = append_format(buf, bufsiz, &offset,
	    "OK BBS MESSAGES count=%llu truncated=%s\n",
	    (unsigned long long)count, truncated != 0 ? "true" : "false");
	if (rc != KN_BBS_CONTROL_OK)
		return rc;
	for (i = 0; i < count && i < KN_BBS_CONTROL_LIST_MAX; i++) {
		rc = append_message_summary(buf, bufsiz, &offset,
		    &messages[i], 0);
		if (rc != KN_BBS_CONTROL_OK)
			return rc;
	}

	return append_format(buf, bufsiz, &offset, "END\n");
}

static enum kn_bbs_control_error
command_stats(struct kn_message_store *store, char *buf, size_t bufsiz)
{
	struct kn_bbs_store_stats stats;
	size_t offset;
	enum kn_bbs_store_maintenance_error src;
	enum kn_bbs_control_error rc;

	src = kn_bbs_store_stats(store->path, &stats);
	if (src != KN_BBS_STORE_MAINTENANCE_OK)
		return KN_BBS_CONTROL_ERR_STORE;
	offset = 0;
	rc = append_format(buf, bufsiz, &offset,
	    "OK BBS STATS total=%llu private=%llu bulletins=%llu "
	    "deleted=%llu users=%llu areas=%llu body_bytes=%llu "
	    "next_id=%llu newest_id=%llu\n",
	    (unsigned long long)stats.total_messages,
	    (unsigned long long)stats.private_messages,
	    (unsigned long long)stats.bulletins,
	    (unsigned long long)stats.deleted_messages,
	    (unsigned long long)stats.users,
	    (unsigned long long)stats.bulletin_areas,
	    (unsigned long long)stats.total_body_bytes,
	    (unsigned long long)stats.next_id,
	    (unsigned long long)stats.newest_message_id);
	if (rc != KN_BBS_CONTROL_OK)
		return rc;
	return append_format(buf, bufsiz, &offset, "END\n");
}

static enum kn_bbs_control_error
command_status(uint8_t enabled, struct kn_message_store *store, char *buf,
	size_t bufsiz)
{
	size_t offset;
	enum kn_bbs_control_error rc;

	offset = 0;
	rc = append_format(buf, bufsiz, &offset,
	    "OK BBS STATUS enabled=%s open=%s",
	    enabled != 0 ? "true" : "false",
	    enabled != 0 && store != NULL && store->open != 0 ? "true" :
	    "false");
	if (rc != KN_BBS_CONTROL_OK)
		return rc;
	if (enabled != 0 && store != NULL && store->open != 0) {
		rc = append_format(buf, bufsiz, &offset, " store=");
		if (rc != KN_BBS_CONTROL_OK)
			return rc;
		rc = append_safe_path(buf, bufsiz, &offset, store->path);
		if (rc != KN_BBS_CONTROL_OK)
			return rc;
	}
	rc = append_format(buf, bufsiz, &offset, "\nEND\n");
	if (rc != KN_BBS_CONTROL_OK)
		return rc;

	return KN_BBS_CONTROL_OK;
}

static enum kn_bbs_control_error
command_users(struct kn_message_store *store, char *buf, size_t bufsiz)
{
	struct kn_bbs_user users[KN_BBS_CONTROL_LIST_MAX];
	size_t count;
	size_t i;
	size_t offset;
	enum kn_bbs_user_error urc;
	enum kn_bbs_control_error rc;

	urc = kn_bbs_user_list(store, users, KN_BBS_CONTROL_LIST_MAX, &count);
	if (urc != KN_BBS_USER_OK)
		return KN_BBS_CONTROL_ERR_STORE;
	offset = 0;
	rc = append_format(buf, bufsiz, &offset,
	    "OK BBS USERS count=%llu truncated=false\n",
	    (unsigned long long)count);
	if (rc != KN_BBS_CONTROL_OK)
		return rc;
	for (i = 0; i < count && i < KN_BBS_CONTROL_LIST_MAX; i++) {
		rc = append_format(buf, bufsiz, &offset,
		    "BBS USER call=%s disabled=%s sysop=%s "
		    "login_count=%llu last_seen=%llu\n",
		    users[i].call, users[i].disabled != 0 ? "true" : "false",
		    users[i].sysop != 0 ? "true" : "false",
		    (unsigned long long)users[i].login_count,
		    (unsigned long long)users[i].last_seen);
		if (rc != KN_BBS_CONTROL_OK)
			return rc;
	}

	return append_format(buf, bufsiz, &offset, "END\n");
}

static enum kn_bbs_control_error
parse_filter(const char *args, enum kn_message_index_filter *filter,
	char *value, size_t value_len, enum kn_bbs_control_error *filter_error)
{
	struct kn_callsign call;

	if (args == NULL || args[0] == '\0') {
		*filter = KN_MESSAGE_INDEX_ALL;
		value[0] = '\0';
		return KN_BBS_CONTROL_OK;
	}
	if (strcmp(args, "PRIVATE") == 0) {
		*filter = KN_MESSAGE_INDEX_PRIVATE;
		value[0] = '\0';
		return KN_BBS_CONTROL_OK;
	}
	if (strcmp(args, "BULLETINS") == 0) {
		*filter = KN_MESSAGE_INDEX_BULLETIN;
		value[0] = '\0';
		return KN_BBS_CONTROL_OK;
	}
	if (strncmp(args, "AREA ", 5) == 0 && strchr(args + 5, ' ') == NULL) {
		if (kn_bbs_area_normalize(args + 5, value,
		    value_len) != KN_BBS_AREA_OK) {
			*filter_error = KN_BBS_CONTROL_ERR_INVALID_AREA;
			return KN_BBS_CONTROL_ERR_INVALID_AREA;
		}
		*filter = KN_MESSAGE_INDEX_AREA;
		return KN_BBS_CONTROL_OK;
	}
	if (strncmp(args, "TO ", 3) == 0 && strchr(args + 3, ' ') == NULL) {
		if (kn_callsign_parse(args + 3, &call) != 0 ||
		    kn_callsign_format(&call, value, value_len) != 0) {
			*filter_error = KN_BBS_CONTROL_ERR_INVALID_CALLSIGN;
			return KN_BBS_CONTROL_ERR_INVALID_CALLSIGN;
		}
		*filter = KN_MESSAGE_INDEX_TO;
		return KN_BBS_CONTROL_OK;
	}
	if (strncmp(args, "FROM ", 5) == 0 && strchr(args + 5, ' ') == NULL) {
		if (kn_callsign_parse(args + 5, &call) != 0 ||
		    kn_callsign_format(&call, value, value_len) != 0) {
			*filter_error = KN_BBS_CONTROL_ERR_INVALID_CALLSIGN;
			return KN_BBS_CONTROL_ERR_INVALID_CALLSIGN;
		}
		*filter = KN_MESSAGE_INDEX_FROM;
		return KN_BBS_CONTROL_OK;
	}

	*filter_error = KN_BBS_CONTROL_ERR_UNKNOWN_COMMAND;
	return KN_BBS_CONTROL_ERR_UNKNOWN_COMMAND;
}

static uint8_t
parse_id(const char *input, uint64_t *id)
{
	char *end;
	unsigned long long value;

	if (input == NULL || input[0] == '\0')
		return 0;
	while (*input == ' ' || *input == '\t')
		input++;
	if (*input == '\0')
		return 0;
	errno = 0;
	value = strtoull(input, &end, 10);
	if (errno != 0 || value == 0)
		return 0;
	while (*end == ' ' || *end == '\t')
		end++;
	if (*end != '\0')
		return 0;
	*id = (uint64_t)value;
	return 1;
}

static enum kn_bbs_control_error
store_error_response(enum kn_message_store_error src, char *buf,
	size_t bufsiz)
{
	const char *text;

	switch (src) {
	case KN_MESSAGE_STORE_ERR_INVALID_ARGUMENT:
	case KN_MESSAGE_STORE_ERR_INVALID_MESSAGE:
		text = "ERR invalid-message-id\n";
		break;
	case KN_MESSAGE_STORE_ERR_NOT_FOUND:
		text = "ERR message-not-found\n";
		break;
	case KN_MESSAGE_STORE_ERR_DELETED:
		text = "ERR message-deleted\n";
		break;
	case KN_MESSAGE_STORE_ERR_CORRUPT:
		text = "ERR corrupt-metadata\n";
		break;
	case KN_MESSAGE_STORE_OK:
	case KN_MESSAGE_STORE_ERR_IO:
	case KN_MESSAGE_STORE_ERR_BODY_TOO_LARGE:
	case KN_MESSAGE_STORE_ERR_BUFFER:
	default:
		text = "ERR store-error\n";
		break;
	}
	if (snprintf(buf, bufsiz, "%s", text) < 0 ||
	    strlen(text) >= bufsiz)
		return KN_BBS_CONTROL_ERR_BUFFER;
	return src == KN_MESSAGE_STORE_ERR_INVALID_ARGUMENT ||
	    src == KN_MESSAGE_STORE_ERR_INVALID_MESSAGE ?
	    KN_BBS_CONTROL_ERR_INVALID_ID : KN_BBS_CONTROL_ERR_STORE;
}

const char *
kn_bbs_control_error_name(enum kn_bbs_control_error error)
{
	switch (error) {
	case KN_BBS_CONTROL_OK:
		return "ok";
	case KN_BBS_CONTROL_ERR_INVALID_ARGUMENT:
		return "invalid argument";
	case KN_BBS_CONTROL_ERR_DISABLED:
		return "disabled";
	case KN_BBS_CONTROL_ERR_STORE:
		return "store";
	case KN_BBS_CONTROL_ERR_BUFFER:
		return "buffer";
	case KN_BBS_CONTROL_ERR_UNKNOWN_COMMAND:
		return "unknown command";
	case KN_BBS_CONTROL_ERR_INVALID_ID:
		return "invalid id";
	case KN_BBS_CONTROL_ERR_INVALID_AREA:
		return "invalid area";
	case KN_BBS_CONTROL_ERR_INVALID_CALLSIGN:
		return "invalid callsign";
	}

	return "unknown";
}

enum kn_bbs_control_error
kn_bbs_control_format(const char *command, uint8_t enabled,
	struct kn_message_store *store, char *buf, size_t bufsiz)
{
	if (command == NULL || buf == NULL || bufsiz == 0)
		return KN_BBS_CONTROL_ERR_INVALID_ARGUMENT;
	buf[0] = '\0';

	if (strcmp(command, "STATUS") == 0)
		return command_status(enabled, store, buf, bufsiz);

	if (enabled == 0) {
		(void)snprintf(buf, bufsiz, "ERR bbs-disabled\n");
		return KN_BBS_CONTROL_ERR_DISABLED;
	}
	if (store == NULL || store->open == 0) {
		(void)snprintf(buf, bufsiz, "ERR store-error\n");
		return KN_BBS_CONTROL_ERR_STORE;
	}

	if (strcmp(command, "STATS") == 0)
		return command_stats(store, buf, bufsiz);
	if (strcmp(command, "AREAS") == 0)
		return command_areas(store, buf, bufsiz);
	if (strcmp(command, "USERS") == 0)
		return command_users(store, buf, bufsiz);
	if (strcmp(command, "MESSAGES") == 0)
		return command_messages(store, NULL, buf, bufsiz);
	if (strncmp(command, "MESSAGES ", 9) == 0) {
		enum kn_bbs_control_error rc;

		rc = command_messages(store, command + 9, buf, bufsiz);
		if (rc == KN_BBS_CONTROL_ERR_INVALID_AREA)
			(void)snprintf(buf, bufsiz, "ERR invalid-area\n");
		else if (rc == KN_BBS_CONTROL_ERR_INVALID_CALLSIGN)
			(void)snprintf(buf, bufsiz, "ERR invalid-callsign\n");
		else if (rc == KN_BBS_CONTROL_ERR_UNKNOWN_COMMAND)
			(void)snprintf(buf, bufsiz, "ERR invalid-bbs-command\n");
		return rc;
	}
	if (strncmp(command, "MESSAGE ", 8) == 0)
		return command_message(store, command + 8, buf, bufsiz);

	(void)snprintf(buf, bufsiz, "ERR unknown-bbs-command\n");
	return KN_BBS_CONTROL_ERR_UNKNOWN_COMMAND;
}
