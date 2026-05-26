/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/tools/kilonode-msg.c */

#include <sys/types.h>

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kilonode/bbs_read_state.h"
#include "kilonode/bbs_store_lock.h"
#include "kilonode/message.h"
#include "kilonode/message_index.h"
#include "kilonode/message_store.h"

#define MSG_LIST_MAX 1024

static int body_file_read(const char *, uint8_t **, size_t *);
static int command_create_bulletin(struct kn_message_store *, int, char **);
static int command_create_private(struct kn_message_store *, int, char **);
static int command_delete(struct kn_message_store *, int, char **);
static int command_areas(struct kn_message_store *, int, char **);
static int command_init(struct kn_message_store *, int, char **);
static int command_list(struct kn_message_store *, int, char **);
static int command_list_filter(struct kn_message_store *,
	enum kn_message_index_filter, const char *, int, char **);
static int command_list_unread(struct kn_message_store *, int, char **);
static int command_mark_read(struct kn_message_store *, int, char **);
static int command_read(struct kn_message_store *, int, char **);
static int command_reindex(struct kn_message_store *, int, char **);
static int id_parse(const char *, uint64_t *);
static void message_print(const struct kn_message *);
static void usage(FILE *, const char *);

int
main(int argc, char *argv[])
{
	struct kn_message_store store;
	const char *store_path;
	const char *command;
	int i;
	int rc;

	store_path = NULL;
	command = NULL;
	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "--help") == 0) {
			usage(stdout, argv[0]);
			return 0;
		}
		if (strcmp(argv[i], "--store") == 0) {
			if (i + 1 >= argc) {
				usage(stderr, argv[0]);
				return 1;
			}
			store_path = argv[++i];
			continue;
		}
		command = argv[i];
		break;
	}

	if (store_path == NULL || command == NULL) {
		usage(stderr, argv[0]);
		return 1;
	}

	kn_message_store_init(&store);
	if (kn_message_store_open(&store, store_path,
	    KN_MESSAGE_BODY_MAX) != KN_MESSAGE_STORE_OK) {
		fprintf(stderr, "store open failed\n");
		return 1;
	}

	if (strcmp(command, "init") == 0)
		rc = command_init(&store, argc - i - 1, argv + i + 1);
	else if (strcmp(command, "create-private") == 0)
		rc = command_create_private(&store, argc - i - 1, argv + i + 1);
	else if (strcmp(command, "create-bulletin") == 0)
		rc = command_create_bulletin(&store, argc - i - 1, argv + i + 1);
	else if (strcmp(command, "list") == 0)
		rc = command_list(&store, argc - i - 1, argv + i + 1);
	else if (strcmp(command, "reindex") == 0)
		rc = command_reindex(&store, argc - i - 1, argv + i + 1);
	else if (strcmp(command, "areas") == 0)
		rc = command_areas(&store, argc - i - 1, argv + i + 1);
	else if (strcmp(command, "list-private") == 0)
		rc = command_list_filter(&store, KN_MESSAGE_INDEX_PRIVATE,
		    NULL, argc - i - 1, argv + i + 1);
	else if (strcmp(command, "list-bulletins") == 0)
		rc = command_list_filter(&store, KN_MESSAGE_INDEX_BULLETIN,
		    NULL, argc - i - 1, argv + i + 1);
	else if (strcmp(command, "list-area") == 0)
		rc = command_list_filter(&store, KN_MESSAGE_INDEX_AREA,
		    argc - i > 1 ? argv[i + 1] : NULL, argc - i - 2,
		    argv + i + 2);
	else if (strcmp(command, "list-to") == 0)
		rc = command_list_filter(&store, KN_MESSAGE_INDEX_TO,
		    argc - i > 1 ? argv[i + 1] : NULL, argc - i - 2,
		    argv + i + 2);
	else if (strcmp(command, "list-from") == 0)
		rc = command_list_filter(&store, KN_MESSAGE_INDEX_FROM,
		    argc - i > 1 ? argv[i + 1] : NULL, argc - i - 2,
		    argv + i + 2);
	else if (strcmp(command, "list-unread") == 0)
		rc = command_list_unread(&store, argc - i - 1, argv + i + 1);
	else if (strcmp(command, "mark-read") == 0)
		rc = command_mark_read(&store, argc - i - 1, argv + i + 1);
	else if (strcmp(command, "read") == 0)
		rc = command_read(&store, argc - i - 1, argv + i + 1);
	else if (strcmp(command, "delete") == 0)
		rc = command_delete(&store, argc - i - 1, argv + i + 1);
	else {
		usage(stderr, argv[0]);
		rc = 1;
	}

	kn_message_store_close(&store);
	return rc;
}

static int
body_file_read(const char *path, uint8_t **body_out, size_t *len_out)
{
	FILE *fp;
	uint8_t *body;
	size_t cap;
	size_t len;
	size_t nread;

	fp = fopen(path, "rb");
	if (fp == NULL)
		return 1;

	body = malloc(KN_MESSAGE_BODY_MAX);
	if (body == NULL) {
		(void)fclose(fp);
		return 1;
	}

	cap = KN_MESSAGE_BODY_MAX;
	len = 0;
	while (len < cap) {
		nread = fread(body + len, 1, cap - len, fp);
		len += nread;
		if (nread == 0)
			break;
	}
	if (ferror(fp) || (len == cap && fgetc(fp) != EOF)) {
		free(body);
		(void)fclose(fp);
		return 1;
	}

	(void)fclose(fp);
	*body_out = body;
	*len_out = len;
	return 0;
}

static int
command_areas(struct kn_message_store *store, int argc, char **argv)
{
	struct kn_message_index_area areas[MSG_LIST_MAX];
	size_t count;
	size_t i;

	(void)argv;
	if (argc != 0)
		return 1;
	if (kn_message_index_areas(store, areas, MSG_LIST_MAX, &count) !=
	    KN_MESSAGE_INDEX_OK)
		return 1;
	for (i = 0; i < count && i < MSG_LIST_MAX; i++) {
		printf("area=%s count=%llu newest=%llu\n", areas[i].name,
		    (unsigned long long)areas[i].count,
		    (unsigned long long)areas[i].newest_id);
	}
	return 0;
}

static int
command_create_bulletin(struct kn_message_store *store, int argc, char **argv)
{
	uint8_t *body;
	uint64_t id;
	size_t body_len;
	enum kn_message_store_error rc;

	if (argc != 4)
		return 1;
	if (body_file_read(argv[3], &body, &body_len) != 0)
		return 1;
	rc = kn_message_store_create_bulletin(store, argv[0], argv[1],
	    argv[2], body, body_len, &id);
	free(body);
	if (rc != KN_MESSAGE_STORE_OK) {
		fprintf(stderr, "create failed: %s\n",
		    kn_message_store_error_name(rc));
		return 1;
	}
	printf("created %llu\n", (unsigned long long)id);
	return 0;
}

static int
command_create_private(struct kn_message_store *store, int argc, char **argv)
{
	uint8_t *body;
	uint64_t id;
	size_t body_len;
	enum kn_message_store_error rc;

	if (argc != 4)
		return 1;
	if (body_file_read(argv[3], &body, &body_len) != 0)
		return 1;
	rc = kn_message_store_create_private(store, argv[0], argv[1],
	    argv[2], body, body_len, &id);
	free(body);
	if (rc != KN_MESSAGE_STORE_OK) {
		fprintf(stderr, "create failed: %s\n",
		    kn_message_store_error_name(rc));
		return 1;
	}
	printf("created %llu\n", (unsigned long long)id);
	return 0;
}

static int
command_delete(struct kn_message_store *store, int argc, char **argv)
{
	uint64_t id;
	enum kn_message_store_error rc;

	if (argc != 1 || id_parse(argv[0], &id) != 0)
		return 1;
	rc = kn_message_store_delete(store, id);
	if (rc != KN_MESSAGE_STORE_OK) {
		fprintf(stderr, "delete failed: %s\n",
		    kn_message_store_error_name(rc));
		return 1;
	}
	printf("deleted %llu\n", (unsigned long long)id);
	return 0;
}

static int
command_init(struct kn_message_store *store, int argc, char **argv)
{
	(void)argv;
	if (argc != 0)
		return 1;
	if (kn_bbs_read_state_init_store(store) != KN_BBS_READ_STATE_OK)
		return 1;
	printf("initialized\n");
	return 0;
}

static int
command_list(struct kn_message_store *store, int argc, char **argv)
{
	return command_list_filter(store, KN_MESSAGE_INDEX_ALL, NULL, argc,
	    argv);
}

static int
command_list_filter(struct kn_message_store *store,
	enum kn_message_index_filter filter, const char *value, int argc,
	char **argv)
{
	struct kn_message messages[MSG_LIST_MAX];
	size_t count;
	size_t i;

	(void)argv;
	if (argc != 0)
		return 1;
	if ((filter == KN_MESSAGE_INDEX_AREA || filter == KN_MESSAGE_INDEX_TO ||
	    filter == KN_MESSAGE_INDEX_FROM) && value == NULL)
		return 1;
	if (kn_message_index_list(store, filter, value, messages, MSG_LIST_MAX,
	    &count) != KN_MESSAGE_INDEX_OK)
		return 1;

	for (i = 0; i < count && i < MSG_LIST_MAX; i++)
		message_print(&messages[i]);
	return 0;
}

static int
command_list_unread(struct kn_message_store *store, int argc, char **argv)
{
	struct kn_message messages[MSG_LIST_MAX];
	size_t count;
	size_t i;
	uint8_t is_read;

	if (argc != 1)
		return 1;
	if (kn_message_index_list(store, KN_MESSAGE_INDEX_ALL, NULL, messages,
	    MSG_LIST_MAX, &count) != KN_MESSAGE_INDEX_OK)
		return 1;
	for (i = 0; i < count && i < MSG_LIST_MAX; i++) {
		if (kn_bbs_read_state_is_read(store, argv[0], messages[i].id,
		    &is_read) != KN_BBS_READ_STATE_OK)
			return 1;
		if (is_read == 0)
			message_print(&messages[i]);
	}
	return 0;
}

static int
command_mark_read(struct kn_message_store *store, int argc, char **argv)
{
	uint64_t id;

	if (argc != 2 || id_parse(argv[1], &id) != 0)
		return 1;
	if (kn_bbs_read_state_init_store(store) != KN_BBS_READ_STATE_OK)
		return 1;
	if (kn_bbs_read_state_mark_read(store, argv[0], id) !=
	    KN_BBS_READ_STATE_OK)
		return 1;
	printf("marked-read %s %llu\n", argv[0], (unsigned long long)id);
	return 0;
}

static int
command_read(struct kn_message_store *store, int argc, char **argv)
{
	struct kn_message message;
	uint8_t *body;
	uint64_t id;
	size_t body_len;
	enum kn_message_store_error rc;

	if (argc != 1 || id_parse(argv[0], &id) != 0)
		return 1;
	rc = kn_message_store_read_metadata(store, id, &message);
	if (rc != KN_MESSAGE_STORE_OK) {
		fprintf(stderr, "read failed: %s\n",
		    kn_message_store_error_name(rc));
		return 1;
	}
	body = malloc(message.body_len == 0 ? 1 : message.body_len);
	if (body == NULL)
		return 1;
	rc = kn_message_store_read_body(store, id, body, message.body_len,
	    &body_len);
	if (rc != KN_MESSAGE_STORE_OK) {
		free(body);
		fprintf(stderr, "read failed: %s\n",
		    kn_message_store_error_name(rc));
		return 1;
	}
	message_print(&message);
	(void)fwrite(body, 1, body_len, stdout);
	free(body);
	return 0;
}

static int
command_reindex(struct kn_message_store *store, int argc, char **argv)
{
	struct kn_bbs_store_lock lock;
	enum kn_message_index_error rc;

	(void)argv;
	if (argc != 0)
		return 1;
	kn_bbs_store_lock_init(&lock);
	if (kn_bbs_store_lock_exclusive(&lock, store->path) !=
	    KN_BBS_STORE_LOCK_OK)
		return 1;
	rc = kn_message_index_rebuild(store);
	kn_bbs_store_lock_release(&lock);
	if (rc != KN_MESSAGE_INDEX_OK)
		return 1;
	printf("reindexed\n");
	return 0;
}

static int
id_parse(const char *input, uint64_t *id)
{
	char *end;
	unsigned long long value;

	errno = 0;
	value = strtoull(input, &end, 10);
	if (errno != 0 || *end != '\0' || value == 0)
		return 1;
	*id = (uint64_t)value;
	return 0;
}

static void
message_print(const struct kn_message *message)
{
	char summary[256];

	if (kn_message_summary(message, summary, sizeof(summary)) !=
	    KN_MESSAGE_OK)
		return;
	printf("%s\n", summary);
}

static void
usage(FILE *out, const char *argv0)
{
	fprintf(out, "usage: %s --store PATH init\n", argv0);
	fprintf(out, "       %s --store PATH create-private FROM TO SUBJECT BODYFILE\n", argv0);
	fprintf(out, "       %s --store PATH create-bulletin FROM AREA SUBJECT BODYFILE\n", argv0);
	fprintf(out, "       %s --store PATH reindex\n", argv0);
	fprintf(out, "       %s --store PATH areas\n", argv0);
	fprintf(out, "       %s --store PATH list\n", argv0);
	fprintf(out, "       %s --store PATH list-private\n", argv0);
	fprintf(out, "       %s --store PATH list-bulletins\n", argv0);
	fprintf(out, "       %s --store PATH list-area AREA\n", argv0);
	fprintf(out, "       %s --store PATH list-to CALLSIGN\n", argv0);
	fprintf(out, "       %s --store PATH list-from CALLSIGN\n", argv0);
	fprintf(out, "       %s --store PATH list-unread CALLSIGN\n", argv0);
	fprintf(out, "       %s --store PATH mark-read CALLSIGN ID\n", argv0);
	fprintf(out, "       %s --store PATH read ID\n", argv0);
	fprintf(out, "       %s --store PATH delete ID\n", argv0);
}
