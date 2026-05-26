/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/bbs/bbs_shell.c */

#include <sys/types.h>

#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kilonode/bbs_read_state.h"
#include "kilonode/bbs_shell.h"
#include "kilonode/bbs_user.h"
#include "kilonode/callsign.h"
#include "kilonode/message_index.h"
#include "kilonode/node_shell.h"

#define BBS_LIST_MAX 256

static enum kn_bbs_shell_error append_format(char *, size_t, size_t *,
	const char *, ...);
static enum kn_bbs_shell_error append_prompt(char *, size_t, size_t *);
static enum kn_bbs_shell_error append_session_prompt(
	const struct kn_bbs_shell_session *, char *, size_t, size_t *);
static enum kn_bbs_shell_error append_safe(char *, size_t, size_t *,
	const char *);
static enum kn_bbs_shell_error append_body(char *, size_t, size_t *,
	const uint8_t *, size_t);
static enum kn_bbs_shell_error command_areas(
	const struct kn_bbs_shell_snapshot *, char *, size_t, size_t *);
static enum kn_bbs_shell_error command_kill(const char *,
	const struct kn_bbs_shell_snapshot *, char *, size_t, size_t *);
static enum kn_bbs_shell_error command_list(const char *,
	struct kn_bbs_shell_session *, const struct kn_bbs_shell_snapshot *,
	char *, size_t, size_t *, uint8_t);
static enum kn_bbs_shell_error command_markread(const char *,
	struct kn_bbs_shell_session *, const struct kn_bbs_shell_snapshot *,
	char *, size_t, size_t *);
static enum kn_bbs_shell_error command_read(const char *,
	struct kn_bbs_shell_session *, const struct kn_bbs_shell_snapshot *,
	char *, size_t, size_t *);
static enum kn_bbs_shell_error command_send(const char *,
	struct kn_bbs_shell_session *, const struct kn_bbs_shell_snapshot *,
	char *, size_t, size_t *);
static enum kn_bbs_shell_error command_users(
	const struct kn_bbs_shell_snapshot *, char *, size_t, size_t *);
static enum kn_bbs_shell_error command_whoami(
	struct kn_bbs_shell_session *, const struct kn_bbs_shell_snapshot *,
	char *, size_t, size_t *);
static enum kn_bbs_shell_error finalize_body(
	struct kn_bbs_shell_session *, const struct kn_bbs_shell_snapshot *,
	char *, size_t, size_t *);
static enum kn_bbs_shell_error format_message_line(
	struct kn_message_store *, const char *, const struct kn_message *,
	char *, size_t, size_t *);
static enum kn_bbs_shell_error handle_pending_line(
	struct kn_bbs_shell_session *, const char *,
	const struct kn_bbs_shell_snapshot *, char *, size_t, size_t *);
static uint8_t parse_id(const char *, uint64_t *);
static enum kn_bbs_shell_error parse_send_args(const char *,
	struct kn_bbs_shell_session *);
static enum kn_bbs_shell_error pop_word(const char **, char *, size_t);
static enum kn_bbs_shell_error store_line(
	struct kn_bbs_shell_session *, const char *,
	const struct kn_bbs_shell_snapshot *);
static void trim_copy(char *, size_t, const char *);

static enum kn_bbs_shell_error
append_format(char *buf, size_t bufsiz, size_t *offset, const char *fmt, ...)
{
	va_list args;
	int needed;

	if (*offset >= bufsiz)
		return KN_BBS_SHELL_ERR_BUFFER;

	va_start(args, fmt);
	needed = vsnprintf(buf + *offset, bufsiz - *offset, fmt, args);
	va_end(args);
	if (needed < 0 || (size_t)needed >= bufsiz - *offset)
		return KN_BBS_SHELL_ERR_BUFFER;

	*offset += (size_t)needed;
	return KN_BBS_SHELL_OK;
}

static enum kn_bbs_shell_error
append_prompt(char *buf, size_t bufsiz, size_t *offset)
{
	return append_format(buf, bufsiz, offset, "%s", KN_BBS_SHELL_PROMPT);
}

static enum kn_bbs_shell_error
append_session_prompt(const struct kn_bbs_shell_session *session, char *buf,
	size_t bufsiz, size_t *offset)
{
	if (session != NULL && session->identity[0] != '\0')
		return append_format(buf, bufsiz, offset, "BBS %s> ",
		    session->identity);
	return append_prompt(buf, bufsiz, offset);
}

static enum kn_bbs_shell_error
append_safe(char *buf, size_t bufsiz, size_t *offset, const char *input)
{
	size_t i;

	for (i = 0; input != NULL && input[i] != '\0'; i++) {
		if (*offset + 1 >= bufsiz)
			return KN_BBS_SHELL_ERR_BUFFER;
		if ((unsigned char)input[i] < 0x20 ||
		    (unsigned char)input[i] > 0x7e)
			buf[(*offset)++] = '?';
		else
			buf[(*offset)++] = input[i];
		buf[*offset] = '\0';
	}

	return KN_BBS_SHELL_OK;
}

static enum kn_bbs_shell_error
append_body(char *buf, size_t bufsiz, size_t *offset, const uint8_t *body,
	size_t body_len)
{
	size_t i;

	for (i = 0; i < body_len; i++) {
		if (body[i] == '\n') {
			if (append_format(buf, bufsiz, offset, "\r\n") !=
			    KN_BBS_SHELL_OK)
				return KN_BBS_SHELL_ERR_BUFFER;
			continue;
		}
		if (*offset + 1 >= bufsiz)
			return KN_BBS_SHELL_ERR_BUFFER;
		if (body[i] < 0x20 || body[i] > 0x7e)
			buf[(*offset)++] = '?';
		else
			buf[(*offset)++] = (char)body[i];
		buf[*offset] = '\0';
	}

	return KN_BBS_SHELL_OK;
}

static enum kn_bbs_shell_error
command_areas(const struct kn_bbs_shell_snapshot *snapshot, char *buf,
	size_t bufsiz, size_t *offset)
{
	struct kn_message_index_area areas[BBS_LIST_MAX];
	size_t count;
	size_t i;
	enum kn_message_index_error irc;
	enum kn_bbs_shell_error rc;

	irc = kn_message_index_areas(snapshot->store, areas, BBS_LIST_MAX,
	    &count);
	if (irc != KN_MESSAGE_INDEX_OK)
		return append_format(buf, bufsiz, offset,
		    "ERR index-error code=%s\r\n",
		    kn_message_index_error_name(irc));

	rc = append_format(buf, bufsiz, offset, "OK AREAS count=%llu\r\n",
	    (unsigned long long)count);
	if (rc != KN_BBS_SHELL_OK)
		return rc;
	for (i = 0; i < count && i < BBS_LIST_MAX; i++) {
		rc = append_format(buf, bufsiz, offset, "AREA name=");
		if (rc != KN_BBS_SHELL_OK)
			return rc;
		rc = append_safe(buf, bufsiz, offset, areas[i].name);
		if (rc != KN_BBS_SHELL_OK)
			return rc;
		rc = append_format(buf, bufsiz, offset,
		    " count=%llu newest=%llu\r\n",
		    (unsigned long long)areas[i].count,
		    (unsigned long long)areas[i].newest_id);
		if (rc != KN_BBS_SHELL_OK)
			return rc;
	}

	return append_format(buf, bufsiz, offset, "END\r\n");
}

static enum kn_bbs_shell_error
command_kill(const char *args, const struct kn_bbs_shell_snapshot *snapshot,
	char *buf, size_t bufsiz, size_t *offset)
{
	uint64_t id;
	enum kn_message_store_error src;

	if (parse_id(args, &id) == 0)
		return append_format(buf, bufsiz, offset,
		    "ERR invalid-id\r\n");

	src = kn_message_store_delete(snapshot->store, id);
	if (src != KN_MESSAGE_STORE_OK)
		return append_format(buf, bufsiz, offset,
		    "ERR store-error code=%s\r\n",
		    kn_message_store_error_name(src));

	return append_format(buf, bufsiz, offset, "OK KILL id=%llu\r\n",
	    (unsigned long long)id);
}

static enum kn_bbs_shell_error
command_list(const char *args, struct kn_bbs_shell_session *session,
	const struct kn_bbs_shell_snapshot *snapshot, char *buf, size_t bufsiz,
	size_t *offset, uint8_t only_unread)
{
	struct kn_message messages[BBS_LIST_MAX];
	char word[16];
	char value[KN_MESSAGE_AREA_MAX + 1];
	size_t count;
	size_t shown;
	size_t i;
	uint8_t is_read;
	enum kn_message_index_filter filter;
	const char *filter_value;
	enum kn_message_index_error irc;
	enum kn_bbs_shell_error rc;

	filter = KN_MESSAGE_INDEX_ALL;
	filter_value = NULL;
	if (args != NULL && args[0] != '\0') {
		if (pop_word(&args, word, sizeof(word)) != KN_BBS_SHELL_OK)
			return append_format(buf, bufsiz, offset,
			    "ERR invalid-list\r\n");
		for (i = 0; word[i] != '\0'; i++)
			word[i] = (char)toupper((unsigned char)word[i]);
		if (strcmp(word, "PRIVATE") == 0)
			filter = KN_MESSAGE_INDEX_PRIVATE;
		else if (strcmp(word, "BULLETINS") == 0)
			filter = KN_MESSAGE_INDEX_BULLETIN;
		else if (strcmp(word, "AREA") == 0)
			filter = KN_MESSAGE_INDEX_AREA;
		else if (strcmp(word, "TO") == 0)
			filter = KN_MESSAGE_INDEX_TO;
		else if (strcmp(word, "FROM") == 0)
			filter = KN_MESSAGE_INDEX_FROM;
		else
			return append_format(buf, bufsiz, offset,
			    "ERR invalid-list\r\n");
		if (filter == KN_MESSAGE_INDEX_AREA ||
		    filter == KN_MESSAGE_INDEX_TO ||
		    filter == KN_MESSAGE_INDEX_FROM) {
			if (pop_word(&args, value, sizeof(value)) !=
			    KN_BBS_SHELL_OK)
				return append_format(buf, bufsiz, offset,
				    "ERR invalid-list\r\n");
			filter_value = value;
		}
		while (*args == ' ' || *args == '\t')
			args++;
		if (*args != '\0')
			return append_format(buf, bufsiz, offset,
			    "ERR invalid-list\r\n");
	}

	irc = kn_message_index_list(snapshot->store, filter, filter_value,
	    messages, BBS_LIST_MAX, &count);
	if (irc != KN_MESSAGE_INDEX_OK)
		return append_format(buf, bufsiz, offset,
		    "ERR index-error code=%s\r\n",
		    kn_message_index_error_name(irc));

	rc = append_format(buf, bufsiz, offset, "OK LIST count=%llu\r\n",
	    (unsigned long long)count);
	if (rc != KN_BBS_SHELL_OK)
		return rc;
	shown = 0;
	for (i = 0; i < count && i < BBS_LIST_MAX; i++) {
		if (only_unread != 0) {
			if (kn_bbs_read_state_is_read(snapshot->store,
			    session->identity, messages[i].id, &is_read) !=
			    KN_BBS_READ_STATE_OK)
				is_read = 0;
			if (is_read != 0)
				continue;
		}
		rc = format_message_line(snapshot->store, session->identity,
		    &messages[i], buf, bufsiz, offset);
		if (rc != KN_BBS_SHELL_OK)
			return rc;
		shown++;
	}
	(void)shown;

	return append_format(buf, bufsiz, offset, "END\r\n");
}

static enum kn_bbs_shell_error
command_markread(const char *args, struct kn_bbs_shell_session *session,
	const struct kn_bbs_shell_snapshot *snapshot, char *buf, size_t bufsiz,
	size_t *offset)
{
	uint64_t id;
	enum kn_bbs_read_state_error rrc;

	if (session->identity[0] == '\0')
		return append_format(buf, bufsiz, offset,
		    "ERR identity-required\r\n");
	if (parse_id(args, &id) == 0)
		return append_format(buf, bufsiz, offset,
		    "ERR invalid-id\r\n");
	rrc = kn_bbs_read_state_mark_read(snapshot->store, session->identity,
	    id);
	if (rrc != KN_BBS_READ_STATE_OK)
		return append_format(buf, bufsiz, offset,
		    "ERR read-state code=%s\r\n",
		    kn_bbs_read_state_error_name(rrc));
	return append_format(buf, bufsiz, offset, "OK MARKREAD id=%llu\r\n",
	    (unsigned long long)id);
}

static enum kn_bbs_shell_error
command_read(const char *args, struct kn_bbs_shell_session *session,
	const struct kn_bbs_shell_snapshot *snapshot, char *buf, size_t bufsiz,
	size_t *offset)
{
	struct kn_message message;
	uint8_t body[KN_MESSAGE_BODY_MAX];
	uint64_t id;
	size_t body_len;
	enum kn_message_store_error src;
	enum kn_bbs_shell_error rc;

	if (session->identity[0] == '\0')
		return append_format(buf, bufsiz, offset,
		    "ERR identity-required\r\n");
	if (parse_id(args, &id) == 0)
		return append_format(buf, bufsiz, offset,
		    "ERR invalid-id\r\n");

	src = kn_message_store_read_metadata(snapshot->store, id, &message);
	if (src != KN_MESSAGE_STORE_OK)
		return append_format(buf, bufsiz, offset,
		    "ERR store-error code=%s\r\n",
		    kn_message_store_error_name(src));
	src = kn_message_store_read_body(snapshot->store, id, body,
	    sizeof(body), &body_len);
	if (src != KN_MESSAGE_STORE_OK)
		return append_format(buf, bufsiz, offset,
		    "ERR store-error code=%s\r\n",
		    kn_message_store_error_name(src));
	(void)kn_bbs_read_state_mark_read(snapshot->store, session->identity,
	    id);

	rc = append_format(buf, bufsiz, offset, "OK READ\r\n");
	if (rc != KN_BBS_SHELL_OK)
		return rc;
	rc = format_message_line(snapshot->store, session->identity, &message,
	    buf, bufsiz, offset);
	if (rc != KN_BBS_SHELL_OK)
		return rc;
	rc = append_format(buf, bufsiz, offset, "BODY\r\n");
	if (rc != KN_BBS_SHELL_OK)
		return rc;
	rc = append_body(buf, bufsiz, offset, body, body_len);
	if (rc != KN_BBS_SHELL_OK)
		return rc;
	if (body_len == 0 || body[body_len - 1] != '\n') {
		rc = append_format(buf, bufsiz, offset, "\r\n");
		if (rc != KN_BBS_SHELL_OK)
			return rc;
	}
	return append_format(buf, bufsiz, offset, "END\r\n");
}

static enum kn_bbs_shell_error
command_send(const char *args, struct kn_bbs_shell_session *session,
	const struct kn_bbs_shell_snapshot *snapshot, char *buf, size_t bufsiz,
	size_t *offset)
{
	struct kn_message message;
	enum kn_bbs_shell_error rc;

	(void)snapshot;
	if (session->identity[0] == '\0')
		return append_format(buf, bufsiz, offset,
		    "ERR identity-required\r\n");
	rc = parse_send_args(args, session);
	if (rc != KN_BBS_SHELL_OK)
		return append_format(buf, bufsiz, offset,
		    "ERR invalid-send\r\n");
	session->body_len = 0;
	session->body_overflow = 0;
	if (session->pending_type == KN_BBS_SHELL_PENDING_PRIVATE) {
		if (kn_message_private_init(&message, session->pending_from,
		    session->pending_dest, session->pending_subject, 1, 1) !=
		    KN_MESSAGE_OK) {
			session->pending_type = KN_BBS_SHELL_PENDING_NONE;
			return append_format(buf, bufsiz, offset,
			    "ERR invalid-message\r\n");
		}
	} else {
		if (kn_message_bulletin_init(&message, session->pending_from,
		    session->pending_dest, session->pending_subject, 1, 1) !=
		    KN_MESSAGE_OK) {
			session->pending_type = KN_BBS_SHELL_PENDING_NONE;
			return append_format(buf, bufsiz, offset,
			    "ERR invalid-message\r\n");
		}
	}
	return append_format(buf, bufsiz, offset,
	    "OK SEND enter-body end-with-dot\r\n");
}

static enum kn_bbs_shell_error
command_users(const struct kn_bbs_shell_snapshot *snapshot, char *buf,
	size_t bufsiz, size_t *offset)
{
	struct kn_bbs_user users[KN_BBS_USER_LIST_MAX];
	char summary[256];
	size_t count;
	size_t i;
	enum kn_bbs_user_error urc;
	enum kn_bbs_shell_error rc;

	urc = kn_bbs_user_list(snapshot->store, users, KN_BBS_USER_LIST_MAX,
	    &count);
	if (urc != KN_BBS_USER_OK)
		return append_format(buf, bufsiz, offset,
		    "ERR user code=%s\r\n", kn_bbs_user_error_name(urc));
	rc = append_format(buf, bufsiz, offset, "OK USERS count=%llu\r\n",
	    (unsigned long long)count);
	if (rc != KN_BBS_SHELL_OK)
		return rc;
	for (i = 0; i < count && i < KN_BBS_USER_LIST_MAX; i++) {
		if (kn_bbs_user_format(&users[i], summary, sizeof(summary)) !=
		    KN_BBS_USER_OK)
			continue;
		rc = append_format(buf, bufsiz, offset, "USER ");
		if (rc != KN_BBS_SHELL_OK)
			return rc;
		rc = append_safe(buf, bufsiz, offset, summary);
		if (rc != KN_BBS_SHELL_OK)
			return rc;
		rc = append_format(buf, bufsiz, offset, "\r\n");
		if (rc != KN_BBS_SHELL_OK)
			return rc;
	}
	return append_format(buf, bufsiz, offset, "END\r\n");
}

static enum kn_bbs_shell_error
command_whoami(struct kn_bbs_shell_session *session,
	const struct kn_bbs_shell_snapshot *snapshot, char *buf, size_t bufsiz,
	size_t *offset)
{
	struct kn_bbs_user user;
	char summary[256];
	enum kn_bbs_user_error urc;

	urc = kn_bbs_user_load(snapshot->store, session->identity, &user);
	if (urc != KN_BBS_USER_OK)
		return append_format(buf, bufsiz, offset,
		    "ERR user code=%s\r\n", kn_bbs_user_error_name(urc));
	if (kn_bbs_user_format(&user, summary, sizeof(summary)) !=
	    KN_BBS_USER_OK)
		return KN_BBS_SHELL_ERR_BUFFER;
	return append_format(buf, bufsiz, offset, "OK WHOAMI %s\r\n",
	    summary);
}

static enum kn_bbs_shell_error
finalize_body(struct kn_bbs_shell_session *session,
	const struct kn_bbs_shell_snapshot *snapshot, char *buf, size_t bufsiz,
	size_t *offset)
{
	char identity[KN_CALLSIGN_MAX + 4];
	uint64_t id;
	enum kn_message_store_error src;

	memcpy(identity, session->identity, sizeof(identity));
	if (session->body_overflow != 0) {
		kn_bbs_shell_reset(session);
		session->active = 1;
		memcpy(session->identity, identity, sizeof(session->identity));
		return append_format(buf, bufsiz, offset,
		    "ERR body-too-large\r\n");
	}
	if (session->body_len == 0) {
		kn_bbs_shell_reset(session);
		session->active = 1;
		memcpy(session->identity, identity, sizeof(session->identity));
		return append_format(buf, bufsiz, offset,
		    "ERR empty-body\r\n");
	}

	if (session->pending_type == KN_BBS_SHELL_PENDING_PRIVATE)
		src = kn_message_store_create_private(snapshot->store,
		    session->pending_from, session->pending_dest,
		    session->pending_subject, session->body, session->body_len,
		    &id);
	else
		src = kn_message_store_create_bulletin(snapshot->store,
		    session->pending_from, session->pending_dest,
		    session->pending_subject, session->body, session->body_len,
		    &id);

	kn_bbs_shell_reset(session);
	session->active = 1;
	memcpy(session->identity, identity, sizeof(session->identity));
	if (src != KN_MESSAGE_STORE_OK)
		return append_format(buf, bufsiz, offset,
		    "ERR store-error code=%s\r\n",
		    kn_message_store_error_name(src));

	return append_format(buf, bufsiz, offset, "OK STORED id=%llu\r\n",
	    (unsigned long long)id);
}

static enum kn_bbs_shell_error
format_message_line(struct kn_message_store *store, const char *identity,
	const struct kn_message *message, char *buf, size_t bufsiz,
	size_t *offset)
{
	char from[KN_CALLSIGN_MAX + 4];
	char to[KN_CALLSIGN_MAX + 4];
	const char *dest;
	enum kn_bbs_shell_error rc;
	uint8_t is_read;

	if (kn_callsign_format(&message->from, from, sizeof(from)) != 0)
		(void)snprintf(from, sizeof(from), "-");
	if (message->type == KN_MESSAGE_TYPE_PRIVATE) {
		if (kn_callsign_format(&message->to, to, sizeof(to)) != 0)
			(void)snprintf(to, sizeof(to), "-");
		dest = to;
	} else {
		dest = message->area;
	}

	is_read = 0;
	if (identity != NULL && identity[0] != '\0')
		(void)kn_bbs_read_state_is_read(store, identity, message->id,
		    &is_read);
	rc = append_format(buf, bufsiz, offset,
	    "MSG id=%llu type=%s read=%s from=%s to=",
	    (unsigned long long)message->id,
	    kn_message_type_name(message->type),
	    is_read != 0 ? "yes" : "no", from);
	if (rc != KN_BBS_SHELL_OK)
		return rc;
	rc = append_safe(buf, bufsiz, offset, dest);
	if (rc != KN_BBS_SHELL_OK)
		return rc;
	rc = append_format(buf, bufsiz, offset,
	    " created=%llu subject=",
	    (unsigned long long)message->created);
	if (rc != KN_BBS_SHELL_OK)
		return rc;
	rc = append_safe(buf, bufsiz, offset, message->subject);
	if (rc != KN_BBS_SHELL_OK)
		return rc;

	return append_format(buf, bufsiz, offset, "\r\n");
}

static enum kn_bbs_shell_error
handle_pending_line(struct kn_bbs_shell_session *session, const char *line,
	const struct kn_bbs_shell_snapshot *snapshot, char *buf, size_t bufsiz,
	size_t *offset)
{
	enum kn_bbs_shell_error rc;

	if (strcmp(line, ".") == 0)
		return finalize_body(session, snapshot, buf, bufsiz, offset);
	if (strcmp(line, "..") == 0)
		rc = store_line(session, ".", snapshot);
	else
		rc = store_line(session, line, snapshot);
	if (rc != KN_BBS_SHELL_OK)
		return rc;

	return append_session_prompt(session, buf, bufsiz, offset);
}

static uint8_t
parse_id(const char *args, uint64_t *id)
{
	char *end;
	unsigned long long value;

	while (args != NULL && (*args == ' ' || *args == '\t'))
		args++;
	if (args == NULL || args[0] == '\0')
		return 0;

	errno = 0;
	value = strtoull(args, &end, 10);
	if (errno != 0 || value == 0)
		return 0;
	while (*end == ' ' || *end == '\t')
		end++;
	if (*end != '\0')
		return 0;

	*id = (uint64_t)value;
	return 1;
}

static enum kn_bbs_shell_error
parse_send_args(const char *args, struct kn_bbs_shell_session *session)
{
	char type[16];
	char from[KN_CALLSIGN_MAX + 4];
	char dest[KN_MESSAGE_AREA_MAX + 1];
	char subject[KN_MESSAGE_SUBJECT_MAX + 1];
	size_t len;

	if (pop_word(&args, type, sizeof(type)) != KN_BBS_SHELL_OK)
		return KN_BBS_SHELL_ERR_INVALID_ARGUMENT;
	for (len = 0; type[len] != '\0'; len++)
		type[len] = (char)toupper((unsigned char)type[len]);

	if (strcmp(type, "PRIVATE") == 0) {
		memcpy(from, session->identity, strlen(session->identity) + 1);
		if (pop_word(&args, dest, sizeof(dest)) != KN_BBS_SHELL_OK)
			return KN_BBS_SHELL_ERR_INVALID_ARGUMENT;
		session->pending_type = KN_BBS_SHELL_PENDING_PRIVATE;
	} else if (strcmp(type, "BULLETIN") == 0) {
		memcpy(from, session->identity, strlen(session->identity) + 1);
		if (pop_word(&args, dest, sizeof(dest)) != KN_BBS_SHELL_OK)
			return KN_BBS_SHELL_ERR_INVALID_ARGUMENT;
		session->pending_type = KN_BBS_SHELL_PENDING_BULLETIN;
	} else {
		return KN_BBS_SHELL_ERR_INVALID_ARGUMENT;
	}
	trim_copy(subject, sizeof(subject), args);
	if (subject[0] == '\0')
		return KN_BBS_SHELL_ERR_INVALID_ARGUMENT;
	if (subject[0] == '"') {
		len = strlen(subject);
		if (len < 2 || subject[len - 1] != '"')
			return KN_BBS_SHELL_ERR_INVALID_ARGUMENT;
		memmove(subject, subject + 1, len - 2);
		subject[len - 2] = '\0';
	}
	memcpy(session->pending_from, from, strlen(from) + 1);
	memcpy(session->pending_dest, dest, strlen(dest) + 1);
	memcpy(session->pending_subject, subject, strlen(subject) + 1);
	return KN_BBS_SHELL_OK;
}

static enum kn_bbs_shell_error
pop_word(const char **input, char *word, size_t wordsiz)
{
	size_t len;

	while (**input == ' ' || **input == '\t')
		(*input)++;
	if (**input == '\0')
		return KN_BBS_SHELL_ERR_INVALID_ARGUMENT;

	len = 0;
	while ((*input)[len] != '\0' && (*input)[len] != ' ' &&
	    (*input)[len] != '\t') {
		if (len + 1 >= wordsiz)
			return KN_BBS_SHELL_ERR_BUFFER;
		word[len] = (*input)[len];
		len++;
	}
	word[len] = '\0';
	*input += len;
	return KN_BBS_SHELL_OK;
}

static enum kn_bbs_shell_error
store_line(struct kn_bbs_shell_session *session, const char *line,
	const struct kn_bbs_shell_snapshot *snapshot)
{
	size_t max;
	size_t len;
	size_t i;

	max = snapshot->max_body_bytes;
	if (max == 0 || max > KN_MESSAGE_BODY_MAX)
		max = KN_MESSAGE_BODY_MAX;
	len = strlen(line);
	if (session->body_len + len + 1 > max ||
	    session->body_len + len + 1 > sizeof(session->body)) {
		session->body_overflow = 1;
		return KN_BBS_SHELL_OK;
	}
	for (i = 0; i < len; i++) {
		if ((unsigned char)line[i] < 0x20 ||
		    (unsigned char)line[i] > 0x7e)
			session->body[session->body_len++] = '?';
		else
			session->body[session->body_len++] = (uint8_t)line[i];
	}
	session->body[session->body_len++] = '\n';
	return KN_BBS_SHELL_OK;
}

static void
trim_copy(char *dst, size_t dst_len, const char *src)
{
	size_t len;

	if (dst_len == 0)
		return;
	while (src != NULL && (*src == ' ' || *src == '\t'))
		src++;
	if (src == NULL) {
		dst[0] = '\0';
		return;
	}
	len = strlen(src);
	while (len > 0 && (src[len - 1] == ' ' || src[len - 1] == '\t'))
		len--;
	if (len >= dst_len)
		len = dst_len - 1;
	memcpy(dst, src, len);
	dst[len] = '\0';
}

enum kn_bbs_shell_error
kn_bbs_shell_format(struct kn_bbs_shell_session *session, const char *line,
	const struct kn_bbs_shell_snapshot *snapshot, char *out, size_t out_len,
	uint8_t *close_session, uint8_t *exit_bbs)
{
	char command[KN_NODE_SHELL_LINE_MAX];
	char word[16];
	const char *args;
	size_t i;
	size_t len;
	size_t offset;
	enum kn_bbs_shell_error rc;

	if (session == NULL || line == NULL || snapshot == NULL || out == NULL ||
	    out_len == 0 || close_session == NULL || exit_bbs == NULL)
		return KN_BBS_SHELL_ERR_INVALID_ARGUMENT;
	if (snapshot->enabled == 0 || snapshot->store == NULL)
		return KN_BBS_SHELL_ERR_INVALID_ARGUMENT;

	out[0] = '\0';
	*close_session = 0;
	*exit_bbs = 0;
	offset = 0;
	len = strlen(line);
	if (len >= sizeof(command)) {
		rc = append_format(out, out_len, &offset,
		    "ERR line-too-long\r\n");
		if (rc != KN_BBS_SHELL_OK)
			return rc;
		return append_session_prompt(session, out, out_len, &offset);
	}
	memcpy(command, line, len + 1);
	for (i = 0; command[i] != '\0'; i++) {
		if ((unsigned char)command[i] < 0x20 ||
		    (unsigned char)command[i] > 0x7e)
			command[i] = '?';
	}

	if (session->pending_type != KN_BBS_SHELL_PENDING_NONE)
		return handle_pending_line(session, command, snapshot, out,
		    out_len, &offset);

	while (*line == ' ' || *line == '\t')
		line++;
	len = strlen(line);
	while (len > 0 && (line[len - 1] == ' ' || line[len - 1] == '\t'))
		len--;
	if (len == 0)
		return append_session_prompt(session, out, out_len, &offset);
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

	if (strcmp(word, "HELP") == 0)
		rc = append_format(out, out_len, &offset,
		    "OK HELP HELP WHOAMI USERS AREAS LIST UNREAD READ MARKREAD "
		    "SEND KILL EXIT BYE QUIT "
		    "LIST-PRIVATE LIST-BULLETINS LIST-AREA LIST-TO "
		    "LIST-FROM\r\n");
	else if (strcmp(word, "WHOAMI") == 0)
		rc = command_whoami(session, snapshot, out, out_len, &offset);
	else if (strcmp(word, "USERS") == 0)
		rc = command_users(snapshot, out, out_len, &offset);
	else if (strcmp(word, "AREAS") == 0)
		rc = command_areas(snapshot, out, out_len, &offset);
	else if (strcmp(word, "LIST") == 0)
		rc = command_list(args, session, snapshot, out, out_len,
		    &offset, 0);
	else if (strcmp(word, "UNREAD") == 0)
		rc = command_list(args, session, snapshot, out, out_len,
		    &offset, 1);
	else if (strcmp(word, "READ") == 0)
		rc = command_read(args, session, snapshot, out, out_len,
		    &offset);
	else if (strcmp(word, "MARKREAD") == 0)
		rc = command_markread(args, session, snapshot, out, out_len,
		    &offset);
	else if (strcmp(word, "SEND") == 0)
		rc = command_send(args, session, snapshot, out, out_len,
		    &offset);
	else if (strcmp(word, "KILL") == 0)
		rc = command_kill(args, snapshot, out, out_len, &offset);
	else if (strcmp(word, "EXIT") == 0) {
		*exit_bbs = 1;
		kn_bbs_shell_reset(session);
		return append_format(out, out_len, &offset,
		    "OK EXIT\r\n%s", KN_NODE_SHELL_PROMPT);
	} else if (strcmp(word, "BYE") == 0 || strcmp(word, "QUIT") == 0) {
		*close_session = 1;
		return append_format(out, out_len, &offset, "BYE\r\n");
	} else {
		rc = append_format(out, out_len, &offset,
		    "ERR unknown-command\r\n");
	}
	if (rc != KN_BBS_SHELL_OK)
		return rc;

	return append_session_prompt(session, out, out_len, &offset);
}

void
kn_bbs_shell_reset(struct kn_bbs_shell_session *session)
{
	if (session == NULL)
		return;

	memset(session, 0, sizeof(*session));
}
