/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/compat/compat_command_profile.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "kilonode/compat_command_profile.h"

#define PROF_FIELD_NAME             0x0001u
#define PROF_FIELD_SUBJECT          0x0002u
#define PROF_FIELD_CLEAN_ROOM       0x0004u
#define PROF_FIELD_SOURCE_CODE_USED 0x0008u
#define PROF_BLOCK_CATEGORY         0x0001u
#define PROF_BLOCK_TRANSPORT        0x0002u
#define PROF_BLOCK_ARGS             0x0004u
#define PROF_BLOCK_REPLY            0x0008u
#define PROF_BLOCK_STATEFUL         0x0010u
#define PROF_BLOCK_CONNECTED        0x0020u
#define PROF_BLOCK_STATUS           0x0040u

static enum kn_compat_profile_error append_profile(char *, size_t, size_t *,
	const char *, enum kn_compat_profile_category,
	enum kn_compat_profile_transport, enum kn_compat_profile_args,
	enum kn_compat_profile_reply, uint8_t, uint8_t,
	enum kn_compat_profile_status);
static enum kn_compat_profile_error args_parse(const char *,
	enum kn_compat_profile_args *);
static enum kn_compat_profile_error bool_parse(const char *, uint8_t *);
static enum kn_compat_profile_error category_parse(const char *,
	enum kn_compat_profile_category *);
static void error_set(struct kn_compat_profile_error_info *,
	enum kn_compat_profile_error, size_t, const char *);
static enum kn_compat_profile_error header_set(
	struct kn_compat_command_profiles *, uint32_t *, const char *,
	const char *, size_t, struct kn_compat_profile_error_info *);
static enum kn_compat_profile_error profile_finish(
	struct kn_compat_command_profile *, uint32_t, size_t,
	struct kn_compat_profile_error_info *);
static enum kn_compat_profile_error profile_set(
	struct kn_compat_command_profile *, uint32_t *, const char *,
	const char *, size_t, struct kn_compat_profile_error_info *);
static enum kn_compat_profile_error reply_parse(const char *,
	enum kn_compat_profile_reply *);
static enum kn_compat_profile_error status_parse(const char *,
	enum kn_compat_profile_status *);
static enum kn_compat_profile_error transport_parse(const char *,
	enum kn_compat_profile_transport *);
static uint8_t field_seen(uint32_t *, uint32_t);
static char *trim(char *);

void
kn_compat_command_profiles_clear(struct kn_compat_command_profiles *profiles)
{
	if (profiles == NULL)
		return;
	memset(profiles, 0, sizeof(*profiles));
}

const char *
kn_compat_profile_error_name(enum kn_compat_profile_error error)
{
	switch (error) {
	case KN_COMPAT_PROFILE_OK:
		return "ok";
	case KN_COMPAT_PROFILE_ERR_INVALID_ARGUMENT:
		return "invalid-argument";
	case KN_COMPAT_PROFILE_ERR_IO:
		return "io";
	case KN_COMPAT_PROFILE_ERR_LINE_TOO_LONG:
		return "line-too-long";
	case KN_COMPAT_PROFILE_ERR_PARSE:
		return "parse";
	case KN_COMPAT_PROFILE_ERR_DUPLICATE_KEY:
		return "duplicate-key";
	case KN_COMPAT_PROFILE_ERR_UNKNOWN_KEY:
		return "unknown-key";
	case KN_COMPAT_PROFILE_ERR_MISSING_REQUIRED:
		return "missing-required";
	case KN_COMPAT_PROFILE_ERR_INVALID_VALUE:
		return "invalid-value";
	case KN_COMPAT_PROFILE_ERR_TOO_MANY:
		return "too-many";
	case KN_COMPAT_PROFILE_ERR_BUFFER:
		return "buffer";
	}
	return "unknown";
}

const char *
kn_compat_profile_category_name(enum kn_compat_profile_category category)
{
	switch (category) {
	case KN_COMPAT_PROFILE_CATEGORY_INFORMATIONAL:
		return "informational";
	case KN_COMPAT_PROFILE_CATEGORY_SESSION:
		return "session";
	case KN_COMPAT_PROFILE_CATEGORY_BBS:
		return "bbs";
	case KN_COMPAT_PROFILE_CATEGORY_ROUTING:
		return "routing";
	case KN_COMPAT_PROFILE_CATEGORY_SYSOP:
		return "sysop";
	case KN_COMPAT_PROFILE_CATEGORY_UNKNOWN_HANDLING:
		return "unknown-handling";
	}
	return "unknown";
}

const char *
kn_compat_profile_transport_name(enum kn_compat_profile_transport transport)
{
	switch (transport) {
	case KN_COMPAT_PROFILE_TRANSPORT_LOCAL_SHELL:
		return "local-shell";
	case KN_COMPAT_PROFILE_TRANSPORT_RF_UI:
		return "rf-ui";
	case KN_COMPAT_PROFILE_TRANSPORT_CONNECTED_AX25:
		return "connected-ax25";
	case KN_COMPAT_PROFILE_TRANSPORT_NETROM:
		return "netrom";
	case KN_COMPAT_PROFILE_TRANSPORT_TELNET:
		return "telnet";
	}
	return "unknown";
}

const char *
kn_compat_profile_args_name(enum kn_compat_profile_args args)
{
	switch (args) {
	case KN_COMPAT_PROFILE_ARGS_NONE:
		return "none";
	case KN_COMPAT_PROFILE_ARGS_OPTIONAL:
		return "optional";
	case KN_COMPAT_PROFILE_ARGS_REQUIRED:
		return "required";
	case KN_COMPAT_PROFILE_ARGS_FREE_TEXT:
		return "free-text";
	}
	return "unknown";
}

const char *
kn_compat_profile_reply_name(enum kn_compat_profile_reply reply)
{
	switch (reply) {
	case KN_COMPAT_PROFILE_REPLY_NONE:
		return "none";
	case KN_COMPAT_PROFILE_REPLY_ONE_LINE:
		return "one-line";
	case KN_COMPAT_PROFILE_REPLY_ONE_OR_MORE_LINES:
		return "one-or-more-lines";
	case KN_COMPAT_PROFILE_REPLY_SESSION_TRANSITION:
		return "session-transition";
	}
	return "unknown";
}

const char *
kn_compat_profile_status_name(enum kn_compat_profile_status status)
{
	switch (status) {
	case KN_COMPAT_PROFILE_STATUS_NEEDS_OBSERVATION:
		return "needs-observation";
	case KN_COMPAT_PROFILE_STATUS_PLANNED:
		return "planned";
	case KN_COMPAT_PROFILE_STATUS_BLOCKED:
		return "blocked";
	case KN_COMPAT_PROFILE_STATUS_NATIVE_ONLY:
		return "native-only";
	case KN_COMPAT_PROFILE_STATUS_READY_FOR_DESIGN:
		return "ready-for-design";
	}
	return "unknown";
}

const struct kn_compat_command_profile *
kn_compat_command_profile_find(
	const struct kn_compat_command_profiles *profiles, const char *command)
{
	size_t i;

	if (profiles == NULL || command == NULL)
		return NULL;
	for (i = 0; i < profiles->profile_count; i++) {
		if (strcmp(profiles->profiles[i].command, command) == 0)
			return &profiles->profiles[i];
	}
	return NULL;
}

enum kn_compat_profile_error
kn_compat_command_profiles_parse_file(const char *path,
	struct kn_compat_command_profiles *profiles,
	struct kn_compat_profile_error_info *info)
{
	FILE *fp;
	char text[KN_COMPAT_PROFILE_TEXT_MAX + 1];
	size_t len;
	int ch;

	if (path == NULL || profiles == NULL)
		return KN_COMPAT_PROFILE_ERR_INVALID_ARGUMENT;
	fp = fopen(path, "r");
	if (fp == NULL) {
		error_set(info, KN_COMPAT_PROFILE_ERR_IO, 0, "open failed");
		return KN_COMPAT_PROFILE_ERR_IO;
	}
	len = 0;
	while ((ch = fgetc(fp)) != EOF) {
		if (len >= KN_COMPAT_PROFILE_TEXT_MAX) {
			(void)fclose(fp);
			error_set(info, KN_COMPAT_PROFILE_ERR_INVALID_VALUE, 0,
			    "profiles too large");
			return KN_COMPAT_PROFILE_ERR_INVALID_VALUE;
		}
		text[len++] = (char)ch;
	}
	if (ferror(fp)) {
		(void)fclose(fp);
		error_set(info, KN_COMPAT_PROFILE_ERR_IO, 0, "read failed");
		return KN_COMPAT_PROFILE_ERR_IO;
	}
	(void)fclose(fp);
	text[len] = '\0';
	return kn_compat_command_profiles_parse_text(text, profiles, info);
}

enum kn_compat_profile_error
kn_compat_command_profiles_parse_text(const char *text,
	struct kn_compat_command_profiles *profiles,
	struct kn_compat_profile_error_info *info)
{
	struct kn_compat_command_profile current;
	char line[KN_COMPAT_PROFILE_LINE_MAX];
	char *key;
	char *value;
	size_t i;
	size_t line_len;
	size_t line_no;
	uint32_t header_seen;
	uint32_t block_seen;
	uint8_t in_block;
	enum kn_compat_profile_error rc;

	if (text == NULL || profiles == NULL)
		return KN_COMPAT_PROFILE_ERR_INVALID_ARGUMENT;
	kn_compat_command_profiles_clear(profiles);
	if (info != NULL)
		memset(info, 0, sizeof(*info));
	memset(&current, 0, sizeof(current));
	header_seen = block_seen = 0;
	in_block = 0;
	line_no = 1;
	line_len = 0;
	for (i = 0;; i++) {
		if (text[i] != '\0' && text[i] != '\n') {
			if (line_len + 1 >= sizeof(line)) {
				error_set(info,
				    KN_COMPAT_PROFILE_ERR_LINE_TOO_LONG,
				    line_no, "line too long");
				return KN_COMPAT_PROFILE_ERR_LINE_TOO_LONG;
			}
			line[line_len++] = text[i];
			continue;
		}
		line[line_len] = '\0';
		key = trim(line);
		if (key[0] != '\0' && key[0] != '#') {
			if (strcmp(key, "}") == 0) {
				if (in_block == 0) {
					error_set(info, KN_COMPAT_PROFILE_ERR_PARSE,
					    line_no, "unexpected block end");
					return KN_COMPAT_PROFILE_ERR_PARSE;
				}
				rc = profile_finish(&current, block_seen,
				    line_no, info);
				if (rc != KN_COMPAT_PROFILE_OK)
					return rc;
				profiles->profiles[profiles->profile_count++] =
				    current;
				memset(&current, 0, sizeof(current));
				block_seen = 0;
				in_block = 0;
			} else if (in_block != 0) {
				value = key;
				while (*value != '\0' && *value != ' ' &&
				    *value != '\t')
					value++;
				if (*value == '\0')
					goto parse;
				*value++ = '\0';
				rc = profile_set(&current, &block_seen, key,
				    trim(value), line_no, info);
				if (rc != KN_COMPAT_PROFILE_OK)
					return rc;
			} else if (strncmp(key, "command ", 8) == 0) {
				value = trim(key + 8);
				if (profiles->profile_count >=
				    KN_COMPAT_PROFILE_MAX)
					return KN_COMPAT_PROFILE_ERR_TOO_MANY;
				if (value[0] == '\0' ||
				    value[strlen(value) - 1] != '{')
					goto parse;
				value[strlen(value) - 1] = '\0';
				value = trim(value);
				if (kn_compat_command_profile_find(profiles,
				    value) != NULL)
					return KN_COMPAT_PROFILE_ERR_DUPLICATE_KEY;
				if (strlen(value) >= sizeof(current.command))
					return KN_COMPAT_PROFILE_ERR_INVALID_VALUE;
				(void)snprintf(current.command,
				    sizeof(current.command), "%s", value);
				in_block = 1;
			} else {
				value = key;
				while (*value != '\0' && *value != ' ' &&
				    *value != '\t')
					value++;
				if (*value == '\0')
					goto parse;
				*value++ = '\0';
				rc = header_set(profiles, &header_seen, key,
				    trim(value), line_no, info);
				if (rc != KN_COMPAT_PROFILE_OK)
					return rc;
			}
		}
		if (text[i] == '\0')
			break;
		line_no++;
		line_len = 0;
	}
	if (in_block != 0)
		goto parse;
	if ((header_seen & (PROF_FIELD_NAME | PROF_FIELD_SUBJECT |
	    PROF_FIELD_CLEAN_ROOM | PROF_FIELD_SOURCE_CODE_USED)) !=
	    (PROF_FIELD_NAME | PROF_FIELD_SUBJECT | PROF_FIELD_CLEAN_ROOM |
	    PROF_FIELD_SOURCE_CODE_USED)) {
		error_set(info, KN_COMPAT_PROFILE_ERR_MISSING_REQUIRED, 0,
		    "missing required field");
		return KN_COMPAT_PROFILE_ERR_MISSING_REQUIRED;
	}
	if (profiles->profile_count == 0) {
		error_set(info, KN_COMPAT_PROFILE_ERR_MISSING_REQUIRED, 0,
		    "missing command profiles");
		return KN_COMPAT_PROFILE_ERR_MISSING_REQUIRED;
	}
	return KN_COMPAT_PROFILE_OK;

parse:
	error_set(info, KN_COMPAT_PROFILE_ERR_PARSE, line_no, "parse");
	return KN_COMPAT_PROFILE_ERR_PARSE;
}

enum kn_compat_profile_error
kn_compat_command_profiles_report(
	const struct kn_compat_command_profiles *profiles, char *buf,
	size_t bufsiz)
{
	size_t category_counts[6];
	size_t blocked;
	size_t i;
	size_t off;
	int needed;

	if (profiles == NULL || buf == NULL || bufsiz == 0)
		return KN_COMPAT_PROFILE_ERR_INVALID_ARGUMENT;
	memset(category_counts, 0, sizeof(category_counts));
	blocked = 0;
	for (i = 0; i < profiles->profile_count; i++) {
		category_counts[profiles->profiles[i].category]++;
		if (profiles->profiles[i].compat_status ==
		    KN_COMPAT_PROFILE_STATUS_BLOCKED)
			blocked++;
	}
	needed = snprintf(buf, bufsiz,
	    "COMMAND-PROFILES name=%s subject=%s count=%llu blocked=%llu "
	    "informational=%llu routing=%llu bbs=%llu\n",
	    profiles->name, profiles->subject,
	    (unsigned long long)profiles->profile_count,
	    (unsigned long long)blocked,
	    (unsigned long long)category_counts[
	    KN_COMPAT_PROFILE_CATEGORY_INFORMATIONAL],
	    (unsigned long long)category_counts[
	    KN_COMPAT_PROFILE_CATEGORY_ROUTING],
	    (unsigned long long)category_counts[
	    KN_COMPAT_PROFILE_CATEGORY_BBS]);
	if (needed < 0 || (size_t)needed >= bufsiz)
		return KN_COMPAT_PROFILE_ERR_BUFFER;
	off = (size_t)needed;
	for (i = 0; i < profiles->profile_count; i++) {
		needed = snprintf(buf + off, bufsiz - off,
		    "COMMAND-PROFILE command=%s category=%s transport=%s "
		    "args=%s reply=%s stateful=%s connected=%s status=%s\n",
		    profiles->profiles[i].command,
		    kn_compat_profile_category_name(
		    profiles->profiles[i].category),
		    kn_compat_profile_transport_name(
		    profiles->profiles[i].transport),
		    kn_compat_profile_args_name(profiles->profiles[i].args),
		    kn_compat_profile_reply_name(profiles->profiles[i].reply),
		    profiles->profiles[i].stateful != 0 ? "true" : "false",
		    profiles->profiles[i].requires_connected_mode != 0 ?
		    "true" : "false",
		    kn_compat_profile_status_name(
		    profiles->profiles[i].compat_status));
		if (needed < 0 || (size_t)needed >= bufsiz - off)
			return KN_COMPAT_PROFILE_ERR_BUFFER;
		off += (size_t)needed;
	}
	return KN_COMPAT_PROFILE_OK;
}

enum kn_compat_profile_error
kn_compat_command_profiles_generate_from_pack(
	const struct kn_compat_observation_pack *pack, char *buf, size_t bufsiz)
{
	size_t off;
	enum kn_compat_profile_error rc;

	if (pack == NULL || buf == NULL || bufsiz == 0)
		return KN_COMPAT_PROFILE_ERR_INVALID_ARGUMENT;
	(void)snprintf(buf, bufsiz,
	    "# KiloNode command profiles v1\n"
	    "name %s-command-profiles\n"
	    "subject %s\n"
	    "clean-room true\n"
	    "source-code-used false\n\n",
	    pack->name, pack->subject);
	off = strlen(buf);
#define ADD_PROF(c, cat, tr, a, r, s, cm, st) do { \
	rc = append_profile(buf, bufsiz, &off, (c), (cat), (tr), (a), (r), \
	    (s), (cm), (st)); \
	if (rc != KN_COMPAT_PROFILE_OK) return rc; \
} while (0)
	ADD_PROF("HELP", KN_COMPAT_PROFILE_CATEGORY_INFORMATIONAL,
	    KN_COMPAT_PROFILE_TRANSPORT_TELNET, KN_COMPAT_PROFILE_ARGS_NONE,
	    KN_COMPAT_PROFILE_REPLY_ONE_OR_MORE_LINES, 0, 0,
	    KN_COMPAT_PROFILE_STATUS_NEEDS_OBSERVATION);
	ADD_PROF("INFO", KN_COMPAT_PROFILE_CATEGORY_INFORMATIONAL,
	    KN_COMPAT_PROFILE_TRANSPORT_TELNET, KN_COMPAT_PROFILE_ARGS_NONE,
	    KN_COMPAT_PROFILE_REPLY_ONE_OR_MORE_LINES, 0, 0,
	    KN_COMPAT_PROFILE_STATUS_NEEDS_OBSERVATION);
	ADD_PROF("PORTS", KN_COMPAT_PROFILE_CATEGORY_INFORMATIONAL,
	    KN_COMPAT_PROFILE_TRANSPORT_TELNET, KN_COMPAT_PROFILE_ARGS_NONE,
	    KN_COMPAT_PROFILE_REPLY_ONE_OR_MORE_LINES, 0, 0,
	    KN_COMPAT_PROFILE_STATUS_NEEDS_OBSERVATION);
	ADD_PROF("USERS", KN_COMPAT_PROFILE_CATEGORY_INFORMATIONAL,
	    KN_COMPAT_PROFILE_TRANSPORT_TELNET, KN_COMPAT_PROFILE_ARGS_NONE,
	    KN_COMPAT_PROFILE_REPLY_ONE_OR_MORE_LINES, 0, 0,
	    KN_COMPAT_PROFILE_STATUS_NEEDS_OBSERVATION);
	ADD_PROF("UNKNOWN", KN_COMPAT_PROFILE_CATEGORY_UNKNOWN_HANDLING,
	    KN_COMPAT_PROFILE_TRANSPORT_TELNET, KN_COMPAT_PROFILE_ARGS_FREE_TEXT,
	    KN_COMPAT_PROFILE_REPLY_ONE_LINE, 0, 0,
	    KN_COMPAT_PROFILE_STATUS_NEEDS_OBSERVATION);
	ADD_PROF("BBS", KN_COMPAT_PROFILE_CATEGORY_BBS,
	    KN_COMPAT_PROFILE_TRANSPORT_CONNECTED_AX25,
	    KN_COMPAT_PROFILE_ARGS_OPTIONAL,
	    KN_COMPAT_PROFILE_REPLY_SESSION_TRANSITION, 1, 1,
	    KN_COMPAT_PROFILE_STATUS_BLOCKED);
	ADD_PROF("CONNECT", KN_COMPAT_PROFILE_CATEGORY_SESSION,
	    KN_COMPAT_PROFILE_TRANSPORT_CONNECTED_AX25,
	    KN_COMPAT_PROFILE_ARGS_REQUIRED,
	    KN_COMPAT_PROFILE_REPLY_SESSION_TRANSITION, 1, 1,
	    KN_COMPAT_PROFILE_STATUS_BLOCKED);
	ADD_PROF("NODES", KN_COMPAT_PROFILE_CATEGORY_ROUTING,
	    KN_COMPAT_PROFILE_TRANSPORT_NETROM, KN_COMPAT_PROFILE_ARGS_NONE,
	    KN_COMPAT_PROFILE_REPLY_ONE_OR_MORE_LINES, 1, 1,
	    KN_COMPAT_PROFILE_STATUS_BLOCKED);
	ADD_PROF("ROUTES", KN_COMPAT_PROFILE_CATEGORY_ROUTING,
	    KN_COMPAT_PROFILE_TRANSPORT_NETROM, KN_COMPAT_PROFILE_ARGS_NONE,
	    KN_COMPAT_PROFILE_REPLY_ONE_OR_MORE_LINES, 1, 1,
	    KN_COMPAT_PROFILE_STATUS_BLOCKED);
#undef ADD_PROF
	return KN_COMPAT_PROFILE_OK;
}

static enum kn_compat_profile_error
append_profile(char *buf, size_t bufsiz, size_t *off, const char *command,
	enum kn_compat_profile_category category,
	enum kn_compat_profile_transport transport,
	enum kn_compat_profile_args args, enum kn_compat_profile_reply reply,
	uint8_t stateful, uint8_t connected,
	enum kn_compat_profile_status status)
{
	int needed;

	if (*off >= bufsiz)
		return KN_COMPAT_PROFILE_ERR_BUFFER;
	needed = snprintf(buf + *off, bufsiz - *off,
	    "command %s {\n"
	    "\tcategory %s\n"
	    "\ttransport %s\n"
	    "\targs %s\n"
	    "\treply %s\n"
	    "\tstateful %s\n"
	    "\trequires-connected-mode %s\n"
	    "\tcompat-status %s\n"
	    "}\n\n",
	    command, kn_compat_profile_category_name(category),
	    kn_compat_profile_transport_name(transport),
	    kn_compat_profile_args_name(args),
	    kn_compat_profile_reply_name(reply),
	    stateful != 0 ? "true" : "false",
	    connected != 0 ? "true" : "false",
	    kn_compat_profile_status_name(status));
	if (needed < 0 || (size_t)needed >= bufsiz - *off)
		return KN_COMPAT_PROFILE_ERR_BUFFER;
	*off += (size_t)needed;
	return KN_COMPAT_PROFILE_OK;
}

static enum kn_compat_profile_error
args_parse(const char *value, enum kn_compat_profile_args *args)
{
	if (strcmp(value, "none") == 0)
		*args = KN_COMPAT_PROFILE_ARGS_NONE;
	else if (strcmp(value, "optional") == 0)
		*args = KN_COMPAT_PROFILE_ARGS_OPTIONAL;
	else if (strcmp(value, "required") == 0)
		*args = KN_COMPAT_PROFILE_ARGS_REQUIRED;
	else if (strcmp(value, "free-text") == 0)
		*args = KN_COMPAT_PROFILE_ARGS_FREE_TEXT;
	else
		return KN_COMPAT_PROFILE_ERR_INVALID_VALUE;
	return KN_COMPAT_PROFILE_OK;
}

static enum kn_compat_profile_error
bool_parse(const char *value, uint8_t *out)
{
	if (strcmp(value, "true") == 0)
		*out = 1;
	else if (strcmp(value, "false") == 0)
		*out = 0;
	else
		return KN_COMPAT_PROFILE_ERR_INVALID_VALUE;
	return KN_COMPAT_PROFILE_OK;
}

static enum kn_compat_profile_error
category_parse(const char *value, enum kn_compat_profile_category *category)
{
	if (strcmp(value, "informational") == 0)
		*category = KN_COMPAT_PROFILE_CATEGORY_INFORMATIONAL;
	else if (strcmp(value, "session") == 0)
		*category = KN_COMPAT_PROFILE_CATEGORY_SESSION;
	else if (strcmp(value, "bbs") == 0)
		*category = KN_COMPAT_PROFILE_CATEGORY_BBS;
	else if (strcmp(value, "routing") == 0)
		*category = KN_COMPAT_PROFILE_CATEGORY_ROUTING;
	else if (strcmp(value, "sysop") == 0)
		*category = KN_COMPAT_PROFILE_CATEGORY_SYSOP;
	else if (strcmp(value, "unknown-handling") == 0)
		*category = KN_COMPAT_PROFILE_CATEGORY_UNKNOWN_HANDLING;
	else
		return KN_COMPAT_PROFILE_ERR_INVALID_VALUE;
	return KN_COMPAT_PROFILE_OK;
}

static void
error_set(struct kn_compat_profile_error_info *info,
	enum kn_compat_profile_error error, size_t line, const char *message)
{
	if (info == NULL)
		return;
	info->error = error;
	info->line = line;
	(void)snprintf(info->message, sizeof(info->message), "%s",
	    message == NULL ? "" : message);
}

static enum kn_compat_profile_error
header_set(struct kn_compat_command_profiles *profiles, uint32_t *seen,
	const char *key, const char *value, size_t line,
	struct kn_compat_profile_error_info *info)
{
	char *dst;
	size_t dst_len;
	uint32_t flag;
	uint8_t b;

	dst = NULL;
	dst_len = 0;
	flag = 0;
	if (strcmp(key, "name") == 0) {
		dst = profiles->name;
		dst_len = sizeof(profiles->name);
		flag = PROF_FIELD_NAME;
	} else if (strcmp(key, "subject") == 0) {
		dst = profiles->subject;
		dst_len = sizeof(profiles->subject);
		flag = PROF_FIELD_SUBJECT;
	} else if (strcmp(key, "clean-room") == 0) {
		if (field_seen(seen, PROF_FIELD_CLEAN_ROOM) != 0)
			goto duplicate;
		if (bool_parse(value, &b) != KN_COMPAT_PROFILE_OK || b == 0)
			goto invalid;
		profiles->clean_room = b;
		return KN_COMPAT_PROFILE_OK;
	} else if (strcmp(key, "source-code-used") == 0) {
		if (field_seen(seen, PROF_FIELD_SOURCE_CODE_USED) != 0)
			goto duplicate;
		if (bool_parse(value, &b) != KN_COMPAT_PROFILE_OK || b != 0)
			goto invalid;
		profiles->source_code_used = b;
		return KN_COMPAT_PROFILE_OK;
	} else {
		error_set(info, KN_COMPAT_PROFILE_ERR_UNKNOWN_KEY, line, key);
		return KN_COMPAT_PROFILE_ERR_UNKNOWN_KEY;
	}
	if (field_seen(seen, flag) != 0)
		goto duplicate;
	if (strlen(value) >= dst_len)
		goto invalid;
	(void)snprintf(dst, dst_len, "%s", value);
	return KN_COMPAT_PROFILE_OK;

duplicate:
	error_set(info, KN_COMPAT_PROFILE_ERR_DUPLICATE_KEY, line, key);
	return KN_COMPAT_PROFILE_ERR_DUPLICATE_KEY;
invalid:
	error_set(info, KN_COMPAT_PROFILE_ERR_INVALID_VALUE, line, key);
	return KN_COMPAT_PROFILE_ERR_INVALID_VALUE;
}

static enum kn_compat_profile_error
profile_finish(struct kn_compat_command_profile *profile, uint32_t seen,
	size_t line, struct kn_compat_profile_error_info *info)
{
	const uint32_t required = PROF_BLOCK_CATEGORY | PROF_BLOCK_TRANSPORT |
	    PROF_BLOCK_ARGS | PROF_BLOCK_REPLY | PROF_BLOCK_STATEFUL |
	    PROF_BLOCK_CONNECTED | PROF_BLOCK_STATUS;

	if ((seen & required) != required) {
		error_set(info, KN_COMPAT_PROFILE_ERR_MISSING_REQUIRED, line,
		    profile->command);
		return KN_COMPAT_PROFILE_ERR_MISSING_REQUIRED;
	}
	return KN_COMPAT_PROFILE_OK;
}

static enum kn_compat_profile_error
profile_set(struct kn_compat_command_profile *profile, uint32_t *seen,
	const char *key, const char *value, size_t line,
	struct kn_compat_profile_error_info *info)
{
	uint8_t b;

	if (strcmp(key, "category") == 0) {
		if (field_seen(seen, PROF_BLOCK_CATEGORY) != 0)
			goto duplicate;
		if (category_parse(value, &profile->category) !=
		    KN_COMPAT_PROFILE_OK)
			goto invalid;
		return KN_COMPAT_PROFILE_OK;
	}
	if (strcmp(key, "transport") == 0) {
		if (field_seen(seen, PROF_BLOCK_TRANSPORT) != 0)
			goto duplicate;
		if (transport_parse(value, &profile->transport) !=
		    KN_COMPAT_PROFILE_OK)
			goto invalid;
		return KN_COMPAT_PROFILE_OK;
	}
	if (strcmp(key, "args") == 0) {
		if (field_seen(seen, PROF_BLOCK_ARGS) != 0)
			goto duplicate;
		if (args_parse(value, &profile->args) !=
		    KN_COMPAT_PROFILE_OK)
			goto invalid;
		return KN_COMPAT_PROFILE_OK;
	}
	if (strcmp(key, "reply") == 0) {
		if (field_seen(seen, PROF_BLOCK_REPLY) != 0)
			goto duplicate;
		if (reply_parse(value, &profile->reply) !=
		    KN_COMPAT_PROFILE_OK)
			goto invalid;
		return KN_COMPAT_PROFILE_OK;
	}
	if (strcmp(key, "stateful") == 0) {
		if (field_seen(seen, PROF_BLOCK_STATEFUL) != 0)
			goto duplicate;
		if (bool_parse(value, &b) != KN_COMPAT_PROFILE_OK)
			goto invalid;
		profile->stateful = b;
		return KN_COMPAT_PROFILE_OK;
	}
	if (strcmp(key, "requires-connected-mode") == 0) {
		if (field_seen(seen, PROF_BLOCK_CONNECTED) != 0)
			goto duplicate;
		if (bool_parse(value, &b) != KN_COMPAT_PROFILE_OK)
			goto invalid;
		profile->requires_connected_mode = b;
		return KN_COMPAT_PROFILE_OK;
	}
	if (strcmp(key, "compat-status") == 0) {
		if (field_seen(seen, PROF_BLOCK_STATUS) != 0)
			goto duplicate;
		if (status_parse(value, &profile->compat_status) !=
		    KN_COMPAT_PROFILE_OK)
			goto invalid;
		return KN_COMPAT_PROFILE_OK;
	}
	error_set(info, KN_COMPAT_PROFILE_ERR_UNKNOWN_KEY, line, key);
	return KN_COMPAT_PROFILE_ERR_UNKNOWN_KEY;

duplicate:
	error_set(info, KN_COMPAT_PROFILE_ERR_DUPLICATE_KEY, line, key);
	return KN_COMPAT_PROFILE_ERR_DUPLICATE_KEY;
invalid:
	error_set(info, KN_COMPAT_PROFILE_ERR_INVALID_VALUE, line, key);
	return KN_COMPAT_PROFILE_ERR_INVALID_VALUE;
}

static enum kn_compat_profile_error
reply_parse(const char *value, enum kn_compat_profile_reply *reply)
{
	if (strcmp(value, "none") == 0)
		*reply = KN_COMPAT_PROFILE_REPLY_NONE;
	else if (strcmp(value, "one-line") == 0)
		*reply = KN_COMPAT_PROFILE_REPLY_ONE_LINE;
	else if (strcmp(value, "one-or-more-lines") == 0)
		*reply = KN_COMPAT_PROFILE_REPLY_ONE_OR_MORE_LINES;
	else if (strcmp(value, "session-transition") == 0)
		*reply = KN_COMPAT_PROFILE_REPLY_SESSION_TRANSITION;
	else
		return KN_COMPAT_PROFILE_ERR_INVALID_VALUE;
	return KN_COMPAT_PROFILE_OK;
}

static enum kn_compat_profile_error
status_parse(const char *value, enum kn_compat_profile_status *status)
{
	if (strcmp(value, "needs-observation") == 0)
		*status = KN_COMPAT_PROFILE_STATUS_NEEDS_OBSERVATION;
	else if (strcmp(value, "planned") == 0)
		*status = KN_COMPAT_PROFILE_STATUS_PLANNED;
	else if (strcmp(value, "blocked") == 0)
		*status = KN_COMPAT_PROFILE_STATUS_BLOCKED;
	else if (strcmp(value, "native-only") == 0)
		*status = KN_COMPAT_PROFILE_STATUS_NATIVE_ONLY;
	else if (strcmp(value, "ready-for-design") == 0)
		*status = KN_COMPAT_PROFILE_STATUS_READY_FOR_DESIGN;
	else
		return KN_COMPAT_PROFILE_ERR_INVALID_VALUE;
	return KN_COMPAT_PROFILE_OK;
}

static enum kn_compat_profile_error
transport_parse(const char *value, enum kn_compat_profile_transport *transport)
{
	if (strcmp(value, "local-shell") == 0)
		*transport = KN_COMPAT_PROFILE_TRANSPORT_LOCAL_SHELL;
	else if (strcmp(value, "rf-ui") == 0)
		*transport = KN_COMPAT_PROFILE_TRANSPORT_RF_UI;
	else if (strcmp(value, "connected-ax25") == 0)
		*transport = KN_COMPAT_PROFILE_TRANSPORT_CONNECTED_AX25;
	else if (strcmp(value, "netrom") == 0)
		*transport = KN_COMPAT_PROFILE_TRANSPORT_NETROM;
	else if (strcmp(value, "telnet") == 0)
		*transport = KN_COMPAT_PROFILE_TRANSPORT_TELNET;
	else
		return KN_COMPAT_PROFILE_ERR_INVALID_VALUE;
	return KN_COMPAT_PROFILE_OK;
}

static uint8_t
field_seen(uint32_t *seen, uint32_t flag)
{
	if ((*seen & flag) != 0)
		return 1;
	*seen |= flag;
	return 0;
}

static char *
trim(char *s)
{
	char *end;

	while (*s == ' ' || *s == '\t' || *s == '\r')
		s++;
	end = s + strlen(s);
	while (end > s &&
	    (end[-1] == ' ' || end[-1] == '\t' || end[-1] == '\r'))
		*--end = '\0';
	return s;
}
