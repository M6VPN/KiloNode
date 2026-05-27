/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_rf_ignore.c */

#define _POSIX_C_SOURCE 200809L

#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "kilonode/rf_ignore.h"

static int test_add_lookup(void);
static int test_load_file(void);
static int test_load_invalid(void);

int
main(void)
{
	if (test_add_lookup() != 0)
		return 1;
	if (test_load_file() != 0)
		return 1;
	if (test_load_invalid() != 0)
		return 1;

	return 0;
}

static int
test_add_lookup(void)
{
	struct kn_rf_ignore_list list;
	struct kn_callsign call;
	const struct kn_rf_ignore_entry *entry;

	kn_rf_ignore_init(&list);
	(void)kn_callsign_parse("N0CALL-1", &call);
	if (kn_rf_ignore_add(&list, &call, "manual") != KN_RF_IGNORE_OK)
		return 1;
	if (kn_rf_ignore_count(&list) != 1)
		return 1;
	if (kn_rf_ignore_source_ignored(&list, &call, &entry) == 0)
		return 1;

	return entry != NULL && strcmp(entry->reason, "manual") == 0 ? 0 : 1;
}

static int
test_load_file(void)
{
	struct kn_rf_ignore_list list;
	struct kn_callsign call;
	const struct kn_rf_ignore_entry *entry;
	char path[] = "/tmp/kilonode-rf-ignore-XXXXXX";
	FILE *fp;
	int fd;

	fd = mkstemp(path);
	if (fd < 0)
		return 1;
	fp = fdopen(fd, "w");
	if (fp == NULL) {
		(void)close(fd);
		(void)unlink(path);
		return 1;
	}
	(void)fprintf(fp, "# ignored sources\n\nn0call-1 manual test\n");
	(void)fprintf(fp, "M6VPN-1\n");
	(void)fclose(fp);

	kn_rf_ignore_init(&list);
	if (kn_rf_ignore_load_file(&list, path) != KN_RF_IGNORE_OK) {
		(void)unlink(path);
		return 1;
	}
	(void)unlink(path);

	(void)kn_callsign_parse("N0CALL-1", &call);
	if (kn_rf_ignore_source_ignored(&list, &call, &entry) == 0)
		return 1;
	if (entry == NULL || strcmp(entry->reason, "manual test") != 0)
		return 1;
	(void)kn_callsign_parse("M6VPN-1", &call);
	return kn_rf_ignore_source_ignored(&list, &call, NULL) != 0 &&
	    kn_rf_ignore_count(&list) == 2 ? 0 : 1;
}

static int
test_load_invalid(void)
{
	struct kn_rf_ignore_list list;
	char path[] = "/tmp/kilonode-rf-ignore-bad-XXXXXX";
	FILE *fp;
	int fd;

	fd = mkstemp(path);
	if (fd < 0)
		return 1;
	fp = fdopen(fd, "w");
	if (fp == NULL) {
		(void)close(fd);
		(void)unlink(path);
		return 1;
	}
	(void)fprintf(fp, "BAD*CALL\n");
	(void)fclose(fp);

	kn_rf_ignore_init(&list);
	if (kn_rf_ignore_load_file(&list, path) != KN_RF_IGNORE_ERR_PARSE) {
		(void)unlink(path);
		return 1;
	}
	(void)unlink(path);
	return 0;
}
