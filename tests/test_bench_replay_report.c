/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_bench_replay_report.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "kilonode/bench_replay_report.h"

static int test_mismatch_report(void);
static int test_pack_report(void);
static int test_single_report(void);
static int test_unsupported_report(void);

int
main(void)
{
	if (test_single_report() != 0)
		return 1;
	if (test_unsupported_report() != 0)
		return 1;
	if (test_mismatch_report() != 0)
		return 1;
	if (test_pack_report() != 0)
		return 1;

	return 0;
}

static int
test_mismatch_report(void)
{
	struct kn_bench_diag_result result;
	char report[KN_BENCH_DIAG_REPORT_MAX];

	kn_bench_diag_result_clear(&result);
	(void)snprintf(result.capture_name, sizeof(result.capture_name),
	    "bad.capture");
	result.pass = 0;
	(void)snprintf(result.mismatches[0].text,
	    sizeof(result.mismatches[0].text), "expected");
	result.mismatch_count = 1;
	if (kn_bench_diag_result_format(&result, report, sizeof(report)) !=
	    KN_BENCH_REPLAY_REPORT_OK)
		return 1;
	if (strstr(report, "result=fail") == NULL)
		return 1;

	return strstr(report, "BENCH-DIAG-MISMATCH expected") != NULL ? 0 :
	    1;
}

static int
test_pack_report(void)
{
	struct kn_bench_pack_diag_result result;
	char report[KN_BENCH_DIAG_REPORT_MAX];

	memset(&result, 0, sizeof(result));
	result.result_count = 1;
	result.pass_count = 1;
	result.total_frames = 1;
	kn_bench_diag_result_clear(&result.results[0]);
	(void)snprintf(result.results[0].capture_name,
	    sizeof(result.results[0].capture_name), "kiss-sabm-node");
	(void)snprintf(result.results[0].final_state,
	    sizeof(result.results[0].final_state), "connected");
	if (kn_bench_pack_diag_result_format(&result, report,
	    sizeof(report)) != KN_BENCH_REPLAY_REPORT_OK)
		return 1;
	if (strstr(report, "BENCH-DIAG-PACK results=1") == NULL)
		return 1;

	return strstr(report, "state=connected") != NULL ? 0 : 1;
}

static int
test_single_report(void)
{
	struct kn_bench_diag_result result;
	char report[KN_BENCH_DIAG_REPORT_MAX];

	kn_bench_diag_result_clear(&result);
	(void)snprintf(result.capture_name, sizeof(result.capture_name),
	    "kiss-ui-cq");
	result.frames_parsed = 1;
	result.ui_ignored = 1;
	if (kn_bench_diag_result_format(&result, report, sizeof(report)) !=
	    KN_BENCH_REPLAY_REPORT_OK)
		return 1;
	if (strstr(report, "BENCH-DIAG capture=kiss-ui-cq") == NULL)
		return 1;

	return strstr(report, "tx_writes=0") != NULL ? 0 : 1;
}

static int
test_unsupported_report(void)
{
	struct kn_bench_diag_result result;
	char report[KN_BENCH_DIAG_REPORT_MAX];

	kn_bench_diag_result_clear(&result);
	(void)snprintf(result.capture_name, sizeof(result.capture_name),
	    "fx25-future-placeholder");
	result.unsupported = 1;
	if (kn_bench_diag_result_format(&result, report, sizeof(report)) !=
	    KN_BENCH_REPLAY_REPORT_OK)
		return 1;

	return strstr(report, "unsupported=true") != NULL ? 0 : 1;
}
