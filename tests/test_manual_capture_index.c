/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_manual_capture_index.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "kilonode/manual_capture_index.h"

static void temp_root(char *, size_t);
static int test_add_save_load(void);
static int test_format_and_update(void);
static int test_reject_unsafe_file(void);

int
main(void)
{
	if (test_add_save_load() != 0)
		return 1;
	if (test_reject_unsafe_file() != 0)
		return 1;
	if (test_format_and_update() != 0)
		return 1;
	return 0;
}

static void
temp_root(char *buf, size_t bufsiz)
{
	static unsigned int counter;

	counter++;
	(void)snprintf(buf, bufsiz, "/tmp/kilonode-manual-index-%ld-%u",
	    (long)getpid(), counter);
}

static int
test_add_save_load(void)
{
	char root[128];
	struct kn_manual_capture_index index;
	struct kn_manual_capture_entry entry;

	temp_root(root, sizeof(root));
	if (kn_manual_capture_workspace_init(root, NULL) !=
	    KN_MANUAL_CAPTURE_OK)
		return 1;
	kn_manual_capture_index_clear(&index);
	memset(&entry, 0, sizeof(entry));
	(void)snprintf(entry.file, sizeof(entry.file), "%s",
	    "imported/a.capture");
	(void)snprintf(entry.method, sizeof(entry.method), "kiss");
	entry.status = KN_MANUAL_CAPTURE_STATUS_VALID;
	entry.replay = KN_MANUAL_CAPTURE_REPLAY_NOT_RUN;
	if (kn_manual_capture_index_add(&index, &entry) !=
	    KN_MANUAL_CAPTURE_OK)
		return 1;
	if (kn_manual_capture_index_add(&index, &entry) !=
	    KN_MANUAL_CAPTURE_OK)
		return 1;
	if (index.entries[0].id != 1 || index.entries[1].id != 2)
		return 1;
	if (kn_manual_capture_index_save(root, &index, NULL) !=
	    KN_MANUAL_CAPTURE_OK)
		return 1;
	kn_manual_capture_index_clear(&index);
	if (kn_manual_capture_index_load(root, &index, NULL) !=
	    KN_MANUAL_CAPTURE_OK)
		return 1;
	return index.entry_count == 2 && index.next_id == 3 ? 0 : 1;
}

static int
test_format_and_update(void)
{
	struct kn_manual_capture_index index;
	struct kn_manual_capture_entry entry;
	char report[KN_MANUAL_CAPTURE_REPORT_MAX];

	kn_manual_capture_index_clear(&index);
	memset(&entry, 0, sizeof(entry));
	(void)snprintf(entry.file, sizeof(entry.file), "%s",
	    "imported/a.capture");
	(void)snprintf(entry.method, sizeof(entry.method), "kiss");
	if (kn_manual_capture_index_add(&index, &entry) !=
	    KN_MANUAL_CAPTURE_OK)
		return 1;
	if (kn_manual_capture_index_update_status(&index, 1,
	    KN_MANUAL_CAPTURE_STATUS_VALID) != KN_MANUAL_CAPTURE_OK)
		return 1;
	if (kn_manual_capture_index_update_replay(&index, 1,
	    KN_MANUAL_CAPTURE_REPLAY_PASS) != KN_MANUAL_CAPTURE_OK)
		return 1;
	if (kn_manual_capture_index_format(&index, report, sizeof(report)) !=
	    KN_MANUAL_CAPTURE_OK)
		return 1;
	return strstr(report, "replay=pass") != NULL ? 0 : 1;
}

static int
test_reject_unsafe_file(void)
{
	struct kn_manual_capture_index index;
	struct kn_manual_capture_entry entry;

	kn_manual_capture_index_clear(&index);
	memset(&entry, 0, sizeof(entry));
	(void)snprintf(entry.file, sizeof(entry.file), "%s",
	    "../bad.capture");
	(void)snprintf(entry.method, sizeof(entry.method), "kiss");
	return kn_manual_capture_index_add(&index, &entry) ==
	    KN_MANUAL_CAPTURE_ERR_UNSAFE_PATH ? 0 : 1;
}
