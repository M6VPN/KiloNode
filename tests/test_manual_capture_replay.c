/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_manual_capture_replay.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "kilonode/manual_capture_replay.h"

static const char *fixture_path(const char *);
static void temp_root(char *, size_t);
static int test_duplicate_import(void);
static int test_import_and_replay(void);
static int test_reject_extension(void);

int
main(void)
{
	if (test_import_and_replay() != 0)
		return 1;
	if (test_duplicate_import() != 0)
		return 1;
	if (test_reject_extension() != 0)
		return 1;
	return 0;
}

static const char *
fixture_path(const char *name)
{
	static char path[256];

	(void)snprintf(path, sizeof(path),
	    "tests/fixtures/manual-captures/import-source/%s", name);
	if (access(path, R_OK) == 0)
		return path;
	(void)snprintf(path, sizeof(path),
	    "../tests/fixtures/manual-captures/import-source/%s", name);
	return path;
}

static void
temp_root(char *buf, size_t bufsiz)
{
	static unsigned int counter;

	counter++;
	(void)snprintf(buf, bufsiz, "/tmp/kilonode-manual-replay-%ld-%u",
	    (long)getpid(), counter);
}

static int
test_duplicate_import(void)
{
	char root[128];
	struct kn_manual_capture_import_request request;
	struct kn_manual_capture_import_result first;
	struct kn_manual_capture_import_result second;

	temp_root(root, sizeof(root));
	if (kn_manual_capture_workspace_init(root, NULL) !=
	    KN_MANUAL_CAPTURE_OK)
		return 1;
	memset(&request, 0, sizeof(request));
	request.source_path = fixture_path("kiss-manual-ui.capture");
	request.workspace_root = root;
	request.source = KN_MANUAL_CAPTURE_SOURCE_MANUAL;
	if (kn_manual_capture_import(&request, &first, NULL) !=
	    KN_MANUAL_CAPTURE_OK)
		return 1;
	if (kn_manual_capture_import(&request, &second, NULL) !=
	    KN_MANUAL_CAPTURE_OK)
		return 1;
	if (first.id != 1 || second.id != 2)
		return 1;
	return strcmp(first.file, second.file) != 0 ? 0 : 1;
}

static int
test_import_and_replay(void)
{
	char root[128];
	struct kn_manual_capture_import_request request;
	struct kn_manual_capture_import_result imported;
	struct kn_manual_capture_replay_result replay;
	struct kn_manual_capture_replay_all_result all;
	struct kn_manual_capture_index index;

	temp_root(root, sizeof(root));
	if (kn_manual_capture_workspace_init(root, NULL) !=
	    KN_MANUAL_CAPTURE_OK)
		return 1;
	memset(&request, 0, sizeof(request));
	request.source_path = fixture_path("kiss-manual-sabm.capture");
	request.workspace_root = root;
	request.source = KN_MANUAL_CAPTURE_SOURCE_MANUAL;
	request.notes = "manual import test";
	if (kn_manual_capture_import(&request, &imported, NULL) !=
	    KN_MANUAL_CAPTURE_OK)
		return 1;
	if (imported.id != 1 ||
	    imported.status != KN_MANUAL_CAPTURE_STATUS_VALID)
		return 1;
	if (kn_manual_capture_replay_one(root, imported.id, &replay, NULL) !=
	    KN_MANUAL_CAPTURE_OK)
		return 1;
	if (replay.diag.tx_writes_attempted != 0 ||
	    replay.diag.prepared_tx_writes_attempted != 0 ||
	    replay.diag.prepared_count != 1 ||
	    strcmp(replay.diag.final_state, "connected") != 0)
		return 1;
	request.source_path = fixture_path("fx25-manual-placeholder.capture");
	if (kn_manual_capture_import(&request, &imported, NULL) !=
	    KN_MANUAL_CAPTURE_OK)
		return 1;
	if (imported.status != KN_MANUAL_CAPTURE_STATUS_UNSUPPORTED)
		return 1;
	if (kn_manual_capture_validate_all(root, &index, NULL) !=
	    KN_MANUAL_CAPTURE_OK)
		return 1;
	if (index.entry_count != 2)
		return 1;
	if (kn_manual_capture_replay_all(root, &all, NULL) !=
	    KN_MANUAL_CAPTURE_OK)
		return 1;
	return all.tx_writes == 0 && all.pass_count == 1 &&
	    all.unsupported_count == 1 ? 0 : 1;
}

static int
test_reject_extension(void)
{
	char root[128];
	struct kn_manual_capture_import_request request;

	temp_root(root, sizeof(root));
	if (kn_manual_capture_workspace_init(root, NULL) !=
	    KN_MANUAL_CAPTURE_OK)
		return 1;
	memset(&request, 0, sizeof(request));
	request.source_path = "not-a-capture.txt";
	request.workspace_root = root;
	request.source = KN_MANUAL_CAPTURE_SOURCE_MANUAL;
	return kn_manual_capture_import(&request, NULL, NULL) ==
	    KN_MANUAL_CAPTURE_ERR_INVALID_VALUE ? 0 : 1;
}
