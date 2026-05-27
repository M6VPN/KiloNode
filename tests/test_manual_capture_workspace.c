/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_manual_capture_workspace.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "kilonode/manual_capture_workspace.h"

static void temp_root(char *, size_t);
static int test_corrupt_manifest_rejected(void);
static int test_init_and_check(void);
static int test_invalid_manifest_values(void);
static int test_unsafe_paths(void);

int
main(void)
{
	if (test_init_and_check() != 0)
		return 1;
	if (test_unsafe_paths() != 0)
		return 1;
	if (test_corrupt_manifest_rejected() != 0)
		return 1;
	if (test_invalid_manifest_values() != 0)
		return 1;
	return 0;
}

static void
temp_root(char *buf, size_t bufsiz)
{
	static unsigned int counter;

	counter++;
	(void)snprintf(buf, bufsiz, "/tmp/kilonode-manual-workspace-%ld-%u",
	    (long)getpid(), counter);
}

static int
test_corrupt_manifest_rejected(void)
{
	char root[128];
	char path[256];
	struct kn_manual_capture_workspace workspace;
	FILE *fp;

	temp_root(root, sizeof(root));
	if (kn_manual_capture_workspace_init(root, NULL) !=
	    KN_MANUAL_CAPTURE_OK)
		return 1;
	if (kn_manual_capture_workspace_join(root, "workspace.manifest", path,
	    sizeof(path)) != KN_MANUAL_CAPTURE_OK)
		return 1;
	fp = fopen(path, "w");
	if (fp == NULL)
		return 1;
	(void)fputs("name x\nsource-code-used true\n", fp);
	(void)fclose(fp);
	return kn_manual_capture_workspace_check(root, &workspace, NULL) !=
	    KN_MANUAL_CAPTURE_OK ? 0 : 1;
}

static int
test_init_and_check(void)
{
	char root[128];
	struct kn_manual_capture_workspace workspace;

	temp_root(root, sizeof(root));
	if (kn_manual_capture_workspace_init(root, NULL) !=
	    KN_MANUAL_CAPTURE_OK)
		return 1;
	if (kn_manual_capture_workspace_check(root, &workspace, NULL) !=
	    KN_MANUAL_CAPTURE_OK)
		return 1;
	if (strcmp(workspace.name, "manual-rx-captures") != 0)
		return 1;
	return workspace.transmit_required == 0 &&
	    workspace.source_code_used == 0 ? 0 : 1;
}

static int
test_invalid_manifest_values(void)
{
	const char text[] =
	    "name x\n"
	    "type manual-capture-workspace\n"
	    "created 2026-05-27\n"
	    "clean-room true\n"
	    "source-code-used true\n"
	    "hardware-required optional\n"
	    "transmit-required false\n";
	struct kn_manual_capture_workspace workspace;

	return kn_manual_capture_workspace_parse_text(text, &workspace, NULL) ==
	    KN_MANUAL_CAPTURE_ERR_INVALID_VALUE ? 0 : 1;
}

static int
test_unsafe_paths(void)
{
	if (kn_manual_capture_workspace_init("", NULL) !=
	    KN_MANUAL_CAPTURE_ERR_UNSAFE_PATH)
		return 1;
	if (kn_manual_capture_workspace_init("/", NULL) !=
	    KN_MANUAL_CAPTURE_ERR_UNSAFE_PATH)
		return 1;
	if (kn_manual_capture_workspace_init("../x", NULL) !=
	    KN_MANUAL_CAPTURE_ERR_UNSAFE_PATH)
		return 1;
	return kn_manual_capture_relative_path_safe("../x") == 0 ? 0 : 1;
}
