/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_bench_ax25_diag_replay.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "kilonode/bench_ax25_diag_replay.h"

static const char *fixture_path(const char *);
static int test_ax25_sabm_replay(void);
static int test_fx25_placeholder(void);
static int test_kiss_sabm_replay(void);
static int test_sequence_placeholder(void);
static int test_ui_ignored(void);

int
main(void)
{
	if (test_ui_ignored() != 0)
		return 1;
	if (test_kiss_sabm_replay() != 0)
		return 1;
	if (test_ax25_sabm_replay() != 0)
		return 1;
	if (test_fx25_placeholder() != 0)
		return 1;
	if (test_sequence_placeholder() != 0)
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
test_ax25_sabm_replay(void)
{
	struct kn_bench_diag_result result;

	if (kn_bench_diag_replay_capture(fixture_path("ax25-sabm-node.capture"),
	    &result) !=
	    KN_BENCH_DIAG_REPLAY_OK)
		return 1;
	if (result.connections_created != 1 || result.final_connections != 1)
		return 1;
	if (strcmp(result.final_state, "connected") != 0)
		return 1;

	return result.tx_writes_attempted == 0 ? 0 : 1;
}

static int
test_fx25_placeholder(void)
{
	struct kn_bench_diag_result result;

	if (kn_bench_diag_replay_capture(
	    fixture_path("fx25-future-placeholder.capture"), &result) !=
	    KN_BENCH_DIAG_REPLAY_OK)
		return 1;

	return result.unsupported != 0 && result.pass != 0 &&
	    result.tx_writes_attempted == 0 ? 0 : 1;
}

static int
test_kiss_sabm_replay(void)
{
	struct kn_bench_diag_result result;

	if (kn_bench_diag_replay_capture(
	    fixture_path("kiss-sabm-node.capture"), &result) !=
	    KN_BENCH_DIAG_REPLAY_OK)
		return 1;
	if (result.frames_parsed != 1 || result.frames_decoded != 1)
		return 1;
	if (result.connected_frames_accepted != 1)
		return 1;
	if (result.connections_created != 1 || result.final_connections != 1)
		return 1;
	if (strcmp(result.final_state, "connected") != 0)
		return 1;
	if (result.frame_plans_retained == 0)
		return 1;

	return result.tx_writes_attempted == 0 ? 0 : 1;
}

static int
test_sequence_placeholder(void)
{
	struct kn_bench_diag_result result;

	if (kn_bench_diag_replay_capture(
	    fixture_path("kiss-sabm-disc-sequence.capture"), &result) !=
	    KN_BENCH_DIAG_REPLAY_OK)
		return 1;

	return result.unsupported != 0 && result.pass != 0 &&
	    result.tx_writes_attempted == 0 ? 0 : 1;
}

static int
test_ui_ignored(void)
{
	struct kn_bench_diag_result result;

	if (kn_bench_diag_replay_capture(fixture_path("kiss-ui-cq.capture"),
	    &result) !=
	    KN_BENCH_DIAG_REPLAY_OK)
		return 1;
	if (result.frames_parsed != 1 || result.frames_decoded != 1)
		return 1;
	if (result.ui_ignored != 1 || result.final_connections != 0)
		return 1;

	return result.tx_writes_attempted == 0 ? 0 : 1;
}
