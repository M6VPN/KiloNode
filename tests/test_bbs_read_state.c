/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_bbs_read_state.c */

#define _XOPEN_SOURCE 700

#include <sys/stat.h>
#include <sys/types.h>

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "kilonode/bbs_read_state.h"

static void cleanup_store(const char *);
static int make_store(char *, size_t);
static int open_read_store(struct kn_message_store *, char *, size_t);
static int write_text(const char *, const char *);
static int test_corrupt_read_state(void);
static int test_duplicate_mark_read(void);
static int test_empty_state_unread(void);
static int test_invalid_inputs(void);
static int test_mark_save_reload(void);
static int test_unread_count(void);

int
main(void)
{
	if (test_empty_state_unread() != 0)
		return 1;
	if (test_mark_save_reload() != 0)
		return 1;
	if (test_duplicate_mark_read() != 0)
		return 1;
	if (test_corrupt_read_state() != 0)
		return 1;
	if (test_invalid_inputs() != 0)
		return 1;
	if (test_unread_count() != 0)
		return 1;

	return 0;
}

static void
cleanup_store(const char *dir)
{
	char path[512];

	(void)snprintf(path, sizeof(path), "%s/read/M6VPN-1.read", dir);
	(void)unlink(path);
	(void)snprintf(path, sizeof(path), "%s/read/M6VPN-1.read.tmp", dir);
	(void)unlink(path);
	(void)snprintf(path, sizeof(path), "%s/read/N0CALL.read", dir);
	(void)unlink(path);
	(void)snprintf(path, sizeof(path), "%s/read", dir);
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
		needed = snprintf(buf, bufsiz, "/tmp/kilonode-read-%ld-%u",
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
open_read_store(struct kn_message_store *store, char *dir, size_t dir_len)
{
	if (make_store(dir, dir_len) != 0)
		return 1;
	kn_message_store_init(store);
	if (kn_message_store_open(store, dir, KN_MESSAGE_BODY_MAX) !=
	    KN_MESSAGE_STORE_OK) {
		cleanup_store(dir);
		return 1;
	}
	if (kn_bbs_read_state_init_store(store) != KN_BBS_READ_STATE_OK) {
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
test_corrupt_read_state(void)
{
	struct kn_message_store store;
	struct kn_bbs_read_state state;
	char dir[256];
	char path[512];
	int needed;
	int rc;

	if (open_read_store(&store, dir, sizeof(dir)) != 0)
		return 1;
	needed = snprintf(path, sizeof(path), "%s/read/M6VPN-1.read", dir);
	if (needed < 0 || (size_t)needed >= sizeof(path)) {
		kn_message_store_close(&store);
		cleanup_store(dir);
		return 1;
	}
	rc = write_text(path, "bad\n") == 0 &&
	    kn_bbs_read_state_load(&store, "M6VPN-1", &state) ==
	    KN_BBS_READ_STATE_ERR_CORRUPT ? 0 : 1;
	kn_message_store_close(&store);
	cleanup_store(dir);
	return rc;
}

static int
test_duplicate_mark_read(void)
{
	struct kn_message_store store;
	struct kn_bbs_read_state state;
	char dir[256];
	int rc;

	if (open_read_store(&store, dir, sizeof(dir)) != 0)
		return 1;
	rc = kn_bbs_read_state_mark_read(&store, "M6VPN-1", 7) ==
	    KN_BBS_READ_STATE_OK &&
	    kn_bbs_read_state_mark_read(&store, "M6VPN-1", 7) ==
	    KN_BBS_READ_STATE_OK &&
	    kn_bbs_read_state_load(&store, "M6VPN-1", &state) ==
	    KN_BBS_READ_STATE_OK && state.count == 1 && state.ids[0] == 7 ?
	    0 : 1;
	kn_message_store_close(&store);
	cleanup_store(dir);
	return rc;
}

static int
test_empty_state_unread(void)
{
	struct kn_message_store store;
	uint8_t is_read;
	char dir[256];
	int rc;

	if (open_read_store(&store, dir, sizeof(dir)) != 0)
		return 1;
	is_read = 1;
	rc = kn_bbs_read_state_is_read(&store, "M6VPN-1", 1, &is_read) ==
	    KN_BBS_READ_STATE_OK && is_read == 0 ? 0 : 1;
	kn_message_store_close(&store);
	cleanup_store(dir);
	return rc;
}

static int
test_invalid_inputs(void)
{
	struct kn_message_store store;
	char dir[256];
	int rc;

	if (open_read_store(&store, dir, sizeof(dir)) != 0)
		return 1;
	rc = kn_bbs_read_state_mark_read(&store, "M6VPN-1", 0) ==
	    KN_BBS_READ_STATE_ERR_INVALID_ID &&
	    kn_bbs_read_state_mark_read(&store, "BAD!", 1) ==
	    KN_BBS_READ_STATE_ERR_INVALID_CALLSIGN ? 0 : 1;
	kn_message_store_close(&store);
	cleanup_store(dir);
	return rc;
}

static int
test_mark_save_reload(void)
{
	struct kn_message_store store;
	struct kn_bbs_read_state state;
	uint8_t is_read;
	char dir[256];
	int rc;

	if (open_read_store(&store, dir, sizeof(dir)) != 0)
		return 1;
	rc = kn_bbs_read_state_mark_read(&store, "M6VPN-1", 1) ==
	    KN_BBS_READ_STATE_OK &&
	    kn_bbs_read_state_mark_read(&store, "M6VPN-1", 2) ==
	    KN_BBS_READ_STATE_OK &&
	    kn_bbs_read_state_load(&store, "M6VPN-1", &state) ==
	    KN_BBS_READ_STATE_OK && state.count == 2 &&
	    kn_bbs_read_state_is_read(&store, "M6VPN-1", 2, &is_read) ==
	    KN_BBS_READ_STATE_OK && is_read == 1 ? 0 : 1;
	kn_message_store_close(&store);
	cleanup_store(dir);
	return rc;
}

static int
test_unread_count(void)
{
	struct kn_message_store store;
	struct kn_message messages[3];
	size_t unread;
	char dir[256];
	int rc;

	memset(messages, 0, sizeof(messages));
	messages[0].id = 1;
	messages[1].id = 2;
	messages[2].id = 3;
	if (open_read_store(&store, dir, sizeof(dir)) != 0)
		return 1;
	rc = kn_bbs_read_state_mark_read(&store, "M6VPN-1", 2) ==
	    KN_BBS_READ_STATE_OK &&
	    kn_bbs_read_state_unread_count(&store, "M6VPN-1", messages, 3,
	    &unread) == KN_BBS_READ_STATE_OK && unread == 2 ? 0 : 1;
	kn_message_store_close(&store);
	cleanup_store(dir);
	return rc;
}
