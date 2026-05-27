/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_manual_capture_report.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "kilonode/manual_capture_report.h"

static int test_entry_report(void);
static int test_replay_all_report(void);
static int test_summary_empty(void);
static int test_summary_populated(void);

int
main(void)
{
	if (test_summary_empty() != 0)
		return 1;
	if (test_summary_populated() != 0)
		return 1;
	if (test_entry_report() != 0)
		return 1;
	if (test_replay_all_report() != 0)
		return 1;
	return 0;
}

static int
test_entry_report(void)
{
	struct kn_manual_capture_entry entry;
	struct kn_bench_diag_result diag;
	char report[KN_MANUAL_CAPTURE_REPORT_MAX];

	memset(&entry, 0, sizeof(entry));
	entry.id = 7;
	(void)snprintf(entry.file, sizeof(entry.file), "imported/a.capture");
	(void)snprintf(entry.method, sizeof(entry.method), "kiss");
	entry.status = KN_MANUAL_CAPTURE_STATUS_VALID;
	entry.replay = KN_MANUAL_CAPTURE_REPLAY_PASS;
	kn_bench_diag_result_clear(&diag);
	(void)snprintf(diag.final_state, sizeof(diag.final_state),
	    "connected");
	if (kn_manual_capture_entry_report_format(&entry, &diag, report,
	    sizeof(report)) != KN_MANUAL_CAPTURE_OK)
		return 1;
	return strstr(report, "state=connected") != NULL ? 0 : 1;
}

static int
test_replay_all_report(void)
{
	struct kn_manual_capture_replay_all_result result;
	char report[KN_MANUAL_CAPTURE_REPORT_MAX];

	memset(&result, 0, sizeof(result));
	result.count = 2;
	result.pass_count = 1;
	result.unsupported_count = 1;
	if (kn_manual_capture_replay_all_format(&result, report,
	    sizeof(report)) != KN_MANUAL_CAPTURE_OK)
		return 1;
	return strstr(report, "unsupported=1") != NULL ? 0 : 1;
}

static int
test_summary_empty(void)
{
	struct kn_manual_capture_index index;
	char report[KN_MANUAL_CAPTURE_REPORT_MAX];

	kn_manual_capture_index_clear(&index);
	if (kn_manual_capture_summary_format(&index, report, sizeof(report)) !=
	    KN_MANUAL_CAPTURE_OK)
		return 1;
	return strstr(report, "count=0") != NULL ? 0 : 1;
}

static int
test_summary_populated(void)
{
	struct kn_manual_capture_index index;
	struct kn_manual_capture_entry entry;
	char report[KN_MANUAL_CAPTURE_REPORT_MAX];

	kn_manual_capture_index_clear(&index);
	memset(&entry, 0, sizeof(entry));
	(void)snprintf(entry.file, sizeof(entry.file), "imported/a.capture");
	(void)snprintf(entry.method, sizeof(entry.method), "kiss");
	entry.status = KN_MANUAL_CAPTURE_STATUS_VALID;
	entry.replay = KN_MANUAL_CAPTURE_REPLAY_PASS;
	if (kn_manual_capture_index_add(&index, &entry) !=
	    KN_MANUAL_CAPTURE_OK)
		return 1;
	if (kn_manual_capture_summary_format(&index, report, sizeof(report)) !=
	    KN_MANUAL_CAPTURE_OK)
		return 1;
	return strstr(report, "valid=1") != NULL &&
	    strstr(report, "pass=1") != NULL ? 0 : 1;
}
