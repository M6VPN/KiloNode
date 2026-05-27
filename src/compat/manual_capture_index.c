/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/compat/manual_capture_index.c */

#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kilonode/manual_capture_index.h"

static void error_set(struct kn_manual_capture_error_info *,
	enum kn_manual_capture_error, size_t, const char *);
static enum kn_manual_capture_error parse_entry(char *,
	struct kn_manual_capture_entry *);
static enum kn_manual_capture_error set_token(struct kn_manual_capture_entry *,
	const char *);
static char *trim(char *);

void
kn_manual_capture_index_clear(struct kn_manual_capture_index *index)
{
	if (index == NULL)
		return;
	memset(index, 0, sizeof(*index));
	index->next_id = 1;
}

const char *
kn_manual_capture_source_name(enum kn_manual_capture_source source)
{
	switch (source) {
	case KN_MANUAL_CAPTURE_SOURCE_MANUAL:
		return "manual";
	case KN_MANUAL_CAPTURE_SOURCE_SYNTHETIC:
		return "synthetic";
	case KN_MANUAL_CAPTURE_SOURCE_BLACK_BOX:
		return "black-box";
	}
	return "manual";
}

const char *
kn_manual_capture_status_name(enum kn_manual_capture_status status)
{
	switch (status) {
	case KN_MANUAL_CAPTURE_STATUS_UNCHECKED:
		return "unchecked";
	case KN_MANUAL_CAPTURE_STATUS_VALID:
		return "valid";
	case KN_MANUAL_CAPTURE_STATUS_INVALID:
		return "invalid";
	case KN_MANUAL_CAPTURE_STATUS_UNSUPPORTED:
		return "unsupported";
	}
	return "invalid";
}

const char *
kn_manual_capture_replay_status_name(
	enum kn_manual_capture_replay_status replay)
{
	switch (replay) {
	case KN_MANUAL_CAPTURE_REPLAY_NOT_RUN:
		return "not-run";
	case KN_MANUAL_CAPTURE_REPLAY_PASS:
		return "pass";
	case KN_MANUAL_CAPTURE_REPLAY_FAIL:
		return "fail";
	case KN_MANUAL_CAPTURE_REPLAY_UNSUPPORTED:
		return "unsupported";
	}
	return "fail";
}

enum kn_manual_capture_error
kn_manual_capture_source_from_text(const char *text,
	enum kn_manual_capture_source *source)
{
	if (text == NULL || source == NULL)
		return KN_MANUAL_CAPTURE_ERR_INVALID_ARGUMENT;
	if (strcmp(text, "manual") == 0)
		*source = KN_MANUAL_CAPTURE_SOURCE_MANUAL;
	else if (strcmp(text, "synthetic") == 0)
		*source = KN_MANUAL_CAPTURE_SOURCE_SYNTHETIC;
	else if (strcmp(text, "black-box") == 0)
		*source = KN_MANUAL_CAPTURE_SOURCE_BLACK_BOX;
	else
		return KN_MANUAL_CAPTURE_ERR_INVALID_VALUE;
	return KN_MANUAL_CAPTURE_OK;
}

enum kn_manual_capture_error
kn_manual_capture_status_from_text(const char *text,
	enum kn_manual_capture_status *status)
{
	if (text == NULL || status == NULL)
		return KN_MANUAL_CAPTURE_ERR_INVALID_ARGUMENT;
	if (strcmp(text, "unchecked") == 0)
		*status = KN_MANUAL_CAPTURE_STATUS_UNCHECKED;
	else if (strcmp(text, "valid") == 0)
		*status = KN_MANUAL_CAPTURE_STATUS_VALID;
	else if (strcmp(text, "invalid") == 0)
		*status = KN_MANUAL_CAPTURE_STATUS_INVALID;
	else if (strcmp(text, "unsupported") == 0)
		*status = KN_MANUAL_CAPTURE_STATUS_UNSUPPORTED;
	else
		return KN_MANUAL_CAPTURE_ERR_INVALID_VALUE;
	return KN_MANUAL_CAPTURE_OK;
}

enum kn_manual_capture_error
kn_manual_capture_replay_status_from_text(const char *text,
	enum kn_manual_capture_replay_status *replay)
{
	if (text == NULL || replay == NULL)
		return KN_MANUAL_CAPTURE_ERR_INVALID_ARGUMENT;
	if (strcmp(text, "not-run") == 0)
		*replay = KN_MANUAL_CAPTURE_REPLAY_NOT_RUN;
	else if (strcmp(text, "pass") == 0)
		*replay = KN_MANUAL_CAPTURE_REPLAY_PASS;
	else if (strcmp(text, "fail") == 0)
		*replay = KN_MANUAL_CAPTURE_REPLAY_FAIL;
	else if (strcmp(text, "unsupported") == 0)
		*replay = KN_MANUAL_CAPTURE_REPLAY_UNSUPPORTED;
	else
		return KN_MANUAL_CAPTURE_ERR_INVALID_VALUE;
	return KN_MANUAL_CAPTURE_OK;
}

enum kn_manual_capture_error
kn_manual_capture_index_load(const char *root,
	struct kn_manual_capture_index *index,
	struct kn_manual_capture_error_info *info)
{
	char path[KN_MANUAL_CAPTURE_PATH_MAX];
	FILE *fp;
	char line[KN_MANUAL_CAPTURE_LINE_MAX];
	size_t line_no;
	enum kn_manual_capture_error rc;
	uint32_t max_id;

	if (root == NULL || index == NULL)
		return KN_MANUAL_CAPTURE_ERR_INVALID_ARGUMENT;
	kn_manual_capture_index_clear(index);
	if (kn_manual_capture_workspace_join(root, "index/captures.index",
	    path, sizeof(path)) != KN_MANUAL_CAPTURE_OK)
		return KN_MANUAL_CAPTURE_ERR_BUFFER;
	fp = fopen(path, "r");
	if (fp == NULL)
		return KN_MANUAL_CAPTURE_OK;
	line_no = 0;
	max_id = 0;
	while (fgets(line, sizeof(line), fp) != NULL) {
		line_no++;
		if (strchr(line, '\n') == NULL && !feof(fp)) {
			(void)fclose(fp);
			error_set(info, KN_MANUAL_CAPTURE_ERR_LINE_TOO_LONG,
			    line_no, "index line too long");
			return KN_MANUAL_CAPTURE_ERR_LINE_TOO_LONG;
		}
		if (trim(line)[0] == '\0' || trim(line)[0] == '#')
			continue;
		if (index->entry_count >= KN_MANUAL_CAPTURE_INDEX_MAX) {
			(void)fclose(fp);
			return KN_MANUAL_CAPTURE_ERR_TOO_MANY;
		}
		rc = parse_entry(line, &index->entries[index->entry_count]);
		if (rc != KN_MANUAL_CAPTURE_OK) {
			(void)fclose(fp);
			error_set(info, rc, line_no, "index entry");
			return rc;
		}
		if (kn_manual_capture_index_find(index,
		    index->entries[index->entry_count].id) != NULL) {
			(void)fclose(fp);
			return KN_MANUAL_CAPTURE_ERR_INVALID_VALUE;
		}
		if (index->entries[index->entry_count].id > max_id)
			max_id = index->entries[index->entry_count].id;
		index->entry_count++;
	}
	(void)fclose(fp);
	index->next_id = max_id + 1;
	if (index->next_id == 0)
		index->next_id = 1;

	return KN_MANUAL_CAPTURE_OK;
}

enum kn_manual_capture_error
kn_manual_capture_index_save(const char *root,
	const struct kn_manual_capture_index *index,
	struct kn_manual_capture_error_info *info)
{
	char path[KN_MANUAL_CAPTURE_PATH_MAX];
	FILE *fp;
	size_t i;

	if (root == NULL || index == NULL)
		return KN_MANUAL_CAPTURE_ERR_INVALID_ARGUMENT;
	if (kn_manual_capture_workspace_join(root, "index/captures.index",
	    path, sizeof(path)) != KN_MANUAL_CAPTURE_OK)
		return KN_MANUAL_CAPTURE_ERR_BUFFER;
	fp = fopen(path, "w");
	if (fp == NULL) {
		error_set(info, KN_MANUAL_CAPTURE_ERR_IO, 0, path);
		return KN_MANUAL_CAPTURE_ERR_IO;
	}
	for (i = 0; i < index->entry_count; i++) {
		(void)fprintf(fp,
		    "capture id=%u file=%s method=%s source=%s status=%s "
		    "replay=%s added=%llu notes=%s\n",
		    index->entries[i].id, index->entries[i].file,
		    index->entries[i].method,
		    kn_manual_capture_source_name(index->entries[i].source),
		    kn_manual_capture_status_name(index->entries[i].status),
		    kn_manual_capture_replay_status_name(
		    index->entries[i].replay),
		    (unsigned long long)index->entries[i].added,
		    index->entries[i].notes[0] == '\0' ? "-" :
		    index->entries[i].notes);
	}
	(void)fclose(fp);
	return KN_MANUAL_CAPTURE_OK;
}

enum kn_manual_capture_error
kn_manual_capture_index_add(struct kn_manual_capture_index *index,
	const struct kn_manual_capture_entry *entry)
{
	struct kn_manual_capture_entry *dst;

	if (index == NULL || entry == NULL)
		return KN_MANUAL_CAPTURE_ERR_INVALID_ARGUMENT;
	if (index->entry_count >= KN_MANUAL_CAPTURE_INDEX_MAX)
		return KN_MANUAL_CAPTURE_ERR_TOO_MANY;
	if (entry->file[0] == '\0' ||
	    kn_manual_capture_relative_path_safe(entry->file) == 0)
		return KN_MANUAL_CAPTURE_ERR_UNSAFE_PATH;
	dst = &index->entries[index->entry_count];
	*dst = *entry;
	if (dst->id == 0)
		dst->id = index->next_id;
	if (kn_manual_capture_index_find(index, dst->id) != NULL)
		return KN_MANUAL_CAPTURE_ERR_INVALID_VALUE;
	index->entry_count++;
	if (dst->id >= index->next_id)
		index->next_id = dst->id + 1;
	return KN_MANUAL_CAPTURE_OK;
}

struct kn_manual_capture_entry *
kn_manual_capture_index_find(struct kn_manual_capture_index *index,
	uint32_t id)
{
	size_t i;

	if (index == NULL || id == 0)
		return NULL;
	for (i = 0; i < index->entry_count; i++) {
		if (index->entries[i].id == id)
			return &index->entries[i];
	}
	return NULL;
}

enum kn_manual_capture_error
kn_manual_capture_index_update_status(struct kn_manual_capture_index *index,
	uint32_t id, enum kn_manual_capture_status status)
{
	struct kn_manual_capture_entry *entry;

	entry = kn_manual_capture_index_find(index, id);
	if (entry == NULL)
		return KN_MANUAL_CAPTURE_ERR_INVALID_VALUE;
	entry->status = status;
	return KN_MANUAL_CAPTURE_OK;
}

enum kn_manual_capture_error
kn_manual_capture_index_update_replay(struct kn_manual_capture_index *index,
	uint32_t id, enum kn_manual_capture_replay_status replay)
{
	struct kn_manual_capture_entry *entry;

	entry = kn_manual_capture_index_find(index, id);
	if (entry == NULL)
		return KN_MANUAL_CAPTURE_ERR_INVALID_VALUE;
	entry->replay = replay;
	return KN_MANUAL_CAPTURE_OK;
}

enum kn_manual_capture_error
kn_manual_capture_index_validate_refs(const char *root,
	const struct kn_manual_capture_index *index,
	struct kn_manual_capture_error_info *info)
{
	char path[KN_MANUAL_CAPTURE_PATH_MAX];
	FILE *fp;
	size_t i;

	if (root == NULL || index == NULL)
		return KN_MANUAL_CAPTURE_ERR_INVALID_ARGUMENT;
	for (i = 0; i < index->entry_count; i++) {
		if (kn_manual_capture_workspace_join(root,
		    index->entries[i].file, path, sizeof(path)) !=
		    KN_MANUAL_CAPTURE_OK)
			return KN_MANUAL_CAPTURE_ERR_UNSAFE_PATH;
		fp = fopen(path, "r");
		if (fp == NULL) {
			error_set(info, KN_MANUAL_CAPTURE_ERR_IO, i + 1, path);
			return KN_MANUAL_CAPTURE_ERR_IO;
		}
		(void)fclose(fp);
	}
	return KN_MANUAL_CAPTURE_OK;
}

enum kn_manual_capture_error
kn_manual_capture_index_format(const struct kn_manual_capture_index *index,
	char *buf, size_t bufsiz)
{
	size_t off;
	size_t i;
	int needed;

	if (index == NULL || buf == NULL || bufsiz == 0)
		return KN_MANUAL_CAPTURE_ERR_INVALID_ARGUMENT;
	needed = snprintf(buf, bufsiz, "MANUAL-LIST count=%llu\n",
	    (unsigned long long)index->entry_count);
	if (needed < 0 || (size_t)needed >= bufsiz)
		return KN_MANUAL_CAPTURE_ERR_BUFFER;
	off = (size_t)needed;
	for (i = 0; i < index->entry_count; i++) {
		needed = snprintf(buf + off, bufsiz - off,
		    "MANUAL-CAPTURE id=%u file=%s method=%s source=%s "
		    "status=%s replay=%s added=%llu notes=%s\n",
		    index->entries[i].id, index->entries[i].file,
		    index->entries[i].method,
		    kn_manual_capture_source_name(index->entries[i].source),
		    kn_manual_capture_status_name(index->entries[i].status),
		    kn_manual_capture_replay_status_name(
		    index->entries[i].replay),
		    (unsigned long long)index->entries[i].added,
		    index->entries[i].notes[0] == '\0' ? "-" :
		    index->entries[i].notes);
		if (needed < 0 || (size_t)needed >= bufsiz - off)
			return KN_MANUAL_CAPTURE_ERR_BUFFER;
		off += (size_t)needed;
	}
	return KN_MANUAL_CAPTURE_OK;
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
parse_entry(char *line, struct kn_manual_capture_entry *entry)
{
	char *token;

	memset(entry, 0, sizeof(*entry));
	entry->replay = KN_MANUAL_CAPTURE_REPLAY_NOT_RUN;
	token = strtok(trim(line), " \t\r\n");
	if (token == NULL || strcmp(token, "capture") != 0)
		return KN_MANUAL_CAPTURE_ERR_PARSE;
	while ((token = strtok(NULL, " \t\r\n")) != NULL) {
		if (set_token(entry, token) != KN_MANUAL_CAPTURE_OK)
			return KN_MANUAL_CAPTURE_ERR_PARSE;
	}
	if (entry->id == 0 || entry->file[0] == '\0' ||
	    entry->method[0] == '\0')
		return KN_MANUAL_CAPTURE_ERR_MISSING_REQUIRED;
	if (kn_manual_capture_relative_path_safe(entry->file) == 0)
		return KN_MANUAL_CAPTURE_ERR_UNSAFE_PATH;
	return KN_MANUAL_CAPTURE_OK;
}

static enum kn_manual_capture_error
set_token(struct kn_manual_capture_entry *entry, const char *token)
{
	const char *eq;
	const char *value;
	char key[64];
	size_t key_len;

	eq = strchr(token, '=');
	if (eq == NULL)
		return KN_MANUAL_CAPTURE_ERR_PARSE;
	key_len = (size_t)(eq - token);
	if (key_len == 0 || key_len >= sizeof(key))
		return KN_MANUAL_CAPTURE_ERR_PARSE;
	memcpy(key, token, key_len);
	key[key_len] = '\0';
	value = eq + 1;
	if (strcmp(key, "id") == 0)
		entry->id = (uint32_t)strtoul(value, NULL, 10);
	else if (strcmp(key, "file") == 0)
		(void)snprintf(entry->file, sizeof(entry->file), "%s", value);
	else if (strcmp(key, "method") == 0)
		(void)snprintf(entry->method, sizeof(entry->method), "%s",
		    value);
	else if (strcmp(key, "source") == 0)
		return kn_manual_capture_source_from_text(value, &entry->source);
	else if (strcmp(key, "status") == 0)
		return kn_manual_capture_status_from_text(value,
		    &entry->status);
	else if (strcmp(key, "replay") == 0)
		return kn_manual_capture_replay_status_from_text(value,
		    &entry->replay);
	else if (strcmp(key, "added") == 0)
		entry->added = (uint64_t)strtoull(value, NULL, 10);
	else if (strcmp(key, "notes") == 0)
		(void)snprintf(entry->notes, sizeof(entry->notes), "%s",
		    value);
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
	    (end[-1] == ' ' || end[-1] == '\t' || end[-1] == '\r' ||
	    end[-1] == '\n'))
		*--end = '\0';
	return s;
}
