/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_bbs_shell.c */

#define _XOPEN_SOURCE 700

#include <sys/stat.h>
#include <sys/types.h>

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "kilonode/bbs_shell.h"
#include "kilonode/node_shell.h"

static void cleanup_store(const char *);
static int make_store(char *, size_t);
static int run_bbs(const char *, struct kn_bbs_shell_session *,
	struct kn_bbs_shell_snapshot *, char *, size_t, uint8_t *, uint8_t *);
static int test_areas_empty(void);
static int test_bye_closes(void);
static int test_empty_line(void);
static int test_help(void);
static int test_kill_missing(void);
static int test_read_missing(void);
static int test_unknown_command(void);

int
main(void)
{
	if (test_help() != 0)
		return 1;
	if (test_areas_empty() != 0)
		return 1;
	if (test_read_missing() != 0)
		return 1;
	if (test_kill_missing() != 0)
		return 1;
	if (test_bye_closes() != 0)
		return 1;
	if (test_unknown_command() != 0)
		return 1;
	if (test_empty_line() != 0)
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
	(void)snprintf(path, sizeof(path), "%s/meta/next-id", dir);
	(void)unlink(path);
	(void)snprintf(path, sizeof(path), "%s/meta/next-id.tmp", dir);
	(void)unlink(path);
	(void)snprintf(path, sizeof(path), "%s/index/all.idx", dir);
	(void)unlink(path);
	(void)snprintf(path, sizeof(path), "%s/index/private.idx", dir);
	(void)unlink(path);
	(void)snprintf(path, sizeof(path), "%s/index/bulletin.idx", dir);
	(void)unlink(path);
	(void)snprintf(path, sizeof(path), "%s/index", dir);
	(void)rmdir(path);
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
		needed = snprintf(buf, bufsiz, "/tmp/kilonode-bbs-%ld-%u",
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
run_bbs(const char *command, struct kn_bbs_shell_session *session,
	struct kn_bbs_shell_snapshot *snapshot, char *out, size_t out_len,
	uint8_t *close_session, uint8_t *exit_bbs)
{
	return kn_bbs_shell_format(session, command, snapshot, out, out_len,
	    close_session, exit_bbs) == KN_BBS_SHELL_OK ? 0 : 1;
}

static int
test_areas_empty(void)
{
	struct kn_message_store store;
	struct kn_bbs_shell_session session;
	struct kn_bbs_shell_snapshot snapshot;
	char dir[256];
	char out[KN_NODE_SHELL_RESPONSE_MAX];
	uint8_t close_session;
	uint8_t exit_bbs;
	int rc;

	if (make_store(dir, sizeof(dir)) != 0)
		return 1;
	kn_message_store_init(&store);
	kn_bbs_shell_reset(&session);
	memcpy(session.identity, "M6VPN-1", 8);
	snapshot.store = &store;
	snapshot.enabled = 1;
	snapshot.max_body_bytes = KN_MESSAGE_BODY_MAX;
	rc = kn_message_store_open(&store, dir, KN_MESSAGE_BODY_MAX) ==
	    KN_MESSAGE_STORE_OK &&
	    run_bbs("AREAS", &session, &snapshot, out, sizeof(out),
	    &close_session, &exit_bbs) == 0 &&
	    strstr(out, "OK AREAS count=0\r\nEND\r\n") != NULL ? 0 : 1;
	kn_message_store_close(&store);
	cleanup_store(dir);
	return rc;
}

static int
test_bye_closes(void)
{
	struct kn_message_store store;
	struct kn_bbs_shell_session session;
	struct kn_bbs_shell_snapshot snapshot;
	char dir[256];
	char out[KN_NODE_SHELL_RESPONSE_MAX];
	uint8_t close_session;
	uint8_t exit_bbs;
	int rc;

	if (make_store(dir, sizeof(dir)) != 0)
		return 1;
	kn_message_store_init(&store);
	kn_bbs_shell_reset(&session);
	snapshot.store = &store;
	snapshot.enabled = 1;
	snapshot.max_body_bytes = KN_MESSAGE_BODY_MAX;
	rc = kn_message_store_open(&store, dir, KN_MESSAGE_BODY_MAX) ==
	    KN_MESSAGE_STORE_OK &&
	    run_bbs("BYE", &session, &snapshot, out, sizeof(out),
	    &close_session, &exit_bbs) == 0 && close_session == 1 &&
	    strcmp(out, "BYE\r\n") == 0 ? 0 : 1;
	kn_message_store_close(&store);
	cleanup_store(dir);
	return rc;
}

static int
test_empty_line(void)
{
	struct kn_message_store store;
	struct kn_bbs_shell_session session;
	struct kn_bbs_shell_snapshot snapshot;
	char dir[256];
	char out[KN_NODE_SHELL_RESPONSE_MAX];
	uint8_t close_session;
	uint8_t exit_bbs;
	int rc;

	if (make_store(dir, sizeof(dir)) != 0)
		return 1;
	kn_message_store_init(&store);
	kn_bbs_shell_reset(&session);
	snapshot.store = &store;
	snapshot.enabled = 1;
	snapshot.max_body_bytes = KN_MESSAGE_BODY_MAX;
	rc = kn_message_store_open(&store, dir, KN_MESSAGE_BODY_MAX) ==
	    KN_MESSAGE_STORE_OK &&
	    run_bbs("", &session, &snapshot, out, sizeof(out),
	    &close_session, &exit_bbs) == 0 &&
	    strcmp(out, KN_BBS_SHELL_PROMPT) == 0 ? 0 : 1;
	kn_message_store_close(&store);
	cleanup_store(dir);
	return rc;
}

static int
test_help(void)
{
	struct kn_message_store store;
	struct kn_bbs_shell_session session;
	struct kn_bbs_shell_snapshot snapshot;
	char dir[256];
	char out[KN_NODE_SHELL_RESPONSE_MAX];
	uint8_t close_session;
	uint8_t exit_bbs;
	int rc;

	if (make_store(dir, sizeof(dir)) != 0)
		return 1;
	kn_message_store_init(&store);
	kn_bbs_shell_reset(&session);
	snapshot.store = &store;
	snapshot.enabled = 1;
	snapshot.max_body_bytes = KN_MESSAGE_BODY_MAX;
	rc = kn_message_store_open(&store, dir, KN_MESSAGE_BODY_MAX) ==
	    KN_MESSAGE_STORE_OK &&
	    run_bbs("HELP", &session, &snapshot, out, sizeof(out),
	    &close_session, &exit_bbs) == 0 &&
	    strstr(out, "WHOAMI USERS AREAS LIST UNREAD READ MARKREAD") != NULL ?
	    0 : 1;
	kn_message_store_close(&store);
	cleanup_store(dir);
	return rc;
}

static int
test_kill_missing(void)
{
	struct kn_message_store store;
	struct kn_bbs_shell_session session;
	struct kn_bbs_shell_snapshot snapshot;
	char dir[256];
	char out[KN_NODE_SHELL_RESPONSE_MAX];
	uint8_t close_session;
	uint8_t exit_bbs;
	int rc;

	if (make_store(dir, sizeof(dir)) != 0)
		return 1;
	kn_message_store_init(&store);
	kn_bbs_shell_reset(&session);
	memcpy(session.identity, "M6VPN-1", 8);
	snapshot.store = &store;
	snapshot.enabled = 1;
	snapshot.max_body_bytes = KN_MESSAGE_BODY_MAX;
	rc = kn_message_store_open(&store, dir, KN_MESSAGE_BODY_MAX) ==
	    KN_MESSAGE_STORE_OK &&
	    run_bbs("KILL 99", &session, &snapshot, out, sizeof(out),
	    &close_session, &exit_bbs) == 0 &&
	    strstr(out, "ERR store-error") != NULL ? 0 : 1;
	kn_message_store_close(&store);
	cleanup_store(dir);
	return rc;
}

static int
test_read_missing(void)
{
	struct kn_message_store store;
	struct kn_bbs_shell_session session;
	struct kn_bbs_shell_snapshot snapshot;
	char dir[256];
	char out[KN_NODE_SHELL_RESPONSE_MAX];
	uint8_t close_session;
	uint8_t exit_bbs;
	int rc;

	if (make_store(dir, sizeof(dir)) != 0)
		return 1;
	kn_message_store_init(&store);
	kn_bbs_shell_reset(&session);
	memcpy(session.identity, "M6VPN-1", 8);
	snapshot.store = &store;
	snapshot.enabled = 1;
	snapshot.max_body_bytes = KN_MESSAGE_BODY_MAX;
	rc = kn_message_store_open(&store, dir, KN_MESSAGE_BODY_MAX) ==
	    KN_MESSAGE_STORE_OK &&
	    run_bbs("READ 99", &session, &snapshot, out, sizeof(out),
	    &close_session, &exit_bbs) == 0 &&
	    strstr(out, "ERR store-error") != NULL ? 0 : 1;
	kn_message_store_close(&store);
	cleanup_store(dir);
	return rc;
}

static int
test_unknown_command(void)
{
	struct kn_message_store store;
	struct kn_bbs_shell_session session;
	struct kn_bbs_shell_snapshot snapshot;
	char dir[256];
	char out[KN_NODE_SHELL_RESPONSE_MAX];
	uint8_t close_session;
	uint8_t exit_bbs;
	int rc;

	if (make_store(dir, sizeof(dir)) != 0)
		return 1;
	kn_message_store_init(&store);
	kn_bbs_shell_reset(&session);
	snapshot.store = &store;
	snapshot.enabled = 1;
	snapshot.max_body_bytes = KN_MESSAGE_BODY_MAX;
	rc = kn_message_store_open(&store, dir, KN_MESSAGE_BODY_MAX) ==
	    KN_MESSAGE_STORE_OK &&
	    run_bbs("NOPE", &session, &snapshot, out, sizeof(out),
	    &close_session, &exit_bbs) == 0 &&
	    strstr(out, "ERR unknown-command") != NULL ? 0 : 1;
	kn_message_store_close(&store);
	cleanup_store(dir);
	return rc;
}
