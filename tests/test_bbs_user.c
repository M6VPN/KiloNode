/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_bbs_user.c */

#define _XOPEN_SOURCE 700

#include <sys/stat.h>
#include <sys/types.h>

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "kilonode/bbs_user.h"

static void cleanup_store(const char *);
static int make_store(char *, size_t);
static int open_user_store(struct kn_message_store *, char *, size_t);
static int write_text(const char *, const char *);
static int test_create_and_load(void);
static int test_disable_enable(void);
static int test_duplicate_create(void);
static int test_invalid_callsign(void);
static int test_list_sorted_and_corrupt_skipped(void);
static int test_missing_user(void);
static int test_seen_updates(void);

int
main(void)
{
	if (test_create_and_load() != 0)
		return 1;
	if (test_invalid_callsign() != 0)
		return 1;
	if (test_duplicate_create() != 0)
		return 1;
	if (test_seen_updates() != 0)
		return 1;
	if (test_disable_enable() != 0)
		return 1;
	if (test_list_sorted_and_corrupt_skipped() != 0)
		return 1;
	if (test_missing_user() != 0)
		return 1;

	return 0;
}

static void
cleanup_store(const char *dir)
{
	char path[512];

	(void)snprintf(path, sizeof(path), "%s/users/M6VPN-1.user", dir);
	(void)unlink(path);
	(void)snprintf(path, sizeof(path), "%s/users/N0CALL.user", dir);
	(void)unlink(path);
	(void)snprintf(path, sizeof(path), "%s/users/ZZ9ZZ.user", dir);
	(void)unlink(path);
	(void)snprintf(path, sizeof(path), "%s/users/BAD.user", dir);
	(void)unlink(path);
	(void)snprintf(path, sizeof(path), "%s/read", dir);
	(void)rmdir(path);
	(void)snprintf(path, sizeof(path), "%s/users", dir);
	(void)rmdir(path);
	(void)snprintf(path, sizeof(path), "%s/index/all.idx", dir);
	(void)unlink(path);
	(void)snprintf(path, sizeof(path), "%s/index/private.idx", dir);
	(void)unlink(path);
	(void)snprintf(path, sizeof(path), "%s/index/bulletin.idx", dir);
	(void)unlink(path);
	(void)snprintf(path, sizeof(path), "%s/index", dir);
	(void)rmdir(path);
	(void)snprintf(path, sizeof(path), "%s/meta/next-id", dir);
	(void)unlink(path);
	(void)snprintf(path, sizeof(path), "%s/meta/next-id.tmp", dir);
	(void)unlink(path);
	(void)snprintf(path, sizeof(path), "%s/meta", dir);
	(void)rmdir(path);
	(void)snprintf(path, sizeof(path), "%s/msg", dir);
	(void)rmdir(path);
	(void)rmdir(dir);
}

static int
make_store(char *buf, size_t bufsiz)
{
	unsigned int i;
	int needed;

	for (i = 0; i < 100; i++) {
		needed = snprintf(buf, bufsiz, "/tmp/kilonode-user-%ld-%u",
		    (long)getpid(), i);
		if (needed < 0 || (size_t)needed >= bufsiz)
			return 1;
		if (mkdir(buf, 0700) == 0)
			return 0;
		if (errno != EEXIST)
			return 1;
	}

	return 1;
}

static int
open_user_store(struct kn_message_store *store, char *dir, size_t dir_len)
{
	if (make_store(dir, dir_len) != 0)
		return 1;
	kn_message_store_init(store);
	if (kn_message_store_open(store, dir, KN_MESSAGE_BODY_MAX) !=
	    KN_MESSAGE_STORE_OK) {
		cleanup_store(dir);
		return 1;
	}
	if (kn_bbs_user_init_store(store) != KN_BBS_USER_OK) {
		kn_message_store_close(store);
		cleanup_store(dir);
		return 1;
	}
	return 0;
}

static int
write_text(const char *path, const char *text)
{
	FILE *fp;

	fp = fopen(path, "w");
	if (fp == NULL)
		return 1;
	if (fputs(text, fp) < 0) {
		(void)fclose(fp);
		return 1;
	}
	return fclose(fp) == 0 ? 0 : 1;
}

static int
test_create_and_load(void)
{
	struct kn_message_store store;
	struct kn_bbs_user user;
	char dir[256];
	char summary[256];
	int rc;

	if (open_user_store(&store, dir, sizeof(dir)) != 0)
		return 1;
	rc = kn_bbs_user_create(&store, "m6vpn-1", 100, &user) ==
	    KN_BBS_USER_OK &&
	    strcmp(user.call, "M6VPN-1") == 0 &&
	    kn_bbs_user_load(&store, "M6VPN-1", &user) == KN_BBS_USER_OK &&
	    user.created == 100 && user.last_seen == 100 &&
	    kn_bbs_user_format(&user, summary, sizeof(summary)) ==
	    KN_BBS_USER_OK && strstr(summary, "call=M6VPN-1") != NULL ? 0 : 1;
	kn_message_store_close(&store);
	cleanup_store(dir);
	return rc;
}

static int
test_disable_enable(void)
{
	struct kn_message_store store;
	struct kn_bbs_user user;
	char dir[256];
	int rc;

	if (open_user_store(&store, dir, sizeof(dir)) != 0)
		return 1;
	rc = kn_bbs_user_create(&store, "M6VPN-1", 100, NULL) ==
	    KN_BBS_USER_OK &&
	    kn_bbs_user_enable(&store, "M6VPN-1", 0) == KN_BBS_USER_OK &&
	    kn_bbs_user_load(&store, "M6VPN-1", &user) ==
	    KN_BBS_USER_ERR_DISABLED &&
	    kn_bbs_user_enable(&store, "M6VPN-1", 1) == KN_BBS_USER_OK &&
	    kn_bbs_user_load(&store, "M6VPN-1", &user) == KN_BBS_USER_OK &&
	    user.disabled == 0 ? 0 : 1;
	kn_message_store_close(&store);
	cleanup_store(dir);
	return rc;
}

static int
test_duplicate_create(void)
{
	struct kn_message_store store;
	char dir[256];
	int rc;

	if (open_user_store(&store, dir, sizeof(dir)) != 0)
		return 1;
	rc = kn_bbs_user_create(&store, "M6VPN-1", 100, NULL) ==
	    KN_BBS_USER_OK &&
	    kn_bbs_user_create(&store, "M6VPN-1", 101, NULL) ==
	    KN_BBS_USER_ERR_EXISTS ? 0 : 1;
	kn_message_store_close(&store);
	cleanup_store(dir);
	return rc;
}

static int
test_invalid_callsign(void)
{
	struct kn_message_store store;
	char dir[256];
	int rc;

	if (open_user_store(&store, dir, sizeof(dir)) != 0)
		return 1;
	rc = kn_bbs_user_create(&store, "BAD!", 100, NULL) ==
	    KN_BBS_USER_ERR_INVALID_CALLSIGN ? 0 : 1;
	kn_message_store_close(&store);
	cleanup_store(dir);
	return rc;
}

static int
test_list_sorted_and_corrupt_skipped(void)
{
	struct kn_message_store store;
	struct kn_bbs_user users[4];
	char dir[256];
	char path[512];
	size_t count;
	int needed;
	int rc;

	if (open_user_store(&store, dir, sizeof(dir)) != 0)
		return 1;
	needed = snprintf(path, sizeof(path), "%s/users/BAD.user", dir);
	if (needed < 0 || (size_t)needed >= sizeof(path)) {
		kn_message_store_close(&store);
		cleanup_store(dir);
		return 1;
	}
	rc = kn_bbs_user_create(&store, "N0CALL", 100, NULL) ==
	    KN_BBS_USER_OK &&
	    kn_bbs_user_create(&store, "M6VPN-1", 100, NULL) ==
	    KN_BBS_USER_OK &&
	    write_text(path, "not a user\n") == 0 &&
	    kn_bbs_user_list(&store, users, 4, &count) == KN_BBS_USER_OK &&
	    count == 2 && strcmp(users[0].call, "M6VPN-1") == 0 &&
	    strcmp(users[1].call, "N0CALL") == 0 ? 0 : 1;
	kn_message_store_close(&store);
	cleanup_store(dir);
	return rc;
}

static int
test_missing_user(void)
{
	struct kn_message_store store;
	struct kn_bbs_user user;
	char dir[256];
	int rc;

	if (open_user_store(&store, dir, sizeof(dir)) != 0)
		return 1;
	rc = kn_bbs_user_load(&store, "N0CALL", &user) ==
	    KN_BBS_USER_ERR_NOT_FOUND ? 0 : 1;
	kn_message_store_close(&store);
	cleanup_store(dir);
	return rc;
}

static int
test_seen_updates(void)
{
	struct kn_message_store store;
	struct kn_bbs_user user;
	char dir[256];
	int rc;

	if (open_user_store(&store, dir, sizeof(dir)) != 0)
		return 1;
	rc = kn_bbs_user_seen(&store, "M6VPN-1", 100, &user) ==
	    KN_BBS_USER_OK && user.login_count == 1 &&
	    kn_bbs_user_seen(&store, "M6VPN-1", 200, &user) ==
	    KN_BBS_USER_OK && user.login_count == 2 &&
	    user.created == 100 && user.last_seen == 200 ? 0 : 1;
	kn_message_store_close(&store);
	cleanup_store(dir);
	return rc;
}
