/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/bbs/message_store.c */

#define _POSIX_C_SOURCE 200809L

#include <sys/stat.h>
#include <sys/types.h>

#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "kilonode/callsign.h"
#include "kilonode/message.h"
#include "kilonode/message_store.h"

#define META_BUFSIZ 1024
#define NEXT_ID_MAX 99999999ULL

static enum kn_message_store_error create_message(struct kn_message_store *,
	struct kn_message *, const uint8_t *, size_t, uint64_t *);
static enum kn_message_store_error ensure_store_dirs(const char *);
static enum kn_message_store_error id_path(const struct kn_message_store *,
	uint64_t, const char *, char *, size_t);
static enum kn_message_store_error mkdir_one(const char *);
static enum kn_message_store_error next_id_read(struct kn_message_store *);
static enum kn_message_store_error next_id_write(const struct kn_message_store *);
static enum kn_message_store_error parse_meta(const char *,
	struct kn_message *);
static enum kn_message_store_error path_join(const char *, const char *,
	char *, size_t);
static uint8_t path_safe(const char *);
static enum kn_message_store_error read_file(const char *, uint8_t *, size_t,
	size_t *);
static enum kn_message_store_error store_write_atomic(const char *,
	const uint8_t *, size_t);
static enum kn_message_store_error write_body(const struct kn_message_store *,
	uint64_t, const uint8_t *, size_t);
static enum kn_message_store_error write_meta(const struct kn_message_store *,
	const struct kn_message *);

static enum kn_message_store_error
create_message(struct kn_message_store *store, struct kn_message *message,
	const uint8_t *body, size_t body_len, uint64_t *id_out)
{
	enum kn_message_store_error rc;

	if (store == NULL || message == NULL || body == NULL || id_out == NULL ||
	    store->open == 0)
		return KN_MESSAGE_STORE_ERR_INVALID_ARGUMENT;
	if (body_len == 0 || body_len != message->body_len)
		return KN_MESSAGE_STORE_ERR_INVALID_MESSAGE;
	if (body_len > store->max_body_bytes)
		return KN_MESSAGE_STORE_ERR_BODY_TOO_LARGE;

	message->id = store->next_id;
	rc = write_body(store, message->id, body, body_len);
	if (rc != KN_MESSAGE_STORE_OK)
		return rc;
	rc = write_meta(store, message);
	if (rc != KN_MESSAGE_STORE_OK)
		return rc;

	if (store->next_id == UINT64_MAX)
		return KN_MESSAGE_STORE_ERR_CORRUPT;
	store->next_id++;
	rc = next_id_write(store);
	if (rc != KN_MESSAGE_STORE_OK)
		return rc;

	*id_out = message->id;
	return KN_MESSAGE_STORE_OK;
}

static enum kn_message_store_error
ensure_store_dirs(const char *path)
{
	char meta[KN_MESSAGE_STORE_PATH_MAX];
	char msg[KN_MESSAGE_STORE_PATH_MAX];
	enum kn_message_store_error rc;

	rc = mkdir_one(path);
	if (rc != KN_MESSAGE_STORE_OK)
		return rc;
	rc = path_join(path, "meta", meta, sizeof(meta));
	if (rc != KN_MESSAGE_STORE_OK)
		return rc;
	rc = mkdir_one(meta);
	if (rc != KN_MESSAGE_STORE_OK)
		return rc;
	rc = path_join(path, "msg", msg, sizeof(msg));
	if (rc != KN_MESSAGE_STORE_OK)
		return rc;

	return mkdir_one(msg);
}

static enum kn_message_store_error
id_path(const struct kn_message_store *store, uint64_t id, const char *suffix,
	char *buf, size_t bufsiz)
{
	char name[32];
	char msg_dir[KN_MESSAGE_STORE_PATH_MAX];
	int needed;
	enum kn_message_store_error rc;

	if (id == 0 || id > NEXT_ID_MAX)
		return KN_MESSAGE_STORE_ERR_INVALID_ARGUMENT;

	needed = snprintf(name, sizeof(name), "%08llu.%s",
	    (unsigned long long)id, suffix);
	if (needed < 0 || (size_t)needed >= sizeof(name))
		return KN_MESSAGE_STORE_ERR_BUFFER;

	rc = path_join(store->path, "msg", msg_dir, sizeof(msg_dir));
	if (rc != KN_MESSAGE_STORE_OK)
		return rc;
	return path_join(msg_dir, name, buf, bufsiz);
}

static enum kn_message_store_error
mkdir_one(const char *path)
{
	struct stat st;

	if (mkdir(path, 0700) == 0)
		return KN_MESSAGE_STORE_OK;
	if (errno != EEXIST)
		return KN_MESSAGE_STORE_ERR_IO;
	if (stat(path, &st) != 0)
		return KN_MESSAGE_STORE_ERR_IO;
	if (!S_ISDIR(st.st_mode))
		return KN_MESSAGE_STORE_ERR_IO;

	return KN_MESSAGE_STORE_OK;
}

static enum kn_message_store_error
next_id_read(struct kn_message_store *store)
{
	char path[KN_MESSAGE_STORE_PATH_MAX];
	char buf[64];
	char *end;
	unsigned long long value;
	size_t len;
	enum kn_message_store_error rc;

	rc = path_join(store->path, "meta/next-id", path, sizeof(path));
	if (rc != KN_MESSAGE_STORE_OK)
		return rc;

	if (access(path, F_OK) != 0) {
		store->next_id = 1;
		return next_id_write(store);
	}

	rc = read_file(path, (uint8_t *)buf, sizeof(buf) - 1, &len);
	if (rc != KN_MESSAGE_STORE_OK)
		return rc;
	buf[len] = '\0';
	errno = 0;
	value = strtoull(buf, &end, 10);
	if (errno != 0 || value == 0 || value > NEXT_ID_MAX)
		return KN_MESSAGE_STORE_ERR_CORRUPT;
	if (*end == '\n')
		end++;
	if (*end != '\0')
		return KN_MESSAGE_STORE_ERR_CORRUPT;

	store->next_id = (uint64_t)value;
	return KN_MESSAGE_STORE_OK;
}

static enum kn_message_store_error
next_id_write(const struct kn_message_store *store)
{
	char path[KN_MESSAGE_STORE_PATH_MAX];
	char buf[64];
	int needed;
	enum kn_message_store_error rc;

	rc = path_join(store->path, "meta/next-id", path, sizeof(path));
	if (rc != KN_MESSAGE_STORE_OK)
		return rc;
	needed = snprintf(buf, sizeof(buf), "%llu\n",
	    (unsigned long long)store->next_id);
	if (needed < 0 || (size_t)needed >= sizeof(buf))
		return KN_MESSAGE_STORE_ERR_BUFFER;

	return store_write_atomic(path, (const uint8_t *)buf, (size_t)needed);
}

static enum kn_message_store_error
parse_meta(const char *buf, struct kn_message *message)
{
	char copy[META_BUFSIZ];
	char *line;
	char *value;
	char *saveptr;
	uint8_t have_id;
	uint8_t have_type;
	uint8_t have_from;
	uint8_t have_dest;
	uint8_t have_subject;
	uint8_t have_created;
	uint8_t have_body;
	unsigned long long number;
	char *end;

	if (strlen(buf) >= sizeof(copy))
		return KN_MESSAGE_STORE_ERR_CORRUPT;

	memset(message, 0, sizeof(*message));
	memcpy(copy, buf, strlen(buf) + 1);
	have_id = 0;
	have_type = 0;
	have_from = 0;
	have_dest = 0;
	have_subject = 0;
	have_created = 0;
	have_body = 0;

	line = strtok_r(copy, "\n", &saveptr);
	while (line != NULL) {
		value = strchr(line, ' ');
		if (value == NULL)
			return KN_MESSAGE_STORE_ERR_CORRUPT;
		*value++ = '\0';

		if (strcmp(line, "id") == 0) {
			errno = 0;
			number = strtoull(value, &end, 10);
			if (errno != 0 || *end != '\0' || number == 0)
				return KN_MESSAGE_STORE_ERR_CORRUPT;
			message->id = (uint64_t)number;
			have_id = 1;
		} else if (strcmp(line, "type") == 0) {
			if (strcmp(value, "private") == 0)
				message->type = KN_MESSAGE_TYPE_PRIVATE;
			else if (strcmp(value, "bulletin") == 0)
				message->type = KN_MESSAGE_TYPE_BULLETIN;
			else
				return KN_MESSAGE_STORE_ERR_CORRUPT;
			have_type = 1;
		} else if (strcmp(line, "from") == 0) {
			if (kn_callsign_parse(value, &message->from) != 0)
				return KN_MESSAGE_STORE_ERR_CORRUPT;
			have_from = 1;
		} else if (strcmp(line, "to") == 0) {
			if (message->type == KN_MESSAGE_TYPE_PRIVATE) {
				if (kn_callsign_parse(value, &message->to) != 0)
					return KN_MESSAGE_STORE_ERR_CORRUPT;
			} else {
				if (kn_message_area_valid(value) == 0)
					return KN_MESSAGE_STORE_ERR_CORRUPT;
				memcpy(message->area, value, strlen(value) + 1);
			}
			have_dest = 1;
		} else if (strcmp(line, "subject") == 0) {
			if (strlen(value) == 0 ||
			    strlen(value) > KN_MESSAGE_SUBJECT_MAX)
				return KN_MESSAGE_STORE_ERR_CORRUPT;
			memcpy(message->subject, value, strlen(value) + 1);
			have_subject = 1;
		} else if (strcmp(line, "created") == 0 ||
		    strcmp(line, "updated") == 0 ||
		    strcmp(line, "body-length") == 0) {
			errno = 0;
			number = strtoull(value, &end, 10);
			if (errno != 0 || *end != '\0')
				return KN_MESSAGE_STORE_ERR_CORRUPT;
			if (strcmp(line, "created") == 0) {
				message->created = (uint64_t)number;
				have_created = 1;
			} else if (strcmp(line, "updated") == 0) {
				message->updated = (uint64_t)number;
			} else {
				message->body_len = (size_t)number;
				have_body = 1;
			}
		} else if (strcmp(line, "read") == 0 ||
		    strcmp(line, "deleted") == 0) {
			if (strcmp(value, "0") != 0 && strcmp(value, "1") != 0)
				return KN_MESSAGE_STORE_ERR_CORRUPT;
			if (strcmp(line, "read") == 0)
				message->read = (uint8_t)(value[0] - '0');
			else
				message->deleted = (uint8_t)(value[0] - '0');
		} else {
			return KN_MESSAGE_STORE_ERR_CORRUPT;
		}
		line = strtok_r(NULL, "\n", &saveptr);
	}

	if (have_id == 0 || have_type == 0 || have_from == 0 ||
	    have_dest == 0 || have_subject == 0 || have_created == 0 ||
	    have_body == 0)
		return KN_MESSAGE_STORE_ERR_CORRUPT;

	return KN_MESSAGE_STORE_OK;
}

static enum kn_message_store_error
path_join(const char *base, const char *name, char *buf, size_t bufsiz)
{
	int needed;

	if (base == NULL || name == NULL || buf == NULL || bufsiz == 0 ||
	    base[0] == '\0' || name[0] == '\0')
		return KN_MESSAGE_STORE_ERR_INVALID_ARGUMENT;

	needed = snprintf(buf, bufsiz, "%s/%s", base, name);
	if (needed < 0 || (size_t)needed >= bufsiz)
		return KN_MESSAGE_STORE_ERR_BUFFER;

	return KN_MESSAGE_STORE_OK;
}

static uint8_t
path_safe(const char *path)
{
	size_t i;

	if (path == NULL || path[0] == '\0')
		return 0;
	for (i = 0; path[i] != '\0'; i++) {
		if ((unsigned char)path[i] < 0x20 ||
		    (unsigned char)path[i] > 0x7e)
			return 0;
	}

	return 1;
}

static enum kn_message_store_error
read_file(const char *path, uint8_t *buf, size_t bufsiz, size_t *len_out)
{
	int fd;
	uint8_t extra;
	ssize_t nread;
	size_t len;

	fd = open(path, O_RDONLY);
	if (fd < 0)
		return KN_MESSAGE_STORE_ERR_NOT_FOUND;

	len = 0;
	for (;;) {
		if (len == bufsiz) {
			nread = read(fd, &extra, 1);
			if (nread == 0)
				break;
			if (nread < 0 && errno == EINTR)
				continue;
			(void)close(fd);
			return nread > 0 ? KN_MESSAGE_STORE_ERR_BODY_TOO_LARGE :
			    KN_MESSAGE_STORE_ERR_IO;
		}
		nread = read(fd, buf + len, bufsiz - len);
		if (nread > 0) {
			len += (size_t)nread;
			continue;
		}
		if (nread == 0)
			break;
		if (errno == EINTR)
			continue;
		(void)close(fd);
		return KN_MESSAGE_STORE_ERR_IO;
	}
	(void)close(fd);
	*len_out = len;
	return KN_MESSAGE_STORE_OK;
}

static enum kn_message_store_error
store_write_atomic(const char *path, const uint8_t *data, size_t len)
{
	char tmp[KN_MESSAGE_STORE_PATH_MAX + 8];
	size_t done;
	ssize_t nwritten;
	int needed;
	int fd;

	needed = snprintf(tmp, sizeof(tmp), "%s.tmp", path);
	if (needed < 0 || (size_t)needed >= sizeof(tmp))
		return KN_MESSAGE_STORE_ERR_BUFFER;

	fd = open(tmp, O_WRONLY | O_CREAT | O_TRUNC, 0600);
	if (fd < 0)
		return KN_MESSAGE_STORE_ERR_IO;
	done = 0;
	while (done < len) {
		nwritten = write(fd, data + done, len - done);
		if (nwritten > 0) {
			done += (size_t)nwritten;
			continue;
		}
		if (nwritten < 0 && errno == EINTR)
			continue;
		(void)close(fd);
		(void)unlink(tmp);
		return KN_MESSAGE_STORE_ERR_IO;
	}
	(void)fsync(fd);
	if (close(fd) != 0) {
		(void)unlink(tmp);
		return KN_MESSAGE_STORE_ERR_IO;
	}
	if (rename(tmp, path) != 0) {
		(void)unlink(tmp);
		return KN_MESSAGE_STORE_ERR_IO;
	}

	return KN_MESSAGE_STORE_OK;
}

static enum kn_message_store_error
write_body(const struct kn_message_store *store, uint64_t id,
	const uint8_t *body, size_t body_len)
{
	char path[KN_MESSAGE_STORE_PATH_MAX];
	enum kn_message_store_error rc;

	rc = id_path(store, id, "body", path, sizeof(path));
	if (rc != KN_MESSAGE_STORE_OK)
		return rc;

	return store_write_atomic(path, body, body_len);
}

static enum kn_message_store_error
write_meta(const struct kn_message_store *store, const struct kn_message *msg)
{
	char path[KN_MESSAGE_STORE_PATH_MAX];
	char from[KN_CALLSIGN_MAX + 4];
	char to[KN_CALLSIGN_MAX + 4];
	char meta[META_BUFSIZ];
	const char *dest;
	int needed;
	enum kn_message_store_error rc;

	if (kn_callsign_format(&msg->from, from, sizeof(from)) != 0)
		return KN_MESSAGE_STORE_ERR_INVALID_MESSAGE;
	if (msg->type == KN_MESSAGE_TYPE_PRIVATE) {
		if (kn_callsign_format(&msg->to, to, sizeof(to)) != 0)
			return KN_MESSAGE_STORE_ERR_INVALID_MESSAGE;
		dest = to;
	} else {
		dest = msg->area;
	}

	rc = id_path(store, msg->id, "meta", path, sizeof(path));
	if (rc != KN_MESSAGE_STORE_OK)
		return rc;
	needed = snprintf(meta, sizeof(meta),
	    "id %llu\n"
	    "type %s\n"
	    "from %s\n"
	    "to %s\n"
	    "subject %s\n"
	    "created %llu\n"
	    "updated %llu\n"
	    "read %u\n"
	    "deleted %u\n"
	    "body-length %llu\n",
	    (unsigned long long)msg->id, kn_message_type_name(msg->type),
	    from, dest, msg->subject, (unsigned long long)msg->created,
	    (unsigned long long)msg->updated, (unsigned int)msg->read,
	    (unsigned int)msg->deleted, (unsigned long long)msg->body_len);
	if (needed < 0 || (size_t)needed >= sizeof(meta))
		return KN_MESSAGE_STORE_ERR_BUFFER;

	return store_write_atomic(path, (const uint8_t *)meta, (size_t)needed);
}

void
kn_message_store_close(struct kn_message_store *store)
{
	if (store == NULL)
		return;

	kn_message_store_init(store);
}

enum kn_message_store_error
kn_message_store_create_bulletin(struct kn_message_store *store,
	const char *from, const char *area, const char *subject,
	const uint8_t *body, size_t body_len, uint64_t *id_out)
{
	struct kn_message message;
	uint64_t now;

	now = (uint64_t)time(NULL);
	if (kn_message_bulletin_init(&message, from, area, subject, body_len,
	    now) != KN_MESSAGE_OK)
		return KN_MESSAGE_STORE_ERR_INVALID_MESSAGE;

	return create_message(store, &message, body, body_len, id_out);
}

enum kn_message_store_error
kn_message_store_create_private(struct kn_message_store *store,
	const char *from, const char *to, const char *subject,
	const uint8_t *body, size_t body_len, uint64_t *id_out)
{
	struct kn_message message;
	uint64_t now;

	now = (uint64_t)time(NULL);
	if (kn_message_private_init(&message, from, to, subject, body_len,
	    now) != KN_MESSAGE_OK)
		return KN_MESSAGE_STORE_ERR_INVALID_MESSAGE;

	return create_message(store, &message, body, body_len, id_out);
}

enum kn_message_store_error
kn_message_store_delete(struct kn_message_store *store, uint64_t id)
{
	struct kn_message message;
	enum kn_message_store_error rc;

	rc = kn_message_store_read_metadata(store, id, &message);
	if (rc != KN_MESSAGE_STORE_OK)
		return rc;

	message.deleted = 1;
	message.updated = message.created;
	return write_meta(store, &message);
}

void
kn_message_store_init(struct kn_message_store *store)
{
	if (store == NULL)
		return;

	memset(store, 0, sizeof(*store));
	store->max_body_bytes = KN_MESSAGE_BODY_MAX;
	store->next_id = 1;
}

enum kn_message_store_error
kn_message_store_list(struct kn_message_store *store, struct kn_message *msgs,
	size_t max_msgs, size_t *count_out)
{
	struct kn_message message;
	uint64_t id;
	size_t count;
	enum kn_message_store_error rc;

	if (store == NULL || msgs == NULL || count_out == NULL ||
	    store->open == 0)
		return KN_MESSAGE_STORE_ERR_INVALID_ARGUMENT;

	count = 0;
	for (id = 1; id < store->next_id; id++) {
		rc = kn_message_store_read_metadata(store, id, &message);
		if (rc == KN_MESSAGE_STORE_ERR_NOT_FOUND ||
		    rc == KN_MESSAGE_STORE_ERR_CORRUPT ||
		    rc == KN_MESSAGE_STORE_ERR_DELETED)
			continue;
		if (rc != KN_MESSAGE_STORE_OK)
			return rc;
		if (count < max_msgs)
			msgs[count] = message;
		count++;
	}

	*count_out = count;
	return KN_MESSAGE_STORE_OK;
}

enum kn_message_store_error
kn_message_store_open(struct kn_message_store *store, const char *path,
	size_t max_body_bytes)
{
	enum kn_message_store_error rc;
	size_t path_len;

	if (store == NULL || path_safe(path) == 0)
		return KN_MESSAGE_STORE_ERR_INVALID_ARGUMENT;
	if (max_body_bytes == 0 || max_body_bytes > KN_MESSAGE_BODY_MAX)
		return KN_MESSAGE_STORE_ERR_INVALID_ARGUMENT;

	kn_message_store_init(store);
	path_len = strlen(path);
	if (path_len >= sizeof(store->path))
		return KN_MESSAGE_STORE_ERR_INVALID_ARGUMENT;
	memcpy(store->path, path, path_len + 1);
	store->max_body_bytes = max_body_bytes;

	rc = ensure_store_dirs(store->path);
	if (rc != KN_MESSAGE_STORE_OK)
		return rc;
	rc = next_id_read(store);
	if (rc != KN_MESSAGE_STORE_OK)
		return rc;

	store->open = 1;
	return KN_MESSAGE_STORE_OK;
}

enum kn_message_store_error
kn_message_store_read_body(struct kn_message_store *store, uint64_t id,
	uint8_t *buf, size_t bufsiz, size_t *len_out)
{
	struct kn_message message;
	char path[KN_MESSAGE_STORE_PATH_MAX];
	size_t len;
	enum kn_message_store_error rc;

	if (store == NULL || buf == NULL || len_out == NULL || store->open == 0)
		return KN_MESSAGE_STORE_ERR_INVALID_ARGUMENT;

	rc = kn_message_store_read_metadata(store, id, &message);
	if (rc != KN_MESSAGE_STORE_OK)
		return rc;
	if (message.body_len > bufsiz)
		return KN_MESSAGE_STORE_ERR_BUFFER;
	rc = id_path(store, id, "body", path, sizeof(path));
	if (rc != KN_MESSAGE_STORE_OK)
		return rc;
	rc = read_file(path, buf, bufsiz, &len);
	if (rc == KN_MESSAGE_STORE_ERR_NOT_FOUND)
		return KN_MESSAGE_STORE_ERR_CORRUPT;
	if (rc != KN_MESSAGE_STORE_OK)
		return rc;
	if (len != message.body_len)
		return KN_MESSAGE_STORE_ERR_CORRUPT;

	*len_out = len;
	return KN_MESSAGE_STORE_OK;
}

enum kn_message_store_error
kn_message_store_read_metadata(struct kn_message_store *store, uint64_t id,
	struct kn_message *message)
{
	char path[KN_MESSAGE_STORE_PATH_MAX];
	char meta[META_BUFSIZ];
	size_t len;
	enum kn_message_store_error rc;

	if (store == NULL || message == NULL || store->open == 0)
		return KN_MESSAGE_STORE_ERR_INVALID_ARGUMENT;

	rc = id_path(store, id, "meta", path, sizeof(path));
	if (rc != KN_MESSAGE_STORE_OK)
		return rc;
	rc = read_file(path, (uint8_t *)meta, sizeof(meta) - 1, &len);
	if (rc == KN_MESSAGE_STORE_ERR_BODY_TOO_LARGE)
		return KN_MESSAGE_STORE_ERR_CORRUPT;
	if (rc != KN_MESSAGE_STORE_OK)
		return rc;
	meta[len] = '\0';
	rc = parse_meta(meta, message);
	if (rc != KN_MESSAGE_STORE_OK)
		return rc;
	if (message->deleted != 0)
		return KN_MESSAGE_STORE_ERR_DELETED;

	return KN_MESSAGE_STORE_OK;
}

const char *
kn_message_store_error_name(enum kn_message_store_error error)
{
	switch (error) {
	case KN_MESSAGE_STORE_OK:
		return "ok";
	case KN_MESSAGE_STORE_ERR_INVALID_ARGUMENT:
		return "invalid argument";
	case KN_MESSAGE_STORE_ERR_INVALID_MESSAGE:
		return "invalid message";
	case KN_MESSAGE_STORE_ERR_IO:
		return "io";
	case KN_MESSAGE_STORE_ERR_CORRUPT:
		return "corrupt";
	case KN_MESSAGE_STORE_ERR_NOT_FOUND:
		return "not found";
	case KN_MESSAGE_STORE_ERR_DELETED:
		return "deleted";
	case KN_MESSAGE_STORE_ERR_BODY_TOO_LARGE:
		return "body too large";
	case KN_MESSAGE_STORE_ERR_BUFFER:
		return "buffer";
	}

	return "unknown";
}
