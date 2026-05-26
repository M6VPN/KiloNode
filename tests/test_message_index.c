/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_message_index.c */

#define _XOPEN_SOURCE 700

#include <sys/stat.h>
#include <sys/types.h>

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "kilonode/message_index.h"

static void cleanup_store(const char *);
static int make_store(char *, size_t);
static int path_join3(const char *, const char *, const char *, char *,
	size_t);
static int write_text(const char *, const char *);
static int test_area_counts(void);
static int test_corrupt_index_rebuild(void);
static int test_create_bulletin_indexes(void);
static int test_create_empty_index_dir(void);
static int test_create_private_indexes(void);
static int test_deleted_hidden(void);
static int test_duplicate_index_entry_ignored(void);
static int test_list_filters(void);
static int test_missing_body_rebuild_skips(void);
static int test_missing_index_rebuild(void);
static int test_path_traversal_filter_rejected(void);

int
main(void)
{
	if (test_create_empty_index_dir() != 0)
		return 1;
	if (test_create_private_indexes() != 0)
		return 1;
	if (test_create_bulletin_indexes() != 0)
		return 1;
	if (test_list_filters() != 0)
		return 1;
	if (test_area_counts() != 0)
		return 1;
	if (test_deleted_hidden() != 0)
		return 1;
	if (test_missing_body_rebuild_skips() != 0)
		return 1;
	if (test_missing_index_rebuild() != 0)
		return 1;
	if (test_corrupt_index_rebuild() != 0)
		return 1;
	if (test_duplicate_index_entry_ignored() != 0)
		return 1;
	if (test_path_traversal_filter_rejected() != 0)
		return 1;

	return 0;
}

static void
cleanup_store(const char *dir)
{
	char path[512];
	unsigned int i;

	for (i = 1; i <= 8; i++) {
		(void)snprintf(path, sizeof(path), "%s/msg/%08u.meta", dir, i);
		(void)unlink(path);
		(void)snprintf(path, sizeof(path), "%s/msg/%08u.body", dir, i);
		(void)unlink(path);
		(void)snprintf(path, sizeof(path), "%s/msg/%08u.meta.tmp", dir,
		    i);
		(void)unlink(path);
		(void)snprintf(path, sizeof(path), "%s/msg/%08u.body.tmp", dir,
		    i);
		(void)unlink(path);
	}
	(void)snprintf(path, sizeof(path), "%s/index/all.idx", dir);
	(void)unlink(path);
	(void)snprintf(path, sizeof(path), "%s/index/private.idx", dir);
	(void)unlink(path);
	(void)snprintf(path, sizeof(path), "%s/index/bulletin.idx", dir);
	(void)unlink(path);
	(void)snprintf(path, sizeof(path), "%s/index/area-GENERAL.idx", dir);
	(void)unlink(path);
	(void)snprintf(path, sizeof(path), "%s/index/area-LOCAL.idx", dir);
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
make_store(char *buf, size_t bufsiz)
{
	unsigned int i;
	int needed;

	for (i = 0; i < 100; i++) {
		needed = snprintf(buf, bufsiz, "/tmp/kilonode-index-%ld-%u",
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
test_area_counts(void)
{
	struct kn_message_store store;
	struct kn_message_index_area areas[4];
	size_t count;
	uint64_t id;
	char dir[256];
	int rc;

	if (make_store(dir, sizeof(dir)) != 0)
		return 1;
	kn_message_store_init(&store);
	rc = kn_message_store_open(&store, dir, KN_MESSAGE_BODY_MAX) ==
	    KN_MESSAGE_STORE_OK &&
	    kn_message_store_create_bulletin(&store, "M6VPN-1", "GENERAL",
	    "One", (const uint8_t *)"body", 4, &id) ==
	    KN_MESSAGE_STORE_OK &&
	    kn_message_store_create_bulletin(&store, "M6VPN-1", "LOCAL",
	    "Two", (const uint8_t *)"body", 4, &id) ==
	    KN_MESSAGE_STORE_OK &&
	    kn_message_store_create_bulletin(&store, "M6VPN-1", "GENERAL",
	    "Three", (const uint8_t *)"body", 4, &id) ==
	    KN_MESSAGE_STORE_OK &&
	    kn_message_index_areas(&store, areas, 4, &count) ==
	    KN_MESSAGE_INDEX_OK && count == 2 &&
	    strcmp(areas[0].name, "GENERAL") == 0 && areas[0].count == 2 &&
	    strcmp(areas[1].name, "LOCAL") == 0 && areas[1].count == 1 ? 0 : 1;
	kn_message_store_close(&store);
	cleanup_store(dir);
	return rc;
}

static int
test_corrupt_index_rebuild(void)
{
	struct kn_message_store store;
	struct kn_message messages[4];
	size_t count;
	uint64_t id;
	char dir[256];
	char path[512];
	int rc;

	if (make_store(dir, sizeof(dir)) != 0)
		return 1;
	kn_message_store_init(&store);
	rc = 1;
	if (kn_message_store_open(&store, dir, KN_MESSAGE_BODY_MAX) ==
	    KN_MESSAGE_STORE_OK &&
	    kn_message_store_create_private(&store, "M6VPN-1", "N0CALL",
	    "Subject", (const uint8_t *)"body", 4, &id) ==
	    KN_MESSAGE_STORE_OK &&
	    path_join3(dir, "index", "all.idx", path, sizeof(path)) == 0 &&
	    write_text(path, "bad\n") == 0 &&
	    kn_message_index_list(&store, KN_MESSAGE_INDEX_ALL, NULL,
	    messages, 4, &count) == KN_MESSAGE_INDEX_OK && count == 1)
		rc = 0;
	kn_message_store_close(&store);
	cleanup_store(dir);
	return rc;
}

static int
test_create_bulletin_indexes(void)
{
	struct kn_message_store store;
	struct kn_message messages[4];
	size_t count;
	uint64_t id;
	char dir[256];
	int rc;

	if (make_store(dir, sizeof(dir)) != 0)
		return 1;
	kn_message_store_init(&store);
	rc = kn_message_store_open(&store, dir, KN_MESSAGE_BODY_MAX) ==
	    KN_MESSAGE_STORE_OK &&
	    kn_message_store_create_bulletin(&store, "M6VPN-1", "GENERAL",
	    "Subject", (const uint8_t *)"body", 4, &id) ==
	    KN_MESSAGE_STORE_OK &&
	    kn_message_index_list(&store, KN_MESSAGE_INDEX_BULLETIN, NULL,
	    messages, 4, &count) == KN_MESSAGE_INDEX_OK && count == 1 &&
	    kn_message_index_list(&store, KN_MESSAGE_INDEX_AREA, "GENERAL",
	    messages, 4, &count) == KN_MESSAGE_INDEX_OK && count == 1 &&
	    kn_message_index_list(&store, KN_MESSAGE_INDEX_FROM, "M6VPN-1",
	    messages, 4, &count) == KN_MESSAGE_INDEX_OK && count == 1 ? 0 : 1;
	kn_message_store_close(&store);
	cleanup_store(dir);
	return rc;
}

static int
test_create_empty_index_dir(void)
{
	struct kn_message_store store;
	struct kn_message messages[4];
	size_t count;
	char dir[256];
	int rc;

	if (make_store(dir, sizeof(dir)) != 0)
		return 1;
	kn_message_store_init(&store);
	rc = kn_message_store_open(&store, dir, KN_MESSAGE_BODY_MAX) ==
	    KN_MESSAGE_STORE_OK &&
	    kn_message_index_rebuild(&store) == KN_MESSAGE_INDEX_OK &&
	    kn_message_index_list(&store, KN_MESSAGE_INDEX_ALL, NULL,
	    messages, 4, &count) == KN_MESSAGE_INDEX_OK && count == 0 ? 0 : 1;
	kn_message_store_close(&store);
	cleanup_store(dir);
	return rc;
}

static int
test_create_private_indexes(void)
{
	struct kn_message_store store;
	struct kn_message messages[4];
	size_t count;
	uint64_t id;
	char dir[256];
	int rc;

	if (make_store(dir, sizeof(dir)) != 0)
		return 1;
	kn_message_store_init(&store);
	rc = kn_message_store_open(&store, dir, KN_MESSAGE_BODY_MAX) ==
	    KN_MESSAGE_STORE_OK &&
	    kn_message_store_create_private(&store, "M6VPN-1", "N0CALL",
	    "Subject", (const uint8_t *)"body", 4, &id) ==
	    KN_MESSAGE_STORE_OK &&
	    kn_message_index_list(&store, KN_MESSAGE_INDEX_PRIVATE, NULL,
	    messages, 4, &count) == KN_MESSAGE_INDEX_OK && count == 1 &&
	    kn_message_index_list(&store, KN_MESSAGE_INDEX_TO, "N0CALL",
	    messages, 4, &count) == KN_MESSAGE_INDEX_OK && count == 1 &&
	    kn_message_index_list(&store, KN_MESSAGE_INDEX_FROM, "M6VPN-1",
	    messages, 4, &count) == KN_MESSAGE_INDEX_OK && count == 1 ? 0 : 1;
	kn_message_store_close(&store);
	cleanup_store(dir);
	return rc;
}

static int
test_deleted_hidden(void)
{
	struct kn_message_store store;
	struct kn_message messages[4];
	size_t count;
	uint64_t id;
	char dir[256];
	int rc;

	if (make_store(dir, sizeof(dir)) != 0)
		return 1;
	kn_message_store_init(&store);
	rc = kn_message_store_open(&store, dir, KN_MESSAGE_BODY_MAX) ==
	    KN_MESSAGE_STORE_OK &&
	    kn_message_store_create_private(&store, "M6VPN-1", "N0CALL",
	    "Subject", (const uint8_t *)"body", 4, &id) ==
	    KN_MESSAGE_STORE_OK &&
	    kn_message_store_delete(&store, id) == KN_MESSAGE_STORE_OK &&
	    kn_message_index_list(&store, KN_MESSAGE_INDEX_ALL, NULL,
	    messages, 4, &count) == KN_MESSAGE_INDEX_OK && count == 0 ? 0 : 1;
	kn_message_store_close(&store);
	cleanup_store(dir);
	return rc;
}

static int
test_duplicate_index_entry_ignored(void)
{
	struct kn_message_store store;
	struct kn_message messages[4];
	size_t count;
	uint64_t id;
	char dir[256];
	char path[512];
	int rc;

	if (make_store(dir, sizeof(dir)) != 0)
		return 1;
	kn_message_store_init(&store);
	rc = kn_message_store_open(&store, dir, KN_MESSAGE_BODY_MAX) ==
	    KN_MESSAGE_STORE_OK &&
	    kn_message_store_create_private(&store, "M6VPN-1", "N0CALL",
	    "Subject", (const uint8_t *)"body", 4, &id) ==
	    KN_MESSAGE_STORE_OK &&
	    path_join3(dir, "index", "all.idx", path, sizeof(path)) == 0 &&
	    write_text(path,
	    "1|private|M6VPN-1|N0CALL|1|0|0|Subject\n"
	    "1|private|M6VPN-1|N0CALL|1|0|0|Subject\n") == 0 &&
	    kn_message_index_list(&store, KN_MESSAGE_INDEX_ALL, NULL,
	    messages, 4, &count) == KN_MESSAGE_INDEX_OK && count == 1 ? 0 : 1;
	kn_message_store_close(&store);
	cleanup_store(dir);
	return rc;
}

static int
test_list_filters(void)
{
	struct kn_message_store store;
	struct kn_message messages[4];
	size_t count;
	uint64_t id;
	char dir[256];
	int rc;

	if (make_store(dir, sizeof(dir)) != 0)
		return 1;
	kn_message_store_init(&store);
	rc = kn_message_store_open(&store, dir, KN_MESSAGE_BODY_MAX) ==
	    KN_MESSAGE_STORE_OK &&
	    kn_message_store_create_private(&store, "M6VPN-1", "N0CALL",
	    "Private", (const uint8_t *)"body", 4, &id) ==
	    KN_MESSAGE_STORE_OK &&
	    kn_message_store_create_bulletin(&store, "M6VPN-1", "GENERAL",
	    "Bulletin", (const uint8_t *)"body", 4, &id) ==
	    KN_MESSAGE_STORE_OK &&
	    kn_message_index_list(&store, KN_MESSAGE_INDEX_ALL, NULL,
	    messages, 4, &count) == KN_MESSAGE_INDEX_OK && count == 2 &&
	    kn_message_index_list(&store, KN_MESSAGE_INDEX_PRIVATE, NULL,
	    messages, 4, &count) == KN_MESSAGE_INDEX_OK && count == 1 &&
	    kn_message_index_list(&store, KN_MESSAGE_INDEX_BULLETIN, NULL,
	    messages, 4, &count) == KN_MESSAGE_INDEX_OK && count == 1 ? 0 : 1;
	kn_message_store_close(&store);
	cleanup_store(dir);
	return rc;
}

static int
test_missing_body_rebuild_skips(void)
{
	struct kn_message_store store;
	struct kn_message messages[4];
	size_t count;
	uint64_t id;
	char dir[256];
	char path[512];
	int rc;

	if (make_store(dir, sizeof(dir)) != 0)
		return 1;
	kn_message_store_init(&store);
	rc = kn_message_store_open(&store, dir, KN_MESSAGE_BODY_MAX) ==
	    KN_MESSAGE_STORE_OK &&
	    kn_message_store_create_private(&store, "M6VPN-1", "N0CALL",
	    "Subject", (const uint8_t *)"body", 4, &id) ==
	    KN_MESSAGE_STORE_OK &&
	    path_join3(dir, "msg", "00000001.body", path, sizeof(path)) == 0 &&
	    unlink(path) == 0 &&
	    kn_message_index_rebuild(&store) == KN_MESSAGE_INDEX_OK &&
	    kn_message_index_list(&store, KN_MESSAGE_INDEX_ALL, NULL,
	    messages, 4, &count) == KN_MESSAGE_INDEX_OK && count == 0 ? 0 : 1;
	kn_message_store_close(&store);
	cleanup_store(dir);
	return rc;
}

static int
test_missing_index_rebuild(void)
{
	struct kn_message_store store;
	struct kn_message messages[4];
	size_t count;
	uint64_t id;
	char dir[256];
	char path[512];
	int rc;

	if (make_store(dir, sizeof(dir)) != 0)
		return 1;
	kn_message_store_init(&store);
	rc = kn_message_store_open(&store, dir, KN_MESSAGE_BODY_MAX) ==
	    KN_MESSAGE_STORE_OK &&
	    kn_message_store_create_private(&store, "M6VPN-1", "N0CALL",
	    "Subject", (const uint8_t *)"body", 4, &id) ==
	    KN_MESSAGE_STORE_OK &&
	    path_join3(dir, "index", "all.idx", path, sizeof(path)) == 0 &&
	    unlink(path) == 0 &&
	    kn_message_index_rebuild(&store) == KN_MESSAGE_INDEX_OK &&
	    kn_message_index_list(&store, KN_MESSAGE_INDEX_ALL, NULL,
	    messages, 4, &count) == KN_MESSAGE_INDEX_OK && count == 1 ? 0 : 1;
	kn_message_store_close(&store);
	cleanup_store(dir);
	return rc;
}

static int
test_path_traversal_filter_rejected(void)
{
	struct kn_message_store store;
	struct kn_message messages[4];
	size_t count;
	char dir[256];
	int rc;

	if (make_store(dir, sizeof(dir)) != 0)
		return 1;
	kn_message_store_init(&store);
	rc = kn_message_store_open(&store, dir, KN_MESSAGE_BODY_MAX) ==
	    KN_MESSAGE_STORE_OK &&
	    kn_message_index_list(&store, KN_MESSAGE_INDEX_AREA, "../BAD",
	    messages, 4, &count) == KN_MESSAGE_INDEX_ERR_INVALID_ARGUMENT ?
	    0 : 1;
	kn_message_store_close(&store);
	cleanup_store(dir);
	return rc;
}
