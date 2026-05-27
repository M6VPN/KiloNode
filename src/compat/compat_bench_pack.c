/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/compat/compat_bench_pack.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "kilonode/compat_axip_capture.h"
#include "kilonode/compat_bench_pack.h"
#include "kilonode/compat_capture_report.h"
#include "kilonode/compat_kiss_capture.h"
#include "kilonode/compat_packet_capture.h"

#define BENCH_FIELD_NAME             0x0001u
#define BENCH_FIELD_TYPE             0x0002u
#define BENCH_FIELD_SOURCE           0x0004u
#define BENCH_FIELD_CLEAN_ROOM       0x0008u
#define BENCH_FIELD_SOURCE_CODE_USED 0x0010u
#define BENCH_FIELD_HARDWARE         0x0020u
#define BENCH_FIELD_TRANSMIT         0x0040u

static enum kn_compat_bench_error bool_parse(const char *, uint8_t *);
static enum kn_compat_bench_error capture_decode(
	const struct kn_compat_packet_capture *,
	struct kn_compat_packet_decode *);
static void coverage_add(struct kn_compat_bench_coverage *,
	const struct kn_compat_packet_capture *,
	const struct kn_compat_packet_decode *, uint8_t);
static void error_set(struct kn_compat_bench_error_info *,
	enum kn_compat_bench_error, size_t, const char *);
static enum kn_compat_bench_error field_set(
	struct kn_compat_bench_pack *, uint32_t *, const char *,
	const char *, size_t, struct kn_compat_bench_error_info *);
static uint8_t field_seen(uint32_t *, uint32_t);
static uint8_t planned_placeholder(const char *);
static enum kn_compat_bench_error list_add(struct kn_compat_bench_entry *,
	size_t *, const char *);
static enum kn_compat_bench_error require_fields(
	const struct kn_compat_bench_pack *, uint32_t,
	struct kn_compat_bench_error_info *);
static void set_base_dir(struct kn_compat_bench_pack *, const char *);
static char *trim(char *);
static uint8_t unsafe_ref(const char *);

void
kn_compat_bench_pack_clear(struct kn_compat_bench_pack *pack)
{
	if (pack == NULL)
		return;

	memset(pack, 0, sizeof(*pack));
}

const char *
kn_compat_bench_error_name(enum kn_compat_bench_error error)
{
	switch (error) {
	case KN_COMPAT_BENCH_OK:
		return "ok";
	case KN_COMPAT_BENCH_ERR_INVALID_ARGUMENT:
		return "invalid-argument";
	case KN_COMPAT_BENCH_ERR_IO:
		return "io";
	case KN_COMPAT_BENCH_ERR_LINE_TOO_LONG:
		return "line-too-long";
	case KN_COMPAT_BENCH_ERR_PARSE:
		return "parse";
	case KN_COMPAT_BENCH_ERR_DUPLICATE_KEY:
		return "duplicate-key";
	case KN_COMPAT_BENCH_ERR_UNKNOWN_KEY:
		return "unknown-key";
	case KN_COMPAT_BENCH_ERR_MISSING_REQUIRED:
		return "missing-required";
	case KN_COMPAT_BENCH_ERR_INVALID_VALUE:
		return "invalid-value";
	case KN_COMPAT_BENCH_ERR_TOO_MANY:
		return "too-many";
	case KN_COMPAT_BENCH_ERR_REFERENCE:
		return "reference";
	case KN_COMPAT_BENCH_ERR_REPLAY:
		return "replay";
	case KN_COMPAT_BENCH_ERR_BUFFER:
		return "buffer";
	}

	return "unknown";
}

enum kn_compat_bench_error
kn_compat_bench_pack_parse_file(const char *path,
	struct kn_compat_bench_pack *pack,
	struct kn_compat_bench_error_info *info)
{
	FILE *fp;
	char text[KN_COMPAT_BENCH_TEXT_MAX + 1];
	size_t len;
	int ch;
	enum kn_compat_bench_error rc;

	if (path == NULL || pack == NULL)
		return KN_COMPAT_BENCH_ERR_INVALID_ARGUMENT;

	fp = fopen(path, "r");
	if (fp == NULL) {
		error_set(info, KN_COMPAT_BENCH_ERR_IO, 0, "open failed");
		return KN_COMPAT_BENCH_ERR_IO;
	}
	len = 0;
	while ((ch = fgetc(fp)) != EOF) {
		if (len >= KN_COMPAT_BENCH_TEXT_MAX) {
			(void)fclose(fp);
			error_set(info, KN_COMPAT_BENCH_ERR_INVALID_VALUE, 0,
			    "manifest too large");
			return KN_COMPAT_BENCH_ERR_INVALID_VALUE;
		}
		text[len++] = (char)ch;
	}
	if (ferror(fp)) {
		(void)fclose(fp);
		error_set(info, KN_COMPAT_BENCH_ERR_IO, 0, "read failed");
		return KN_COMPAT_BENCH_ERR_IO;
	}
	(void)fclose(fp);
	text[len] = '\0';

	rc = kn_compat_bench_pack_parse_text(text, pack, info);
	if (rc != KN_COMPAT_BENCH_OK)
		return rc;
	(void)snprintf(pack->manifest_path, sizeof(pack->manifest_path),
	    "%s", path);
	set_base_dir(pack, path);
	return KN_COMPAT_BENCH_OK;
}

enum kn_compat_bench_error
kn_compat_bench_pack_parse_text(const char *text,
	struct kn_compat_bench_pack *pack,
	struct kn_compat_bench_error_info *info)
{
	char line[KN_COMPAT_BENCH_LINE_MAX];
	char *key;
	char *value;
	size_t i;
	size_t line_len;
	size_t line_no;
	uint32_t seen;
	enum kn_compat_bench_error rc;

	if (text == NULL || pack == NULL)
		return KN_COMPAT_BENCH_ERR_INVALID_ARGUMENT;

	kn_compat_bench_pack_clear(pack);
	if (info != NULL)
		memset(info, 0, sizeof(*info));
	line_no = 1;
	line_len = 0;
	seen = 0;

	for (i = 0;; i++) {
		if (text[i] != '\0' && text[i] != '\n') {
			if (line_len + 1 >= sizeof(line)) {
				error_set(info,
				    KN_COMPAT_BENCH_ERR_LINE_TOO_LONG,
				    line_no, "line too long");
				return KN_COMPAT_BENCH_ERR_LINE_TOO_LONG;
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
				error_set(info, KN_COMPAT_BENCH_ERR_PARSE,
				    line_no, "missing value");
				return KN_COMPAT_BENCH_ERR_PARSE;
			}
			*value++ = '\0';
			value = trim(value);
			if (value[0] == '\0') {
				error_set(info, KN_COMPAT_BENCH_ERR_PARSE,
				    line_no, "missing value");
				return KN_COMPAT_BENCH_ERR_PARSE;
			}
			rc = field_set(pack, &seen, key, value, line_no,
			    info);
			if (rc != KN_COMPAT_BENCH_OK)
				return rc;
		}

		if (text[i] == '\0')
			break;
		line_no++;
		line_len = 0;
	}

	return require_fields(pack, seen, info);
}

enum kn_compat_bench_error
kn_compat_bench_pack_validate_refs(const struct kn_compat_bench_pack *pack,
	struct kn_compat_bench_error_info *info)
{
	struct kn_compat_packet_capture capture;
	struct kn_compat_packet_capture_error_info capture_error;
	char path[KN_COMPAT_BENCH_FIELD_MAX * 2];
	size_t i;

	if (pack == NULL)
		return KN_COMPAT_BENCH_ERR_INVALID_ARGUMENT;

	for (i = 0; i < pack->fixture_count; i++) {
		if (kn_compat_bench_pack_join_path(pack,
		    pack->fixtures[i].path, path, sizeof(path)) !=
		    KN_COMPAT_BENCH_OK)
			goto reference;
		memset(&capture_error, 0, sizeof(capture_error));
		if (kn_compat_packet_capture_parse_file(path, &capture,
		    &capture_error) != KN_COMPAT_PACKET_CAPTURE_OK)
			goto reference;
	}

	return KN_COMPAT_BENCH_OK;

reference:
	error_set(info, KN_COMPAT_BENCH_ERR_REFERENCE, 0,
	    "invalid referenced capture");
	return KN_COMPAT_BENCH_ERR_REFERENCE;
}

enum kn_compat_bench_error
kn_compat_bench_pack_join_path(const struct kn_compat_bench_pack *pack,
	const char *rel, char *buf, size_t bufsiz)
{
	int needed;

	if (pack == NULL || rel == NULL || buf == NULL || bufsiz == 0)
		return KN_COMPAT_BENCH_ERR_INVALID_ARGUMENT;
	if (unsafe_ref(rel) != 0)
		return KN_COMPAT_BENCH_ERR_INVALID_VALUE;
	if (pack->base_dir[0] == '\0' || strcmp(pack->base_dir, ".") == 0)
		needed = snprintf(buf, bufsiz, "%s", rel);
	else
		needed = snprintf(buf, bufsiz, "%s/%s", pack->base_dir, rel);

	return needed >= 0 && (size_t)needed < bufsiz ?
	    KN_COMPAT_BENCH_OK : KN_COMPAT_BENCH_ERR_INVALID_VALUE;
}

enum kn_compat_bench_error
kn_compat_bench_pack_coverage(const struct kn_compat_bench_pack *pack,
	struct kn_compat_bench_coverage *coverage)
{
	struct kn_compat_packet_capture capture;
	struct kn_compat_packet_decode decode;
	char path[KN_COMPAT_BENCH_FIELD_MAX * 2];
	size_t i;
	uint8_t fx25;

	if (pack == NULL || coverage == NULL)
		return KN_COMPAT_BENCH_ERR_INVALID_ARGUMENT;

	memset(coverage, 0, sizeof(*coverage));
	coverage->fixture_count = pack->fixture_count;
	for (i = 0; i < pack->fixture_count; i++) {
		if (kn_compat_bench_pack_join_path(pack,
		    pack->fixtures[i].path, path, sizeof(path)) !=
		    KN_COMPAT_BENCH_OK)
			return KN_COMPAT_BENCH_ERR_REFERENCE;
		if (kn_compat_packet_capture_parse_file(path, &capture,
		    NULL) != KN_COMPAT_PACKET_CAPTURE_OK)
			return KN_COMPAT_BENCH_ERR_REFERENCE;
		fx25 = planned_placeholder(pack->fixtures[i].path);
		if (fx25 == 0 && capture_decode(&capture, &decode) !=
		    KN_COMPAT_BENCH_OK)
			return KN_COMPAT_BENCH_ERR_REPLAY;
		coverage_add(coverage, &capture, &decode, fx25);
	}

	return KN_COMPAT_BENCH_OK;
}

enum kn_compat_bench_error
kn_compat_bench_pack_coverage_report(
	const struct kn_compat_bench_pack *pack, char *buf, size_t bufsiz)
{
	struct kn_compat_bench_coverage coverage;
	int needed;

	if (pack == NULL || buf == NULL || bufsiz == 0)
		return KN_COMPAT_BENCH_ERR_INVALID_ARGUMENT;
	if (kn_compat_bench_pack_coverage(pack, &coverage) !=
	    KN_COMPAT_BENCH_OK)
		return KN_COMPAT_BENCH_ERR_REPLAY;

	needed = snprintf(buf, bufsiz,
	    "BENCH-COVERAGE name=%s fixtures=%llu kiss=%llu raw_ax25=%llu "
	    "ui=%llu setup=%llu supervisory=%llu disconnect=%llu "
	    "fx25_placeholders=%llu\n",
	    pack->name, (unsigned long long)coverage.fixture_count,
	    (unsigned long long)coverage.kiss_count,
	    (unsigned long long)coverage.raw_ax25_count,
	    (unsigned long long)coverage.ui_count,
	    (unsigned long long)coverage.setup_count,
	    (unsigned long long)coverage.supervisory_count,
	    (unsigned long long)coverage.disconnect_count,
	    (unsigned long long)coverage.fx25_placeholder_count);

	return needed >= 0 && (size_t)needed < bufsiz ?
	    KN_COMPAT_BENCH_OK : KN_COMPAT_BENCH_ERR_BUFFER;
}

enum kn_compat_bench_error
kn_compat_bench_pack_report(const struct kn_compat_bench_pack *pack,
	char *buf, size_t bufsiz)
{
	size_t off;
	size_t i;
	int needed;

	if (pack == NULL || buf == NULL || bufsiz == 0)
		return KN_COMPAT_BENCH_ERR_INVALID_ARGUMENT;

	needed = snprintf(buf, bufsiz,
	    "BENCH-PACK name=%s type=%s source=%s clean_room=%s "
	    "source_code_used=%s hardware_required=%s "
	    "transmit_required=%s fixtures=%llu\n",
	    pack->name, pack->type, pack->source,
	    pack->clean_room != 0 ? "true" : "false",
	    pack->source_code_used != 0 ? "true" : "false",
	    pack->hardware_required != 0 ? "true" : "false",
	    pack->transmit_required != 0 ? "true" : "false",
	    (unsigned long long)pack->fixture_count);
	if (needed < 0 || (size_t)needed >= bufsiz)
		return KN_COMPAT_BENCH_ERR_BUFFER;
	off = (size_t)needed;
	for (i = 0; i < pack->fixture_count; i++) {
		needed = snprintf(buf + off, bufsiz - off,
		    "BENCH FIXTURE path=%s planned=%s\n",
		    pack->fixtures[i].path,
		    planned_placeholder(pack->fixtures[i].path) != 0 ?
		    "true" : "false");
		if (needed < 0 || (size_t)needed >= bufsiz - off)
			return KN_COMPAT_BENCH_ERR_BUFFER;
		off += (size_t)needed;
	}

	return KN_COMPAT_BENCH_OK;
}

enum kn_compat_bench_error
kn_compat_bench_pack_replay_report(
	const struct kn_compat_bench_pack *pack, char *buf, size_t bufsiz)
{
	struct kn_compat_packet_capture capture;
	struct kn_compat_packet_decode decode;
	char path[KN_COMPAT_BENCH_FIELD_MAX * 2];
	size_t off;
	size_t i;
	int needed;

	if (pack == NULL || buf == NULL || bufsiz == 0)
		return KN_COMPAT_BENCH_ERR_INVALID_ARGUMENT;

	needed = snprintf(buf, bufsiz, "BENCH-REPLAY name=%s fixtures=%llu\n",
	    pack->name, (unsigned long long)pack->fixture_count);
	if (needed < 0 || (size_t)needed >= bufsiz)
		return KN_COMPAT_BENCH_ERR_BUFFER;
	off = (size_t)needed;
	for (i = 0; i < pack->fixture_count; i++) {
		if (planned_placeholder(pack->fixtures[i].path) != 0) {
			needed = snprintf(buf + off, bufsiz - off,
			    "BENCH-SKIP fixture=%s reason=fx25-planned\n",
			    pack->fixtures[i].path);
			if (needed < 0 || (size_t)needed >= bufsiz - off)
				return KN_COMPAT_BENCH_ERR_BUFFER;
			off += (size_t)needed;
			continue;
		}
		if (kn_compat_bench_pack_join_path(pack,
		    pack->fixtures[i].path, path, sizeof(path)) !=
		    KN_COMPAT_BENCH_OK)
			return KN_COMPAT_BENCH_ERR_REFERENCE;
		if (kn_compat_packet_capture_parse_file(path, &capture,
		    NULL) != KN_COMPAT_PACKET_CAPTURE_OK)
			return KN_COMPAT_BENCH_ERR_REFERENCE;
		if (capture_decode(&capture, &decode) != KN_COMPAT_BENCH_OK ||
		    decode.passed == 0)
			return KN_COMPAT_BENCH_ERR_REPLAY;
		needed = snprintf(buf + off, bufsiz - off,
		    "BENCH-PASS fixture=%s method=%s kind=%s source=%s "
		    "destination=%s\n",
		    pack->fixtures[i].path,
		    kn_compat_packet_method_name(capture.method), decode.kind,
		    decode.source, decode.destination);
		if (needed < 0 || (size_t)needed >= bufsiz - off)
			return KN_COMPAT_BENCH_ERR_BUFFER;
		off += (size_t)needed;
	}

	return KN_COMPAT_BENCH_OK;
}

static enum kn_compat_bench_error
bool_parse(const char *value, uint8_t *out)
{
	if (strcmp(value, "true") == 0)
		*out = 1;
	else if (strcmp(value, "false") == 0)
		*out = 0;
	else
		return KN_COMPAT_BENCH_ERR_INVALID_VALUE;

	return KN_COMPAT_BENCH_OK;
}

static enum kn_compat_bench_error
capture_decode(const struct kn_compat_packet_capture *capture,
	struct kn_compat_packet_decode *decode)
{
	if (capture->method == KN_COMPAT_PACKET_METHOD_KISS) {
		return kn_compat_kiss_capture_decode(capture, decode) ==
		    KN_COMPAT_KISS_CAPTURE_OK ? KN_COMPAT_BENCH_OK :
		    KN_COMPAT_BENCH_ERR_REPLAY;
	}
	if (capture->method == KN_COMPAT_PACKET_METHOD_AXIP ||
	    capture->method == KN_COMPAT_PACKET_METHOD_AXUDP) {
		return kn_compat_axip_capture_decode(capture, decode) ==
		    KN_COMPAT_AXIP_CAPTURE_OK ? KN_COMPAT_BENCH_OK :
		    KN_COMPAT_BENCH_ERR_REPLAY;
	}

	return KN_COMPAT_BENCH_ERR_REPLAY;
}

static void
coverage_add(struct kn_compat_bench_coverage *coverage,
	const struct kn_compat_packet_capture *capture,
	const struct kn_compat_packet_decode *decode, uint8_t fx25)
{
	if (fx25 != 0) {
		coverage->fx25_placeholder_count++;
		return;
	}
	if (capture->method == KN_COMPAT_PACKET_METHOD_KISS)
		coverage->kiss_count++;
	if (capture->method == KN_COMPAT_PACKET_METHOD_AXIP ||
	    capture->method == KN_COMPAT_PACKET_METHOD_AXUDP)
		coverage->raw_ax25_count++;
	if (strcmp(decode->kind, "UI") == 0)
		coverage->ui_count++;
	else if (strcmp(decode->kind, "SABM") == 0 ||
	    strcmp(decode->kind, "SABME") == 0)
		coverage->setup_count++;
	else if (strcmp(decode->kind, "RR") == 0 ||
	    strcmp(decode->kind, "RNR") == 0 ||
	    strcmp(decode->kind, "REJ") == 0)
		coverage->supervisory_count++;
	else if (strcmp(decode->kind, "DISC") == 0)
		coverage->disconnect_count++;
}

static void
error_set(struct kn_compat_bench_error_info *info,
	enum kn_compat_bench_error error, size_t line, const char *message)
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

static enum kn_compat_bench_error
field_set(struct kn_compat_bench_pack *pack, uint32_t *seen,
	const char *key, const char *value, size_t line,
	struct kn_compat_bench_error_info *info)
{
	char *dst;
	size_t dst_len;
	uint32_t flag;
	enum kn_compat_bench_error rc;

	dst = NULL;
	dst_len = 0;
	flag = 0;
	if (strcmp(key, "name") == 0) {
		dst = pack->name;
		dst_len = sizeof(pack->name);
		flag = BENCH_FIELD_NAME;
	} else if (strcmp(key, "type") == 0) {
		dst = pack->type;
		dst_len = sizeof(pack->type);
		flag = BENCH_FIELD_TYPE;
	} else if (strcmp(key, "source") == 0) {
		dst = pack->source;
		dst_len = sizeof(pack->source);
		flag = BENCH_FIELD_SOURCE;
	} else if (strcmp(key, "clean-room") == 0) {
		if (field_seen(seen, BENCH_FIELD_CLEAN_ROOM) != 0)
			goto duplicate;
		rc = bool_parse(value, &pack->clean_room);
		if (rc != KN_COMPAT_BENCH_OK || pack->clean_room == 0)
			goto invalid;
		return KN_COMPAT_BENCH_OK;
	} else if (strcmp(key, "source-code-used") == 0) {
		if (field_seen(seen, BENCH_FIELD_SOURCE_CODE_USED) != 0)
			goto duplicate;
		rc = bool_parse(value, &pack->source_code_used);
		if (rc != KN_COMPAT_BENCH_OK ||
		    pack->source_code_used != 0)
			goto invalid;
		return KN_COMPAT_BENCH_OK;
	} else if (strcmp(key, "hardware-required") == 0) {
		if (field_seen(seen, BENCH_FIELD_HARDWARE) != 0)
			goto duplicate;
		rc = bool_parse(value, &pack->hardware_required);
		if (rc != KN_COMPAT_BENCH_OK ||
		    pack->hardware_required != 0)
			goto invalid;
		return KN_COMPAT_BENCH_OK;
	} else if (strcmp(key, "transmit-required") == 0) {
		if (field_seen(seen, BENCH_FIELD_TRANSMIT) != 0)
			goto duplicate;
		rc = bool_parse(value, &pack->transmit_required);
		if (rc != KN_COMPAT_BENCH_OK ||
		    pack->transmit_required != 0)
			goto invalid;
		return KN_COMPAT_BENCH_OK;
	} else if (strcmp(key, "fixture") == 0) {
		if (list_add(pack->fixtures, &pack->fixture_count, value) !=
		    KN_COMPAT_BENCH_OK)
			goto invalid;
		return KN_COMPAT_BENCH_OK;
	} else {
		error_set(info, KN_COMPAT_BENCH_ERR_UNKNOWN_KEY, line, key);
		return KN_COMPAT_BENCH_ERR_UNKNOWN_KEY;
	}

	if (field_seen(seen, flag) != 0)
		goto duplicate;
	if (strlen(value) >= dst_len)
		goto invalid;
	(void)snprintf(dst, dst_len, "%s", value);
	return KN_COMPAT_BENCH_OK;

duplicate:
	error_set(info, KN_COMPAT_BENCH_ERR_DUPLICATE_KEY, line, key);
	return KN_COMPAT_BENCH_ERR_DUPLICATE_KEY;

invalid:
	error_set(info, KN_COMPAT_BENCH_ERR_INVALID_VALUE, line, key);
	return KN_COMPAT_BENCH_ERR_INVALID_VALUE;
}

static uint8_t
field_seen(uint32_t *seen, uint32_t flag)
{
	if ((*seen & flag) != 0)
		return 1;
	*seen |= flag;
	return 0;
}

static uint8_t
planned_placeholder(const char *path)
{
	if (path == NULL)
		return 0;
	if (strncmp(path, "fx25-", 5) == 0)
		return 1;
	if (strstr(path, "sequence") != NULL)
		return 1;

	return 0;
}

static enum kn_compat_bench_error
list_add(struct kn_compat_bench_entry *entries, size_t *count,
	const char *value)
{
	size_t i;

	if (*count >= KN_COMPAT_BENCH_FIXTURE_MAX)
		return KN_COMPAT_BENCH_ERR_TOO_MANY;
	if (unsafe_ref(value) != 0 ||
	    strlen(value) >= sizeof(entries[*count].path))
		return KN_COMPAT_BENCH_ERR_INVALID_VALUE;
	for (i = 0; i < *count; i++) {
		if (strcmp(entries[i].path, value) == 0)
			return KN_COMPAT_BENCH_ERR_DUPLICATE_KEY;
	}
	(void)snprintf(entries[*count].path, sizeof(entries[*count].path),
	    "%s", value);
	(*count)++;
	return KN_COMPAT_BENCH_OK;
}

static enum kn_compat_bench_error
require_fields(const struct kn_compat_bench_pack *pack, uint32_t seen,
	struct kn_compat_bench_error_info *info)
{
	const uint32_t required = BENCH_FIELD_NAME | BENCH_FIELD_TYPE |
	    BENCH_FIELD_SOURCE | BENCH_FIELD_CLEAN_ROOM |
	    BENCH_FIELD_SOURCE_CODE_USED | BENCH_FIELD_HARDWARE |
	    BENCH_FIELD_TRANSMIT;

	if ((seen & required) != required) {
		error_set(info, KN_COMPAT_BENCH_ERR_MISSING_REQUIRED, 0,
		    "missing required field");
		return KN_COMPAT_BENCH_ERR_MISSING_REQUIRED;
	}
	if (pack->fixture_count == 0) {
		error_set(info, KN_COMPAT_BENCH_ERR_MISSING_REQUIRED, 0,
		    "missing fixtures");
		return KN_COMPAT_BENCH_ERR_MISSING_REQUIRED;
	}

	return KN_COMPAT_BENCH_OK;
}

static void
set_base_dir(struct kn_compat_bench_pack *pack, const char *path)
{
	const char *slash;
	size_t len;

	slash = strrchr(path, '/');
	if (slash == NULL) {
		(void)snprintf(pack->base_dir, sizeof(pack->base_dir), ".");
		return;
	}
	len = (size_t)(slash - path);
	if (len == 0)
		len = 1;
	if (len >= sizeof(pack->base_dir))
		len = sizeof(pack->base_dir) - 1;
	memcpy(pack->base_dir, path, len);
	pack->base_dir[len] = '\0';
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

static uint8_t
unsafe_ref(const char *path)
{
	if (path == NULL || path[0] == '\0' || path[0] == '/')
		return 1;
	if (strstr(path, "..") != NULL || strchr(path, '\\') != NULL)
		return 1;
	return 0;
}
