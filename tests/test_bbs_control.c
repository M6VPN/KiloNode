/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_bbs_control.c */

#define _XOPEN_SOURCE 700

#include <sys/stat.h>
#include <sys/types.h>

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "kilonode/bbs_control.h"
#include "kilonode/bbs_read_state.h"
#include "kilonode/bbs_user.h"
#include "kilonode/control.h"

static void cleanup_store(const char *);
static int make_store(char *, size_t);
static int open_store(struct kn_message_store *, char *, size_t);
static int populate_store(struct kn_message_store *, uint64_t *, uint64_t *);
static int test_areas_empty(void);
static int test_areas_populated(void);
static int test_disabled_status(void);
static int test_invalid_filters(void);
static int test_message_deleted(void);
static int test_message_missing(void);
static int test_message_preview_binary(void);
static int test_message_preview_text(void);
static int test_messages_filters(void);
static int test_messages_truncated(void);
static int test_status_enabled(void);
static int test_stats_empty(void);
static int test_stats_populated(void);
static int test_unknown_command(void);
static int test_users_empty(void);
static int test_users_populated(void);

int
main(void)
{
	if (test_disabled_status() != 0)
		return 1;
	if (test_status_enabled() != 0)
		return 1;
	if (test_stats_empty() != 0)
		return 1;
	if (test_stats_populated() != 0)
		return 1;
	if (test_areas_empty() != 0)
		return 1;
	if (test_areas_populated() != 0)
		return 1;
	if (test_users_empty() != 0)
		return 1;
	if (test_users_populated() != 0)
		return 1;
	if (test_messages_filters() != 0)
		return 1;
	if (test_message_preview_text() != 0)
		return 1;
	if (test_message_preview_binary() != 0)
		return 1;
	if (test_message_deleted() != 0)
		return 1;
	if (test_message_missing() != 0)
		return 1;
	if (test_invalid_filters() != 0)
		return 1;
	if (test_messages_truncated() != 0)
		return 1;
	if (test_unknown_command() != 0)
		return 1;
	return 0;
}

static void
cleanup_store(const char *dir)
{
	char path[512];
	const char *areas[] = { "GENERAL", "LOCAL" };
	const char *calls[] = { "M6VPN-1", "N0CALL" };
	unsigned int i;
	size_t j;

	for (i = 1; i <= 48; i++) {
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
	(void)snprintf(path, sizeof(path), "%s/.kilonode-store.lock", dir);
	(void)unlink(path);
	for (j = 0; j < sizeof(calls) / sizeof(calls[0]); j++) {
		(void)snprintf(path, sizeof(path), "%s/read/%s.read", dir,
		    calls[j]);
		(void)unlink(path);
		(void)snprintf(path, sizeof(path), "%s/users/%s.user", dir,
		    calls[j]);
		(void)unlink(path);
		(void)snprintf(path, sizeof(path), "%s/index/to-%s.idx", dir,
		    calls[j]);
		(void)unlink(path);
		(void)snprintf(path, sizeof(path), "%s/index/from-%s.idx", dir,
		    calls[j]);
		(void)unlink(path);
	}
	for (j = 0; j < sizeof(areas) / sizeof(areas[0]); j++) {
		(void)snprintf(path, sizeof(path), "%s/index/area-%s.idx", dir,
		    areas[j]);
		(void)unlink(path);
	}
	(void)snprintf(path, sizeof(path), "%s/index/all.idx", dir);
	(void)unlink(path);
	(void)snprintf(path, sizeof(path), "%s/index/private.idx", dir);
	(void)unlink(path);
	(void)snprintf(path, sizeof(path), "%s/index/bulletin.idx", dir);
	(void)unlink(path);
	(void)snprintf(path, sizeof(path), "%s/index", dir);
	(void)rmdir(path);
	(void)snprintf(path, sizeof(path), "%s/read", dir);
	(void)rmdir(path);
	(void)snprintf(path, sizeof(path), "%s/users", dir);
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
		needed = snprintf(buf, bufsiz, "/tmp/kilonode-bbsctl-%ld-%u",
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
open_store(struct kn_message_store *store, char *dir, size_t dir_len)
{
	if (make_store(dir, dir_len) != 0)
		return 1;
	kn_message_store_init(store);
	if (kn_message_store_open(store, dir, KN_MESSAGE_BODY_MAX) !=
	    KN_MESSAGE_STORE_OK) {
		cleanup_store(dir);
		return 1;
	}
	if (kn_bbs_user_init_store(store) != KN_BBS_USER_OK ||
	    kn_bbs_read_state_init_store(store) != KN_BBS_READ_STATE_OK) {
		cleanup_store(dir);
		return 1;
	}
	return 0;
}

static int
populate_store(struct kn_message_store *store, uint64_t *private_id,
	uint64_t *bulletin_id)
{
	const uint8_t private_body[] = "Hello from KiloNode\n";
	const uint8_t bulletin_body[] = "Local bulletin\n";

	if (kn_message_store_create_private(store, "M6VPN-1", "N0CALL",
	    "Test private", private_body, sizeof(private_body) - 1,
	    private_id) != KN_MESSAGE_STORE_OK)
		return 1;
	if (kn_message_store_create_bulletin(store, "M6VPN-1", "GENERAL",
	    "Test bulletin", bulletin_body, sizeof(bulletin_body) - 1,
	    bulletin_id) != KN_MESSAGE_STORE_OK)
		return 1;
	if (kn_bbs_user_create(store, "M6VPN-1", 1710000000, NULL) !=
	    KN_BBS_USER_OK)
		return 1;
	if (kn_bbs_user_seen(store, "N0CALL", 1710000100, NULL) !=
	    KN_BBS_USER_OK)
		return 1;
	return 0;
}

static int
test_areas_empty(void)
{
	struct kn_message_store store;
	char dir[256];
	char out[KN_CONTROL_QUEUE_MAX];
	int rc;

	if (open_store(&store, dir, sizeof(dir)) != 0)
		return 1;
	rc = kn_bbs_control_format("AREAS", 1, &store, out, sizeof(out)) ==
	    KN_BBS_CONTROL_OK && strstr(out, "OK BBS AREAS count=0") != NULL &&
	    strstr(out, "END\n") != NULL ? 0 : 1;
	kn_message_store_close(&store);
	cleanup_store(dir);
	return rc;
}

static int
test_areas_populated(void)
{
	struct kn_message_store store;
	uint64_t private_id;
	uint64_t bulletin_id;
	char dir[256];
	char out[KN_CONTROL_QUEUE_MAX];
	int rc;

	if (open_store(&store, dir, sizeof(dir)) != 0)
		return 1;
	if (populate_store(&store, &private_id, &bulletin_id) != 0)
		return 1;
	rc = kn_bbs_control_format("AREAS", 1, &store, out, sizeof(out)) ==
	    KN_BBS_CONTROL_OK && strstr(out, "BBS AREA name=GENERAL") != NULL ?
	    0 : 1;
	kn_message_store_close(&store);
	cleanup_store(dir);
	return rc;
}

static int
test_disabled_status(void)
{
	char out[KN_CONTROL_QUEUE_MAX];

	if (kn_bbs_control_format("STATUS", 0, NULL, out, sizeof(out)) !=
	    KN_BBS_CONTROL_OK)
		return 1;
	if (strcmp(out,
	    "OK BBS STATUS enabled=false open=false\nEND\n") != 0)
		return 1;
	if (kn_bbs_control_format("STATS", 0, NULL, out, sizeof(out)) !=
	    KN_BBS_CONTROL_ERR_DISABLED)
		return 1;
	return strcmp(out, "ERR bbs-disabled\n") == 0 ? 0 : 1;
}

static int
test_invalid_filters(void)
{
	struct kn_message_store store;
	char dir[256];
	char out[KN_CONTROL_QUEUE_MAX];
	int rc;

	if (open_store(&store, dir, sizeof(dir)) != 0)
		return 1;
	rc = 0;
	if (kn_bbs_control_format("MESSAGES AREA ../BAD", 1, &store, out,
	    sizeof(out)) != KN_BBS_CONTROL_ERR_INVALID_AREA)
		rc = 1;
	if (strcmp(out, "ERR invalid-area\n") != 0)
		rc = 1;
	if (kn_bbs_control_format("MESSAGES TO BAD-99", 1, &store, out,
	    sizeof(out)) != KN_BBS_CONTROL_ERR_INVALID_CALLSIGN)
		rc = 1;
	if (strcmp(out, "ERR invalid-callsign\n") != 0)
		rc = 1;
	kn_message_store_close(&store);
	cleanup_store(dir);
	return rc;
}

static int
test_message_deleted(void)
{
	struct kn_message_store store;
	uint64_t private_id;
	uint64_t bulletin_id;
	char command[64];
	char dir[256];
	char out[KN_CONTROL_QUEUE_MAX];
	int rc;

	if (open_store(&store, dir, sizeof(dir)) != 0)
		return 1;
	if (populate_store(&store, &private_id, &bulletin_id) != 0)
		return 1;
	(void)bulletin_id;
	if (kn_message_store_delete(&store, private_id) != KN_MESSAGE_STORE_OK)
		return 1;
	(void)snprintf(command, sizeof(command), "MESSAGE %llu",
	    (unsigned long long)private_id);
	rc = kn_bbs_control_format(command, 1, &store, out, sizeof(out)) ==
	    KN_BBS_CONTROL_ERR_STORE && strcmp(out, "ERR message-deleted\n") ==
	    0 ? 0 : 1;
	kn_message_store_close(&store);
	cleanup_store(dir);
	return rc;
}

static int
test_message_missing(void)
{
	struct kn_message_store store;
	char dir[256];
	char out[KN_CONTROL_QUEUE_MAX];
	int rc;

	if (open_store(&store, dir, sizeof(dir)) != 0)
		return 1;
	rc = kn_bbs_control_format("MESSAGE 99", 1, &store, out,
	    sizeof(out)) == KN_BBS_CONTROL_ERR_STORE &&
	    strcmp(out, "ERR message-not-found\n") == 0 ? 0 : 1;
	kn_message_store_close(&store);
	cleanup_store(dir);
	return rc;
}

static int
test_message_preview_binary(void)
{
	struct kn_message_store store;
	const uint8_t body[] = { 0x00, 0x41, 0xff };
	uint64_t id;
	char command[64];
	char dir[256];
	char out[KN_CONTROL_QUEUE_MAX];
	int rc;

	if (open_store(&store, dir, sizeof(dir)) != 0)
		return 1;
	if (kn_message_store_create_private(&store, "M6VPN-1", "N0CALL",
	    "Binary", body, sizeof(body), &id) != KN_MESSAGE_STORE_OK)
		return 1;
	(void)snprintf(command, sizeof(command), "MESSAGE %llu",
	    (unsigned long long)id);
	rc = kn_bbs_control_format(command, 1, &store, out, sizeof(out)) ==
	    KN_BBS_CONTROL_OK && strstr(out, "binary=true") != NULL &&
	    strstr(out, "hex=0041ff") != NULL ? 0 : 1;
	kn_message_store_close(&store);
	cleanup_store(dir);
	return rc;
}

static int
test_message_preview_text(void)
{
	struct kn_message_store store;
	uint64_t private_id;
	uint64_t bulletin_id;
	char command[64];
	char dir[256];
	char out[KN_CONTROL_QUEUE_MAX];
	int rc;

	if (open_store(&store, dir, sizeof(dir)) != 0)
		return 1;
	if (populate_store(&store, &private_id, &bulletin_id) != 0)
		return 1;
	(void)bulletin_id;
	(void)snprintf(command, sizeof(command), "MESSAGE %llu",
	    (unsigned long long)private_id);
	rc = kn_bbs_control_format(command, 1, &store, out, sizeof(out)) ==
	    KN_BBS_CONTROL_OK && strstr(out, "OK BBS MESSAGE id=") != NULL &&
	    strstr(out, "binary=false") != NULL &&
	    strstr(out, "Hello from KiloNode") != NULL ? 0 : 1;
	kn_message_store_close(&store);
	cleanup_store(dir);
	return rc;
}

static int
test_messages_filters(void)
{
	struct kn_message_store store;
	uint64_t private_id;
	uint64_t bulletin_id;
	char dir[256];
	char out[KN_CONTROL_QUEUE_MAX];
	int rc;

	if (open_store(&store, dir, sizeof(dir)) != 0)
		return 1;
	if (populate_store(&store, &private_id, &bulletin_id) != 0)
		return 1;
	(void)private_id;
	(void)bulletin_id;
	rc = 0;
	if (kn_bbs_control_format("MESSAGES", 1, &store, out,
	    sizeof(out)) != KN_BBS_CONTROL_OK ||
	    strstr(out, "OK BBS MESSAGES count=2") == NULL)
		rc = 1;
	if (kn_bbs_control_format("MESSAGES PRIVATE", 1, &store, out,
	    sizeof(out)) != KN_BBS_CONTROL_OK ||
	    strstr(out, "type=private") == NULL)
		rc = 1;
	if (kn_bbs_control_format("MESSAGES BULLETINS", 1, &store, out,
	    sizeof(out)) != KN_BBS_CONTROL_OK ||
	    strstr(out, "type=bulletin") == NULL)
		rc = 1;
	if (kn_bbs_control_format("MESSAGES AREA GENERAL", 1, &store, out,
	    sizeof(out)) != KN_BBS_CONTROL_OK ||
	    strstr(out, "area=GENERAL") == NULL)
		rc = 1;
	if (kn_bbs_control_format("MESSAGES TO N0CALL", 1, &store, out,
	    sizeof(out)) != KN_BBS_CONTROL_OK ||
	    strstr(out, "to=N0CALL") == NULL)
		rc = 1;
	if (kn_bbs_control_format("MESSAGES FROM M6VPN-1", 1, &store, out,
	    sizeof(out)) != KN_BBS_CONTROL_OK ||
	    strstr(out, "from=M6VPN-1") == NULL)
		rc = 1;
	kn_message_store_close(&store);
	cleanup_store(dir);
	return rc;
}

static int
test_messages_truncated(void)
{
	struct kn_message_store store;
	const uint8_t body[] = "x";
	uint64_t id;
	char dir[256];
	char out[KN_CONTROL_QUEUE_MAX];
	unsigned int i;
	int rc;

	if (open_store(&store, dir, sizeof(dir)) != 0)
		return 1;
	for (i = 0; i < KN_BBS_CONTROL_LIST_MAX + 2; i++) {
		if (kn_message_store_create_private(&store, "M6VPN-1",
		    "N0CALL", "Many", body, sizeof(body) - 1, &id) !=
		    KN_MESSAGE_STORE_OK)
			return 1;
	}
	rc = kn_bbs_control_format("MESSAGES", 1, &store, out, sizeof(out)) ==
	    KN_BBS_CONTROL_OK && strstr(out, "truncated=true") != NULL ? 0 : 1;
	kn_message_store_close(&store);
	cleanup_store(dir);
	return rc;
}

static int
test_status_enabled(void)
{
	struct kn_message_store store;
	char dir[256];
	char out[KN_CONTROL_QUEUE_MAX];
	int rc;

	if (open_store(&store, dir, sizeof(dir)) != 0)
		return 1;
	rc = kn_bbs_control_format("STATUS", 1, &store, out, sizeof(out)) ==
	    KN_BBS_CONTROL_OK &&
	    strstr(out, "OK BBS STATUS enabled=true open=true store=") != NULL &&
	    strstr(out, "END\n") != NULL ? 0 : 1;
	kn_message_store_close(&store);
	cleanup_store(dir);
	return rc;
}

static int
test_stats_empty(void)
{
	struct kn_message_store store;
	char dir[256];
	char out[KN_CONTROL_QUEUE_MAX];
	int rc;

	if (open_store(&store, dir, sizeof(dir)) != 0)
		return 1;
	rc = kn_bbs_control_format("STATS", 1, &store, out, sizeof(out)) ==
	    KN_BBS_CONTROL_OK && strstr(out, "OK BBS STATS total=0") != NULL ?
	    0 : 1;
	kn_message_store_close(&store);
	cleanup_store(dir);
	return rc;
}

static int
test_stats_populated(void)
{
	struct kn_message_store store;
	uint64_t private_id;
	uint64_t bulletin_id;
	char dir[256];
	char out[KN_CONTROL_QUEUE_MAX];
	int rc;

	if (open_store(&store, dir, sizeof(dir)) != 0)
		return 1;
	if (populate_store(&store, &private_id, &bulletin_id) != 0)
		return 1;
	rc = kn_bbs_control_format("STATS", 1, &store, out, sizeof(out)) ==
	    KN_BBS_CONTROL_OK && strstr(out, "private=1") != NULL &&
	    strstr(out, "bulletins=1") != NULL &&
	    strstr(out, "users=2") != NULL ? 0 : 1;
	kn_message_store_close(&store);
	cleanup_store(dir);
	return rc;
}

static int
test_unknown_command(void)
{
	struct kn_message_store store;
	char dir[256];
	char out[KN_CONTROL_QUEUE_MAX];
	int rc;

	if (open_store(&store, dir, sizeof(dir)) != 0)
		return 1;
	rc = kn_bbs_control_format("NOPE", 1, &store, out, sizeof(out)) ==
	    KN_BBS_CONTROL_ERR_UNKNOWN_COMMAND &&
	    strcmp(out, "ERR unknown-bbs-command\n") == 0 ? 0 : 1;
	kn_message_store_close(&store);
	cleanup_store(dir);
	return rc;
}

static int
test_users_empty(void)
{
	struct kn_message_store store;
	char dir[256];
	char out[KN_CONTROL_QUEUE_MAX];
	int rc;

	if (open_store(&store, dir, sizeof(dir)) != 0)
		return 1;
	rc = kn_bbs_control_format("USERS", 1, &store, out, sizeof(out)) ==
	    KN_BBS_CONTROL_OK && strstr(out, "OK BBS USERS count=0") != NULL ?
	    0 : 1;
	kn_message_store_close(&store);
	cleanup_store(dir);
	return rc;
}

static int
test_users_populated(void)
{
	struct kn_message_store store;
	uint64_t private_id;
	uint64_t bulletin_id;
	char dir[256];
	char out[KN_CONTROL_QUEUE_MAX];
	int rc;

	if (open_store(&store, dir, sizeof(dir)) != 0)
		return 1;
	if (populate_store(&store, &private_id, &bulletin_id) != 0)
		return 1;
	rc = kn_bbs_control_format("USERS", 1, &store, out, sizeof(out)) ==
	    KN_BBS_CONTROL_OK && strstr(out, "BBS USER call=M6VPN-1") != NULL &&
	    strstr(out, "BBS USER call=N0CALL") != NULL ? 0 : 1;
	kn_message_store_close(&store);
	cleanup_store(dir);
	return rc;
}
