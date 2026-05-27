/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/compat/bench_replay.c */

#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kilonode/bench_replay.h"

static void add_mismatch(struct kn_bench_diag_result *, const char *);
static void error_set(struct kn_bench_replay_error_info *,
	enum kn_bench_replay_error, size_t, const char *);
static enum kn_bench_replay_error expected_capture_add(
	struct kn_bench_expected_file *, const char *);
static struct kn_bench_expected_capture *expected_find(
	const struct kn_bench_expected_file *, const char *);
static enum kn_bench_replay_error expected_set(
	struct kn_bench_expected_capture *, const char *, const char *);
static const char *path_basename(const char *);
static enum kn_bench_replay_error parse_u64(const char *, uint64_t *);
static char *trim(char *);

const char *
kn_bench_replay_error_name(enum kn_bench_replay_error error)
{
	switch (error) {
	case KN_BENCH_REPLAY_OK:
		return "ok";
	case KN_BENCH_REPLAY_ERR_INVALID_ARGUMENT:
		return "invalid-argument";
	case KN_BENCH_REPLAY_ERR_IO:
		return "io";
	case KN_BENCH_REPLAY_ERR_PARSE:
		return "parse";
	case KN_BENCH_REPLAY_ERR_LINE_TOO_LONG:
		return "line-too-long";
	case KN_BENCH_REPLAY_ERR_UNKNOWN_KEY:
		return "unknown-key";
	case KN_BENCH_REPLAY_ERR_INVALID_VALUE:
		return "invalid-value";
	case KN_BENCH_REPLAY_ERR_TOO_MANY:
		return "too-many";
	case KN_BENCH_REPLAY_ERR_REPLAY:
		return "replay";
	case KN_BENCH_REPLAY_ERR_BUFFER:
		return "buffer";
	}

	return "unknown";
}

void
kn_bench_expected_file_clear(struct kn_bench_expected_file *expected)
{
	if (expected == NULL)
		return;

	memset(expected, 0, sizeof(*expected));
}

enum kn_bench_replay_error
kn_bench_expected_parse_file(const char *path,
	struct kn_bench_expected_file *expected,
	struct kn_bench_replay_error_info *info)
{
	FILE *fp;
	char text[KN_BENCH_EXPECT_TEXT_MAX + 1];
	size_t len;
	int ch;

	if (path == NULL || expected == NULL)
		return KN_BENCH_REPLAY_ERR_INVALID_ARGUMENT;

	fp = fopen(path, "r");
	if (fp == NULL) {
		error_set(info, KN_BENCH_REPLAY_ERR_IO, 0, "open failed");
		return KN_BENCH_REPLAY_ERR_IO;
	}
	len = 0;
	while ((ch = fgetc(fp)) != EOF) {
		if (len >= KN_BENCH_EXPECT_TEXT_MAX) {
			(void)fclose(fp);
			error_set(info, KN_BENCH_REPLAY_ERR_INVALID_VALUE, 0,
			    "expected file too large");
			return KN_BENCH_REPLAY_ERR_INVALID_VALUE;
		}
		text[len++] = (char)ch;
	}
	if (ferror(fp)) {
		(void)fclose(fp);
		error_set(info, KN_BENCH_REPLAY_ERR_IO, 0, "read failed");
		return KN_BENCH_REPLAY_ERR_IO;
	}
	(void)fclose(fp);
	text[len] = '\0';

	return kn_bench_expected_parse_text(text, expected, info);
}

enum kn_bench_replay_error
kn_bench_expected_parse_text(const char *text,
	struct kn_bench_expected_file *expected,
	struct kn_bench_replay_error_info *info)
{
	char line[KN_BENCH_EXPECT_LINE_MAX];
	char *key;
	char *value;
	size_t i;
	size_t line_len;
	size_t line_no;
	struct kn_bench_expected_capture *current;
	enum kn_bench_replay_error rc;

	if (text == NULL || expected == NULL)
		return KN_BENCH_REPLAY_ERR_INVALID_ARGUMENT;

	kn_bench_expected_file_clear(expected);
	if (info != NULL)
		memset(info, 0, sizeof(*info));
	line_no = 1;
	line_len = 0;
	current = NULL;

	for (i = 0;; i++) {
		if (text[i] != '\0' && text[i] != '\n') {
			if (line_len + 1 >= sizeof(line)) {
				error_set(info, KN_BENCH_REPLAY_ERR_LINE_TOO_LONG,
				    line_no, "line too long");
				return KN_BENCH_REPLAY_ERR_LINE_TOO_LONG;
			}
			line[line_len++] = text[i];
			continue;
		}
		line[line_len] = '\0';
		key = trim(line);
		if (key[0] != '\0' && key[0] != '#') {
			if (strncmp(key, "capture ", 8) == 0) {
				value = trim(key + 8);
				if (strlen(value) < 4 ||
				    strcmp(value + strlen(value) - 1, "{") != 0) {
					error_set(info, KN_BENCH_REPLAY_ERR_PARSE,
					    line_no, "capture block");
					return KN_BENCH_REPLAY_ERR_PARSE;
				}
				value[strlen(value) - 1] = '\0';
				value = trim(value);
				rc = expected_capture_add(expected, value);
				if (rc != KN_BENCH_REPLAY_OK) {
					error_set(info, rc, line_no, value);
					return rc;
				}
				current = &expected->captures[
				    expected->capture_count - 1];
			} else if (strcmp(key, "}") == 0) {
				current = NULL;
			} else {
				if (current == NULL) {
					error_set(info, KN_BENCH_REPLAY_ERR_PARSE,
					    line_no, "field outside capture");
					return KN_BENCH_REPLAY_ERR_PARSE;
				}
				value = key;
				while (*value != '\0' && *value != ' ' &&
				    *value != '\t')
					value++;
				if (*value == '\0') {
					error_set(info, KN_BENCH_REPLAY_ERR_PARSE,
					    line_no, "missing value");
					return KN_BENCH_REPLAY_ERR_PARSE;
				}
				*value++ = '\0';
				value = trim(value);
				rc = expected_set(current, key, value);
				if (rc != KN_BENCH_REPLAY_OK) {
					error_set(info, rc, line_no, key);
					return rc;
				}
			}
		}
		if (text[i] == '\0')
			break;
		line_no++;
		line_len = 0;
	}

	return current == NULL ? KN_BENCH_REPLAY_OK : KN_BENCH_REPLAY_ERR_PARSE;
}

enum kn_bench_replay_error
kn_bench_expected_check_result(const struct kn_bench_expected_file *expected,
	const struct kn_bench_diag_result *result)
{
	struct kn_bench_expected_capture *capture;

	if (expected == NULL || result == NULL)
		return KN_BENCH_REPLAY_ERR_INVALID_ARGUMENT;

	capture = expected_find(expected, path_basename(result->capture_path));
	if (capture == NULL)
		return KN_BENCH_REPLAY_OK;
	if (capture->has_frames_parsed != 0 &&
	    capture->frames_parsed != result->frames_parsed)
		goto mismatch;
	if (capture->has_connections_created != 0 &&
	    capture->connections_created != result->connections_created)
		goto mismatch;
	if (capture->has_final_connections != 0 &&
	    capture->final_connections != result->final_connections)
		goto mismatch;
	if (capture->has_state != 0 &&
	    strcmp(capture->state, result->final_state) != 0)
		goto mismatch;
	if (capture->has_tx_writes != 0 &&
	    capture->tx_writes != result->tx_writes_attempted)
		goto mismatch;
	if (capture->has_ui_ignored != 0 &&
	    capture->ui_ignored != result->ui_ignored)
		goto mismatch;
	if (capture->has_frame_plans_retained != 0 &&
	    capture->frame_plans_retained != result->frame_plans_retained)
		goto mismatch;

	return KN_BENCH_REPLAY_OK;

mismatch:
	return KN_BENCH_REPLAY_ERR_REPLAY;
}

enum kn_bench_replay_error
kn_bench_replay_pack_diagnostics(const struct kn_compat_bench_pack *pack,
	const struct kn_bench_expected_file *expected,
	struct kn_bench_pack_diag_result *pack_result)
{
	char path[KN_COMPAT_BENCH_FIELD_MAX * 2];
	size_t i;
	enum kn_bench_diag_replay_error replay_rc;
	uint8_t replay_failed;

	if (pack == NULL || pack_result == NULL)
		return KN_BENCH_REPLAY_ERR_INVALID_ARGUMENT;

	memset(pack_result, 0, sizeof(*pack_result));
	for (i = 0; i < pack->fixture_count; i++) {
		if (pack_result->result_count >=
		    sizeof(pack_result->results) / sizeof(pack_result->results[0]))
			return KN_BENCH_REPLAY_ERR_TOO_MANY;
		if (kn_compat_bench_pack_join_path(pack, pack->fixtures[i].path,
		    path, sizeof(path)) != KN_COMPAT_BENCH_OK)
			return KN_BENCH_REPLAY_ERR_REPLAY;
		replay_rc = kn_bench_diag_replay_capture(path,
		    &pack_result->results[pack_result->result_count]);
		replay_failed = replay_rc != KN_BENCH_DIAG_REPLAY_OK;
		if (expected != NULL &&
		    kn_bench_expected_check_result(expected,
		    &pack_result->results[pack_result->result_count]) !=
		    KN_BENCH_REPLAY_OK) {
			add_mismatch(
			    &pack_result->results[pack_result->result_count],
			    "expected");
			pack_result->results[pack_result->result_count].pass = 0;
		}
		if (pack_result->results[pack_result->result_count].unsupported)
			pack_result->unsupported_count++;
		else if (replay_failed != 0)
			pack_result->fail_count++;
		else if (pack_result->results[
		    pack_result->result_count].pass != 0)
			pack_result->pass_count++;
		else
			pack_result->fail_count++;
		pack_result->total_frames +=
		    pack_result->results[pack_result->result_count].frames_parsed;
		pack_result->total_tx_writes +=
		    pack_result->results[
		    pack_result->result_count].tx_writes_attempted;
		pack_result->result_count++;
	}

	return pack_result->fail_count == 0 ? KN_BENCH_REPLAY_OK :
	    KN_BENCH_REPLAY_ERR_REPLAY;
}

static void
add_mismatch(struct kn_bench_diag_result *result, const char *text)
{
	if (result == NULL ||
	    result->mismatch_count >= KN_BENCH_DIAG_MISMATCH_MAX)
		return;
	(void)snprintf(result->mismatches[result->mismatch_count].text,
	    sizeof(result->mismatches[result->mismatch_count].text), "%s",
	    text);
	result->mismatch_count++;
}

static void
error_set(struct kn_bench_replay_error_info *info,
	enum kn_bench_replay_error error, size_t line, const char *message)
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

static enum kn_bench_replay_error
expected_capture_add(struct kn_bench_expected_file *expected,
	const char *path)
{
	if (expected->capture_count >= KN_BENCH_EXPECT_CAPTURE_MAX)
		return KN_BENCH_REPLAY_ERR_TOO_MANY;
	if (path == NULL || path[0] == '\0' || path[0] == '/' ||
	    strstr(path, "..") != NULL || strchr(path, '\\') != NULL ||
	    strlen(path) >= sizeof(expected->captures[0].path))
		return KN_BENCH_REPLAY_ERR_INVALID_VALUE;
	if (expected_find(expected, path) != NULL)
		return KN_BENCH_REPLAY_ERR_INVALID_VALUE;
	(void)snprintf(expected->captures[expected->capture_count].path,
	    sizeof(expected->captures[expected->capture_count].path), "%s",
	    path);
	expected->capture_count++;
	return KN_BENCH_REPLAY_OK;
}

static struct kn_bench_expected_capture *
expected_find(const struct kn_bench_expected_file *expected, const char *path)
{
	size_t i;

	if (expected == NULL || path == NULL)
		return NULL;
	for (i = 0; i < expected->capture_count; i++) {
		if (strcmp(expected->captures[i].path, path) == 0)
			return (struct kn_bench_expected_capture *)
			    &expected->captures[i];
	}

	return NULL;
}

static enum kn_bench_replay_error
expected_set(struct kn_bench_expected_capture *capture, const char *key,
	const char *value)
{
	uint64_t parsed;

	if (strcmp(key, "expected-state") == 0) {
		if (strlen(value) >= sizeof(capture->state))
			return KN_BENCH_REPLAY_ERR_INVALID_VALUE;
		(void)snprintf(capture->state, sizeof(capture->state),
		    "%s", value);
		capture->has_state = 1;
		return KN_BENCH_REPLAY_OK;
	}
	if (parse_u64(value, &parsed) != KN_BENCH_REPLAY_OK)
		return KN_BENCH_REPLAY_ERR_INVALID_VALUE;
	if (strcmp(key, "frames-parsed") == 0) {
		capture->frames_parsed = parsed;
		capture->has_frames_parsed = 1;
	} else if (strcmp(key, "connections-created") == 0) {
		capture->connections_created = parsed;
		capture->has_connections_created = 1;
	} else if (strcmp(key, "final-connections") == 0) {
		capture->final_connections = parsed;
		capture->has_final_connections = 1;
	} else if (strcmp(key, "tx-writes") == 0) {
		capture->tx_writes = parsed;
		capture->has_tx_writes = 1;
	} else if (strcmp(key, "ui-ignored") == 0) {
		capture->ui_ignored = parsed;
		capture->has_ui_ignored = 1;
	} else if (strcmp(key, "frame-plans-retained") == 0) {
		capture->frame_plans_retained = parsed;
		capture->has_frame_plans_retained = 1;
	} else {
		return KN_BENCH_REPLAY_ERR_UNKNOWN_KEY;
	}

	return KN_BENCH_REPLAY_OK;
}

static const char *
path_basename(const char *path)
{
	const char *slash;

	if (path == NULL)
		return "";
	slash = strrchr(path, '/');
	return slash == NULL ? path : slash + 1;
}

static enum kn_bench_replay_error
parse_u64(const char *value, uint64_t *out)
{
	char *end;
	unsigned long long parsed;

	parsed = strtoull(value, &end, 10);
	if (value[0] == '\0' || *end != '\0')
		return KN_BENCH_REPLAY_ERR_INVALID_VALUE;
	*out = (uint64_t)parsed;
	return KN_BENCH_REPLAY_OK;
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
