/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/tools/kilonode-store.c */

#include <sys/types.h>

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "kilonode/bbs_store_lock.h"
#include "kilonode/bbs_store_maintenance.h"
#include "kilonode/message_index.h"
#include "kilonode/message_store.h"

#define STORE_FINDING_MAX 512

static int command_check(const char *, int, char **);
static int command_export(const char *, int, char **);
static int command_purge(const char *, int, char **);
static int command_reindex(const char *, int, char **);
static int command_repair(const char *, int, char **);
static int command_stats(const char *, int, char **);
static void print_findings(const struct kn_bbs_store_finding *, size_t);
static void usage(FILE *, const char *);

int
main(int argc, char *argv[])
{
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

	if (strcmp(command, "check") == 0)
		rc = command_check(store_path, argc - i - 1, argv + i + 1);
	else if (strcmp(command, "repair") == 0)
		rc = command_repair(store_path, argc - i - 1, argv + i + 1);
	else if (strcmp(command, "reindex") == 0)
		rc = command_reindex(store_path, argc - i - 1, argv + i + 1);
	else if (strcmp(command, "stats") == 0)
		rc = command_stats(store_path, argc - i - 1, argv + i + 1);
	else if (strcmp(command, "purge-deleted") == 0)
		rc = command_purge(store_path, argc - i - 1, argv + i + 1);
	else if (strcmp(command, "export") == 0)
		rc = command_export(store_path, argc - i - 1, argv + i + 1);
	else {
		usage(stderr, argv[0]);
		rc = 1;
	}

	return rc;
}

static int
command_check(const char *store_path, int argc, char **argv)
{
	struct kn_bbs_store_finding findings[STORE_FINDING_MAX];
	size_t count;
	size_t errors;
	enum kn_bbs_store_maintenance_error rc;

	(void)argv;
	if (argc != 0)
		return 1;
	rc = kn_bbs_store_check(store_path, findings, STORE_FINDING_MAX,
	    &count, &errors);
	if (rc != KN_BBS_STORE_MAINTENANCE_OK) {
		fprintf(stderr, "check failed: %s\n",
		    kn_bbs_store_maintenance_error_name(rc));
		return 1;
	}
	print_findings(findings, count);
	printf("check findings=%llu errors=%llu\n",
	    (unsigned long long)count, (unsigned long long)errors);
	return errors == 0 ? 0 : 1;
}

static int
command_export(const char *store_path, int argc, char **argv)
{
	enum kn_bbs_store_maintenance_error rc;

	if (argc != 1)
		return 1;
	rc = kn_bbs_store_export(store_path, argv[0]);
	if (rc != KN_BBS_STORE_MAINTENANCE_OK) {
		fprintf(stderr, "export failed: %s\n",
		    kn_bbs_store_maintenance_error_name(rc));
		return 1;
	}
	printf("exported %s\n", argv[0]);
	return 0;
}

static int
command_purge(const char *store_path, int argc, char **argv)
{
	size_t purged;
	enum kn_bbs_store_maintenance_error rc;

	(void)argv;
	if (argc != 0)
		return 1;
	rc = kn_bbs_store_purge_deleted(store_path, &purged);
	if (rc != KN_BBS_STORE_MAINTENANCE_OK) {
		fprintf(stderr, "purge failed: %s\n",
		    kn_bbs_store_maintenance_error_name(rc));
		return 1;
	}
	printf("purged %llu\n", (unsigned long long)purged);
	return 0;
}

static int
command_reindex(const char *store_path, int argc, char **argv)
{
	struct kn_bbs_store_lock lock;
	struct kn_message_store store;
	enum kn_message_index_error irc;
	int ok;

	(void)argv;
	if (argc != 0)
		return 1;
	kn_bbs_store_lock_init(&lock);
	if (kn_bbs_store_lock_exclusive(&lock, store_path) !=
	    KN_BBS_STORE_LOCK_OK)
		return 1;
	kn_message_store_init(&store);
	ok = kn_message_store_open(&store, store_path, KN_MESSAGE_BODY_MAX) ==
	    KN_MESSAGE_STORE_OK;
	irc = ok != 0 ? kn_message_index_rebuild(&store) :
	    KN_MESSAGE_INDEX_ERR_STORE;
	kn_message_store_close(&store);
	kn_bbs_store_lock_release(&lock);
	if (ok == 0 || irc != KN_MESSAGE_INDEX_OK)
		return 1;
	printf("reindexed\n");
	return 0;
}

static int
command_repair(const char *store_path, int argc, char **argv)
{
	struct kn_bbs_store_finding findings[STORE_FINDING_MAX];
	size_t count;
	enum kn_bbs_store_maintenance_error rc;

	(void)argv;
	if (argc != 0)
		return 1;
	rc = kn_bbs_store_repair(store_path, findings, STORE_FINDING_MAX,
	    &count);
	if (rc != KN_BBS_STORE_MAINTENANCE_OK) {
		fprintf(stderr, "repair failed: %s\n",
		    kn_bbs_store_maintenance_error_name(rc));
		return 1;
	}
	print_findings(findings, count);
	printf("repair changes=%llu\n", (unsigned long long)count);
	return 0;
}

static int
command_stats(const char *store_path, int argc, char **argv)
{
	struct kn_bbs_store_stats stats;
	enum kn_bbs_store_maintenance_error rc;

	(void)argv;
	if (argc != 0)
		return 1;
	rc = kn_bbs_store_stats(store_path, &stats);
	if (rc != KN_BBS_STORE_MAINTENANCE_OK) {
		fprintf(stderr, "stats failed: %s\n",
		    kn_bbs_store_maintenance_error_name(rc));
		return 1;
	}
	printf("messages=%llu private=%llu bulletins=%llu deleted=%llu "
	    "users=%llu read_state=%llu areas=%llu body_bytes=%llu "
	    "newest=%llu next_id=%llu\n",
	    (unsigned long long)stats.total_messages,
	    (unsigned long long)stats.private_messages,
	    (unsigned long long)stats.bulletins,
	    (unsigned long long)stats.deleted_messages,
	    (unsigned long long)stats.users,
	    (unsigned long long)stats.read_state_files,
	    (unsigned long long)stats.bulletin_areas,
	    (unsigned long long)stats.total_body_bytes,
	    (unsigned long long)stats.newest_message_id,
	    (unsigned long long)stats.next_id);
	return 0;
}

static void
print_findings(const struct kn_bbs_store_finding *findings, size_t count)
{
	size_t i;

	for (i = 0; i < count && i < STORE_FINDING_MAX; i++) {
		printf("%s code=%s path=%s id=%llu message=%s\n",
		    kn_bbs_store_finding_severity_name(findings[i].severity),
		    findings[i].code, findings[i].path,
		    (unsigned long long)findings[i].message_id,
		    findings[i].message);
	}
}

static void
usage(FILE *out, const char *argv0)
{
	fprintf(out, "usage: %s --store PATH check\n", argv0);
	fprintf(out, "       %s --store PATH repair\n", argv0);
	fprintf(out, "       %s --store PATH reindex\n", argv0);
	fprintf(out, "       %s --store PATH stats\n", argv0);
	fprintf(out, "       %s --store PATH purge-deleted\n", argv0);
	fprintf(out, "       %s --store PATH export DEST\n", argv0);
}
