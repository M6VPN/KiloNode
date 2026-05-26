/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_bbs_store_maintenance.c */

#define _XOPEN_SOURCE 700

#include <sys/stat.h>
#include <sys/types.h>

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "kilonode/bbs_read_state.h"
#include "kilonode/bbs_store_maintenance.h"
#include "kilonode/bbs_user.h"
#include "kilonode/message_store.h"

static void cleanup_export(const char *);
static void cleanup_store(const char *);
static int make_path(char *, size_t, const char *);
static int open_store(struct kn_message_store *, char *, size_t);
static int path_join3(const char *, const char *, const char *, char *,
	size_t);
static int write_text(const char *, const char *);
static int test_check_valid_store(void);
static int test_export_store(void);
static int test_missing_directory_check(void);
static int test_purge_deleted(void);
static int test_repair_next_id_and_index(void);
static int test_stats(void);

int
main(void)
{
	if (test_check_valid_store() != 0)
		return 1;
	if (test_missing_directory_check() != 0)
		return 1;
	if (test_repair_next_id_and_index() != 0)
		return 1;
	if (test_purge_deleted() != 0)
		return 1;
	if (test_export_store() != 0)
		return 1;
	if (test_stats() != 0)
		return 1;
	return 0;
}

static void
cleanup_export(const char *dir)
{
	char path[512];
	const char *dirs[] = { "meta", "msg", "index", "users", "read" };
	size_t i;

	for (i = 0; i < sizeof(dirs) / sizeof(dirs[0]); i++) {
		(void)snprintf(path, sizeof(path), "%s/%s/next-id", dir,
		    dirs[i]);
		(void)unlink(path);
		(void)snprintf(path, sizeof(path), "%s/%s/all.idx", dir,
		    dirs[i]);
		(void)unlink(path);
		(void)snprintf(path, sizeof(path), "%s/%s/private.idx", dir,
		    dirs[i]);
		(void)unlink(path);
		(void)snprintf(path, sizeof(path), "%s/%s/bulletin.idx", dir,
		    dirs[i]);
		(void)unlink(path);
		(void)snprintf(path, sizeof(path), "%s/%s/00000001.meta", dir,
		    dirs[i]);
		(void)unlink(path);
		(void)snprintf(path, sizeof(path), "%s/%s/00000001.body", dir,
		    dirs[i]);
		(void)unlink(path);
		(void)snprintf(path, sizeof(path), "%s/%s/M6VPN-1.user", dir,
		    dirs[i]);
		(void)unlink(path);
		(void)snprintf(path, sizeof(path), "%s/%s/M6VPN-1.read", dir,
		    dirs[i]);
		(void)unlink(path);
		(void)snprintf(path, sizeof(path), "%s/%s", dir, dirs[i]);
		(void)rmdir(path);
	}
	(void)snprintf(path, sizeof(path), "%s/manifest.txt", dir);
	(void)unlink(path);
	(void)rmdir(dir);
}

static void
cleanup_store(const char *dir)
{
	char path[512];
	unsigned int i;

	for (i = 1; i <= 4; i++) {
		(void)snprintf(path, sizeof(path), "%s/msg/%08u.meta", dir, i);
		(void)unlink(path);
		(void)snprintf(path, sizeof(path), "%s/msg/%08u.body", dir, i);
		(void)unlink(path);
	}
	(void)snprintf(path, sizeof(path), "%s/.kilonode-store.lock", dir);
	(void)unlink(path);
	(void)snprintf(path, sizeof(path), "%s/read/M6VPN-1.read", dir);
	(void)unlink(path);
	(void)snprintf(path, sizeof(path), "%s/read/M6VPN-1.read.tmp", dir);
	(void)unlink(path);
	(void)snprintf(path, sizeof(path), "%s/read", dir);
	(void)rmdir(path);
	(void)snprintf(path, sizeof(path), "%s/users/M6VPN-1.user", dir);
	(void)unlink(path);
	(void)snprintf(path, sizeof(path), "%s/users", dir);
	(void)rmdir(path);
	(void)snprintf(path, sizeof(path), "%s/index/all.idx", dir);
	(void)unlink(path);
	(void)snprintf(path, sizeof(path), "%s/index/private.idx", dir);
	(void)unlink(path);
	(void)snprintf(path, sizeof(path), "%s/index/bulletin.idx", dir);
	(void)unlink(path);
	(void)snprintf(path, sizeof(path), "%s/index/to-N0CALL.idx", dir);
	(void)unlink(path);
	(void)snprintf(path, sizeof(path), "%s/index/from-M6VPN-1.idx", dir);
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
make_path(char *buf, size_t bufsiz, const char *tag)
{
	unsigned int i;
	int needed;

	for (i = 0; i < 100; i++) {
		needed = snprintf(buf, bufsiz, "/tmp/kilonode-%s-%ld-%u",
		    tag, (long)getpid(), i);
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
open_store(struct kn_message_store *store, char *dir, size_t dir_len)
{
	if (make_path(dir, dir_len, "maint") != 0)
		return 1;
	kn_message_store_init(store);
	if (kn_message_store_open(store, dir, KN_MESSAGE_BODY_MAX) !=
	    KN_MESSAGE_STORE_OK) {
		cleanup_store(dir);
		return 1;
	}
	(void)kn_bbs_user_init_store(store);
	(void)kn_bbs_read_state_init_store(store);
	return 0;
}

static int
path_join3(const char *a, const char *b, const char *c, char *buf,
	size_t bufsiz)
{
	int needed;

	needed = snprintf(buf, bufsiz, "%s/%s/%s", a, b, c);
	if (needed < 0 || (size_t)needed >= bufsiz)
		return 1;
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
test_check_valid_store(void)
{
	struct kn_message_store store;
	struct kn_bbs_store_finding findings[8];
	size_t count;
	size_t errors;
	char dir[256];
	int rc;

	if (open_store(&store, dir, sizeof(dir)) != 0)
		return 1;
	rc = kn_bbs_store_check(dir, findings, 8, &count, &errors) ==
	    KN_BBS_STORE_MAINTENANCE_OK && errors == 0 ? 0 : 1;
	kn_message_store_close(&store);
	cleanup_store(dir);
	return rc;
}

static int
test_export_store(void)
{
	struct kn_message_store store;
	uint64_t id;
	char dir[256];
	char dest[256];
	char path[512];
	int rc;

	if (open_store(&store, dir, sizeof(dir)) != 0)
		return 1;
	if (make_path(dest, sizeof(dest), "export") != 0) {
		kn_message_store_close(&store);
		cleanup_store(dir);
		return 1;
	}
	(void)rmdir(dest);
	rc = kn_message_store_create_private(&store, "M6VPN-1", "N0CALL",
	    "Subject", (const uint8_t *)"body", 4, &id) ==
	    KN_MESSAGE_STORE_OK &&
	    kn_bbs_user_create(&store, "M6VPN-1", 1, NULL) ==
	    KN_BBS_USER_OK &&
	    kn_bbs_read_state_mark_read(&store, "M6VPN-1", id) ==
	    KN_BBS_READ_STATE_OK &&
	    kn_bbs_store_export(dir, dest) == KN_BBS_STORE_MAINTENANCE_OK &&
	    path_join3(dest, "msg", "00000001.meta", path, sizeof(path)) == 0 &&
	    access(path, F_OK) == 0 &&
	    path_join3(dest, "read", "M6VPN-1.read", path, sizeof(path)) == 0 &&
	    access(path, F_OK) == 0 &&
	    path_join3(dest, ".", "manifest.txt", path, sizeof(path)) == 0 ?
	    0 : 1;
	kn_message_store_close(&store);
	cleanup_export(dest);
	cleanup_store(dir);
	return rc;
}

static int
test_missing_directory_check(void)
{
	struct kn_message_store store;
	struct kn_bbs_store_finding findings[8];
	size_t count;
	size_t errors;
	char dir[256];
	char path[512];
	int rc;

	if (open_store(&store, dir, sizeof(dir)) != 0)
		return 1;
	kn_message_store_close(&store);
	(void)snprintf(path, sizeof(path), "%s/read", dir);
	(void)rmdir(path);
	rc = kn_bbs_store_check(dir, findings, 8, &count, &errors) ==
	    KN_BBS_STORE_MAINTENANCE_OK && errors > 0 &&
	    strcmp(findings[0].code, "dir-missing") == 0 ? 0 : 1;
	cleanup_store(dir);
	return rc;
}

static int
test_purge_deleted(void)
{
	struct kn_message_store store;
	size_t purged;
	uint64_t id;
	char dir[256];
	char path[512];
	int rc;

	if (open_store(&store, dir, sizeof(dir)) != 0)
		return 1;
	rc = kn_message_store_create_private(&store, "M6VPN-1", "N0CALL",
	    "Subject", (const uint8_t *)"body", 4, &id) ==
	    KN_MESSAGE_STORE_OK &&
	    kn_message_store_delete(&store, id) == KN_MESSAGE_STORE_OK &&
	    kn_bbs_store_purge_deleted(dir, &purged) ==
	    KN_BBS_STORE_MAINTENANCE_OK && purged == 1 &&
	    path_join3(dir, "msg", "00000001.meta", path, sizeof(path)) == 0 &&
	    access(path, F_OK) != 0 && store.next_id == 2 ? 0 : 1;
	kn_message_store_close(&store);
	cleanup_store(dir);
	return rc;
}

static int
test_repair_next_id_and_index(void)
{
	struct kn_message_store store;
	struct kn_bbs_store_finding findings[8];
	size_t count;
	uint64_t id;
	char dir[256];
	char path[512];
	int rc;

	if (open_store(&store, dir, sizeof(dir)) != 0)
		return 1;
	rc = kn_message_store_create_private(&store, "M6VPN-1", "N0CALL",
	    "Subject", (const uint8_t *)"body", 4, &id) ==
	    KN_MESSAGE_STORE_OK &&
	    path_join3(dir, "meta", "next-id", path, sizeof(path)) == 0 &&
	    write_text(path, "1\n") == 0 &&
	    path_join3(dir, "index", "all.idx", path, sizeof(path)) == 0 &&
	    unlink(path) == 0 &&
	    kn_bbs_store_repair(dir, findings, 8, &count) ==
	    KN_BBS_STORE_MAINTENANCE_OK &&
	    path_join3(dir, "index", "all.idx", path, sizeof(path)) == 0 &&
	    access(path, F_OK) == 0 ? 0 : 1;
	kn_message_store_close(&store);
	cleanup_store(dir);
	return rc;
}

static int
test_stats(void)
{
	struct kn_message_store store;
	struct kn_bbs_store_stats stats;
	uint64_t id;
	char dir[256];
	int rc;

	if (open_store(&store, dir, sizeof(dir)) != 0)
		return 1;
	rc = kn_message_store_create_private(&store, "M6VPN-1", "N0CALL",
	    "Subject", (const uint8_t *)"body", 4, &id) ==
	    KN_MESSAGE_STORE_OK &&
	    kn_bbs_user_create(&store, "M6VPN-1", 1, NULL) ==
	    KN_BBS_USER_OK &&
	    kn_bbs_read_state_mark_read(&store, "M6VPN-1", id) ==
	    KN_BBS_READ_STATE_OK &&
	    kn_bbs_store_stats(dir, &stats) == KN_BBS_STORE_MAINTENANCE_OK &&
	    stats.total_messages == 1 && stats.private_messages == 1 &&
	    stats.users == 1 && stats.read_state_files == 1 &&
	    stats.total_body_bytes == 4 ? 0 : 1;
	kn_message_store_close(&store);
	cleanup_store(dir);
	return rc;
}
