/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/bbs/message_index.c */

#define _POSIX_C_SOURCE 200809L

#include <sys/stat.h>
#include <sys/types.h>

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "kilonode/bbs_area.h"
#include "kilonode/callsign.h"
#include "kilonode/message_index.h"

#define INDEX_LINE_MAX 512
#define INDEX_REBUILD_MAX 4096

static enum kn_message_index_error append_message(FILE *,
	const struct kn_message *);
static void append_subject(FILE *, const char *);
static enum kn_message_index_error callsign_string(const struct kn_callsign *,
	char *, size_t);
static uint8_t filter_match(const struct kn_message *,
	enum kn_message_index_filter, const char *);
static enum kn_message_index_error index_path(struct kn_message_store *,
	const char *, char *, size_t);
static enum kn_message_index_error mkdir_index(struct kn_message_store *);
static enum kn_message_index_error parse_index_file(const char *);
static enum kn_message_index_error read_index_ids(struct kn_message_store *,
	const char *, enum kn_message_index_filter, const char *,
	struct kn_message *, size_t, size_t *);
static enum kn_message_index_error rebuild_write(struct kn_message_store *,
	struct kn_message *, size_t);
static enum kn_message_index_error validate_filter(
	enum kn_message_index_filter, const char *, char *, size_t);
static enum kn_message_index_error write_one_index(struct kn_message_store *,
	const char *, struct kn_message *, size_t,
	enum kn_message_index_filter, const char *);

static enum kn_message_index_error
append_message(FILE *fp, const struct kn_message *message)
{
	char from[KN_CALLSIGN_MAX + 4];
	char to[KN_MESSAGE_AREA_MAX + 1];
	const char *dest;

	if (callsign_string(&message->from, from, sizeof(from)) !=
	    KN_MESSAGE_INDEX_OK)
		return KN_MESSAGE_INDEX_ERR_CORRUPT;
	if (message->type == KN_MESSAGE_TYPE_PRIVATE) {
		if (callsign_string(&message->to, to, sizeof(to)) !=
		    KN_MESSAGE_INDEX_OK)
			return KN_MESSAGE_INDEX_ERR_CORRUPT;
		dest = to;
	} else {
		dest = message->area;
	}

	if (fprintf(fp, "%llu|%s|%s|%s|%llu|%u|%u|",
	    (unsigned long long)message->id,
	    kn_message_type_name(message->type), from, dest,
	    (unsigned long long)message->created, (unsigned int)message->read,
	    (unsigned int)message->deleted) < 0)
		return KN_MESSAGE_INDEX_ERR_IO;
	append_subject(fp, message->subject);
	if (fputc('\n', fp) == EOF)
		return KN_MESSAGE_INDEX_ERR_IO;

	return KN_MESSAGE_INDEX_OK;
}

static void
append_subject(FILE *fp, const char *subject)
{
	size_t i;

	for (i = 0; subject[i] != '\0'; i++) {
		if ((unsigned char)subject[i] < 0x20 ||
		    (unsigned char)subject[i] > 0x7e || subject[i] == '|')
			(void)fputc('?', fp);
		else
			(void)fputc(subject[i], fp);
	}
}

static enum kn_message_index_error
callsign_string(const struct kn_callsign *callsign, char *buf, size_t bufsiz)
{
	if (kn_callsign_format(callsign, buf, bufsiz) != 0)
		return KN_MESSAGE_INDEX_ERR_CORRUPT;

	return KN_MESSAGE_INDEX_OK;
}

static uint8_t
filter_match(const struct kn_message *message, enum kn_message_index_filter filter,
	const char *value)
{
	char call[KN_CALLSIGN_MAX + 4];

	switch (filter) {
	case KN_MESSAGE_INDEX_ALL:
		return 1;
	case KN_MESSAGE_INDEX_PRIVATE:
		return message->type == KN_MESSAGE_TYPE_PRIVATE ? 1 : 0;
	case KN_MESSAGE_INDEX_BULLETIN:
		return message->type == KN_MESSAGE_TYPE_BULLETIN ? 1 : 0;
	case KN_MESSAGE_INDEX_AREA:
		return message->type == KN_MESSAGE_TYPE_BULLETIN &&
		    strcmp(message->area, value) == 0 ? 1 : 0;
	case KN_MESSAGE_INDEX_TO:
		if (message->type != KN_MESSAGE_TYPE_PRIVATE)
			return 0;
		if (kn_callsign_format(&message->to, call, sizeof(call)) != 0)
			return 0;
		return strcmp(call, value) == 0 ? 1 : 0;
	case KN_MESSAGE_INDEX_FROM:
		if (kn_callsign_format(&message->from, call, sizeof(call)) != 0)
			return 0;
		return strcmp(call, value) == 0 ? 1 : 0;
	}

	return 0;
}

static enum kn_message_index_error
index_path(struct kn_message_store *store, const char *name, char *buf,
	size_t bufsiz)
{
	int needed;

	if (store == NULL || name == NULL || buf == NULL || bufsiz == 0)
		return KN_MESSAGE_INDEX_ERR_INVALID_ARGUMENT;
	needed = snprintf(buf, bufsiz, "%s/index/%s", store->path, name);
	if (needed < 0 || (size_t)needed >= bufsiz)
		return KN_MESSAGE_INDEX_ERR_BUFFER;

	return KN_MESSAGE_INDEX_OK;
}

static enum kn_message_index_error
mkdir_index(struct kn_message_store *store)
{
	char path[KN_MESSAGE_STORE_PATH_MAX];
	struct stat st;
	enum kn_message_index_error rc;

	rc = index_path(store, ".", path, sizeof(path));
	if (rc != KN_MESSAGE_INDEX_OK)
		return rc;
	path[strlen(path) - 2] = '\0';
	if (mkdir(path, 0700) == 0)
		return KN_MESSAGE_INDEX_OK;
	if (errno != EEXIST)
		return KN_MESSAGE_INDEX_ERR_IO;
	if (stat(path, &st) != 0 || !S_ISDIR(st.st_mode))
		return KN_MESSAGE_INDEX_ERR_IO;

	return KN_MESSAGE_INDEX_OK;
}

static enum kn_message_index_error
parse_index_file(const char *path)
{
	FILE *fp;
	char line[INDEX_LINE_MAX];
	char *parts[8];
	char *cursor;
	size_t i;

	fp = fopen(path, "r");
	if (fp == NULL)
		return KN_MESSAGE_INDEX_ERR_IO;

	while (fgets(line, sizeof(line), fp) != NULL) {
		if (strchr(line, '\n') == NULL) {
			(void)fclose(fp);
			return KN_MESSAGE_INDEX_ERR_CORRUPT;
		}
		cursor = line;
		for (i = 0; i < 8; i++) {
			parts[i] = cursor;
			cursor = strchr(cursor, i == 7 ? '\n' : '|');
			if (cursor == NULL) {
				(void)fclose(fp);
				return KN_MESSAGE_INDEX_ERR_CORRUPT;
			}
			*cursor++ = '\0';
		}
		for (i = 0; i < 8; i++) {
			if (parts[i][0] == '\0') {
				(void)fclose(fp);
				return KN_MESSAGE_INDEX_ERR_CORRUPT;
			}
		}
	}
	if (ferror(fp)) {
		(void)fclose(fp);
		return KN_MESSAGE_INDEX_ERR_IO;
	}
	(void)fclose(fp);
	return KN_MESSAGE_INDEX_OK;
}

static enum kn_message_index_error
read_index_ids(struct kn_message_store *store, const char *path,
	enum kn_message_index_filter filter, const char *value,
	struct kn_message *messages, size_t max_messages, size_t *count_out)
{
	struct kn_message message;
	uint64_t seen_ids[2048];
	FILE *fp;
	char line[INDEX_LINE_MAX];
	char *end;
	unsigned long long id;
	size_t count;
	size_t seen_count;
	size_t i;
	enum kn_message_store_error src;

	fp = fopen(path, "r");
	if (fp == NULL)
		return KN_MESSAGE_INDEX_ERR_IO;

	count = 0;
	seen_count = 0;
	while (fgets(line, sizeof(line), fp) != NULL) {
		if (strchr(line, '\n') == NULL) {
			(void)fclose(fp);
			return KN_MESSAGE_INDEX_ERR_CORRUPT;
		}
		errno = 0;
		id = strtoull(line, &end, 10);
		if (errno != 0 || id == 0 || *end != '|') {
			(void)fclose(fp);
			return KN_MESSAGE_INDEX_ERR_CORRUPT;
		}
		for (i = 0; i < seen_count; i++) {
			if (seen_ids[i] == (uint64_t)id)
				break;
		}
		if (i < seen_count)
			continue;
		if (seen_count < sizeof(seen_ids) / sizeof(seen_ids[0]))
			seen_ids[seen_count++] = (uint64_t)id;
		src = kn_message_store_read_metadata(store, (uint64_t)id,
		    &message);
		if (src == KN_MESSAGE_STORE_ERR_NOT_FOUND ||
		    src == KN_MESSAGE_STORE_ERR_CORRUPT ||
		    src == KN_MESSAGE_STORE_ERR_DELETED)
			continue;
		if (src != KN_MESSAGE_STORE_OK) {
			(void)fclose(fp);
			return KN_MESSAGE_INDEX_ERR_STORE;
		}
		if (filter_match(&message, filter, value) == 0)
			continue;
		if (count < max_messages)
			messages[count] = message;
		count++;
	}
	if (ferror(fp)) {
		(void)fclose(fp);
		return KN_MESSAGE_INDEX_ERR_IO;
	}
	(void)fclose(fp);
	*count_out = count;
	return KN_MESSAGE_INDEX_OK;
}

static enum kn_message_index_error
rebuild_write(struct kn_message_store *store, struct kn_message *messages,
	size_t count)
{
	size_t i;
	enum kn_message_index_error rc;

	rc = write_one_index(store, "all.idx", messages, count,
	    KN_MESSAGE_INDEX_ALL, NULL);
	if (rc != KN_MESSAGE_INDEX_OK)
		return rc;
	rc = write_one_index(store, "private.idx", messages, count,
	    KN_MESSAGE_INDEX_PRIVATE, NULL);
	if (rc != KN_MESSAGE_INDEX_OK)
		return rc;
	rc = write_one_index(store, "bulletin.idx", messages, count,
	    KN_MESSAGE_INDEX_BULLETIN, NULL);
	if (rc != KN_MESSAGE_INDEX_OK)
		return rc;

	for (i = 0; i < count; i++) {
		char name[64];
		char value[KN_MESSAGE_AREA_MAX + 1];

		if (messages[i].type != KN_MESSAGE_TYPE_BULLETIN)
			continue;
		if (kn_bbs_area_normalize(messages[i].area, value,
		    sizeof(value)) != KN_BBS_AREA_OK)
			return KN_MESSAGE_INDEX_ERR_CORRUPT;
		if (snprintf(name, sizeof(name), "area-%s.idx", value) < 0)
			return KN_MESSAGE_INDEX_ERR_BUFFER;
		rc = write_one_index(store, name, messages, count,
		    KN_MESSAGE_INDEX_AREA, value);
		if (rc != KN_MESSAGE_INDEX_OK)
			return rc;
	}
	for (i = 0; i < count; i++) {
		char name[64];
		char value[KN_CALLSIGN_MAX + 4];
		int needed;

		if (callsign_string(&messages[i].from, value, sizeof(value)) !=
		    KN_MESSAGE_INDEX_OK)
			return KN_MESSAGE_INDEX_ERR_CORRUPT;
		needed = snprintf(name, sizeof(name), "from-%s.idx", value);
		if (needed < 0 || (size_t)needed >= sizeof(name))
			return KN_MESSAGE_INDEX_ERR_BUFFER;
		rc = write_one_index(store, name, messages, count,
		    KN_MESSAGE_INDEX_FROM, value);
		if (rc != KN_MESSAGE_INDEX_OK)
			return rc;
		if (messages[i].type != KN_MESSAGE_TYPE_PRIVATE)
			continue;
		if (callsign_string(&messages[i].to, value, sizeof(value)) !=
		    KN_MESSAGE_INDEX_OK)
			return KN_MESSAGE_INDEX_ERR_CORRUPT;
		needed = snprintf(name, sizeof(name), "to-%s.idx", value);
		if (needed < 0 || (size_t)needed >= sizeof(name))
			return KN_MESSAGE_INDEX_ERR_BUFFER;
		rc = write_one_index(store, name, messages, count,
		    KN_MESSAGE_INDEX_TO, value);
		if (rc != KN_MESSAGE_INDEX_OK)
			return rc;
	}

	return KN_MESSAGE_INDEX_OK;
}

static enum kn_message_index_error
validate_filter(enum kn_message_index_filter filter, const char *value,
	char *normalized, size_t normalized_len)
{
	struct kn_callsign call;

	if (filter == KN_MESSAGE_INDEX_AREA) {
		if (kn_bbs_area_normalize(value, normalized,
		    normalized_len) != KN_BBS_AREA_OK)
			return KN_MESSAGE_INDEX_ERR_INVALID_ARGUMENT;
		return KN_MESSAGE_INDEX_OK;
	}
	if (filter == KN_MESSAGE_INDEX_TO || filter == KN_MESSAGE_INDEX_FROM) {
		if (kn_callsign_parse(value, &call) != 0 ||
		    kn_callsign_format(&call, normalized,
		    normalized_len) != 0)
			return KN_MESSAGE_INDEX_ERR_INVALID_ARGUMENT;
		return KN_MESSAGE_INDEX_OK;
	}
	if (normalized_len > 0)
		normalized[0] = '\0';
	return KN_MESSAGE_INDEX_OK;
}

static enum kn_message_index_error
write_one_index(struct kn_message_store *store, const char *name,
	struct kn_message *messages, size_t count,
	enum kn_message_index_filter filter, const char *value)
{
	char path[KN_MESSAGE_STORE_PATH_MAX];
	char tmp[KN_MESSAGE_STORE_PATH_MAX + 8];
	FILE *fp;
	size_t i;
	int needed;
	enum kn_message_index_error rc;

	rc = index_path(store, name, path, sizeof(path));
	if (rc != KN_MESSAGE_INDEX_OK)
		return rc;
	needed = snprintf(tmp, sizeof(tmp), "%s.tmp", path);
	if (needed < 0 || (size_t)needed >= sizeof(tmp))
		return KN_MESSAGE_INDEX_ERR_BUFFER;

	fp = fopen(tmp, "w");
	if (fp == NULL)
		return KN_MESSAGE_INDEX_ERR_IO;
	for (i = 0; i < count; i++) {
		if (filter_match(&messages[i], filter, value) == 0)
			continue;
		rc = append_message(fp, &messages[i]);
		if (rc != KN_MESSAGE_INDEX_OK) {
			(void)fclose(fp);
			(void)unlink(tmp);
			return rc;
		}
	}
	if (fflush(fp) != 0 || fsync(fileno(fp)) != 0 || fclose(fp) != 0) {
		(void)unlink(tmp);
		return KN_MESSAGE_INDEX_ERR_IO;
	}
	if (rename(tmp, path) != 0) {
		(void)unlink(tmp);
		return KN_MESSAGE_INDEX_ERR_IO;
	}

	return KN_MESSAGE_INDEX_OK;
}

enum kn_message_index_error
kn_message_index_add(struct kn_message_store *store,
	const struct kn_message *message)
{
	(void)message;
	return kn_message_index_rebuild(store);
}

enum kn_message_index_error
kn_message_index_areas(struct kn_message_store *store,
	struct kn_message_index_area *areas, size_t max_areas, size_t *count_out)
{
	struct kn_message messages[KN_MESSAGE_INDEX_AREA_MAX];
	char normalized[KN_MESSAGE_AREA_MAX + 1];
	size_t message_count;
	size_t area_count;
	size_t i;
	size_t j;
	enum kn_message_index_error rc;

	if (store == NULL || areas == NULL || count_out == NULL)
		return KN_MESSAGE_INDEX_ERR_INVALID_ARGUMENT;
	rc = kn_message_index_list(store, KN_MESSAGE_INDEX_BULLETIN, NULL,
	    messages, KN_MESSAGE_INDEX_AREA_MAX, &message_count);
	if (rc != KN_MESSAGE_INDEX_OK)
		return rc;

	area_count = 0;
	for (i = 0; i < message_count && i < KN_MESSAGE_INDEX_AREA_MAX; i++) {
		if (kn_bbs_area_normalize(messages[i].area, normalized,
		    sizeof(normalized)) != KN_BBS_AREA_OK)
			continue;
		for (j = 0; j < area_count; j++) {
			if (strcmp(areas[j].name, normalized) == 0)
				break;
		}
		if (j == area_count) {
			if (area_count < max_areas) {
				memset(&areas[area_count], 0,
				    sizeof(areas[area_count]));
				memcpy(areas[area_count].name, normalized,
				    strlen(normalized) + 1);
			}
			area_count++;
		}
		if (j < max_areas) {
			areas[j].count++;
			if (messages[i].created > areas[j].newest_created) {
				areas[j].newest_created = messages[i].created;
				areas[j].newest_id = messages[i].id;
			}
		}
	}

	for (i = 0; i < area_count && i < max_areas; i++) {
		for (j = i + 1; j < area_count && j < max_areas; j++) {
			struct kn_message_index_area tmp;

			if (strcmp(areas[i].name, areas[j].name) <= 0)
				continue;
			tmp = areas[i];
			areas[i] = areas[j];
			areas[j] = tmp;
		}
	}

	*count_out = area_count;
	return KN_MESSAGE_INDEX_OK;
}

enum kn_message_index_error
kn_message_index_count(struct kn_message_store *store,
	enum kn_message_index_filter filter, const char *value, size_t *count_out)
{
	return kn_message_index_list(store, filter, value, NULL, 0, count_out);
}

enum kn_message_index_error
kn_message_index_init(struct kn_message_store *store)
{
	char path[KN_MESSAGE_STORE_PATH_MAX];
	enum kn_message_index_error rc;

	if (store == NULL || store->open == 0)
		return KN_MESSAGE_INDEX_ERR_INVALID_ARGUMENT;
	rc = mkdir_index(store);
	if (rc != KN_MESSAGE_INDEX_OK)
		return rc;

	rc = index_path(store, "all.idx", path, sizeof(path));
	if (rc != KN_MESSAGE_INDEX_OK)
		return rc;
	if (parse_index_file(path) != KN_MESSAGE_INDEX_OK)
		return kn_message_index_rebuild(store);
	rc = index_path(store, "private.idx", path, sizeof(path));
	if (rc != KN_MESSAGE_INDEX_OK)
		return rc;
	if (parse_index_file(path) != KN_MESSAGE_INDEX_OK)
		return kn_message_index_rebuild(store);
	rc = index_path(store, "bulletin.idx", path, sizeof(path));
	if (rc != KN_MESSAGE_INDEX_OK)
		return rc;
	if (parse_index_file(path) != KN_MESSAGE_INDEX_OK)
		return kn_message_index_rebuild(store);

	return KN_MESSAGE_INDEX_OK;
}

enum kn_message_index_error
kn_message_index_list(struct kn_message_store *store,
	enum kn_message_index_filter filter, const char *value,
	struct kn_message *messages, size_t max_messages, size_t *count_out)
{
	char name[64];
	char path[KN_MESSAGE_STORE_PATH_MAX];
	char normalized[KN_MESSAGE_AREA_MAX + 1];
	enum kn_message_index_error rc;
	int needed;

	if (store == NULL || count_out == NULL || store->open == 0)
		return KN_MESSAGE_INDEX_ERR_INVALID_ARGUMENT;
	if (messages == NULL && max_messages != 0)
		return KN_MESSAGE_INDEX_ERR_INVALID_ARGUMENT;
	rc = validate_filter(filter, value, normalized, sizeof(normalized));
	if (rc != KN_MESSAGE_INDEX_OK)
		return rc;

	switch (filter) {
	case KN_MESSAGE_INDEX_ALL:
		needed = snprintf(name, sizeof(name), "all.idx");
		break;
	case KN_MESSAGE_INDEX_PRIVATE:
		needed = snprintf(name, sizeof(name), "private.idx");
		break;
	case KN_MESSAGE_INDEX_BULLETIN:
		needed = snprintf(name, sizeof(name), "bulletin.idx");
		break;
	case KN_MESSAGE_INDEX_AREA:
		needed = snprintf(name, sizeof(name), "area-%s.idx",
		    normalized);
		break;
	case KN_MESSAGE_INDEX_TO:
		needed = snprintf(name, sizeof(name), "to-%s.idx", normalized);
		break;
	case KN_MESSAGE_INDEX_FROM:
		needed = snprintf(name, sizeof(name), "from-%s.idx",
		    normalized);
		break;
	default:
		return KN_MESSAGE_INDEX_ERR_INVALID_ARGUMENT;
	}
	if (needed < 0 || (size_t)needed >= sizeof(name))
		return KN_MESSAGE_INDEX_ERR_BUFFER;
	rc = index_path(store, name, path, sizeof(path));
	if (rc != KN_MESSAGE_INDEX_OK)
		return rc;
	if (access(path, F_OK) != 0) {
		*count_out = 0;
		return KN_MESSAGE_INDEX_OK;
	}
	rc = parse_index_file(path);
	if (rc != KN_MESSAGE_INDEX_OK) {
		rc = kn_message_index_rebuild(store);
		if (rc != KN_MESSAGE_INDEX_OK)
			return rc;
	}

	return read_index_ids(store, path, filter, normalized, messages,
	    max_messages, count_out);
}

enum kn_message_index_error
kn_message_index_rebuild(struct kn_message_store *store)
{
	struct kn_message *messages;
	uint8_t *body;
	size_t count;
	size_t body_len;
	uint64_t id;
	enum kn_message_store_error src;
	enum kn_message_index_error rc;

	if (store == NULL || store->open == 0)
		return KN_MESSAGE_INDEX_ERR_INVALID_ARGUMENT;
	rc = mkdir_index(store);
	if (rc != KN_MESSAGE_INDEX_OK)
		return rc;
	if (store->next_id > INDEX_REBUILD_MAX)
		return KN_MESSAGE_INDEX_ERR_BUFFER;
	messages = calloc(INDEX_REBUILD_MAX, sizeof(*messages));
	if (messages == NULL)
		return KN_MESSAGE_INDEX_ERR_IO;
	body = malloc(store->max_body_bytes == 0 ? 1 : store->max_body_bytes);
	if (body == NULL) {
		free(messages);
		return KN_MESSAGE_INDEX_ERR_IO;
	}

	count = 0;
	for (id = 1; id < store->next_id; id++) {
		src = kn_message_store_read_metadata(store, id, &messages[count]);
		if (src == KN_MESSAGE_STORE_ERR_NOT_FOUND ||
		    src == KN_MESSAGE_STORE_ERR_CORRUPT ||
		    src == KN_MESSAGE_STORE_ERR_DELETED)
			continue;
		if (src != KN_MESSAGE_STORE_OK) {
			free(body);
			free(messages);
			return KN_MESSAGE_INDEX_ERR_STORE;
		}
		src = kn_message_store_read_body(store, id, body,
		    store->max_body_bytes, &body_len);
		if (src != KN_MESSAGE_STORE_OK)
			continue;
		count++;
	}
	rc = rebuild_write(store, messages, count);
	free(body);
	free(messages);
	return rc;
}

const char *
kn_message_index_error_name(enum kn_message_index_error error)
{
	switch (error) {
	case KN_MESSAGE_INDEX_OK:
		return "ok";
	case KN_MESSAGE_INDEX_ERR_INVALID_ARGUMENT:
		return "invalid argument";
	case KN_MESSAGE_INDEX_ERR_IO:
		return "io";
	case KN_MESSAGE_INDEX_ERR_CORRUPT:
		return "corrupt";
	case KN_MESSAGE_INDEX_ERR_BUFFER:
		return "buffer";
	case KN_MESSAGE_INDEX_ERR_STORE:
		return "store";
	}

	return "unknown";
}
