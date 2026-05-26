/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/tools/kilonode-user.c */

#include <sys/types.h>

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kilonode/bbs_read_state.h"
#include "kilonode/bbs_user.h"

static int command_create(struct kn_message_store *, int, char **);
static int command_enable(struct kn_message_store *, int, char **, uint8_t);
static int command_init(struct kn_message_store *, int, char **);
static int command_is_read(struct kn_message_store *, int, char **);
static int command_list(struct kn_message_store *, int, char **);
static int command_mark_read(struct kn_message_store *, int, char **);
static int command_seen(struct kn_message_store *, int, char **);
static int command_show(struct kn_message_store *, int, char **);
static int id_parse(const char *, uint64_t *);
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
	(void)kn_bbs_user_init_store(&store);
	(void)kn_bbs_read_state_init_store(&store);

	if (strcmp(command, "init") == 0)
		rc = command_init(&store, argc - i - 1, argv + i + 1);
	else if (strcmp(command, "create") == 0)
		rc = command_create(&store, argc - i - 1, argv + i + 1);
	else if (strcmp(command, "show") == 0)
		rc = command_show(&store, argc - i - 1, argv + i + 1);
	else if (strcmp(command, "list") == 0)
		rc = command_list(&store, argc - i - 1, argv + i + 1);
	else if (strcmp(command, "seen") == 0)
		rc = command_seen(&store, argc - i - 1, argv + i + 1);
	else if (strcmp(command, "disable") == 0)
		rc = command_enable(&store, argc - i - 1, argv + i + 1, 0);
	else if (strcmp(command, "enable") == 0)
		rc = command_enable(&store, argc - i - 1, argv + i + 1, 1);
	else if (strcmp(command, "mark-read") == 0)
		rc = command_mark_read(&store, argc - i - 1, argv + i + 1);
	else if (strcmp(command, "is-read") == 0)
		rc = command_is_read(&store, argc - i - 1, argv + i + 1);
	else {
		usage(stderr, argv[0]);
		rc = 1;
	}

	kn_message_store_close(&store);
	return rc;
}

static int
command_create(struct kn_message_store *store, int argc, char **argv)
{
	struct kn_bbs_user user;
	enum kn_bbs_user_error rc;

	if (argc != 1)
		return 1;
	rc = kn_bbs_user_create(store, argv[0], 1, &user);
	if (rc != KN_BBS_USER_OK && rc != KN_BBS_USER_ERR_EXISTS) {
		fprintf(stderr, "create failed: %s\n",
		    kn_bbs_user_error_name(rc));
		return 1;
	}
	if (rc == KN_BBS_USER_ERR_EXISTS &&
	    kn_bbs_user_load(store, argv[0], &user) != KN_BBS_USER_OK)
		return 1;
	printf("created %s\n", user.call);
	return 0;
}

static int
command_enable(struct kn_message_store *store, int argc, char **argv,
	uint8_t enabled)
{
	enum kn_bbs_user_error rc;

	if (argc != 1)
		return 1;
	rc = kn_bbs_user_enable(store, argv[0], enabled);
	if (rc != KN_BBS_USER_OK) {
		fprintf(stderr, "update failed: %s\n",
		    kn_bbs_user_error_name(rc));
		return 1;
	}
	printf("%s %s\n", enabled != 0 ? "enabled" : "disabled", argv[0]);
	return 0;
}

static int
command_init(struct kn_message_store *store, int argc, char **argv)
{
	(void)argv;
	if (argc != 0)
		return 1;
	if (kn_bbs_user_init_store(store) != KN_BBS_USER_OK ||
	    kn_bbs_read_state_init_store(store) != KN_BBS_READ_STATE_OK)
		return 1;
	printf("initialized\n");
	return 0;
}

static int
command_is_read(struct kn_message_store *store, int argc, char **argv)
{
	uint64_t id;
	uint8_t is_read;

	if (argc != 2 || id_parse(argv[1], &id) != 0)
		return 1;
	if (kn_bbs_read_state_is_read(store, argv[0], id, &is_read) !=
	    KN_BBS_READ_STATE_OK)
		return 1;
	printf("%s\n", is_read != 0 ? "read" : "unread");
	return 0;
}

static int
command_list(struct kn_message_store *store, int argc, char **argv)
{
	struct kn_bbs_user users[KN_BBS_USER_LIST_MAX];
	char summary[256];
	size_t count;
	size_t i;

	(void)argv;
	if (argc != 0)
		return 1;
	if (kn_bbs_user_list(store, users, KN_BBS_USER_LIST_MAX, &count) !=
	    KN_BBS_USER_OK)
		return 1;
	for (i = 0; i < count && i < KN_BBS_USER_LIST_MAX; i++) {
		if (kn_bbs_user_format(&users[i], summary, sizeof(summary)) ==
		    KN_BBS_USER_OK)
			printf("%s\n", summary);
	}
	return 0;
}

static int
command_mark_read(struct kn_message_store *store, int argc, char **argv)
{
	uint64_t id;

	if (argc != 2 || id_parse(argv[1], &id) != 0)
		return 1;
	if (kn_bbs_read_state_mark_read(store, argv[0], id) !=
	    KN_BBS_READ_STATE_OK)
		return 1;
	printf("marked-read %s %llu\n", argv[0], (unsigned long long)id);
	return 0;
}

static int
command_seen(struct kn_message_store *store, int argc, char **argv)
{
	struct kn_bbs_user user;
	enum kn_bbs_user_error rc;

	if (argc != 1)
		return 1;
	rc = kn_bbs_user_seen(store, argv[0], 1, &user);
	if (rc != KN_BBS_USER_OK) {
		fprintf(stderr, "seen failed: %s\n",
		    kn_bbs_user_error_name(rc));
		return 1;
	}
	printf("seen %s logins=%llu\n", user.call,
	    (unsigned long long)user.login_count);
	return 0;
}

static int
command_show(struct kn_message_store *store, int argc, char **argv)
{
	struct kn_bbs_user user;
	char summary[256];

	if (argc != 1)
		return 1;
	if (kn_bbs_user_load(store, argv[0], &user) != KN_BBS_USER_OK)
		return 1;
	if (kn_bbs_user_format(&user, summary, sizeof(summary)) !=
	    KN_BBS_USER_OK)
		return 1;
	printf("%s\n", summary);
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
usage(FILE *out, const char *argv0)
{
	fprintf(out, "usage: %s --store PATH init\n", argv0);
	fprintf(out, "       %s --store PATH create CALLSIGN\n", argv0);
	fprintf(out, "       %s --store PATH show CALLSIGN\n", argv0);
	fprintf(out, "       %s --store PATH list\n", argv0);
	fprintf(out, "       %s --store PATH seen CALLSIGN\n", argv0);
	fprintf(out, "       %s --store PATH disable CALLSIGN\n", argv0);
	fprintf(out, "       %s --store PATH enable CALLSIGN\n", argv0);
	fprintf(out, "       %s --store PATH mark-read CALLSIGN ID\n", argv0);
	fprintf(out, "       %s --store PATH is-read CALLSIGN ID\n", argv0);
}
