/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_message_store.c */

#define _XOPEN_SOURCE 700

#include <sys/stat.h>
#include <sys/types.h>

#include <stdint.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "kilonode/message_store.h"

static void cleanup_store(const char *);
static int make_store(char *, size_t);
static int path_join3(const char *, const char *, const char *, char *,
	size_t);
static int write_text(const char *, const char *);
static int test_body_missing(void);
static int test_close_partial_safe(void);
static int test_corrupt_metadata_skipped(void);
static int test_corrupt_next_id(void);
static int test_create_bulletin_next_id(void);
static int test_create_first_private(void);
static int test_init_empty_store(void);
static int test_missing_message(void);
static int test_next_id_persists(void);
static int test_overlarge_body(void);
static int test_path_traversal_rejected(void);
static int test_read_body(void);
static int test_read_deleted(void);
static int test_read_metadata(void);
static int test_soft_delete_hides_from_list(void);

int
main(void)
{
	if (test_init_empty_store() != 0)
		return 1;
	if (test_create_first_private() != 0)
		return 1;
	if (test_create_bulletin_next_id() != 0)
		return 1;
	if (test_next_id_persists() != 0)
		return 1;
	if (test_read_metadata() != 0)
		return 1;
	if (test_read_body() != 0)
		return 1;
	if (test_soft_delete_hides_from_list() != 0)
		return 1;
	if (test_read_deleted() != 0)
		return 1;
	if (test_missing_message() != 0)
		return 1;
	if (test_corrupt_next_id() != 0)
		return 1;
	if (test_corrupt_metadata_skipped() != 0)
		return 1;
	if (test_body_missing() != 0)
		return 1;
	if (test_overlarge_body() != 0)
		return 1;
	if (test_path_traversal_rejected() != 0)
		return 1;
	if (test_close_partial_safe() != 0)
		return 1;

	return 0;
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
		(void)snprintf(path, sizeof(path), "%s/msg/%08u.meta.tmp", dir,
		    i);
		(void)unlink(path);
		(void)snprintf(path, sizeof(path), "%s/msg/%08u.body.tmp", dir,
		    i);
		(void)unlink(path);
	}
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
		needed = snprintf(buf, bufsiz, "/tmp/kilonode-msg-%ld-%u",
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
test_body_missing(void)
{
	struct kn_message_store store;
	uint8_t body[8];
	size_t body_len;
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
	    kn_message_store_read_body(&store, id, body, sizeof(body),
	    &body_len) == KN_MESSAGE_STORE_ERR_CORRUPT ? 0 : 1;
	kn_message_store_close(&store);
	cleanup_store(dir);
	return rc;
}

static int
test_close_partial_safe(void)
{
	struct kn_message_store store;

	kn_message_store_init(&store);
	kn_message_store_close(&store);
	kn_message_store_close(NULL);
	return 0;
}

static int
test_corrupt_metadata_skipped(void)
{
	struct kn_message_store store;
	struct kn_message messages[4];
	size_t count;
	char dir[256];
	char path[512];
	int rc;

	if (make_store(dir, sizeof(dir)) != 0)
		return 1;
	kn_message_store_init(&store);
	rc = 1;
	if (kn_message_store_open(&store, dir, KN_MESSAGE_BODY_MAX) ==
	    KN_MESSAGE_STORE_OK &&
	    path_join3(dir, "msg", "00000001.meta", path, sizeof(path)) == 0 &&
	    write_text(path, "bad\n") == 0 &&
	    path_join3(dir, "meta", "next-id", path, sizeof(path)) == 0 &&
	    write_text(path, "2\n") == 0 &&
	    kn_message_store_list(&store, messages, 4, &count) ==
	    KN_MESSAGE_STORE_OK && count == 0)
		rc = 0;
	kn_message_store_close(&store);
	cleanup_store(dir);
	return rc;
}

static int
test_corrupt_next_id(void)
{
	struct kn_message_store store;
	char dir[256];
	char path[512];
	int rc;

	if (make_store(dir, sizeof(dir)) != 0)
		return 1;
	kn_message_store_init(&store);
	rc = 1;
	if (kn_message_store_open(&store, dir, KN_MESSAGE_BODY_MAX) ==
	    KN_MESSAGE_STORE_OK &&
	    path_join3(dir, "meta", "next-id", path, sizeof(path)) == 0 &&
	    write_text(path, "bad\n") == 0) {
		kn_message_store_close(&store);
		rc = kn_message_store_open(&store, dir, KN_MESSAGE_BODY_MAX) ==
		    KN_MESSAGE_STORE_ERR_CORRUPT ? 0 : 1;
	}
	kn_message_store_close(&store);
	cleanup_store(dir);
	return rc;
}

static int
test_create_bulletin_next_id(void)
{
	struct kn_message_store store;
	uint64_t first;
	uint64_t second;
	char dir[256];
	int rc;

	if (make_store(dir, sizeof(dir)) != 0)
		return 1;
	kn_message_store_init(&store);
	rc = kn_message_store_open(&store, dir, KN_MESSAGE_BODY_MAX) ==
	    KN_MESSAGE_STORE_OK &&
	    kn_message_store_create_private(&store, "M6VPN-1", "N0CALL",
	    "Subject", (const uint8_t *)"body", 4, &first) ==
	    KN_MESSAGE_STORE_OK &&
	    kn_message_store_create_bulletin(&store, "M6VPN-1", "GENERAL",
	    "Notice", (const uint8_t *)"body", 4, &second) ==
	    KN_MESSAGE_STORE_OK && first == 1 && second == 2 ? 0 : 1;
	kn_message_store_close(&store);
	cleanup_store(dir);
	return rc;
}

static int
test_create_first_private(void)
{
	struct kn_message_store store;
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
	    KN_MESSAGE_STORE_OK && id == 1 ? 0 : 1;
	kn_message_store_close(&store);
	cleanup_store(dir);
	return rc;
}

static int
test_init_empty_store(void)
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
	    kn_message_store_list(&store, messages, 4, &count) ==
	    KN_MESSAGE_STORE_OK && count == 0 ? 0 : 1;
	kn_message_store_close(&store);
	cleanup_store(dir);
	return rc;
}

static int
test_missing_message(void)
{
	struct kn_message_store store;
	struct kn_message message;
	char dir[256];
	int rc;

	if (make_store(dir, sizeof(dir)) != 0)
		return 1;
	kn_message_store_init(&store);
	rc = kn_message_store_open(&store, dir, KN_MESSAGE_BODY_MAX) ==
	    KN_MESSAGE_STORE_OK &&
	    kn_message_store_read_metadata(&store, 99, &message) ==
	    KN_MESSAGE_STORE_ERR_NOT_FOUND ? 0 : 1;
	kn_message_store_close(&store);
	cleanup_store(dir);
	return rc;
}

static int
test_next_id_persists(void)
{
	struct kn_message_store store;
	uint64_t id;
	char dir[256];
	int rc;

	if (make_store(dir, sizeof(dir)) != 0)
		return 1;
	kn_message_store_init(&store);
	rc = 1;
	if (kn_message_store_open(&store, dir, KN_MESSAGE_BODY_MAX) ==
	    KN_MESSAGE_STORE_OK &&
	    kn_message_store_create_private(&store, "M6VPN-1", "N0CALL",
	    "Subject", (const uint8_t *)"body", 4, &id) ==
	    KN_MESSAGE_STORE_OK && id == 1) {
		kn_message_store_close(&store);
		if (kn_message_store_open(&store, dir, KN_MESSAGE_BODY_MAX) ==
		    KN_MESSAGE_STORE_OK &&
		    kn_message_store_create_bulletin(&store, "M6VPN-1",
		    "GENERAL", "Notice", (const uint8_t *)"body", 4, &id) ==
		    KN_MESSAGE_STORE_OK && id == 2)
			rc = 0;
	}
	kn_message_store_close(&store);
	cleanup_store(dir);
	return rc;
}

static int
test_overlarge_body(void)
{
	struct kn_message_store store;
	uint64_t id;
	char dir[256];
	int rc;

	if (make_store(dir, sizeof(dir)) != 0)
		return 1;
	kn_message_store_init(&store);
	rc = kn_message_store_open(&store, dir, 4) == KN_MESSAGE_STORE_OK &&
	    kn_message_store_create_private(&store, "M6VPN-1", "N0CALL",
	    "Subject", (const uint8_t *)"12345", 5, &id) ==
	    KN_MESSAGE_STORE_ERR_BODY_TOO_LARGE ? 0 : 1;
	kn_message_store_close(&store);
	cleanup_store(dir);
	return rc;
}

static int
test_path_traversal_rejected(void)
{
	struct kn_message_store store;
	uint64_t id;
	char dir[256];
	int rc;

	if (make_store(dir, sizeof(dir)) != 0)
		return 1;
	kn_message_store_init(&store);
	rc = kn_message_store_open(&store, dir, KN_MESSAGE_BODY_MAX) ==
	    KN_MESSAGE_STORE_OK &&
	    kn_message_store_create_bulletin(&store, "M6VPN-1", "../BAD",
	    "Subject", (const uint8_t *)"body", 4, &id) ==
	    KN_MESSAGE_STORE_ERR_INVALID_MESSAGE ? 0 : 1;
	kn_message_store_close(&store);
	cleanup_store(dir);
	return rc;
}

static int
test_read_body(void)
{
	struct kn_message_store store;
	uint8_t body[8];
	size_t body_len;
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
	    kn_message_store_read_body(&store, id, body, sizeof(body),
	    &body_len) == KN_MESSAGE_STORE_OK && body_len == 4 &&
	    memcmp(body, "body", 4) == 0 ? 0 : 1;
	kn_message_store_close(&store);
	cleanup_store(dir);
	return rc;
}

static int
test_read_deleted(void)
{
	struct kn_message_store store;
	struct kn_message message;
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
	    kn_message_store_read_metadata(&store, id, &message) ==
	    KN_MESSAGE_STORE_ERR_DELETED ? 0 : 1;
	kn_message_store_close(&store);
	cleanup_store(dir);
	return rc;
}

static int
test_read_metadata(void)
{
	struct kn_message_store store;
	struct kn_message message;
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
	    kn_message_store_read_metadata(&store, id, &message) ==
	    KN_MESSAGE_STORE_OK &&
	    strcmp(message.subject, "Subject") == 0 ? 0 : 1;
	kn_message_store_close(&store);
	cleanup_store(dir);
	return rc;
}

static int
test_soft_delete_hides_from_list(void)
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
	    kn_message_store_list(&store, messages, 4, &count) ==
	    KN_MESSAGE_STORE_OK && count == 0 ? 0 : 1;
	kn_message_store_close(&store);
	cleanup_store(dir);
	return rc;
}
