/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/bbs/bbs_user.c */

#define _POSIX_C_SOURCE 200809L

#include <sys/stat.h>
#include <sys/types.h>

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "kilonode/bbs_store_lock.h"
#include "kilonode/bbs_user.h"

#define USER_META_MAX 1024

static enum kn_bbs_user_error call_normalize(const char *, char *, size_t,
	struct kn_callsign *);
static enum kn_bbs_user_error mkdir_child(struct kn_message_store *,
	const char *);
static enum kn_bbs_user_error parse_user(const char *, struct kn_bbs_user *);
static enum kn_bbs_user_error read_file(const char *, char *, size_t,
	size_t *);
static uint8_t text_safe(const char *);
static enum kn_bbs_user_error user_path(struct kn_message_store *,
	const char *, char *, size_t);
static enum kn_bbs_user_error write_atomic(const char *, const char *,
	size_t);

static enum kn_bbs_user_error
call_normalize(const char *input, char *out, size_t out_len,
	struct kn_callsign *callsign)
{
	struct kn_callsign parsed;
	char normalized[KN_CALLSIGN_MAX + 4];
	size_t i;

	if (input == NULL || out == NULL || out_len == 0)
		return KN_BBS_USER_ERR_INVALID_ARGUMENT;
	if (strlen(input) >= sizeof(normalized))
		return KN_BBS_USER_ERR_INVALID_CALLSIGN;
	for (i = 0; input[i] != '\0'; i++)
		normalized[i] = (char)toupper((unsigned char)input[i]);
	normalized[i] = '\0';
	if (kn_callsign_parse(normalized, &parsed) != 0)
		return KN_BBS_USER_ERR_INVALID_CALLSIGN;
	if (kn_callsign_format(&parsed, out, out_len) != 0)
		return KN_BBS_USER_ERR_BUFFER;
	for (i = 0; out[i] != '\0'; i++) {
		if (!((out[i] >= 'A' && out[i] <= 'Z') ||
		    (out[i] >= '0' && out[i] <= '9') || out[i] == '-'))
			return KN_BBS_USER_ERR_INVALID_CALLSIGN;
	}
	if (callsign != NULL)
		*callsign = parsed;
	return KN_BBS_USER_OK;
}

static enum kn_bbs_user_error
mkdir_child(struct kn_message_store *store, const char *child)
{
	char path[KN_MESSAGE_STORE_PATH_MAX];
	struct stat st;
	int needed;

	needed = snprintf(path, sizeof(path), "%s/%s", store->path, child);
	if (needed < 0 || (size_t)needed >= sizeof(path))
		return KN_BBS_USER_ERR_BUFFER;
	if (mkdir(path, 0700) == 0)
		return KN_BBS_USER_OK;
	if (errno != EEXIST)
		return KN_BBS_USER_ERR_IO;
	if (stat(path, &st) != 0 || !S_ISDIR(st.st_mode))
		return KN_BBS_USER_ERR_IO;
	return KN_BBS_USER_OK;
}

static enum kn_bbs_user_error
parse_user(const char *buf, struct kn_bbs_user *user)
{
	char copy[USER_META_MAX];
	char *line;
	char *saveptr;
	char *value;
	unsigned long long number;
	char *end;
	uint8_t have_call;

	if (strlen(buf) >= sizeof(copy))
		return KN_BBS_USER_ERR_CORRUPT;
	memset(user, 0, sizeof(*user));
	memcpy(copy, buf, strlen(buf) + 1);
	have_call = 0;

	line = strtok_r(copy, "\n", &saveptr);
	while (line != NULL) {
		value = strchr(line, ' ');
		if (value == NULL)
			return KN_BBS_USER_ERR_CORRUPT;
		*value++ = '\0';
		if (strcmp(line, "call") == 0) {
			if (call_normalize(value, user->call, sizeof(user->call),
			    &user->callsign) != KN_BBS_USER_OK)
				return KN_BBS_USER_ERR_CORRUPT;
			have_call = 1;
		} else if (strcmp(line, "display") == 0) {
			if (strlen(value) >= sizeof(user->display) ||
			    text_safe(value) == 0)
				return KN_BBS_USER_ERR_CORRUPT;
			memcpy(user->display, value, strlen(value) + 1);
		} else if (strcmp(line, "home") == 0) {
			if (strlen(value) >= sizeof(user->home_bbs) ||
			    text_safe(value) == 0)
				return KN_BBS_USER_ERR_CORRUPT;
			memcpy(user->home_bbs, value, strlen(value) + 1);
		} else if (strcmp(line, "notes") == 0) {
			if (strlen(value) >= sizeof(user->notes) ||
			    text_safe(value) == 0)
				return KN_BBS_USER_ERR_CORRUPT;
			memcpy(user->notes, value, strlen(value) + 1);
		} else if (strcmp(line, "created") == 0 ||
		    strcmp(line, "last-seen") == 0 ||
		    strcmp(line, "login-count") == 0) {
			errno = 0;
			number = strtoull(value, &end, 10);
			if (errno != 0 || *end != '\0')
				return KN_BBS_USER_ERR_CORRUPT;
			if (strcmp(line, "created") == 0)
				user->created = (uint64_t)number;
			else if (strcmp(line, "last-seen") == 0)
				user->last_seen = (uint64_t)number;
			else
				user->login_count = (uint64_t)number;
		} else if (strcmp(line, "sysop") == 0 ||
		    strcmp(line, "disabled") == 0) {
			if (strcmp(value, "0") != 0 && strcmp(value, "1") != 0)
				return KN_BBS_USER_ERR_CORRUPT;
			if (strcmp(line, "sysop") == 0)
				user->sysop = (uint8_t)(value[0] - '0');
			else
				user->disabled = (uint8_t)(value[0] - '0');
		} else {
			return KN_BBS_USER_ERR_CORRUPT;
		}
		line = strtok_r(NULL, "\n", &saveptr);
	}
	if (have_call == 0)
		return KN_BBS_USER_ERR_CORRUPT;
	return KN_BBS_USER_OK;
}

static enum kn_bbs_user_error
read_file(const char *path, char *buf, size_t bufsiz, size_t *len_out)
{
	FILE *fp;
	size_t len;

	fp = fopen(path, "r");
	if (fp == NULL)
		return errno == ENOENT ? KN_BBS_USER_ERR_NOT_FOUND :
		    KN_BBS_USER_ERR_IO;
	len = fread(buf, 1, bufsiz - 1, fp);
	if (ferror(fp)) {
		(void)fclose(fp);
		return KN_BBS_USER_ERR_IO;
	}
	if (len == bufsiz - 1 && fgetc(fp) != EOF) {
		(void)fclose(fp);
		return KN_BBS_USER_ERR_BUFFER;
	}
	(void)fclose(fp);
	buf[len] = '\0';
	*len_out = len;
	return KN_BBS_USER_OK;
}

static uint8_t
text_safe(const char *value)
{
	size_t i;

	for (i = 0; value != NULL && value[i] != '\0'; i++) {
		if ((unsigned char)value[i] < 0x20 ||
		    (unsigned char)value[i] > 0x7e)
			return 0;
	}
	return 1;
}

static enum kn_bbs_user_error
user_path(struct kn_message_store *store, const char *call, char *buf,
	size_t bufsiz)
{
	char normalized[KN_CALLSIGN_MAX + 4];
	int needed;
	enum kn_bbs_user_error rc;

	rc = call_normalize(call, normalized, sizeof(normalized), NULL);
	if (rc != KN_BBS_USER_OK)
		return rc;
	needed = snprintf(buf, bufsiz, "%s/users/%s.user", store->path,
	    normalized);
	if (needed < 0 || (size_t)needed >= bufsiz)
		return KN_BBS_USER_ERR_BUFFER;
	return KN_BBS_USER_OK;
}

static enum kn_bbs_user_error
write_atomic(const char *path, const char *buf, size_t len)
{
	char tmp[KN_MESSAGE_STORE_PATH_MAX + 8];
	FILE *fp;
	int needed;

	needed = snprintf(tmp, sizeof(tmp), "%s.tmp", path);
	if (needed < 0 || (size_t)needed >= sizeof(tmp))
		return KN_BBS_USER_ERR_BUFFER;
	fp = fopen(tmp, "w");
	if (fp == NULL)
		return KN_BBS_USER_ERR_IO;
	if (fwrite(buf, 1, len, fp) != len || fflush(fp) != 0 ||
	    fsync(fileno(fp)) != 0 || fclose(fp) != 0) {
		(void)unlink(tmp);
		return KN_BBS_USER_ERR_IO;
	}
	if (rename(tmp, path) != 0) {
		(void)unlink(tmp);
		return KN_BBS_USER_ERR_IO;
	}
	return KN_BBS_USER_OK;
}

enum kn_bbs_user_error
kn_bbs_user_create(struct kn_message_store *store, const char *call,
	uint64_t now, struct kn_bbs_user *out)
{
	struct kn_bbs_user user;
	char path[KN_MESSAGE_STORE_PATH_MAX];
	enum kn_bbs_user_error rc;

	if (store == NULL || store->open == 0)
		return KN_BBS_USER_ERR_INVALID_ARGUMENT;
	rc = user_path(store, call, path, sizeof(path));
	if (rc != KN_BBS_USER_OK)
		return rc;
	if (access(path, F_OK) == 0)
		return KN_BBS_USER_ERR_EXISTS;
	memset(&user, 0, sizeof(user));
	rc = call_normalize(call, user.call, sizeof(user.call), &user.callsign);
	if (rc != KN_BBS_USER_OK)
		return rc;
	user.created = now;
	user.last_seen = now;
	rc = kn_bbs_user_save(store, &user);
	if (rc != KN_BBS_USER_OK)
		return rc;
	if (out != NULL)
		*out = user;
	return KN_BBS_USER_OK;
}

enum kn_bbs_user_error
kn_bbs_user_enable(struct kn_message_store *store, const char *call,
	uint8_t enabled)
{
	struct kn_bbs_user user;
	char path[KN_MESSAGE_STORE_PATH_MAX];
	char buf[USER_META_MAX];
	size_t len;
	enum kn_bbs_user_error rc;

	if (store == NULL || store->open == 0)
		return KN_BBS_USER_ERR_INVALID_ARGUMENT;
	rc = user_path(store, call, path, sizeof(path));
	if (rc != KN_BBS_USER_OK)
		return rc;
	rc = read_file(path, buf, sizeof(buf), &len);
	if (rc != KN_BBS_USER_OK)
		return rc;
	(void)len;
	rc = parse_user(buf, &user);
	if (rc != KN_BBS_USER_OK)
		return rc;
	user.disabled = enabled == 0 ? 1 : 0;
	return kn_bbs_user_save(store, &user);
}

const char *
kn_bbs_user_error_name(enum kn_bbs_user_error error)
{
	switch (error) {
	case KN_BBS_USER_OK:
		return "ok";
	case KN_BBS_USER_ERR_INVALID_ARGUMENT:
		return "invalid argument";
	case KN_BBS_USER_ERR_INVALID_CALLSIGN:
		return "invalid callsign";
	case KN_BBS_USER_ERR_IO:
		return "io";
	case KN_BBS_USER_ERR_NOT_FOUND:
		return "not found";
	case KN_BBS_USER_ERR_EXISTS:
		return "exists";
	case KN_BBS_USER_ERR_DISABLED:
		return "disabled";
	case KN_BBS_USER_ERR_CORRUPT:
		return "corrupt";
	case KN_BBS_USER_ERR_BUFFER:
		return "buffer";
	}
	return "unknown";
}

enum kn_bbs_user_error
kn_bbs_user_format(const struct kn_bbs_user *user, char *buf, size_t bufsiz)
{
	int needed;

	if (user == NULL || buf == NULL || bufsiz == 0)
		return KN_BBS_USER_ERR_INVALID_ARGUMENT;
	needed = snprintf(buf, bufsiz,
	    "call=%s created=%llu last_seen=%llu logins=%llu sysop=%s disabled=%s",
	    user->call, (unsigned long long)user->created,
	    (unsigned long long)user->last_seen,
	    (unsigned long long)user->login_count,
	    user->sysop != 0 ? "true" : "false",
	    user->disabled != 0 ? "true" : "false");
	if (needed < 0 || (size_t)needed >= bufsiz)
		return KN_BBS_USER_ERR_BUFFER;
	return KN_BBS_USER_OK;
}

enum kn_bbs_user_error
kn_bbs_user_init_store(struct kn_message_store *store)
{
	enum kn_bbs_user_error rc;

	if (store == NULL || store->open == 0)
		return KN_BBS_USER_ERR_INVALID_ARGUMENT;
	rc = mkdir_child(store, "users");
	if (rc != KN_BBS_USER_OK)
		return rc;
	return mkdir_child(store, "read");
}

enum kn_bbs_user_error
kn_bbs_user_list(struct kn_message_store *store, struct kn_bbs_user *users,
	size_t max_users, size_t *count_out)
{
	struct dirent *entry;
	DIR *dir;
	char path[KN_MESSAGE_STORE_PATH_MAX];
	char file[KN_MESSAGE_STORE_PATH_MAX];
	char buf[USER_META_MAX];
	size_t len;
	size_t count;
	size_t i;
	size_t j;
	int needed;
	enum kn_bbs_user_error rc;

	if (store == NULL || users == NULL || count_out == NULL ||
	    store->open == 0)
		return KN_BBS_USER_ERR_INVALID_ARGUMENT;
	needed = snprintf(path, sizeof(path), "%s/users", store->path);
	if (needed < 0 || (size_t)needed >= sizeof(path))
		return KN_BBS_USER_ERR_BUFFER;
	dir = opendir(path);
	if (dir == NULL)
		return KN_BBS_USER_ERR_IO;
	count = 0;
	while ((entry = readdir(dir)) != NULL) {
		if (entry->d_name[0] == '.')
			continue;
		if (strstr(entry->d_name, ".user") == NULL)
			continue;
		needed = snprintf(file, sizeof(file), "%s/%s", path,
		    entry->d_name);
		if (needed < 0 || (size_t)needed >= sizeof(file)) {
			(void)closedir(dir);
			return KN_BBS_USER_ERR_BUFFER;
		}
		rc = read_file(file, buf, sizeof(buf), &len);
		(void)len;
		if (rc != KN_BBS_USER_OK)
			continue;
		if (count < max_users &&
		    parse_user(buf, &users[count]) == KN_BBS_USER_OK)
			count++;
	}
	(void)closedir(dir);
	for (i = 0; i < count && i < max_users; i++) {
		for (j = i + 1; j < count && j < max_users; j++) {
			struct kn_bbs_user tmp;

			if (strcmp(users[i].call, users[j].call) <= 0)
				continue;
			tmp = users[i];
			users[i] = users[j];
			users[j] = tmp;
		}
	}
	*count_out = count;
	return KN_BBS_USER_OK;
}

enum kn_bbs_user_error
kn_bbs_user_load(struct kn_message_store *store, const char *call,
	struct kn_bbs_user *user)
{
	char path[KN_MESSAGE_STORE_PATH_MAX];
	char buf[USER_META_MAX];
	size_t len;
	enum kn_bbs_user_error rc;

	if (store == NULL || user == NULL || store->open == 0)
		return KN_BBS_USER_ERR_INVALID_ARGUMENT;
	rc = user_path(store, call, path, sizeof(path));
	if (rc != KN_BBS_USER_OK)
		return rc;
	rc = read_file(path, buf, sizeof(buf), &len);
	(void)len;
	if (rc != KN_BBS_USER_OK)
		return rc;
	rc = parse_user(buf, user);
	if (rc != KN_BBS_USER_OK)
		return rc;
	if (user->disabled != 0)
		return KN_BBS_USER_ERR_DISABLED;
	return KN_BBS_USER_OK;
}

enum kn_bbs_user_error
kn_bbs_user_seen(struct kn_message_store *store, const char *call,
	uint64_t now, struct kn_bbs_user *out)
{
	struct kn_bbs_user user;
	enum kn_bbs_user_error rc;

	rc = kn_bbs_user_load(store, call, &user);
	if (rc == KN_BBS_USER_ERR_NOT_FOUND || rc == KN_BBS_USER_ERR_DISABLED) {
		if (rc == KN_BBS_USER_ERR_DISABLED)
			return rc;
		rc = kn_bbs_user_create(store, call, now, &user);
		if (rc != KN_BBS_USER_OK)
			return rc;
	} else if (rc != KN_BBS_USER_OK) {
		return rc;
	}
	user.last_seen = now;
	user.login_count++;
	rc = kn_bbs_user_save(store, &user);
	if (rc != KN_BBS_USER_OK)
		return rc;
	if (out != NULL)
		*out = user;
	return KN_BBS_USER_OK;
}

enum kn_bbs_user_error
kn_bbs_user_save(struct kn_message_store *store, const struct kn_bbs_user *user)
{
	struct kn_bbs_store_lock lock;
	char path[KN_MESSAGE_STORE_PATH_MAX];
	char buf[USER_META_MAX];
	int needed;
	enum kn_bbs_user_error rc;
	enum kn_bbs_store_lock_error lrc;

	if (store == NULL || user == NULL || store->open == 0)
		return KN_BBS_USER_ERR_INVALID_ARGUMENT;
	rc = user_path(store, user->call, path, sizeof(path));
	if (rc != KN_BBS_USER_OK)
		return rc;
	needed = snprintf(buf, sizeof(buf),
	    "call %s\n"
	    "display %s\n"
	    "home %s\n"
	    "created %llu\n"
	    "last-seen %llu\n"
	    "login-count %llu\n"
	    "sysop %u\n"
	    "disabled %u\n"
	    "notes %s\n",
	    user->call, user->display, user->home_bbs,
	    (unsigned long long)user->created,
	    (unsigned long long)user->last_seen,
	    (unsigned long long)user->login_count,
	    (unsigned int)user->sysop, (unsigned int)user->disabled,
	    user->notes);
	if (needed < 0 || (size_t)needed >= sizeof(buf))
		return KN_BBS_USER_ERR_BUFFER;
	kn_bbs_store_lock_init(&lock);
	lrc = kn_bbs_store_lock_exclusive(&lock, store->path);
	if (lrc != KN_BBS_STORE_LOCK_OK)
		return KN_BBS_USER_ERR_IO;
	rc = write_atomic(path, buf, (size_t)needed);
	kn_bbs_store_lock_release(&lock);
	return rc;
}
