/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_bbs_shell_store.c */

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
static int open_shell(struct kn_message_store *, struct kn_bbs_shell_session *,
	struct kn_bbs_shell_snapshot *, char *, size_t);
static int run_bbs(const char *, struct kn_bbs_shell_session *,
	struct kn_bbs_shell_snapshot *, char *, size_t);
static int test_areas_after_bulletin(void);
static int test_invalid_bulletin_area(void);
static int test_invalid_private_destination(void);
static int test_kill_hides_message(void);
static int test_list_filters(void);
static int test_overlarge_body(void);
static int test_read_marks_read(void);
static int test_read_sanitizes_body(void);
static int test_send_bulletin_and_list(void);
static int test_send_private_and_read(void);

int
main(void)
{
	if (test_send_private_and_read() != 0)
		return 1;
	if (test_send_bulletin_and_list() != 0)
		return 1;
	if (test_kill_hides_message() != 0)
		return 1;
	if (test_areas_after_bulletin() != 0)
		return 1;
	if (test_list_filters() != 0)
		return 1;
	if (test_read_marks_read() != 0)
		return 1;
	if (test_invalid_private_destination() != 0)
		return 1;
	if (test_invalid_bulletin_area() != 0)
		return 1;
	if (test_overlarge_body() != 0)
		return 1;
	if (test_read_sanitizes_body() != 0)
		return 1;

	return 0;
}

static void
cleanup_store(const char *dir)
{
	char path[512];
	unsigned int i;

	for (i = 1; i <= 16; i++) {
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
		needed = snprintf(buf, bufsiz, "/tmp/kilonode-bbs-store-%ld-%u",
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
open_shell(struct kn_message_store *store, struct kn_bbs_shell_session *session,
	struct kn_bbs_shell_snapshot *snapshot, char *dir, size_t max_body)
{
	if (make_store(dir, 256) != 0)
		return 1;
	kn_message_store_init(store);
	kn_bbs_shell_reset(session);
	snapshot->store = store;
	snapshot->enabled = 1;
	snapshot->max_body_bytes = max_body;
	if (kn_message_store_open(store, dir, max_body) != KN_MESSAGE_STORE_OK) {
		cleanup_store(dir);
		return 1;
	}

	return 0;
}

static int
run_bbs(const char *command, struct kn_bbs_shell_session *session,
	struct kn_bbs_shell_snapshot *snapshot, char *out, size_t out_len)
{
	uint8_t close_session;
	uint8_t exit_bbs;

	return kn_bbs_shell_format(session, command, snapshot, out, out_len,
	    &close_session, &exit_bbs) == KN_BBS_SHELL_OK ? 0 : 1;
}

static int
test_areas_after_bulletin(void)
{
	struct kn_message_store store;
	struct kn_bbs_shell_session session;
	struct kn_bbs_shell_snapshot snapshot;
	char dir[256];
	char out[KN_NODE_SHELL_RESPONSE_MAX];
	int rc;

	if (open_shell(&store, &session, &snapshot, dir,
	    KN_MESSAGE_BODY_MAX) != 0)
		return 1;
	rc = run_bbs("SEND BULLETIN M6VPN-1 GENERAL \"Bulletin\"",
	    &session, &snapshot, out, sizeof(out)) == 0 &&
	    run_bbs("body", &session, &snapshot, out, sizeof(out)) == 0 &&
	    run_bbs(".", &session, &snapshot, out, sizeof(out)) == 0 &&
	    run_bbs("AREAS", &session, &snapshot, out, sizeof(out)) == 0 &&
	    strstr(out, "AREA name=GENERAL") != NULL ? 0 : 1;
	kn_message_store_close(&store);
	cleanup_store(dir);
	return rc;
}

static int
test_invalid_bulletin_area(void)
{
	struct kn_message_store store;
	struct kn_bbs_shell_session session;
	struct kn_bbs_shell_snapshot snapshot;
	char dir[256];
	char out[KN_NODE_SHELL_RESPONSE_MAX];
	int rc;

	if (open_shell(&store, &session, &snapshot, dir,
	    KN_MESSAGE_BODY_MAX) != 0)
		return 1;
	rc = run_bbs("SEND BULLETIN M6VPN-1 BAD! \"Subject\"",
	    &session, &snapshot, out, sizeof(out)) == 0 &&
	    strstr(out, "ERR invalid-message") != NULL ? 0 : 1;
	kn_message_store_close(&store);
	cleanup_store(dir);
	return rc;
}

static int
test_invalid_private_destination(void)
{
	struct kn_message_store store;
	struct kn_bbs_shell_session session;
	struct kn_bbs_shell_snapshot snapshot;
	char dir[256];
	char out[KN_NODE_SHELL_RESPONSE_MAX];
	int rc;

	if (open_shell(&store, &session, &snapshot, dir,
	    KN_MESSAGE_BODY_MAX) != 0)
		return 1;
	rc = run_bbs("SEND PRIVATE M6VPN-1 BAD! \"Subject\"",
	    &session, &snapshot, out, sizeof(out)) == 0 &&
	    strstr(out, "ERR invalid-message") != NULL ? 0 : 1;
	kn_message_store_close(&store);
	cleanup_store(dir);
	return rc;
}

static int
test_kill_hides_message(void)
{
	struct kn_message_store store;
	struct kn_bbs_shell_session session;
	struct kn_bbs_shell_snapshot snapshot;
	char dir[256];
	char out[KN_NODE_SHELL_RESPONSE_MAX];
	int rc;

	if (open_shell(&store, &session, &snapshot, dir,
	    KN_MESSAGE_BODY_MAX) != 0)
		return 1;
	rc = run_bbs("SEND PRIVATE M6VPN-1 N0CALL \"Subject\"",
	    &session, &snapshot, out, sizeof(out)) == 0 &&
	    run_bbs("body", &session, &snapshot, out, sizeof(out)) == 0 &&
	    run_bbs(".", &session, &snapshot, out, sizeof(out)) == 0 &&
	    run_bbs("KILL 1", &session, &snapshot, out, sizeof(out)) == 0 &&
	    strstr(out, "OK KILL id=1") != NULL &&
	    run_bbs("LIST", &session, &snapshot, out, sizeof(out)) == 0 &&
	    strstr(out, "OK LIST count=0") != NULL ? 0 : 1;
	kn_message_store_close(&store);
	cleanup_store(dir);
	return rc;
}

static int
test_list_filters(void)
{
	struct kn_message_store store;
	struct kn_bbs_shell_session session;
	struct kn_bbs_shell_snapshot snapshot;
	char dir[256];
	char out[KN_NODE_SHELL_RESPONSE_MAX];
	int rc;

	if (open_shell(&store, &session, &snapshot, dir,
	    KN_MESSAGE_BODY_MAX) != 0)
		return 1;
	rc = run_bbs("SEND PRIVATE M6VPN-1 N0CALL \"Private\"",
	    &session, &snapshot, out, sizeof(out)) == 0 &&
	    run_bbs("body", &session, &snapshot, out, sizeof(out)) == 0 &&
	    run_bbs(".", &session, &snapshot, out, sizeof(out)) == 0 &&
	    run_bbs("SEND BULLETIN M6VPN-1 GENERAL \"Bulletin\"",
	    &session, &snapshot, out, sizeof(out)) == 0 &&
	    run_bbs("body", &session, &snapshot, out, sizeof(out)) == 0 &&
	    run_bbs(".", &session, &snapshot, out, sizeof(out)) == 0 &&
	    run_bbs("LIST PRIVATE", &session, &snapshot, out, sizeof(out)) ==
	    0 && strstr(out, "OK LIST count=1") != NULL &&
	    strstr(out, "type=private") != NULL &&
	    run_bbs("LIST BULLETINS", &session, &snapshot, out,
	    sizeof(out)) == 0 && strstr(out, "type=bulletin") != NULL &&
	    run_bbs("LIST AREA GENERAL", &session, &snapshot, out,
	    sizeof(out)) == 0 && strstr(out, "to=GENERAL") != NULL &&
	    run_bbs("LIST TO N0CALL", &session, &snapshot, out,
	    sizeof(out)) == 0 && strstr(out, "type=private") != NULL &&
	    run_bbs("LIST FROM M6VPN-1", &session, &snapshot, out,
	    sizeof(out)) == 0 && strstr(out, "OK LIST count=2") != NULL &&
	    run_bbs("LIST AREA UNKNOWN", &session, &snapshot, out,
	    sizeof(out)) == 0 && strstr(out, "OK LIST count=0") != NULL &&
	    run_bbs("LIST TO BAD!", &session, &snapshot, out,
	    sizeof(out)) == 0 && strstr(out, "ERR index-error") != NULL &&
	    run_bbs("LIST BAD", &session, &snapshot, out, sizeof(out)) == 0 &&
	    strstr(out, "ERR invalid-list") != NULL ? 0 : 1;
	kn_message_store_close(&store);
	cleanup_store(dir);
	return rc;
}

static int
test_overlarge_body(void)
{
	struct kn_message_store store;
	struct kn_bbs_shell_session session;
	struct kn_bbs_shell_snapshot snapshot;
	char dir[256];
	char out[KN_NODE_SHELL_RESPONSE_MAX];
	int rc;

	if (open_shell(&store, &session, &snapshot, dir, 8) != 0)
		return 1;
	rc = run_bbs("SEND PRIVATE M6VPN-1 N0CALL \"Subject\"",
	    &session, &snapshot, out, sizeof(out)) == 0 &&
	    run_bbs("123456789", &session, &snapshot, out, sizeof(out)) == 0 &&
	    run_bbs(".", &session, &snapshot, out, sizeof(out)) == 0 &&
	    strstr(out, "ERR body-too-large") != NULL ? 0 : 1;
	kn_message_store_close(&store);
	cleanup_store(dir);
	return rc;
}

static int
test_read_marks_read(void)
{
	struct kn_message_store store;
	struct kn_bbs_shell_session session;
	struct kn_bbs_shell_snapshot snapshot;
	char dir[256];
	char out[KN_NODE_SHELL_RESPONSE_MAX];
	int rc;

	if (open_shell(&store, &session, &snapshot, dir,
	    KN_MESSAGE_BODY_MAX) != 0)
		return 1;
	rc = run_bbs("SEND PRIVATE M6VPN-1 N0CALL \"Subject\"",
	    &session, &snapshot, out, sizeof(out)) == 0 &&
	    run_bbs("body", &session, &snapshot, out, sizeof(out)) == 0 &&
	    run_bbs(".", &session, &snapshot, out, sizeof(out)) == 0 &&
	    run_bbs("LIST", &session, &snapshot, out, sizeof(out)) == 0 &&
	    strstr(out, "read=no") != NULL &&
	    run_bbs("READ 1", &session, &snapshot, out, sizeof(out)) == 0 &&
	    strstr(out, "read=yes") != NULL &&
	    run_bbs("LIST", &session, &snapshot, out, sizeof(out)) == 0 &&
	    strstr(out, "read=yes") != NULL ? 0 : 1;
	kn_message_store_close(&store);
	cleanup_store(dir);
	return rc;
}

static int
test_read_sanitizes_body(void)
{
	struct kn_message_store store;
	struct kn_bbs_shell_session session;
	struct kn_bbs_shell_snapshot snapshot;
	char dir[256];
	char out[KN_NODE_SHELL_RESPONSE_MAX];
	int rc;

	if (open_shell(&store, &session, &snapshot, dir,
	    KN_MESSAGE_BODY_MAX) != 0)
		return 1;
	rc = run_bbs("SEND PRIVATE M6VPN-1 N0CALL \"Subject\"",
	    &session, &snapshot, out, sizeof(out)) == 0 &&
	    run_bbs("hello\001there", &session, &snapshot, out,
	    sizeof(out)) == 0 &&
	    run_bbs(".", &session, &snapshot, out, sizeof(out)) == 0 &&
	    run_bbs("READ 1", &session, &snapshot, out, sizeof(out)) == 0 &&
	    strstr(out, "hello?there") != NULL ? 0 : 1;
	kn_message_store_close(&store);
	cleanup_store(dir);
	return rc;
}

static int
test_send_bulletin_and_list(void)
{
	struct kn_message_store store;
	struct kn_bbs_shell_session session;
	struct kn_bbs_shell_snapshot snapshot;
	char dir[256];
	char out[KN_NODE_SHELL_RESPONSE_MAX];
	int rc;

	if (open_shell(&store, &session, &snapshot, dir,
	    KN_MESSAGE_BODY_MAX) != 0)
		return 1;
	rc = run_bbs("SEND BULLETIN M6VPN-1 GENERAL \"Bulletin\"",
	    &session, &snapshot, out, sizeof(out)) == 0 &&
	    run_bbs("body", &session, &snapshot, out, sizeof(out)) == 0 &&
	    run_bbs(".", &session, &snapshot, out, sizeof(out)) == 0 &&
	    strstr(out, "OK STORED id=1") != NULL &&
	    run_bbs("LIST", &session, &snapshot, out, sizeof(out)) == 0 &&
	    strstr(out, "type=bulletin") != NULL &&
	    strstr(out, "to=GENERAL") != NULL ? 0 : 1;
	kn_message_store_close(&store);
	cleanup_store(dir);
	return rc;
}

static int
test_send_private_and_read(void)
{
	struct kn_message_store store;
	struct kn_bbs_shell_session session;
	struct kn_bbs_shell_snapshot snapshot;
	char dir[256];
	char out[KN_NODE_SHELL_RESPONSE_MAX];
	int rc;

	if (open_shell(&store, &session, &snapshot, dir,
	    KN_MESSAGE_BODY_MAX) != 0)
		return 1;
	rc = run_bbs("SEND PRIVATE M6VPN-1 N0CALL \"Test private\"",
	    &session, &snapshot, out, sizeof(out)) == 0 &&
	    run_bbs("Hello from KiloNode.", &session, &snapshot, out,
	    sizeof(out)) == 0 &&
	    run_bbs(".", &session, &snapshot, out, sizeof(out)) == 0 &&
	    strstr(out, "OK STORED id=1") != NULL &&
	    run_bbs("READ 1", &session, &snapshot, out, sizeof(out)) == 0 &&
	    strstr(out, "subject=Test private") != NULL &&
	    strstr(out, "Hello from KiloNode.") != NULL ? 0 : 1;
	kn_message_store_close(&store);
	cleanup_store(dir);
	return rc;
}
