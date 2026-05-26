/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/bbs/bbs_read_state.c */

#define _POSIX_C_SOURCE 200809L

#include <sys/stat.h>
#include <sys/types.h>

#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "kilonode/bbs_read_state.h"
#include "kilonode/bbs_store_lock.h"
#include "kilonode/callsign.h"

static enum kn_bbs_read_state_error call_normalize(const char *, char *,
	size_t);
static enum kn_bbs_read_state_error read_path(struct kn_message_store *,
	const char *, char *, size_t);
static enum kn_bbs_read_state_error write_state(struct kn_message_store *,
	const struct kn_bbs_read_state *);

static enum kn_bbs_read_state_error
call_normalize(const char *input, char *out, size_t out_len)
{
	struct kn_callsign call;
	char normalized[KN_CALLSIGN_MAX + 4];
	size_t i;

	if (input == NULL || out == NULL || out_len == 0)
		return KN_BBS_READ_STATE_ERR_INVALID_ARGUMENT;
	if (strlen(input) >= sizeof(normalized))
		return KN_BBS_READ_STATE_ERR_INVALID_CALLSIGN;
	for (i = 0; input[i] != '\0'; i++)
		normalized[i] = (char)toupper((unsigned char)input[i]);
	normalized[i] = '\0';
	if (kn_callsign_parse(normalized, &call) != 0)
		return KN_BBS_READ_STATE_ERR_INVALID_CALLSIGN;
	if (kn_callsign_format(&call, out, out_len) != 0)
		return KN_BBS_READ_STATE_ERR_BUFFER;
	return KN_BBS_READ_STATE_OK;
}

static enum kn_bbs_read_state_error
read_path(struct kn_message_store *store, const char *call, char *buf,
	size_t bufsiz)
{
	char normalized[KN_CALLSIGN_MAX + 4];
	int needed;
	enum kn_bbs_read_state_error rc;

	rc = call_normalize(call, normalized, sizeof(normalized));
	if (rc != KN_BBS_READ_STATE_OK)
		return rc;
	needed = snprintf(buf, bufsiz, "%s/read/%s.read", store->path,
	    normalized);
	if (needed < 0 || (size_t)needed >= bufsiz)
		return KN_BBS_READ_STATE_ERR_BUFFER;
	return KN_BBS_READ_STATE_OK;
}

static enum kn_bbs_read_state_error
write_state(struct kn_message_store *store, const struct kn_bbs_read_state *state)
{
	struct kn_bbs_store_lock lock;
	char path[KN_MESSAGE_STORE_PATH_MAX];
	char tmp[KN_MESSAGE_STORE_PATH_MAX + 8];
	FILE *fp;
	size_t i;
	int needed;
	enum kn_bbs_read_state_error rc;
	enum kn_bbs_store_lock_error lrc;

	kn_bbs_store_lock_init(&lock);
	lrc = kn_bbs_store_lock_exclusive(&lock, store->path);
	if (lrc != KN_BBS_STORE_LOCK_OK)
		return KN_BBS_READ_STATE_ERR_IO;
	rc = read_path(store, state->call, path, sizeof(path));
	if (rc != KN_BBS_READ_STATE_OK)
		goto done;
	needed = snprintf(tmp, sizeof(tmp), "%s.tmp", path);
	if (needed < 0 || (size_t)needed >= sizeof(tmp)) {
		rc = KN_BBS_READ_STATE_ERR_BUFFER;
		goto done;
	}
	fp = fopen(tmp, "w");
	if (fp == NULL) {
		rc = KN_BBS_READ_STATE_ERR_IO;
		goto done;
	}
	for (i = 0; i < state->count; i++) {
		if (fprintf(fp, "%llu\n",
		    (unsigned long long)state->ids[i]) < 0) {
			(void)fclose(fp);
			(void)unlink(tmp);
			rc = KN_BBS_READ_STATE_ERR_IO;
			goto done;
		}
	}
	if (fflush(fp) != 0 || fsync(fileno(fp)) != 0 || fclose(fp) != 0) {
		(void)unlink(tmp);
		rc = KN_BBS_READ_STATE_ERR_IO;
		goto done;
	}
	if (rename(tmp, path) != 0) {
		(void)unlink(tmp);
		rc = KN_BBS_READ_STATE_ERR_IO;
		goto done;
	}
	rc = KN_BBS_READ_STATE_OK;
done:
	kn_bbs_store_lock_release(&lock);
	return rc;
}

const char *
kn_bbs_read_state_error_name(enum kn_bbs_read_state_error error)
{
	switch (error) {
	case KN_BBS_READ_STATE_OK:
		return "ok";
	case KN_BBS_READ_STATE_ERR_INVALID_ARGUMENT:
		return "invalid argument";
	case KN_BBS_READ_STATE_ERR_INVALID_CALLSIGN:
		return "invalid callsign";
	case KN_BBS_READ_STATE_ERR_INVALID_ID:
		return "invalid id";
	case KN_BBS_READ_STATE_ERR_IO:
		return "io";
	case KN_BBS_READ_STATE_ERR_CORRUPT:
		return "corrupt";
	case KN_BBS_READ_STATE_ERR_BUFFER:
		return "buffer";
	}
	return "unknown";
}

enum kn_bbs_read_state_error
kn_bbs_read_state_init_store(struct kn_message_store *store)
{
	char path[KN_MESSAGE_STORE_PATH_MAX];
	struct stat st;
	int needed;

	if (store == NULL || store->open == 0)
		return KN_BBS_READ_STATE_ERR_INVALID_ARGUMENT;
	needed = snprintf(path, sizeof(path), "%s/read", store->path);
	if (needed < 0 || (size_t)needed >= sizeof(path))
		return KN_BBS_READ_STATE_ERR_BUFFER;
	if (mkdir(path, 0700) == 0)
		return KN_BBS_READ_STATE_OK;
	if (errno != EEXIST)
		return KN_BBS_READ_STATE_ERR_IO;
	if (stat(path, &st) != 0 || !S_ISDIR(st.st_mode))
		return KN_BBS_READ_STATE_ERR_IO;
	return KN_BBS_READ_STATE_OK;
}

enum kn_bbs_read_state_error
kn_bbs_read_state_is_read(struct kn_message_store *store, const char *call,
	uint64_t id, uint8_t *is_read)
{
	struct kn_bbs_read_state state;
	size_t i;
	enum kn_bbs_read_state_error rc;

	if (is_read == NULL || id == 0)
		return KN_BBS_READ_STATE_ERR_INVALID_ARGUMENT;
	*is_read = 0;
	rc = kn_bbs_read_state_load(store, call, &state);
	if (rc != KN_BBS_READ_STATE_OK)
		return rc;
	for (i = 0; i < state.count; i++) {
		if (state.ids[i] == id) {
			*is_read = 1;
			break;
		}
	}
	return KN_BBS_READ_STATE_OK;
}

enum kn_bbs_read_state_error
kn_bbs_read_state_load(struct kn_message_store *store, const char *call,
	struct kn_bbs_read_state *state)
{
	char path[KN_MESSAGE_STORE_PATH_MAX];
	char normalized[KN_CALLSIGN_MAX + 4];
	char line[64];
	char *end;
	unsigned long long id;
	FILE *fp;
	enum kn_bbs_read_state_error rc;

	if (store == NULL || state == NULL || store->open == 0)
		return KN_BBS_READ_STATE_ERR_INVALID_ARGUMENT;
	rc = call_normalize(call, normalized, sizeof(normalized));
	if (rc != KN_BBS_READ_STATE_OK)
		return rc;
	rc = read_path(store, normalized, path, sizeof(path));
	if (rc != KN_BBS_READ_STATE_OK)
		return rc;
	memset(state, 0, sizeof(*state));
	memcpy(state->call, normalized, strlen(normalized) + 1);
	fp = fopen(path, "r");
	if (fp == NULL) {
		if (errno == ENOENT)
			return KN_BBS_READ_STATE_OK;
		return KN_BBS_READ_STATE_ERR_IO;
	}
	while (fgets(line, sizeof(line), fp) != NULL) {
		if (strchr(line, '\n') == NULL) {
			(void)fclose(fp);
			return KN_BBS_READ_STATE_ERR_CORRUPT;
		}
		errno = 0;
		id = strtoull(line, &end, 10);
		if (errno != 0 || id == 0 || (*end != '\n' && *end != '\0')) {
			(void)fclose(fp);
			return KN_BBS_READ_STATE_ERR_CORRUPT;
		}
		if (state->count >= KN_BBS_READ_STATE_MAX) {
			(void)fclose(fp);
			return KN_BBS_READ_STATE_ERR_BUFFER;
		}
		state->ids[state->count++] = (uint64_t)id;
	}
	if (ferror(fp)) {
		(void)fclose(fp);
		return KN_BBS_READ_STATE_ERR_IO;
	}
	(void)fclose(fp);
	return KN_BBS_READ_STATE_OK;
}

enum kn_bbs_read_state_error
kn_bbs_read_state_mark_read(struct kn_message_store *store, const char *call,
	uint64_t id)
{
	struct kn_bbs_read_state state;
	size_t i;
	enum kn_bbs_read_state_error rc;

	if (id == 0)
		return KN_BBS_READ_STATE_ERR_INVALID_ID;
	rc = kn_bbs_read_state_load(store, call, &state);
	if (rc != KN_BBS_READ_STATE_OK)
		return rc;
	for (i = 0; i < state.count; i++) {
		if (state.ids[i] == id)
			return KN_BBS_READ_STATE_OK;
	}
	if (state.count >= KN_BBS_READ_STATE_MAX)
		return KN_BBS_READ_STATE_ERR_BUFFER;
	state.ids[state.count++] = id;
	return write_state(store, &state);
}

enum kn_bbs_read_state_error
kn_bbs_read_state_save(struct kn_message_store *store,
	const struct kn_bbs_read_state *state)
{
	if (store == NULL || state == NULL || store->open == 0)
		return KN_BBS_READ_STATE_ERR_INVALID_ARGUMENT;
	return write_state(store, state);
}

enum kn_bbs_read_state_error
kn_bbs_read_state_unread_count(struct kn_message_store *store,
	const char *call, const struct kn_message *messages, size_t message_count,
	size_t *count_out)
{
	size_t i;
	size_t unread;
	uint8_t read;
	enum kn_bbs_read_state_error rc;

	if (messages == NULL || count_out == NULL)
		return KN_BBS_READ_STATE_ERR_INVALID_ARGUMENT;
	unread = 0;
	for (i = 0; i < message_count; i++) {
		rc = kn_bbs_read_state_is_read(store, call, messages[i].id,
		    &read);
		if (rc != KN_BBS_READ_STATE_OK)
			return rc;
		if (read == 0)
			unread++;
	}
	*count_out = unread;
	return KN_BBS_READ_STATE_OK;
}
