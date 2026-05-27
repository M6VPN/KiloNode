/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_bench_replay.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "kilonode/bench_replay.h"

static const char *fixture_path(const char *);
static int test_expected_invalid_integer(void);
static int test_expected_path_traversal(void);
static int test_expected_unknown_key(void);
static int test_expected_valid_file(void);
static int test_mismatch_fails(void);
static int test_pack_replay_with_expected(void);

int
main(void)
{
	if (test_expected_valid_file() != 0)
		return 1;
	if (test_expected_unknown_key() != 0)
		return 1;
	if (test_expected_invalid_integer() != 0)
		return 1;
	if (test_expected_path_traversal() != 0)
		return 1;
	if (test_pack_replay_with_expected() != 0)
		return 1;
	if (test_mismatch_fails() != 0)
		return 1;

	return 0;
}

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

static int
test_expected_invalid_integer(void)
{
	const char text[] =
	    "capture kiss-ui-cq.capture {\n"
	    "\tframes-parsed nope\n"
	    "}\n";
	struct kn_bench_expected_file expected;

	return kn_bench_expected_parse_text(text, &expected, NULL) ==
	    KN_BENCH_REPLAY_ERR_INVALID_VALUE ? 0 : 1;
}

static int
test_expected_path_traversal(void)
{
	const char text[] =
	    "capture ../kiss-ui-cq.capture {\n"
	    "\tframes-parsed 1\n"
	    "}\n";
	struct kn_bench_expected_file expected;

	return kn_bench_expected_parse_text(text, &expected, NULL) ==
	    KN_BENCH_REPLAY_ERR_INVALID_VALUE ? 0 : 1;
}

static int
test_expected_unknown_key(void)
{
	const char text[] =
	    "capture kiss-ui-cq.capture {\n"
	    "\tunknown-key 1\n"
	    "}\n";
	struct kn_bench_expected_file expected;

	return kn_bench_expected_parse_text(text, &expected, NULL) ==
	    KN_BENCH_REPLAY_ERR_UNKNOWN_KEY ? 0 : 1;
}

static int
test_expected_valid_file(void)
{
	struct kn_bench_expected_file expected;

	if (kn_bench_expected_parse_file(
	    fixture_path("ax25-diag-replay.expected"), &expected, NULL) !=
	    KN_BENCH_REPLAY_OK)
		return 1;

	return expected.capture_count == 5 ? 0 : 1;
}

static int
test_mismatch_fails(void)
{
	const char text[] =
	    "capture kiss-sabm-node.capture {\n"
	    "\tfinal-connections 0\n"
	    "\ttx-writes 0\n"
	    "}\n";
	struct kn_bench_expected_file expected;
	struct kn_compat_bench_pack pack;
	struct kn_bench_pack_diag_result result;

	if (kn_bench_expected_parse_text(text, &expected, NULL) !=
	    KN_BENCH_REPLAY_OK)
		return 1;
	if (kn_compat_bench_pack_parse_file(fixture_path("manifest.bench"),
	    &pack, NULL) !=
	    KN_COMPAT_BENCH_OK)
		return 1;

	return kn_bench_replay_pack_diagnostics(&pack, &expected, &result) ==
	    KN_BENCH_REPLAY_ERR_REPLAY && result.fail_count == 1 ? 0 : 1;
}

static int
test_pack_replay_with_expected(void)
{
	struct kn_bench_expected_file expected;
	struct kn_compat_bench_pack pack;
	struct kn_bench_pack_diag_result result;

	if (kn_bench_expected_parse_file(
	    fixture_path("ax25-diag-replay.expected"), &expected, NULL) !=
	    KN_BENCH_REPLAY_OK)
		return 1;
	if (kn_compat_bench_pack_parse_file(fixture_path("manifest.bench"),
	    &pack, NULL) !=
	    KN_COMPAT_BENCH_OK)
		return 1;
	if (kn_bench_replay_pack_diagnostics(&pack, &expected, &result) !=
	    KN_BENCH_REPLAY_OK)
		return 1;
	if (result.result_count != 8 || result.fail_count != 0)
		return 1;
	if (result.unsupported_count != 1 || result.total_tx_writes != 0)
		return 1;

	return result.total_frames == 8 ? 0 : 1;
}
