/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/compat/compat_observe.c */

#include <sys/types.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "kilonode/compat_observe.h"

#define OBS_FIELD_NAME        0x0001u
#define OBS_FIELD_SUBJECT     0x0002u
#define OBS_FIELD_METHOD      0x0004u
#define OBS_FIELD_DATE        0x0008u
#define OBS_FIELD_MODE        0x0010u
#define OBS_FIELD_INPUT       0x0020u
#define OBS_FIELD_OBSERVED    0x0040u
#define OBS_FIELD_OBSERVER    0x0080u
#define OBS_FIELD_BINARY      0x0100u
#define OBS_FIELD_CONFIG      0x0200u
#define OBS_FIELD_CONNECT     0x0400u
#define OBS_FIELD_ENVIRONMENT 0x0800u
#define OBS_FIELD_NOTES       0x1000u
#define OBS_FIELD_PACKET      0x2000u
#define OBS_FIELD_MAILBOX     0x4000u
#define OBS_FIELD_RESULT      0x8000u
#define OBS_FIELD_SOURCE      0x10000u

static enum kn_compat_observe_error append_observed(
	struct kn_compat_observation *, const char *);
static void error_set(struct kn_compat_observe_error_info *,
	enum kn_compat_observe_error, size_t, const char *);
static enum kn_compat_observe_error field_set(
	struct kn_compat_observation *, uint32_t *, const char *,
	const char *, size_t, struct kn_compat_observe_error_info *);
static uint8_t field_seen(uint32_t *, uint32_t);
static enum kn_compat_observe_error method_parse(const char *,
	enum kn_compat_observe_method *);
static enum kn_compat_observe_error mode_parse(const char *,
	enum kn_compat_observe_mode *);
static enum kn_compat_observe_error require_fields(uint32_t,
	struct kn_compat_observe_error_info *);
static uint8_t unsafe_reference_path(const char *);
static void sanitize_text(const char *, char *, size_t);
static char *trim(char *);

void
kn_compat_observation_clear(struct kn_compat_observation *observation)
{
	if (observation == NULL)
		return;

	memset(observation, 0, sizeof(*observation));
	observation->method = KN_COMPAT_OBSERVE_METHOD_NONE;
	observation->mode = KN_COMPAT_OBSERVE_MODE_NONE;
}

const char *
kn_compat_observe_error_name(enum kn_compat_observe_error error)
{
	switch (error) {
	case KN_COMPAT_OBSERVE_OK:
		return "ok";
	case KN_COMPAT_OBSERVE_ERR_INVALID_ARGUMENT:
		return "invalid-argument";
	case KN_COMPAT_OBSERVE_ERR_IO:
		return "io";
	case KN_COMPAT_OBSERVE_ERR_LINE_TOO_LONG:
		return "line-too-long";
	case KN_COMPAT_OBSERVE_ERR_PARSE:
		return "parse";
	case KN_COMPAT_OBSERVE_ERR_DUPLICATE_KEY:
		return "duplicate-key";
	case KN_COMPAT_OBSERVE_ERR_UNKNOWN_KEY:
		return "unknown-key";
	case KN_COMPAT_OBSERVE_ERR_MISSING_REQUIRED:
		return "missing-required";
	case KN_COMPAT_OBSERVE_ERR_INVALID_VALUE:
		return "invalid-value";
	case KN_COMPAT_OBSERVE_ERR_TOO_LARGE:
		return "too-large";
	}

	return "unknown";
}

const char *
kn_compat_observe_method_name(enum kn_compat_observe_method method)
{
	switch (method) {
	case KN_COMPAT_OBSERVE_METHOD_PROCESS:
		return "process";
	case KN_COMPAT_OBSERVE_METHOD_TELNET:
		return "telnet";
	case KN_COMPAT_OBSERVE_METHOD_TCP_LINE:
		return "tcp-line";
	case KN_COMPAT_OBSERVE_METHOD_PACKET_CAPTURE:
		return "packet-capture";
	case KN_COMPAT_OBSERVE_METHOD_MAILBOX:
		return "mailbox";
	case KN_COMPAT_OBSERVE_METHOD_NONE:
		return "none";
	}

	return "unknown";
}

const char *
kn_compat_observe_mode_name(enum kn_compat_observe_mode mode)
{
	switch (mode) {
	case KN_COMPAT_OBSERVE_MODE_PROCESS_OUTPUT:
		return "process-output";
	case KN_COMPAT_OBSERVE_MODE_TCP_LINE:
		return "tcp-line";
	case KN_COMPAT_OBSERVE_MODE_NODE_SHELL:
		return "node-shell";
	case KN_COMPAT_OBSERVE_MODE_BBS_SHELL:
		return "bbs-shell";
	case KN_COMPAT_OBSERVE_MODE_NONE:
		return "none";
	}

	return "unknown";
}

enum kn_compat_observe_error
kn_compat_observation_parse_file(const char *path,
	struct kn_compat_observation *observation,
	struct kn_compat_observe_error_info *info)
{
	FILE *fp;
	char text[KN_COMPAT_OBSERVE_TEXT_MAX + 1];
	size_t len;
	int ch;

	if (path == NULL || observation == NULL)
		return KN_COMPAT_OBSERVE_ERR_INVALID_ARGUMENT;

	fp = fopen(path, "r");
	if (fp == NULL) {
		error_set(info, KN_COMPAT_OBSERVE_ERR_IO, 0, "open failed");
		return KN_COMPAT_OBSERVE_ERR_IO;
	}

	len = 0;
	while ((ch = fgetc(fp)) != EOF) {
		if (len >= KN_COMPAT_OBSERVE_TEXT_MAX) {
			(void)fclose(fp);
			error_set(info, KN_COMPAT_OBSERVE_ERR_TOO_LARGE, 0,
			    "observation too large");
			return KN_COMPAT_OBSERVE_ERR_TOO_LARGE;
		}
		text[len++] = (char)ch;
	}
	if (ferror(fp)) {
		(void)fclose(fp);
		error_set(info, KN_COMPAT_OBSERVE_ERR_IO, 0, "read failed");
		return KN_COMPAT_OBSERVE_ERR_IO;
	}
	(void)fclose(fp);
	text[len] = '\0';

	return kn_compat_observation_parse_text(text, observation, info);
}

enum kn_compat_observe_error
kn_compat_observation_parse_text(const char *text,
	struct kn_compat_observation *observation,
	struct kn_compat_observe_error_info *info)
{
	char line[KN_COMPAT_OBSERVE_LINE_MAX];
	char *key;
	char *value;
	size_t i;
	size_t line_len;
	size_t line_no;
	uint32_t seen;
	uint8_t in_block;
	enum kn_compat_observe_error rc;

	if (text == NULL || observation == NULL)
		return KN_COMPAT_OBSERVE_ERR_INVALID_ARGUMENT;

	kn_compat_observation_clear(observation);
	if (info != NULL)
		memset(info, 0, sizeof(*info));
	seen = 0;
	in_block = 0;
	line_no = 1;
	line_len = 0;

	for (i = 0;; i++) {
		if (text[i] != '\0' && text[i] != '\n') {
			if (line_len + 1 >= sizeof(line)) {
				error_set(info, KN_COMPAT_OBSERVE_ERR_LINE_TOO_LONG,
				    line_no, "line too long");
				return KN_COMPAT_OBSERVE_ERR_LINE_TOO_LONG;
			}
			line[line_len++] = text[i];
			continue;
		}

		line[line_len] = '\0';
		if (in_block != 0) {
			if (strcmp(line, "observed-end") == 0) {
				in_block = 0;
				seen |= OBS_FIELD_OBSERVED;
			} else {
				rc = append_observed(observation, line);
				if (rc != KN_COMPAT_OBSERVE_OK) {
					error_set(info, rc, line_no,
					    "observed block too large");
					return rc;
				}
			}
		} else {
			key = trim(line);
			if (key[0] != '\0' && key[0] != '#') {
				if (strcmp(key, "observed-begin") == 0) {
					if (field_seen(&seen,
					    OBS_FIELD_OBSERVED) != 0) {
						error_set(info,
						    KN_COMPAT_OBSERVE_ERR_DUPLICATE_KEY,
						    line_no, "observed");
						return KN_COMPAT_OBSERVE_ERR_DUPLICATE_KEY;
					}
					in_block = 1;
				} else if (strcmp(key, "observed-end") == 0) {
					error_set(info, KN_COMPAT_OBSERVE_ERR_PARSE,
					    line_no, "unexpected observed-end");
					return KN_COMPAT_OBSERVE_ERR_PARSE;
				} else {
					value = key;
					while (*value != '\0' && *value != ' ' &&
					    *value != '\t')
						value++;
					if (*value == '\0') {
						error_set(info,
						    KN_COMPAT_OBSERVE_ERR_PARSE,
						    line_no, "missing value");
						return KN_COMPAT_OBSERVE_ERR_PARSE;
					}
					*value++ = '\0';
					value = trim(value);
					if (value[0] == '\0') {
						error_set(info,
						    KN_COMPAT_OBSERVE_ERR_PARSE,
						    line_no, "missing value");
						return KN_COMPAT_OBSERVE_ERR_PARSE;
					}
					rc = field_set(observation, &seen, key,
					    value, line_no, info);
					if (rc != KN_COMPAT_OBSERVE_OK)
						return rc;
				}
			}
		}

		if (text[i] == '\0')
			break;
		line_no++;
		line_len = 0;
	}

	if (in_block != 0) {
		error_set(info, KN_COMPAT_OBSERVE_ERR_PARSE, line_no,
		    "missing observed-end");
		return KN_COMPAT_OBSERVE_ERR_PARSE;
	}

	return require_fields(seen, info);
}

enum kn_compat_observe_error
kn_compat_observation_report(const struct kn_compat_observation *observation,
	char *buf, size_t bufsiz)
{
	char preview[256];
	int needed;

	if (observation == NULL || buf == NULL || bufsiz == 0)
		return KN_COMPAT_OBSERVE_ERR_INVALID_ARGUMENT;

	sanitize_text(observation->observed, preview, sizeof(preview));
	needed = snprintf(buf, bufsiz,
	    "OBSERVATION name=%s subject=%s method=%s date=%s mode=%s "
	    "input_len=%llu observed_len=%llu preview=\"%s\"",
	    observation->name, observation->subject,
	    kn_compat_observe_method_name(observation->method),
	    observation->date, kn_compat_observe_mode_name(observation->mode),
	    (unsigned long long)strlen(observation->input),
	    (unsigned long long)observation->observed_len, preview);

	return needed >= 0 && (size_t)needed < bufsiz ?
	    KN_COMPAT_OBSERVE_OK : KN_COMPAT_OBSERVE_ERR_TOO_LARGE;
}

static enum kn_compat_observe_error
append_observed(struct kn_compat_observation *observation, const char *line)
{
	size_t line_len;

	line_len = strlen(line);
	if (observation->observed_len + line_len + 2 >
	    sizeof(observation->observed))
		return KN_COMPAT_OBSERVE_ERR_TOO_LARGE;
	memcpy(observation->observed + observation->observed_len, line,
	    line_len);
	observation->observed_len += line_len;
	observation->observed[observation->observed_len++] = '\n';
	observation->observed[observation->observed_len] = '\0';

	return KN_COMPAT_OBSERVE_OK;
}

static void
error_set(struct kn_compat_observe_error_info *info,
	enum kn_compat_observe_error error, size_t line, const char *message)
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

static enum kn_compat_observe_error
field_set(struct kn_compat_observation *observation, uint32_t *seen,
	const char *key, const char *value, size_t line,
	struct kn_compat_observe_error_info *info)
{
	char *dst;
	size_t dst_len;
	uint32_t flag;
	enum kn_compat_observe_error rc;

	dst = NULL;
	dst_len = 0;
	flag = 0;
	if (strcmp(key, "name") == 0) {
		dst = observation->name;
		dst_len = sizeof(observation->name);
		flag = OBS_FIELD_NAME;
	} else if (strcmp(key, "subject") == 0) {
		dst = observation->subject;
		dst_len = sizeof(observation->subject);
		flag = OBS_FIELD_SUBJECT;
	} else if (strcmp(key, "source") == 0) {
		dst = observation->source;
		dst_len = sizeof(observation->source);
		flag = OBS_FIELD_SOURCE;
	} else if (strcmp(key, "method") == 0) {
		if (field_seen(seen, OBS_FIELD_METHOD) != 0)
			goto duplicate;
		rc = method_parse(value, &observation->method);
		if (rc != KN_COMPAT_OBSERVE_OK)
			goto invalid;
		return KN_COMPAT_OBSERVE_OK;
	} else if (strcmp(key, "date") == 0) {
		dst = observation->date;
		dst_len = sizeof(observation->date);
		flag = OBS_FIELD_DATE;
	} else if (strcmp(key, "observer") == 0) {
		dst = observation->observer;
		dst_len = sizeof(observation->observer);
		flag = OBS_FIELD_OBSERVER;
	} else if (strcmp(key, "binary") == 0) {
		dst = observation->binary_path;
		dst_len = sizeof(observation->binary_path);
		flag = OBS_FIELD_BINARY;
	} else if (strcmp(key, "config") == 0) {
		dst = observation->config_path;
		dst_len = sizeof(observation->config_path);
		flag = OBS_FIELD_CONFIG;
	} else if (strcmp(key, "mode") == 0) {
		if (field_seen(seen, OBS_FIELD_MODE) != 0)
			goto duplicate;
		rc = mode_parse(value, &observation->mode);
		if (rc != KN_COMPAT_OBSERVE_OK)
			goto invalid;
		return KN_COMPAT_OBSERVE_OK;
	} else if (strcmp(key, "connect") == 0) {
		dst = observation->connect_target;
		dst_len = sizeof(observation->connect_target);
		flag = OBS_FIELD_CONNECT;
	} else if (strcmp(key, "input") == 0) {
		dst = observation->input;
		dst_len = sizeof(observation->input);
		flag = OBS_FIELD_INPUT;
	} else if (strcmp(key, "environment") == 0) {
		dst = observation->environment;
		dst_len = sizeof(observation->environment);
		flag = OBS_FIELD_ENVIRONMENT;
	} else if (strcmp(key, "notes") == 0) {
		dst = observation->notes;
		dst_len = sizeof(observation->notes);
		flag = OBS_FIELD_NOTES;
	} else if (strcmp(key, "packet-capture") == 0) {
		if (unsafe_reference_path(value) != 0)
			goto invalid;
		dst = observation->packet_capture_path;
		dst_len = sizeof(observation->packet_capture_path);
		flag = OBS_FIELD_PACKET;
	} else if (strcmp(key, "mailbox") == 0) {
		if (unsafe_reference_path(value) != 0)
			goto invalid;
		dst = observation->mailbox_path;
		dst_len = sizeof(observation->mailbox_path);
		flag = OBS_FIELD_MAILBOX;
	} else if (strcmp(key, "result") == 0) {
		dst = observation->result;
		dst_len = sizeof(observation->result);
		flag = OBS_FIELD_RESULT;
	} else {
		error_set(info, KN_COMPAT_OBSERVE_ERR_UNKNOWN_KEY, line, key);
		return KN_COMPAT_OBSERVE_ERR_UNKNOWN_KEY;
	}

	if (field_seen(seen, flag) != 0)
		goto duplicate;
	if (strlen(value) >= dst_len)
		goto invalid;
	(void)snprintf(dst, dst_len, "%s", value);
	return KN_COMPAT_OBSERVE_OK;

duplicate:
	error_set(info, KN_COMPAT_OBSERVE_ERR_DUPLICATE_KEY, line, key);
	return KN_COMPAT_OBSERVE_ERR_DUPLICATE_KEY;

invalid:
	error_set(info, KN_COMPAT_OBSERVE_ERR_INVALID_VALUE, line, key);
	return KN_COMPAT_OBSERVE_ERR_INVALID_VALUE;
}

static uint8_t
field_seen(uint32_t *seen, uint32_t flag)
{
	if ((*seen & flag) != 0)
		return 1;
	*seen |= flag;
	return 0;
}

static enum kn_compat_observe_error
method_parse(const char *value, enum kn_compat_observe_method *method)
{
	if (strcmp(value, "process") == 0)
		*method = KN_COMPAT_OBSERVE_METHOD_PROCESS;
	else if (strcmp(value, "telnet") == 0)
		*method = KN_COMPAT_OBSERVE_METHOD_TELNET;
	else if (strcmp(value, "tcp-line") == 0)
		*method = KN_COMPAT_OBSERVE_METHOD_TCP_LINE;
	else if (strcmp(value, "packet-capture") == 0)
		*method = KN_COMPAT_OBSERVE_METHOD_PACKET_CAPTURE;
	else if (strcmp(value, "mailbox") == 0)
		*method = KN_COMPAT_OBSERVE_METHOD_MAILBOX;
	else
		return KN_COMPAT_OBSERVE_ERR_INVALID_VALUE;

	return KN_COMPAT_OBSERVE_OK;
}

static enum kn_compat_observe_error
mode_parse(const char *value, enum kn_compat_observe_mode *mode)
{
	if (strcmp(value, "process-output") == 0)
		*mode = KN_COMPAT_OBSERVE_MODE_PROCESS_OUTPUT;
	else if (strcmp(value, "tcp-line") == 0)
		*mode = KN_COMPAT_OBSERVE_MODE_TCP_LINE;
	else if (strcmp(value, "node-shell") == 0)
		*mode = KN_COMPAT_OBSERVE_MODE_NODE_SHELL;
	else if (strcmp(value, "bbs-shell") == 0)
		*mode = KN_COMPAT_OBSERVE_MODE_BBS_SHELL;
	else
		return KN_COMPAT_OBSERVE_ERR_INVALID_VALUE;

	return KN_COMPAT_OBSERVE_OK;
}

static enum kn_compat_observe_error
require_fields(uint32_t seen, struct kn_compat_observe_error_info *info)
{
	const uint32_t required = OBS_FIELD_NAME | OBS_FIELD_SUBJECT |
	    OBS_FIELD_METHOD | OBS_FIELD_DATE | OBS_FIELD_MODE |
	    OBS_FIELD_INPUT | OBS_FIELD_OBSERVED;

	if ((seen & required) != required) {
		error_set(info, KN_COMPAT_OBSERVE_ERR_MISSING_REQUIRED, 0,
		    "missing required field");
		return KN_COMPAT_OBSERVE_ERR_MISSING_REQUIRED;
	}

	return KN_COMPAT_OBSERVE_OK;
}

static void
sanitize_text(const char *src, char *dst, size_t dst_len)
{
	size_t i;
	size_t off;
	int needed;
	unsigned char ch;

	if (dst == NULL || dst_len == 0)
		return;
	dst[0] = '\0';
	if (src == NULL)
		return;

	off = 0;
	for (i = 0; src[i] != '\0' && off + 1 < dst_len; i++) {
		ch = (unsigned char)src[i];
		if (ch == '\n') {
			needed = snprintf(dst + off, dst_len - off, "\\n");
		} else if (ch == '\t') {
			needed = snprintf(dst + off, dst_len - off, "\\t");
		} else if (ch >= 0x20 && ch <= 0x7e) {
			needed = snprintf(dst + off, dst_len - off, "%c",
			    (int)ch);
		} else {
			needed = snprintf(dst + off, dst_len - off, "\\x%02x",
			    (unsigned int)ch);
		}
		if (needed < 0 || (size_t)needed >= dst_len - off)
			return;
		off += (size_t)needed;
	}
}

static char *
trim(char *text)
{
	char *end;

	while (*text == ' ' || *text == '\t' || *text == '\r')
		text++;
	end = text + strlen(text);
	while (end > text && (end[-1] == ' ' || end[-1] == '\t' ||
	    end[-1] == '\r')) {
		end--;
		*end = '\0';
	}

	return text;
}

static uint8_t
unsafe_reference_path(const char *path)
{
	if (path == NULL || path[0] == '\0')
		return 1;
	if (strstr(path, "..") != NULL)
		return 1;

	return 0;
}
