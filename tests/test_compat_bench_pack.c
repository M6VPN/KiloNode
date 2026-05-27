/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_compat_bench_pack.c */

#include <sys/types.h>
#include <sys/stat.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "kilonode/compat_bench_pack.h"

static const char *fixture_path(const char *);
static int test_absolute_fixture_rejected(void);
static int test_clean_room_false(void);
static int test_coverage_summary(void);
static int test_hardware_required_true(void);
static int test_invalid_referenced_capture(void);
static int test_missing_name(void);
static int test_missing_reference(void);
static int test_path_traversal_rejected(void);
static int test_replay_supported_captures(void);
static int test_report_deterministic(void);
static int test_source_code_used_true(void);
static int test_transmit_required_true(void);
static int test_valid_manifest(void);

static const char valid_manifest[] =
    "name receive-only-bench\n"
    "type bench-capture-fixtures\n"
    "source synthetic\n"
    "clean-room true\n"
    "source-code-used false\n"
    "hardware-required false\n"
    "transmit-required false\n"
    "fixture kiss-ui-cq.capture\n";

static const char *
fixture_path(const char *name)
{
	static char path[256];

	(void)snprintf(path, sizeof(path), "tests/fixtures/bench/%s", name);
	if (access(path, R_OK) == 0)
		return path;
	(void)snprintf(path, sizeof(path), "../tests/fixtures/bench/%s",
	    name);

	return path;
}

int
main(void)
{
	if (test_valid_manifest() != 0)
		return 1;
	if (test_missing_name() != 0)
		return 1;
	if (test_clean_room_false() != 0)
		return 1;
	if (test_source_code_used_true() != 0)
		return 1;
	if (test_hardware_required_true() != 0)
		return 1;
	if (test_transmit_required_true() != 0)
		return 1;
	if (test_absolute_fixture_rejected() != 0)
		return 1;
	if (test_path_traversal_rejected() != 0)
		return 1;
	if (test_missing_reference() != 0)
		return 1;
	if (test_invalid_referenced_capture() != 0)
		return 1;
	if (test_replay_supported_captures() != 0)
		return 1;
	if (test_coverage_summary() != 0)
		return 1;
	if (test_report_deterministic() != 0)
		return 1;

	return 0;
}

static int
test_absolute_fixture_rejected(void)
{
	const char text[] =
	    "name x\ntype y\nsource synthetic\nclean-room true\n"
	    "source-code-used false\nhardware-required false\n"
	    "transmit-required false\nfixture /tmp/x.capture\n";
	struct kn_compat_bench_pack pack;

	return kn_compat_bench_pack_parse_text(text, &pack, NULL) ==
	    KN_COMPAT_BENCH_ERR_INVALID_VALUE ? 0 : 1;
}

static int
test_clean_room_false(void)
{
	const char text[] =
	    "name x\ntype y\nsource synthetic\nclean-room false\n"
	    "source-code-used false\nhardware-required false\n"
	    "transmit-required false\nfixture x.capture\n";
	struct kn_compat_bench_pack pack;

	return kn_compat_bench_pack_parse_text(text, &pack, NULL) ==
	    KN_COMPAT_BENCH_ERR_INVALID_VALUE ? 0 : 1;
}

static int
test_coverage_summary(void)
{
	struct kn_compat_bench_pack pack;
	char report[KN_COMPAT_BENCH_REPORT_MAX];

	if (kn_compat_bench_pack_parse_file(fixture_path("manifest.bench"),
	    &pack, NULL) !=
	    KN_COMPAT_BENCH_OK)
		return 1;
	if (kn_compat_bench_pack_coverage_report(&pack, report,
	    sizeof(report)) != KN_COMPAT_BENCH_OK)
		return 1;
	if (strstr(report, "kiss=5") == NULL)
		return 1;
	if (strstr(report, "raw_ax25=2") == NULL)
		return 1;
	if (strstr(report, "fx25_placeholders=1") == NULL)
		return 1;

	return strstr(report, "setup=2") != NULL ? 0 : 1;
}

static int
test_hardware_required_true(void)
{
	const char text[] =
	    "name x\ntype y\nsource synthetic\nclean-room true\n"
	    "source-code-used false\nhardware-required true\n"
	    "transmit-required false\nfixture x.capture\n";
	struct kn_compat_bench_pack pack;

	return kn_compat_bench_pack_parse_text(text, &pack, NULL) ==
	    KN_COMPAT_BENCH_ERR_INVALID_VALUE ? 0 : 1;
}

static int
test_invalid_referenced_capture(void)
{
	const char manifest[] =
	    "name x\ntype y\nsource synthetic\nclean-room true\n"
	    "source-code-used false\nhardware-required false\n"
	    "transmit-required false\nfixture invalid.capture\n";
	const char capture[] = "not a capture\n";
	const char *dir = "/tmp/kilonode-bench-pack-test";
	char manifest_path[128];
	char capture_path[128];
	struct kn_compat_bench_pack pack;
	FILE *fp;

	(void)snprintf(manifest_path, sizeof(manifest_path),
	    "%s/manifest.bench", dir);
	(void)snprintf(capture_path, sizeof(capture_path),
	    "%s/invalid.capture", dir);
	(void)mkdir(dir, 0700);
	fp = fopen(manifest_path, "w");
	if (fp == NULL)
		return 1;
	(void)fputs(manifest, fp);
	(void)fclose(fp);
	fp = fopen(capture_path, "w");
	if (fp == NULL)
		return 1;
	(void)fputs(capture, fp);
	(void)fclose(fp);

	if (kn_compat_bench_pack_parse_file(manifest_path, &pack, NULL) !=
	    KN_COMPAT_BENCH_OK)
		return 1;
	(void)unlink(manifest_path);
	(void)unlink(capture_path);

	return kn_compat_bench_pack_validate_refs(&pack, NULL) ==
	    KN_COMPAT_BENCH_ERR_REFERENCE ? 0 : 1;
}

static int
test_missing_name(void)
{
	const char text[] =
	    "type y\nsource synthetic\nclean-room true\n"
	    "source-code-used false\nhardware-required false\n"
	    "transmit-required false\nfixture x.capture\n";
	struct kn_compat_bench_pack pack;

	return kn_compat_bench_pack_parse_text(text, &pack, NULL) ==
	    KN_COMPAT_BENCH_ERR_MISSING_REQUIRED ? 0 : 1;
}

static int
test_missing_reference(void)
{
	const char text[] =
	    "name x\ntype y\nsource synthetic\nclean-room true\n"
	    "source-code-used false\nhardware-required false\n"
	    "transmit-required false\nfixture missing.capture\n";
	struct kn_compat_bench_pack pack;

	if (kn_compat_bench_pack_parse_text(text, &pack, NULL) !=
	    KN_COMPAT_BENCH_OK)
		return 1;

	return kn_compat_bench_pack_validate_refs(&pack, NULL) ==
	    KN_COMPAT_BENCH_ERR_REFERENCE ? 0 : 1;
}

static int
test_path_traversal_rejected(void)
{
	const char text[] =
	    "name x\ntype y\nsource synthetic\nclean-room true\n"
	    "source-code-used false\nhardware-required false\n"
	    "transmit-required false\nfixture ../x.capture\n";
	struct kn_compat_bench_pack pack;

	return kn_compat_bench_pack_parse_text(text, &pack, NULL) ==
	    KN_COMPAT_BENCH_ERR_INVALID_VALUE ? 0 : 1;
}

static int
test_replay_supported_captures(void)
{
	struct kn_compat_bench_pack pack;
	char report[KN_COMPAT_BENCH_REPORT_MAX];

	if (kn_compat_bench_pack_parse_file(fixture_path("manifest.bench"),
	    &pack, NULL) !=
	    KN_COMPAT_BENCH_OK)
		return 1;
	if (kn_compat_bench_pack_replay_report(&pack, report,
	    sizeof(report)) != KN_COMPAT_BENCH_OK)
		return 1;
	if (strstr(report, "BENCH-PASS fixture=kiss-sabm-node.capture") ==
	    NULL)
		return 1;

	return strstr(report,
	    "BENCH-SKIP fixture=fx25-future-placeholder.capture") != NULL ?
	    0 : 1;
}

static int
test_report_deterministic(void)
{
	struct kn_compat_bench_pack pack;
	char report[KN_COMPAT_BENCH_REPORT_MAX];

	if (kn_compat_bench_pack_parse_file(fixture_path("manifest.bench"),
	    &pack, NULL) !=
	    KN_COMPAT_BENCH_OK)
		return 1;
	if (kn_compat_bench_pack_report(&pack, report, sizeof(report)) !=
	    KN_COMPAT_BENCH_OK)
		return 1;
	if (strstr(report, "BENCH-PACK name=receive-only-bench") == NULL)
		return 1;

	return strstr(report, "BENCH FIXTURE path=kiss-ui-cq.capture") !=
	    NULL ? 0 : 1;
}

static int
test_source_code_used_true(void)
{
	const char text[] =
	    "name x\ntype y\nsource synthetic\nclean-room true\n"
	    "source-code-used true\nhardware-required false\n"
	    "transmit-required false\nfixture x.capture\n";
	struct kn_compat_bench_pack pack;

	return kn_compat_bench_pack_parse_text(text, &pack, NULL) ==
	    KN_COMPAT_BENCH_ERR_INVALID_VALUE ? 0 : 1;
}

static int
test_transmit_required_true(void)
{
	const char text[] =
	    "name x\ntype y\nsource synthetic\nclean-room true\n"
	    "source-code-used false\nhardware-required false\n"
	    "transmit-required true\nfixture x.capture\n";
	struct kn_compat_bench_pack pack;

	return kn_compat_bench_pack_parse_text(text, &pack, NULL) ==
	    KN_COMPAT_BENCH_ERR_INVALID_VALUE ? 0 : 1;
}

static int
test_valid_manifest(void)
{
	struct kn_compat_bench_pack pack;

	if (kn_compat_bench_pack_parse_text(valid_manifest, &pack, NULL) !=
	    KN_COMPAT_BENCH_OK)
		return 1;
	if (strcmp(pack.name, "receive-only-bench") != 0)
		return 1;

	return pack.fixture_count == 1 && pack.hardware_required == 0 &&
	    pack.transmit_required == 0 ? 0 : 1;
}
