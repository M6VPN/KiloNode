/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/compat/compat_transcript.c */

#include <sys/types.h>

#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kilonode/compat_transcript.h"

#define FIELD_NAME                 0x0001u
#define FIELD_MODE                 0x0002u
#define FIELD_NODE                 0x0004u
#define FIELD_PORT                 0x0008u
#define FIELD_SOURCE               0x0010u
#define FIELD_DESTINATION          0x0020u
#define FIELD_PID                  0x0040u
#define FIELD_INPUT                0x0080u
#define FIELD_EXPECT_EVENT         0x0100u
#define FIELD_EXPECT_REPLY         0x0200u
#define FIELD_EXPECT_REPLY_QUEUED  0x0400u
#define FIELD_EXPECT_NO_DISPATCH   0x0800u
#define FIELD_EXPECT_ERROR         0x1000u

static enum kn_compat_transcript_error bool_parse(const char *, uint8_t *);
static enum kn_compat_transcript_error command_parse(const char *,
	enum kn_rf_command_name *);
static void error_set(struct kn_compat_transcript_error_info *,
	enum kn_compat_transcript_error, size_t, const char *);
static enum kn_compat_transcript_error expect_event_parse(
	struct kn_compat_transcript *, const char *);
static enum kn_compat_transcript_error expect_reply_parse(
	struct kn_compat_transcript *, const char *);
static enum kn_compat_transcript_error field_set(struct kn_compat_transcript *,
	uint32_t *, const char *, const char *, size_t,
	struct kn_compat_transcript_error_info *);
static uint8_t field_seen(uint32_t *, uint32_t);
static enum kn_compat_transcript_error mode_parse(const char *,
	enum kn_compat_mode *);
static enum kn_compat_transcript_error pid_parse(const char *, uint8_t *);
static enum kn_compat_transcript_error require_fields(
	const struct kn_compat_transcript *, uint32_t,
	struct kn_compat_transcript_error_info *);
static enum kn_compat_transcript_error status_parse(const char *,
	enum kn_rf_command_status *);
static char *trim(char *);

void
kn_compat_transcript_clear(struct kn_compat_transcript *transcript)
{
	if (transcript == NULL)
		return;

	memset(transcript, 0, sizeof(*transcript));
	transcript->mode = KN_COMPAT_MODE_NONE;
	transcript->expect_command = KN_RF_COMMAND_UNKNOWN;
	transcript->expect_status = KN_RF_COMMAND_STATUS_IGNORED;
	transcript->expect_reply = KN_COMPAT_REPLY_UNSET;
	transcript->expect_no_dispatch = 1;
}

const char *
kn_compat_transcript_error_name(enum kn_compat_transcript_error error)
{
	switch (error) {
	case KN_COMPAT_TRANSCRIPT_OK:
		return "ok";
	case KN_COMPAT_TRANSCRIPT_ERR_INVALID_ARGUMENT:
		return "invalid-argument";
	case KN_COMPAT_TRANSCRIPT_ERR_IO:
		return "io";
	case KN_COMPAT_TRANSCRIPT_ERR_LINE_TOO_LONG:
		return "line-too-long";
	case KN_COMPAT_TRANSCRIPT_ERR_PARSE:
		return "parse";
	case KN_COMPAT_TRANSCRIPT_ERR_DUPLICATE_KEY:
		return "duplicate-key";
	case KN_COMPAT_TRANSCRIPT_ERR_UNKNOWN_KEY:
		return "unknown-key";
	case KN_COMPAT_TRANSCRIPT_ERR_MISSING_REQUIRED:
		return "missing-required";
	case KN_COMPAT_TRANSCRIPT_ERR_INVALID_VALUE:
		return "invalid-value";
	}

	return "unknown";
}

const char *
kn_compat_mode_name(enum kn_compat_mode mode)
{
	switch (mode) {
	case KN_COMPAT_MODE_RF_UI:
		return "rf-ui";
	case KN_COMPAT_MODE_NONE:
		return "none";
	}

	return "unknown";
}

enum kn_compat_transcript_error
kn_compat_transcript_parse_file(const char *path,
	struct kn_compat_transcript *transcript,
	struct kn_compat_transcript_error_info *info)
{
	FILE *fp;
	char text[KN_COMPAT_TRANSCRIPT_MAX + 1];
	size_t len;
	int ch;

	if (path == NULL || transcript == NULL)
		return KN_COMPAT_TRANSCRIPT_ERR_INVALID_ARGUMENT;

	fp = fopen(path, "r");
	if (fp == NULL) {
		error_set(info, KN_COMPAT_TRANSCRIPT_ERR_IO, 0, "open failed");
		return KN_COMPAT_TRANSCRIPT_ERR_IO;
	}

	len = 0;
	while ((ch = fgetc(fp)) != EOF) {
		if (len >= KN_COMPAT_TRANSCRIPT_MAX) {
			(void)fclose(fp);
			error_set(info, KN_COMPAT_TRANSCRIPT_ERR_INVALID_VALUE, 0,
			    "transcript too large");
			return KN_COMPAT_TRANSCRIPT_ERR_INVALID_VALUE;
		}
		text[len++] = (char)ch;
	}
	if (ferror(fp)) {
		(void)fclose(fp);
		error_set(info, KN_COMPAT_TRANSCRIPT_ERR_IO, 0, "read failed");
		return KN_COMPAT_TRANSCRIPT_ERR_IO;
	}
	(void)fclose(fp);
	text[len] = '\0';

	return kn_compat_transcript_parse_text(text, transcript, info);
}

enum kn_compat_transcript_error
kn_compat_transcript_parse_text(const char *text,
	struct kn_compat_transcript *transcript,
	struct kn_compat_transcript_error_info *info)
{
	char line[KN_COMPAT_LINE_MAX];
	char *key;
	char *value;
	size_t i;
	size_t line_len;
	size_t line_no;
	uint32_t seen;
	enum kn_compat_transcript_error rc;

	if (text == NULL || transcript == NULL)
		return KN_COMPAT_TRANSCRIPT_ERR_INVALID_ARGUMENT;

	kn_compat_transcript_clear(transcript);
	if (info != NULL)
		memset(info, 0, sizeof(*info));
	seen = 0;
	line_no = 1;
	line_len = 0;

	for (i = 0;; i++) {
		if (text[i] != '\0' && text[i] != '\n') {
			if (line_len + 1 >= sizeof(line)) {
				error_set(info,
				    KN_COMPAT_TRANSCRIPT_ERR_LINE_TOO_LONG,
				    line_no, "line too long");
				return KN_COMPAT_TRANSCRIPT_ERR_LINE_TOO_LONG;
			}
			line[line_len++] = text[i];
			continue;
		}

		line[line_len] = '\0';
		key = trim(line);
		if (key[0] != '\0' && key[0] != '#') {
			value = key;
			while (*value != '\0' && *value != ' ' &&
			    *value != '\t')
				value++;
			if (*value == '\0') {
				error_set(info, KN_COMPAT_TRANSCRIPT_ERR_PARSE,
				    line_no, "missing value");
				return KN_COMPAT_TRANSCRIPT_ERR_PARSE;
			}
			*value++ = '\0';
			value = trim(value);
			if (value[0] == '\0') {
				error_set(info, KN_COMPAT_TRANSCRIPT_ERR_PARSE,
				    line_no, "missing value");
				return KN_COMPAT_TRANSCRIPT_ERR_PARSE;
			}
			rc = field_set(transcript, &seen, key, value, line_no,
			    info);
			if (rc != KN_COMPAT_TRANSCRIPT_OK)
				return rc;
		}

		if (text[i] == '\0')
			break;
		line_no++;
		line_len = 0;
	}

	return require_fields(transcript, seen, info);
}

enum kn_compat_transcript_error
kn_compat_transcript_report(const struct kn_compat_transcript *transcript,
	char *buf, size_t bufsiz)
{
	char node[KN_CALLSIGN_MAX + 4];
	char source[KN_CALLSIGN_MAX + 4];
	char destination[KN_CALLSIGN_MAX + 4];
	int needed;

	if (transcript == NULL || buf == NULL || bufsiz == 0)
		return KN_COMPAT_TRANSCRIPT_ERR_INVALID_ARGUMENT;
	if (kn_callsign_format(&transcript->node, node, sizeof(node)) != 0 ||
	    kn_callsign_format(&transcript->source, source,
	    sizeof(source)) != 0 ||
	    kn_callsign_format(&transcript->destination, destination,
	    sizeof(destination)) != 0)
		return KN_COMPAT_TRANSCRIPT_ERR_INVALID_VALUE;

	needed = snprintf(buf, bufsiz,
	    "TRANSCRIPT name=%s mode=%s node=%s port=%s source=%s "
	    "destination=%s pid=0x%02x input_len=%llu "
	    "expect_command=%s expect_status=%s expect_reply=%u "
	    "expect_reply_text=\"%s\" expect_reply_queued=%s "
	    "expect_no_dispatch=%s",
	    transcript->name, kn_compat_mode_name(transcript->mode), node,
	    transcript->port_name, source, destination,
	    (unsigned int)transcript->pid,
	    (unsigned long long)transcript->input_len,
	    kn_rf_command_name_string(transcript->expect_command),
	    kn_rf_command_status_string(transcript->expect_status),
	    (unsigned int)transcript->expect_reply,
	    transcript->expect_reply_text,
	    transcript->has_expect_reply_queued != 0 ?
	    (transcript->expect_reply_queued != 0 ? "true" : "false") :
	    "unset",
	    transcript->expect_no_dispatch != 0 ? "true" : "false");

	return needed >= 0 && (size_t)needed < bufsiz ?
	    KN_COMPAT_TRANSCRIPT_OK : KN_COMPAT_TRANSCRIPT_ERR_INVALID_VALUE;
}

static enum kn_compat_transcript_error
bool_parse(const char *value, uint8_t *out)
{
	if (strcmp(value, "true") == 0) {
		*out = 1;
		return KN_COMPAT_TRANSCRIPT_OK;
	}
	if (strcmp(value, "false") == 0) {
		*out = 0;
		return KN_COMPAT_TRANSCRIPT_OK;
	}

	return KN_COMPAT_TRANSCRIPT_ERR_INVALID_VALUE;
}

static enum kn_compat_transcript_error
command_parse(const char *value, enum kn_rf_command_name *command)
{
	if (strcmp(value, "HELP") == 0)
		*command = KN_RF_COMMAND_HELP;
	else if (strcmp(value, "INFO") == 0)
		*command = KN_RF_COMMAND_INFO;
	else if (strcmp(value, "PORTS") == 0)
		*command = KN_RF_COMMAND_PORTS;
	else if (strcmp(value, "HEARD") == 0)
		*command = KN_RF_COMMAND_HEARD;
	else if (strcmp(value, "STATS") == 0)
		*command = KN_RF_COMMAND_STATS;
	else if (strcmp(value, "PING") == 0)
		*command = KN_RF_COMMAND_PING;
	else if (strcmp(value, "UNKNOWN") == 0)
		*command = KN_RF_COMMAND_UNKNOWN;
	else
		return KN_COMPAT_TRANSCRIPT_ERR_INVALID_VALUE;

	return KN_COMPAT_TRANSCRIPT_OK;
}

static void
error_set(struct kn_compat_transcript_error_info *info,
	enum kn_compat_transcript_error error, size_t line, const char *message)
{
	int needed;

	if (info == NULL)
		return;
	info->error = error;
	info->line = line;
	needed = snprintf(info->message, sizeof(info->message), "%s",
	    message == NULL ? "" : message);
	if (needed < 0 || (size_t)needed >= sizeof(info->message))
		info->message[0] = '\0';
}

static enum kn_compat_transcript_error
expect_event_parse(struct kn_compat_transcript *transcript, const char *value)
{
	char copy[KN_COMPAT_EXPECT_MAX];
	char *token;
	char *next;
	char *eq;
	enum kn_compat_transcript_error rc;
	uint8_t saw_command;
	uint8_t saw_status;

	if (strlen(value) >= sizeof(copy))
		return KN_COMPAT_TRANSCRIPT_ERR_INVALID_VALUE;
	memcpy(copy, value, strlen(value) + 1);
	saw_command = 0;
	saw_status = 0;
	token = copy;
	while (token != NULL && *token != '\0') {
		next = strchr(token, ' ');
		if (next != NULL)
			*next++ = '\0';
		eq = strchr(token, '=');
		if (eq == NULL)
			return KN_COMPAT_TRANSCRIPT_ERR_INVALID_VALUE;
		*eq++ = '\0';
		if (strcmp(token, "command") == 0) {
			rc = command_parse(eq, &transcript->expect_command);
			if (rc != KN_COMPAT_TRANSCRIPT_OK)
				return rc;
			saw_command = 1;
		} else if (strcmp(token, "status") == 0) {
			rc = status_parse(eq, &transcript->expect_status);
			if (rc != KN_COMPAT_TRANSCRIPT_OK)
				return rc;
			saw_status = 1;
		} else
			return KN_COMPAT_TRANSCRIPT_ERR_INVALID_VALUE;
		token = next;
	}

	return saw_command != 0 && saw_status != 0 ?
	    KN_COMPAT_TRANSCRIPT_OK : KN_COMPAT_TRANSCRIPT_ERR_INVALID_VALUE;
}

static enum kn_compat_transcript_error
expect_reply_parse(struct kn_compat_transcript *transcript, const char *value)
{
	const char *text;

	if (strcmp(value, "none") == 0) {
		transcript->expect_reply = KN_COMPAT_REPLY_NONE;
		transcript->expect_reply_text[0] = '\0';
		return KN_COMPAT_TRANSCRIPT_OK;
	}
	if (strncmp(value, "contains=", 9) == 0) {
		transcript->expect_reply = KN_COMPAT_REPLY_CONTAINS;
		text = value + 9;
	} else if (strncmp(value, "exact=", 6) == 0) {
		transcript->expect_reply = KN_COMPAT_REPLY_EXACT;
		text = value + 6;
	} else
		return KN_COMPAT_TRANSCRIPT_ERR_INVALID_VALUE;
	if (text[0] == '\0' || strlen(text) >= sizeof(transcript->expect_reply_text))
		return KN_COMPAT_TRANSCRIPT_ERR_INVALID_VALUE;
	memcpy(transcript->expect_reply_text, text, strlen(text) + 1);
	return KN_COMPAT_TRANSCRIPT_OK;
}

static enum kn_compat_transcript_error
field_set(struct kn_compat_transcript *transcript, uint32_t *seen,
	const char *key, const char *value, size_t line,
	struct kn_compat_transcript_error_info *info)
{
	enum kn_compat_transcript_error rc;
	uint32_t field;
	uint8_t bool_value;

	field = 0;
	if (strcmp(key, "name") == 0) {
		field = FIELD_NAME;
		if (field_seen(seen, field) != 0)
			goto duplicate;
		if (strlen(value) == 0 || strlen(value) >= sizeof(transcript->name))
			goto invalid;
		memcpy(transcript->name, value, strlen(value) + 1);
		return KN_COMPAT_TRANSCRIPT_OK;
	}
	if (strcmp(key, "mode") == 0) {
		field = FIELD_MODE;
		if (field_seen(seen, field) != 0)
			goto duplicate;
		if (mode_parse(value, &transcript->mode) !=
		    KN_COMPAT_TRANSCRIPT_OK)
			goto invalid;
		return KN_COMPAT_TRANSCRIPT_OK;
	}
	if (strcmp(key, "node") == 0) {
		field = FIELD_NODE;
		if (field_seen(seen, field) != 0)
			goto duplicate;
		if (kn_callsign_parse(value, &transcript->node) != 0)
			goto invalid;
		return KN_COMPAT_TRANSCRIPT_OK;
	}
	if (strcmp(key, "port") == 0) {
		field = FIELD_PORT;
		if (field_seen(seen, field) != 0)
			goto duplicate;
		if (strlen(value) == 0 || strlen(value) >=
		    sizeof(transcript->port_name) || strchr(value, ' ') != NULL)
			goto invalid;
		memcpy(transcript->port_name, value, strlen(value) + 1);
		return KN_COMPAT_TRANSCRIPT_OK;
	}
	if (strcmp(key, "source") == 0) {
		field = FIELD_SOURCE;
		if (field_seen(seen, field) != 0)
			goto duplicate;
		if (kn_callsign_parse(value, &transcript->source) != 0)
			goto invalid;
		return KN_COMPAT_TRANSCRIPT_OK;
	}
	if (strcmp(key, "destination") == 0) {
		field = FIELD_DESTINATION;
		if (field_seen(seen, field) != 0)
			goto duplicate;
		if (kn_callsign_parse(value, &transcript->destination) != 0)
			goto invalid;
		return KN_COMPAT_TRANSCRIPT_OK;
	}
	if (strcmp(key, "pid") == 0) {
		field = FIELD_PID;
		if (field_seen(seen, field) != 0)
			goto duplicate;
		if (pid_parse(value, &transcript->pid) !=
		    KN_COMPAT_TRANSCRIPT_OK)
			goto invalid;
		return KN_COMPAT_TRANSCRIPT_OK;
	}
	if (strcmp(key, "input") == 0) {
		field = FIELD_INPUT;
		if (field_seen(seen, field) != 0)
			goto duplicate;
		if (strlen(value) > sizeof(transcript->input))
			goto invalid;
		memcpy(transcript->input, value, strlen(value));
		transcript->input_len = strlen(value);
		return KN_COMPAT_TRANSCRIPT_OK;
	}
	if (strcmp(key, "expect-event") == 0) {
		field = FIELD_EXPECT_EVENT;
		if (field_seen(seen, field) != 0)
			goto duplicate;
		rc = expect_event_parse(transcript, value);
		if (rc != KN_COMPAT_TRANSCRIPT_OK)
			goto invalid;
		return KN_COMPAT_TRANSCRIPT_OK;
	}
	if (strcmp(key, "expect-reply") == 0) {
		field = FIELD_EXPECT_REPLY;
		if (field_seen(seen, field) != 0)
			goto duplicate;
		if (expect_reply_parse(transcript, value) !=
		    KN_COMPAT_TRANSCRIPT_OK)
			goto invalid;
		return KN_COMPAT_TRANSCRIPT_OK;
	}
	if (strcmp(key, "expect-reply-queued") == 0) {
		field = FIELD_EXPECT_REPLY_QUEUED;
		if (field_seen(seen, field) != 0)
			goto duplicate;
		if (bool_parse(value, &bool_value) != KN_COMPAT_TRANSCRIPT_OK)
			goto invalid;
		transcript->expect_reply_queued = bool_value;
		transcript->has_expect_reply_queued = 1;
		return KN_COMPAT_TRANSCRIPT_OK;
	}
	if (strcmp(key, "expect-no-dispatch") == 0) {
		field = FIELD_EXPECT_NO_DISPATCH;
		if (field_seen(seen, field) != 0)
			goto duplicate;
		if (bool_parse(value, &transcript->expect_no_dispatch) !=
		    KN_COMPAT_TRANSCRIPT_OK)
			goto invalid;
		return KN_COMPAT_TRANSCRIPT_OK;
	}
	if (strcmp(key, "expect-error") == 0) {
		field = FIELD_EXPECT_ERROR;
		if (field_seen(seen, field) != 0)
			goto duplicate;
		if (strlen(value) >= sizeof(transcript->expect_error))
			goto invalid;
		memcpy(transcript->expect_error, value, strlen(value) + 1);
		transcript->has_expect_error = 1;
		return KN_COMPAT_TRANSCRIPT_OK;
	}

	error_set(info, KN_COMPAT_TRANSCRIPT_ERR_UNKNOWN_KEY, line,
	    "unknown key");
	return KN_COMPAT_TRANSCRIPT_ERR_UNKNOWN_KEY;

duplicate:
	error_set(info, KN_COMPAT_TRANSCRIPT_ERR_DUPLICATE_KEY, line,
	    "duplicate key");
	return KN_COMPAT_TRANSCRIPT_ERR_DUPLICATE_KEY;

invalid:
	error_set(info, KN_COMPAT_TRANSCRIPT_ERR_INVALID_VALUE, line,
	    "invalid value");
	return KN_COMPAT_TRANSCRIPT_ERR_INVALID_VALUE;
}

static uint8_t
field_seen(uint32_t *seen, uint32_t field)
{
	if ((*seen & field) != 0)
		return 1;
	*seen |= field;
	return 0;
}

static enum kn_compat_transcript_error
mode_parse(const char *value, enum kn_compat_mode *mode)
{
	if (strcmp(value, "rf-ui") == 0) {
		*mode = KN_COMPAT_MODE_RF_UI;
		return KN_COMPAT_TRANSCRIPT_OK;
	}

	return KN_COMPAT_TRANSCRIPT_ERR_INVALID_VALUE;
}

static enum kn_compat_transcript_error
pid_parse(const char *value, uint8_t *pid)
{
	char *end;
	unsigned long parsed;
	int base;

	base = 10;
	if (strncmp(value, "0x", 2) == 0 || strncmp(value, "0X", 2) == 0)
		base = 16;
	errno = 0;
	parsed = strtoul(value, &end, base);
	if (errno != 0 || *end != '\0' || parsed > UINT8_MAX)
		return KN_COMPAT_TRANSCRIPT_ERR_INVALID_VALUE;
	*pid = (uint8_t)parsed;
	return KN_COMPAT_TRANSCRIPT_OK;
}

static enum kn_compat_transcript_error
require_fields(const struct kn_compat_transcript *transcript, uint32_t seen,
	struct kn_compat_transcript_error_info *info)
{
	uint32_t required;

	(void)transcript;
	required = FIELD_NAME | FIELD_MODE | FIELD_NODE | FIELD_PORT |
	    FIELD_SOURCE | FIELD_DESTINATION | FIELD_PID | FIELD_INPUT |
	    FIELD_EXPECT_EVENT | FIELD_EXPECT_REPLY;
	if ((seen & required) != required) {
		error_set(info, KN_COMPAT_TRANSCRIPT_ERR_MISSING_REQUIRED, 0,
		    "missing required field");
		return KN_COMPAT_TRANSCRIPT_ERR_MISSING_REQUIRED;
	}
	return KN_COMPAT_TRANSCRIPT_OK;
}

static enum kn_compat_transcript_error
status_parse(const char *value, enum kn_rf_command_status *status)
{
	enum kn_rf_command_status candidate;

	for (candidate = KN_RF_COMMAND_STATUS_OK;
	    candidate <= KN_RF_COMMAND_STATUS_MALFORMED; candidate++) {
		if (strcmp(value, kn_rf_command_status_string(candidate)) == 0) {
			*status = candidate;
			return KN_COMPAT_TRANSCRIPT_OK;
		}
	}

	return KN_COMPAT_TRANSCRIPT_ERR_INVALID_VALUE;
}

static char *
trim(char *text)
{
	char *end;

	while (*text == ' ' || *text == '\t' || *text == '\r')
		text++;
	end = text + strlen(text);
	while (end > text &&
	    (end[-1] == ' ' || end[-1] == '\t' || end[-1] == '\r'))
		*--end = '\0';
	return text;
}
