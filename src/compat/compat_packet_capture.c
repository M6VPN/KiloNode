/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/compat/compat_packet_capture.c */

#include <sys/types.h>

#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kilonode/compat_packet_capture.h"

#define PCAP_FIELD_NAME        0x0001u
#define PCAP_FIELD_METHOD      0x0002u
#define PCAP_FIELD_DIRECTION   0x0004u
#define PCAP_FIELD_FRAME       0x0008u
#define PCAP_FIELD_PORT        0x0010u
#define PCAP_FIELD_SUBJECT     0x0020u
#define PCAP_FIELD_DATE        0x0040u
#define PCAP_FIELD_OBSERVER    0x0080u
#define PCAP_FIELD_MODE        0x0100u
#define PCAP_FIELD_TIMESTAMP   0x0200u
#define PCAP_FIELD_SRC_EP      0x0400u
#define PCAP_FIELD_DST_EP      0x0800u

static enum kn_compat_packet_capture_error direction_parse(const char *,
	enum kn_compat_packet_direction *);
static void error_set(struct kn_compat_packet_capture_error_info *,
	enum kn_compat_packet_capture_error, size_t, const char *);
static enum kn_compat_packet_capture_error expect_decode_parse(const char *,
	enum kn_compat_packet_expect_decode *);
static enum kn_compat_packet_capture_error field_set(
	struct kn_compat_packet_capture *, uint32_t *, const char *,
	const char *, size_t, struct kn_compat_packet_capture_error_info *);
static uint8_t field_seen(uint32_t *, uint32_t);
static int hex_value(int);
static enum kn_compat_packet_capture_error hex_append(
	struct kn_compat_packet_capture *, const char *);
static enum kn_compat_packet_capture_error hex_parse_bytes(const char *,
	uint8_t *, size_t, size_t *);
static enum kn_compat_packet_capture_error method_parse(const char *,
	enum kn_compat_packet_method *);
static enum kn_compat_packet_capture_error require_fields(
	const struct kn_compat_packet_capture *, uint32_t,
	struct kn_compat_packet_capture_error_info *);
static enum kn_compat_packet_capture_error timestamp_parse(const char *,
	uint64_t *);
static char *trim(char *);

void
kn_compat_packet_capture_clear(struct kn_compat_packet_capture *capture)
{
	if (capture == NULL)
		return;

	memset(capture, 0, sizeof(*capture));
}

const char *
kn_compat_packet_capture_error_name(enum kn_compat_packet_capture_error error)
{
	switch (error) {
	case KN_COMPAT_PACKET_CAPTURE_OK:
		return "ok";
	case KN_COMPAT_PACKET_CAPTURE_ERR_INVALID_ARGUMENT:
		return "invalid-argument";
	case KN_COMPAT_PACKET_CAPTURE_ERR_IO:
		return "io";
	case KN_COMPAT_PACKET_CAPTURE_ERR_LINE_TOO_LONG:
		return "line-too-long";
	case KN_COMPAT_PACKET_CAPTURE_ERR_PARSE:
		return "parse";
	case KN_COMPAT_PACKET_CAPTURE_ERR_DUPLICATE_KEY:
		return "duplicate-key";
	case KN_COMPAT_PACKET_CAPTURE_ERR_UNKNOWN_KEY:
		return "unknown-key";
	case KN_COMPAT_PACKET_CAPTURE_ERR_MISSING_REQUIRED:
		return "missing-required";
	case KN_COMPAT_PACKET_CAPTURE_ERR_INVALID_VALUE:
		return "invalid-value";
	case KN_COMPAT_PACKET_CAPTURE_ERR_TOO_LARGE:
		return "too-large";
	}

	return "unknown";
}

const char *
kn_compat_packet_direction_name(enum kn_compat_packet_direction direction)
{
	switch (direction) {
	case KN_COMPAT_PACKET_DIRECTION_RX:
		return "rx";
	case KN_COMPAT_PACKET_DIRECTION_TX:
		return "tx";
	case KN_COMPAT_PACKET_DIRECTION_NONE:
		return "none";
	}

	return "unknown";
}

const char *
kn_compat_packet_expect_decode_name(
	enum kn_compat_packet_expect_decode expect)
{
	switch (expect) {
	case KN_COMPAT_PACKET_EXPECT_AX25_UI:
		return "ax25-ui";
	case KN_COMPAT_PACKET_EXPECT_MALFORMED:
		return "malformed";
	case KN_COMPAT_PACKET_EXPECT_NONE:
		return "none";
	}

	return "unknown";
}

const char *
kn_compat_packet_method_name(enum kn_compat_packet_method method)
{
	switch (method) {
	case KN_COMPAT_PACKET_METHOD_KISS:
		return "kiss";
	case KN_COMPAT_PACKET_METHOD_AXIP:
		return "axip";
	case KN_COMPAT_PACKET_METHOD_AXUDP:
		return "axudp";
	case KN_COMPAT_PACKET_METHOD_NONE:
		return "none";
	}

	return "unknown";
}

enum kn_compat_packet_capture_error
kn_compat_packet_capture_parse_file(const char *path,
	struct kn_compat_packet_capture *capture,
	struct kn_compat_packet_capture_error_info *info)
{
	FILE *fp;
	char text[KN_COMPAT_CAPTURE_TEXT_MAX + 1];
	size_t len;
	int ch;

	if (path == NULL || capture == NULL)
		return KN_COMPAT_PACKET_CAPTURE_ERR_INVALID_ARGUMENT;

	fp = fopen(path, "r");
	if (fp == NULL) {
		error_set(info, KN_COMPAT_PACKET_CAPTURE_ERR_IO, 0,
		    "open failed");
		return KN_COMPAT_PACKET_CAPTURE_ERR_IO;
	}
	len = 0;
	while ((ch = fgetc(fp)) != EOF) {
		if (len >= KN_COMPAT_CAPTURE_TEXT_MAX) {
			(void)fclose(fp);
			error_set(info, KN_COMPAT_PACKET_CAPTURE_ERR_TOO_LARGE, 0,
			    "capture too large");
			return KN_COMPAT_PACKET_CAPTURE_ERR_TOO_LARGE;
		}
		text[len++] = (char)ch;
	}
	if (ferror(fp)) {
		(void)fclose(fp);
		error_set(info, KN_COMPAT_PACKET_CAPTURE_ERR_IO, 0,
		    "read failed");
		return KN_COMPAT_PACKET_CAPTURE_ERR_IO;
	}
	(void)fclose(fp);
	text[len] = '\0';

	return kn_compat_packet_capture_parse_text(text, capture, info);
}

enum kn_compat_packet_capture_error
kn_compat_packet_capture_parse_text(const char *text,
	struct kn_compat_packet_capture *capture,
	struct kn_compat_packet_capture_error_info *info)
{
	char line[KN_COMPAT_CAPTURE_LINE_MAX];
	char *key;
	char *value;
	size_t i;
	size_t line_len;
	size_t line_no;
	uint32_t seen;
	uint8_t in_frame;
	enum kn_compat_packet_capture_error rc;

	if (text == NULL || capture == NULL)
		return KN_COMPAT_PACKET_CAPTURE_ERR_INVALID_ARGUMENT;

	kn_compat_packet_capture_clear(capture);
	if (info != NULL)
		memset(info, 0, sizeof(*info));
	seen = 0;
	in_frame = 0;
	line_no = 1;
	line_len = 0;

	for (i = 0;; i++) {
		if (text[i] != '\0' && text[i] != '\n') {
			if (line_len + 1 >= sizeof(line)) {
				error_set(info,
				    KN_COMPAT_PACKET_CAPTURE_ERR_LINE_TOO_LONG,
				    line_no, "line too long");
				return KN_COMPAT_PACKET_CAPTURE_ERR_LINE_TOO_LONG;
			}
			line[line_len++] = text[i];
			continue;
		}

		line[line_len] = '\0';
		key = trim(line);
		if (key[0] != '\0' && key[0] != '#') {
			if (in_frame != 0) {
				if (strcmp(key, "frame-end") == 0) {
					in_frame = 0;
					seen |= PCAP_FIELD_FRAME;
				} else {
					rc = hex_append(capture, key);
					if (rc != KN_COMPAT_PACKET_CAPTURE_OK) {
						error_set(info, rc, line_no,
						    "invalid frame hex");
						return rc;
					}
				}
			} else if (strcmp(key, "frame-begin hex") == 0) {
				if (field_seen(&seen, PCAP_FIELD_FRAME) != 0) {
					error_set(info,
					    KN_COMPAT_PACKET_CAPTURE_ERR_DUPLICATE_KEY,
					    line_no, "frame");
					return KN_COMPAT_PACKET_CAPTURE_ERR_DUPLICATE_KEY;
				}
				in_frame = 1;
			} else if (strcmp(key, "frame-end") == 0) {
				error_set(info, KN_COMPAT_PACKET_CAPTURE_ERR_PARSE,
				    line_no, "unexpected frame-end");
				return KN_COMPAT_PACKET_CAPTURE_ERR_PARSE;
			} else {
				value = key;
				while (*value != '\0' && *value != ' ' &&
				    *value != '\t')
					value++;
				if (*value == '\0') {
					error_set(info,
					    KN_COMPAT_PACKET_CAPTURE_ERR_PARSE,
					    line_no, "missing value");
					return KN_COMPAT_PACKET_CAPTURE_ERR_PARSE;
				}
				*value++ = '\0';
				value = trim(value);
				rc = field_set(capture, &seen, key, value,
				    line_no, info);
				if (rc != KN_COMPAT_PACKET_CAPTURE_OK)
					return rc;
			}
		}

		if (text[i] == '\0')
			break;
		line_no++;
		line_len = 0;
	}

	if (in_frame != 0) {
		error_set(info, KN_COMPAT_PACKET_CAPTURE_ERR_PARSE, line_no,
		    "missing frame-end");
		return KN_COMPAT_PACKET_CAPTURE_ERR_PARSE;
	}

	return require_fields(capture, seen, info);
}

static enum kn_compat_packet_capture_error
direction_parse(const char *value, enum kn_compat_packet_direction *direction)
{
	if (strcmp(value, "rx") == 0)
		*direction = KN_COMPAT_PACKET_DIRECTION_RX;
	else if (strcmp(value, "tx") == 0)
		*direction = KN_COMPAT_PACKET_DIRECTION_TX;
	else
		return KN_COMPAT_PACKET_CAPTURE_ERR_INVALID_VALUE;

	return KN_COMPAT_PACKET_CAPTURE_OK;
}

static void
error_set(struct kn_compat_packet_capture_error_info *info,
	enum kn_compat_packet_capture_error error, size_t line,
	const char *message)
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

static enum kn_compat_packet_capture_error
expect_decode_parse(const char *value,
	enum kn_compat_packet_expect_decode *expect)
{
	if (strcmp(value, "ax25-ui") == 0)
		*expect = KN_COMPAT_PACKET_EXPECT_AX25_UI;
	else if (strcmp(value, "malformed") == 0)
		*expect = KN_COMPAT_PACKET_EXPECT_MALFORMED;
	else
		return KN_COMPAT_PACKET_CAPTURE_ERR_INVALID_VALUE;

	return KN_COMPAT_PACKET_CAPTURE_OK;
}

static enum kn_compat_packet_capture_error
field_set(struct kn_compat_packet_capture *capture, uint32_t *seen,
	const char *key, const char *value, size_t line,
	struct kn_compat_packet_capture_error_info *info)
{
	char *dst;
	size_t dst_len;
	uint32_t flag;
	enum kn_compat_packet_capture_error rc;

	dst = NULL;
	dst_len = 0;
	flag = 0;
	if (strcmp(key, "name") == 0) {
		dst = capture->name;
		dst_len = sizeof(capture->name);
		flag = PCAP_FIELD_NAME;
	} else if (strcmp(key, "subject") == 0) {
		dst = capture->subject;
		dst_len = sizeof(capture->subject);
		flag = PCAP_FIELD_SUBJECT;
	} else if (strcmp(key, "method") == 0) {
		if (field_seen(seen, PCAP_FIELD_METHOD) != 0)
			goto duplicate;
		rc = method_parse(value, &capture->method);
		if (rc != KN_COMPAT_PACKET_CAPTURE_OK)
			goto invalid;
		return KN_COMPAT_PACKET_CAPTURE_OK;
	} else if (strcmp(key, "date") == 0) {
		dst = capture->date;
		dst_len = sizeof(capture->date);
		flag = PCAP_FIELD_DATE;
	} else if (strcmp(key, "observer") == 0) {
		dst = capture->observer;
		dst_len = sizeof(capture->observer);
		flag = PCAP_FIELD_OBSERVER;
	} else if (strcmp(key, "mode") == 0) {
		dst = capture->mode;
		dst_len = sizeof(capture->mode);
		flag = PCAP_FIELD_MODE;
	} else if (strcmp(key, "direction") == 0) {
		if (field_seen(seen, PCAP_FIELD_DIRECTION) != 0)
			goto duplicate;
		rc = direction_parse(value, &capture->direction);
		if (rc != KN_COMPAT_PACKET_CAPTURE_OK)
			goto invalid;
		return KN_COMPAT_PACKET_CAPTURE_OK;
	} else if (strcmp(key, "port") == 0) {
		dst = capture->port;
		dst_len = sizeof(capture->port);
		flag = PCAP_FIELD_PORT;
	} else if (strcmp(key, "source-endpoint") == 0) {
		dst = capture->source_endpoint;
		dst_len = sizeof(capture->source_endpoint);
		flag = PCAP_FIELD_SRC_EP;
	} else if (strcmp(key, "destination-endpoint") == 0) {
		dst = capture->destination_endpoint;
		dst_len = sizeof(capture->destination_endpoint);
		flag = PCAP_FIELD_DST_EP;
	} else if (strcmp(key, "timestamp") == 0) {
		if (field_seen(seen, PCAP_FIELD_TIMESTAMP) != 0)
			goto duplicate;
		rc = timestamp_parse(value, &capture->timestamp);
		if (rc != KN_COMPAT_PACKET_CAPTURE_OK)
			goto invalid;
		return KN_COMPAT_PACKET_CAPTURE_OK;
	} else if (strcmp(key, "expect-decode") == 0) {
		rc = expect_decode_parse(value, &capture->expect_decode);
		if (rc != KN_COMPAT_PACKET_CAPTURE_OK)
			goto invalid;
		return KN_COMPAT_PACKET_CAPTURE_OK;
	} else if (strcmp(key, "expect-source") == 0) {
		if (kn_callsign_parse(value, &capture->expect_source) != 0)
			goto invalid;
		capture->has_expect_source = 1;
		return KN_COMPAT_PACKET_CAPTURE_OK;
	} else if (strcmp(key, "expect-destination") == 0) {
		if (kn_callsign_parse(value, &capture->expect_destination) != 0)
			goto invalid;
		capture->has_expect_destination = 1;
		return KN_COMPAT_PACKET_CAPTURE_OK;
	} else if (strcmp(key, "expect-pid") == 0) {
		rc = hex_parse_bytes(value, &capture->expect_pid, 1, &dst_len);
		if (rc != KN_COMPAT_PACKET_CAPTURE_OK || dst_len != 1)
			goto invalid;
		capture->has_expect_pid = 1;
		return KN_COMPAT_PACKET_CAPTURE_OK;
	} else if (strcmp(key, "expect-payload-text") == 0) {
		if (strlen(value) >= sizeof(capture->expect_payload_text))
			goto invalid;
		(void)snprintf(capture->expect_payload_text,
		    sizeof(capture->expect_payload_text), "%s", value);
		capture->has_expect_payload_text = 1;
		return KN_COMPAT_PACKET_CAPTURE_OK;
	} else if (strcmp(key, "expect-payload-hex") == 0) {
		rc = hex_parse_bytes(value, capture->expect_payload_hex,
		    sizeof(capture->expect_payload_hex),
		    &capture->expect_payload_hex_len);
		if (rc != KN_COMPAT_PACKET_CAPTURE_OK)
			goto invalid;
		return KN_COMPAT_PACKET_CAPTURE_OK;
	} else if (strcmp(key, "expect-kind") == 0) {
		if (strlen(value) >= sizeof(capture->expect_kind))
			goto invalid;
		(void)snprintf(capture->expect_kind,
		    sizeof(capture->expect_kind), "%s", value);
		capture->has_expect_kind = 1;
		return KN_COMPAT_PACKET_CAPTURE_OK;
	} else {
		error_set(info, KN_COMPAT_PACKET_CAPTURE_ERR_UNKNOWN_KEY, line,
		    key);
		return KN_COMPAT_PACKET_CAPTURE_ERR_UNKNOWN_KEY;
	}

	if (field_seen(seen, flag) != 0)
		goto duplicate;
	if (strlen(value) >= dst_len)
		goto invalid;
	(void)snprintf(dst, dst_len, "%s", value);
	return KN_COMPAT_PACKET_CAPTURE_OK;

duplicate:
	error_set(info, KN_COMPAT_PACKET_CAPTURE_ERR_DUPLICATE_KEY, line, key);
	return KN_COMPAT_PACKET_CAPTURE_ERR_DUPLICATE_KEY;

invalid:
	error_set(info, KN_COMPAT_PACKET_CAPTURE_ERR_INVALID_VALUE, line, key);
	return KN_COMPAT_PACKET_CAPTURE_ERR_INVALID_VALUE;
}

static uint8_t
field_seen(uint32_t *seen, uint32_t flag)
{
	if ((*seen & flag) != 0)
		return 1;
	*seen |= flag;
	return 0;
}

static int
hex_value(int ch)
{
	if (ch >= '0' && ch <= '9')
		return ch - '0';
	if (ch >= 'a' && ch <= 'f')
		return ch - 'a' + 10;
	if (ch >= 'A' && ch <= 'F')
		return ch - 'A' + 10;

	return -1;
}

static enum kn_compat_packet_capture_error
hex_append(struct kn_compat_packet_capture *capture, const char *value)
{
	uint8_t bytes[KN_COMPAT_CAPTURE_FRAME_MAX];
	size_t len;
	enum kn_compat_packet_capture_error rc;

	rc = hex_parse_bytes(value, bytes, sizeof(bytes), &len);
	if (rc != KN_COMPAT_PACKET_CAPTURE_OK)
		return rc;
	if (capture->frame_len + len > sizeof(capture->frame))
		return KN_COMPAT_PACKET_CAPTURE_ERR_TOO_LARGE;
	memcpy(capture->frame + capture->frame_len, bytes, len);
	capture->frame_len += len;
	return KN_COMPAT_PACKET_CAPTURE_OK;
}

static enum kn_compat_packet_capture_error
hex_parse_bytes(const char *value, uint8_t *out, size_t out_len,
	size_t *out_count)
{
	size_t count;
	int hi;
	int lo;

	count = 0;
	while (*value != '\0') {
		while (*value == ' ' || *value == '\t')
			value++;
		if (*value == '\0')
			break;
		if (value[0] == '0' && (value[1] == 'x' || value[1] == 'X'))
			value += 2;
		hi = hex_value((unsigned char)value[0]);
		lo = hex_value((unsigned char)value[1]);
		if (hi < 0 || lo < 0)
			return KN_COMPAT_PACKET_CAPTURE_ERR_INVALID_VALUE;
		if (out != NULL) {
			if (count >= out_len)
				return KN_COMPAT_PACKET_CAPTURE_ERR_TOO_LARGE;
			out[count] = (uint8_t)((hi << 4) | lo);
		}
		count++;
		value += 2;
		if (*value != '\0' && *value != ' ' && *value != '\t')
			return KN_COMPAT_PACKET_CAPTURE_ERR_INVALID_VALUE;
	}

	if (out_count != NULL)
		*out_count = count;
	return count == 0 ? KN_COMPAT_PACKET_CAPTURE_ERR_INVALID_VALUE :
	    KN_COMPAT_PACKET_CAPTURE_OK;
}

static enum kn_compat_packet_capture_error
method_parse(const char *value, enum kn_compat_packet_method *method)
{
	if (strcmp(value, "kiss") == 0)
		*method = KN_COMPAT_PACKET_METHOD_KISS;
	else if (strcmp(value, "axip") == 0)
		*method = KN_COMPAT_PACKET_METHOD_AXIP;
	else if (strcmp(value, "axudp") == 0)
		*method = KN_COMPAT_PACKET_METHOD_AXUDP;
	else
		return KN_COMPAT_PACKET_CAPTURE_ERR_INVALID_VALUE;

	return KN_COMPAT_PACKET_CAPTURE_OK;
}

static enum kn_compat_packet_capture_error
require_fields(const struct kn_compat_packet_capture *capture, uint32_t seen,
	struct kn_compat_packet_capture_error_info *info)
{
	uint32_t required;

	required = PCAP_FIELD_NAME | PCAP_FIELD_METHOD |
	    PCAP_FIELD_DIRECTION | PCAP_FIELD_FRAME;
	if ((seen & required) != required) {
		error_set(info, KN_COMPAT_PACKET_CAPTURE_ERR_MISSING_REQUIRED, 0,
		    "missing required field");
		return KN_COMPAT_PACKET_CAPTURE_ERR_MISSING_REQUIRED;
	}
	if (capture->method == KN_COMPAT_PACKET_METHOD_KISS &&
	    (seen & PCAP_FIELD_PORT) == 0) {
		error_set(info, KN_COMPAT_PACKET_CAPTURE_ERR_MISSING_REQUIRED, 0,
		    "missing port");
		return KN_COMPAT_PACKET_CAPTURE_ERR_MISSING_REQUIRED;
	}
	if ((capture->method == KN_COMPAT_PACKET_METHOD_AXIP ||
	    capture->method == KN_COMPAT_PACKET_METHOD_AXUDP) &&
	    capture->frame_len == 0) {
		error_set(info, KN_COMPAT_PACKET_CAPTURE_ERR_MISSING_REQUIRED, 0,
		    "missing frame");
		return KN_COMPAT_PACKET_CAPTURE_ERR_MISSING_REQUIRED;
	}

	return KN_COMPAT_PACKET_CAPTURE_OK;
}

static enum kn_compat_packet_capture_error
timestamp_parse(const char *value, uint64_t *timestamp)
{
	char *end;
	unsigned long long parsed;

	errno = 0;
	parsed = strtoull(value, &end, 10);
	if (errno != 0 || *end != '\0')
		return KN_COMPAT_PACKET_CAPTURE_ERR_INVALID_VALUE;
	*timestamp = (uint64_t)parsed;
	return KN_COMPAT_PACKET_CAPTURE_OK;
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
