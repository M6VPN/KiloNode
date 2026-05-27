/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/compat/manual_capture_workspace.c */

#include <sys/types.h>
#include <sys/stat.h>

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "kilonode/manual_capture_workspace.h"

static void error_set(struct kn_manual_capture_error_info *,
	enum kn_manual_capture_error, size_t, const char *);
static enum kn_manual_capture_error mkdir_if_needed(const char *);
static enum kn_manual_capture_error set_field(
	struct kn_manual_capture_workspace *, const char *, const char *);
static char *trim(char *);

void
kn_manual_capture_workspace_clear(struct kn_manual_capture_workspace *workspace)
{
	if (workspace == NULL)
		return;

	memset(workspace, 0, sizeof(*workspace));
}

const char *
kn_manual_capture_error_name(enum kn_manual_capture_error error)
{
	switch (error) {
	case KN_MANUAL_CAPTURE_OK:
		return "ok";
	case KN_MANUAL_CAPTURE_ERR_INVALID_ARGUMENT:
		return "invalid-argument";
	case KN_MANUAL_CAPTURE_ERR_IO:
		return "io";
	case KN_MANUAL_CAPTURE_ERR_PARSE:
		return "parse";
	case KN_MANUAL_CAPTURE_ERR_LINE_TOO_LONG:
		return "line-too-long";
	case KN_MANUAL_CAPTURE_ERR_UNKNOWN_KEY:
		return "unknown-key";
	case KN_MANUAL_CAPTURE_ERR_MISSING_REQUIRED:
		return "missing-required";
	case KN_MANUAL_CAPTURE_ERR_INVALID_VALUE:
		return "invalid-value";
	case KN_MANUAL_CAPTURE_ERR_UNSAFE_PATH:
		return "unsafe-path";
	case KN_MANUAL_CAPTURE_ERR_TOO_MANY:
		return "too-many";
	case KN_MANUAL_CAPTURE_ERR_CAPTURE:
		return "capture";
	case KN_MANUAL_CAPTURE_ERR_REPLAY:
		return "replay";
	case KN_MANUAL_CAPTURE_ERR_BUFFER:
		return "buffer";
	}

	return "unknown";
}

enum kn_manual_capture_error
kn_manual_capture_workspace_init(const char *root,
	struct kn_manual_capture_error_info *info)
{
	char path[KN_MANUAL_CAPTURE_PATH_MAX];
	char manifest[KN_MANUAL_CAPTURE_PATH_MAX];
	FILE *fp;
	time_t now;
	struct tm *tm;
	char created[32];
	size_t i;
	const char *dirs[] = {
		"incoming", "imported", "reports", "replay", "index", "notes"
	};

	if (root == NULL || kn_manual_capture_path_safe(root) == 0) {
		error_set(info, KN_MANUAL_CAPTURE_ERR_UNSAFE_PATH, 0, root);
		return KN_MANUAL_CAPTURE_ERR_UNSAFE_PATH;
	}
	if (mkdir_if_needed(root) != KN_MANUAL_CAPTURE_OK) {
		error_set(info, KN_MANUAL_CAPTURE_ERR_IO, 0, root);
		return KN_MANUAL_CAPTURE_ERR_IO;
	}
	for (i = 0; i < sizeof(dirs) / sizeof(dirs[0]); i++) {
		if (kn_manual_capture_workspace_join(root, dirs[i], path,
		    sizeof(path)) != KN_MANUAL_CAPTURE_OK ||
		    mkdir_if_needed(path) != KN_MANUAL_CAPTURE_OK) {
			error_set(info, KN_MANUAL_CAPTURE_ERR_IO, 0, dirs[i]);
			return KN_MANUAL_CAPTURE_ERR_IO;
		}
	}
	if (kn_manual_capture_workspace_join(root, "workspace.manifest",
	    manifest, sizeof(manifest)) != KN_MANUAL_CAPTURE_OK)
		return KN_MANUAL_CAPTURE_ERR_BUFFER;

	now = time(NULL);
	tm = gmtime(&now);
	if (tm == NULL || strftime(created, sizeof(created), "%Y-%m-%d",
	    tm) == 0)
		(void)snprintf(created, sizeof(created), "2026-05-27");
	fp = fopen(manifest, "w");
	if (fp == NULL) {
		error_set(info, KN_MANUAL_CAPTURE_ERR_IO, 0, manifest);
		return KN_MANUAL_CAPTURE_ERR_IO;
	}
	(void)fprintf(fp,
	    "# KiloNode manual capture workspace v1\n"
	    "name manual-rx-captures\n"
	    "type manual-capture-workspace\n"
	    "created %s\n"
	    "clean-room true\n"
	    "source-code-used false\n"
	    "hardware-required optional\n"
	    "transmit-required false\n"
	    "notes User-provided receive-only captures.\n", created);
	(void)fclose(fp);
	if (kn_manual_capture_workspace_join(root, "index/captures.index",
	    path, sizeof(path)) != KN_MANUAL_CAPTURE_OK)
		return KN_MANUAL_CAPTURE_ERR_BUFFER;
	fp = fopen(path, "a");
	if (fp == NULL) {
		error_set(info, KN_MANUAL_CAPTURE_ERR_IO, 0, path);
		return KN_MANUAL_CAPTURE_ERR_IO;
	}
	(void)fclose(fp);

	return KN_MANUAL_CAPTURE_OK;
}

enum kn_manual_capture_error
kn_manual_capture_workspace_check(const char *root,
	struct kn_manual_capture_workspace *workspace,
	struct kn_manual_capture_error_info *info)
{
	char path[KN_MANUAL_CAPTURE_PATH_MAX];
	enum kn_manual_capture_error rc;

	if (root == NULL || kn_manual_capture_path_safe(root) == 0) {
		error_set(info, KN_MANUAL_CAPTURE_ERR_UNSAFE_PATH, 0, root);
		return KN_MANUAL_CAPTURE_ERR_UNSAFE_PATH;
	}
	if (kn_manual_capture_workspace_join(root, "workspace.manifest", path,
	    sizeof(path)) != KN_MANUAL_CAPTURE_OK)
		return KN_MANUAL_CAPTURE_ERR_BUFFER;
	rc = kn_manual_capture_workspace_parse_file(path, workspace, info);
	if (rc != KN_MANUAL_CAPTURE_OK)
		return rc;
	if (workspace != NULL)
		(void)snprintf(workspace->root, sizeof(workspace->root), "%s",
		    root);

	return KN_MANUAL_CAPTURE_OK;
}

enum kn_manual_capture_error
kn_manual_capture_workspace_parse_file(const char *path,
	struct kn_manual_capture_workspace *workspace,
	struct kn_manual_capture_error_info *info)
{
	FILE *fp;
	char text[KN_MANUAL_CAPTURE_TEXT_MAX + 1];
	size_t len;
	int ch;

	if (path == NULL || workspace == NULL)
		return KN_MANUAL_CAPTURE_ERR_INVALID_ARGUMENT;
	fp = fopen(path, "r");
	if (fp == NULL) {
		error_set(info, KN_MANUAL_CAPTURE_ERR_IO, 0, path);
		return KN_MANUAL_CAPTURE_ERR_IO;
	}
	len = 0;
	while ((ch = fgetc(fp)) != EOF) {
		if (len >= KN_MANUAL_CAPTURE_TEXT_MAX) {
			(void)fclose(fp);
			error_set(info, KN_MANUAL_CAPTURE_ERR_INVALID_VALUE, 0,
			    "manifest too large");
			return KN_MANUAL_CAPTURE_ERR_INVALID_VALUE;
		}
		text[len++] = (char)ch;
	}
	if (ferror(fp)) {
		(void)fclose(fp);
		return KN_MANUAL_CAPTURE_ERR_IO;
	}
	(void)fclose(fp);
	text[len] = '\0';

	return kn_manual_capture_workspace_parse_text(text, workspace, info);
}

enum kn_manual_capture_error
kn_manual_capture_workspace_parse_text(const char *text,
	struct kn_manual_capture_workspace *workspace,
	struct kn_manual_capture_error_info *info)
{
	char line[KN_MANUAL_CAPTURE_LINE_MAX];
	char *key;
	char *value;
	size_t i;
	size_t line_len;
	size_t line_no;
	enum kn_manual_capture_error rc;

	if (text == NULL || workspace == NULL)
		return KN_MANUAL_CAPTURE_ERR_INVALID_ARGUMENT;
	kn_manual_capture_workspace_clear(workspace);
	if (info != NULL)
		memset(info, 0, sizeof(*info));
	line_len = 0;
	line_no = 1;
	for (i = 0;; i++) {
		if (text[i] != '\0' && text[i] != '\n') {
			if (line_len + 1 >= sizeof(line)) {
				error_set(info,
				    KN_MANUAL_CAPTURE_ERR_LINE_TOO_LONG, line_no,
				    "line too long");
				return KN_MANUAL_CAPTURE_ERR_LINE_TOO_LONG;
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
				error_set(info, KN_MANUAL_CAPTURE_ERR_PARSE,
				    line_no, "missing value");
				return KN_MANUAL_CAPTURE_ERR_PARSE;
			}
			*value++ = '\0';
			value = trim(value);
			rc = set_field(workspace, key, value);
			if (rc != KN_MANUAL_CAPTURE_OK) {
				error_set(info, rc, line_no, key);
				return rc;
			}
		}
		if (text[i] == '\0')
			break;
		line_len = 0;
		line_no++;
	}
	if (workspace->name[0] == '\0' || workspace->type[0] == '\0' ||
	    workspace->created[0] == '\0' || workspace->clean_room == 0)
		return KN_MANUAL_CAPTURE_ERR_MISSING_REQUIRED;
	if (strcmp(workspace->type, "manual-capture-workspace") != 0 ||
	    workspace->source_code_used != 0 ||
	    workspace->transmit_required != 0)
		return KN_MANUAL_CAPTURE_ERR_INVALID_VALUE;

	return KN_MANUAL_CAPTURE_OK;
}

enum kn_manual_capture_error
kn_manual_capture_workspace_join(const char *root, const char *rel,
	char *buf, size_t bufsiz)
{
	int needed;

	if (root == NULL || rel == NULL || buf == NULL || bufsiz == 0 ||
	    kn_manual_capture_relative_path_safe(rel) == 0)
		return KN_MANUAL_CAPTURE_ERR_INVALID_ARGUMENT;
	needed = snprintf(buf, bufsiz, "%s/%s", root, rel);
	if (needed < 0 || (size_t)needed >= bufsiz)
		return KN_MANUAL_CAPTURE_ERR_BUFFER;

	return KN_MANUAL_CAPTURE_OK;
}

int
kn_manual_capture_path_safe(const char *path)
{
	if (path == NULL || path[0] == '\0')
		return 0;
	if (strcmp(path, "/") == 0 || strcmp(path, ".") == 0 ||
	    strcmp(path, "..") == 0)
		return 0;
	if (strstr(path, "..") != NULL || strstr(path, "\\") != NULL)
		return 0;
	if (strstr(path, "/.git") != NULL)
		return 0;

	return 1;
}

int
kn_manual_capture_relative_path_safe(const char *path)
{
	if (path == NULL || path[0] == '\0' || path[0] == '/')
		return 0;
	if (strcmp(path, ".") == 0 || strcmp(path, "..") == 0 ||
	    strstr(path, "..") != NULL || strstr(path, "\\") != NULL)
		return 0;

	return 1;
}

static void
error_set(struct kn_manual_capture_error_info *info,
	enum kn_manual_capture_error error, size_t line, const char *message)
{
	if (info == NULL)
		return;
	info->error = error;
	info->line = line;
	(void)snprintf(info->message, sizeof(info->message), "%s",
	    message == NULL ? "-" : message);
}

static enum kn_manual_capture_error
mkdir_if_needed(const char *path)
{
	if (mkdir(path, 0700) == 0 || errno == EEXIST)
		return KN_MANUAL_CAPTURE_OK;

	return KN_MANUAL_CAPTURE_ERR_IO;
}

static enum kn_manual_capture_error
set_field(struct kn_manual_capture_workspace *workspace, const char *key,
	const char *value)
{
	if (strlen(value) >= KN_MANUAL_CAPTURE_FIELD_MAX)
		return KN_MANUAL_CAPTURE_ERR_INVALID_VALUE;
	if (strcmp(key, "name") == 0)
		(void)snprintf(workspace->name, sizeof(workspace->name), "%s",
		    value);
	else if (strcmp(key, "type") == 0)
		(void)snprintf(workspace->type, sizeof(workspace->type), "%s",
		    value);
	else if (strcmp(key, "created") == 0)
		(void)snprintf(workspace->created, sizeof(workspace->created),
		    "%s", value);
	else if (strcmp(key, "clean-room") == 0)
		workspace->clean_room = strcmp(value, "true") == 0;
	else if (strcmp(key, "source-code-used") == 0)
		workspace->source_code_used = strcmp(value, "true") == 0;
	else if (strcmp(key, "hardware-required") == 0)
		(void)snprintf(workspace->hardware_required,
		    sizeof(workspace->hardware_required), "%s", value);
	else if (strcmp(key, "transmit-required") == 0)
		workspace->transmit_required = strcmp(value, "true") == 0;
	else if (strcmp(key, "notes") == 0)
		(void)snprintf(workspace->notes, sizeof(workspace->notes),
		    "%s", value);
	else
		return KN_MANUAL_CAPTURE_ERR_UNKNOWN_KEY;

	return KN_MANUAL_CAPTURE_OK;
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
