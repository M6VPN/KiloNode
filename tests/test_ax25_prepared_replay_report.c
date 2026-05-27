/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_ax25_prepared_replay_report.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "kilonode/ax25_prepared_replay_report.h"

static int test_fail_report(void);
static int test_pass_report(void);

int
main(void)
{
	if (test_pass_report() != 0)
		return 1;
	if (test_fail_report() != 0)
		return 1;

	return 0;
}

static int
test_fail_report(void)
{
	struct kn_ax25_prepared_replay_check_result result;
	char report[512];

	kn_ax25_prepared_replay_check_result_clear(&result);
	result.pass = 0;
	result.prepared_count = 1;
	result.mismatch_count = 1;
	result.mismatches[0].line = 7;
	(void)snprintf(result.mismatches[0].detail,
	    sizeof(result.mismatches[0].detail), "prepared-kind");
	if (kn_ax25_prepared_replay_report_format("one", &result, report,
	    sizeof(report)) != KN_AX25_PREPARED_REPLAY_REPORT_OK)
		return 1;

	return strstr(report, "result=fail") != NULL &&
	    strstr(report, "line=7") != NULL ? 0 : 1;
}

static int
test_pass_report(void)
{
	struct kn_ax25_prepared_replay_check_result result;
	char report[256];

	kn_ax25_prepared_replay_check_result_clear(&result);
	result.pass = 1;
	result.prepared_count = 1;
	if (kn_ax25_prepared_replay_report_format("one", &result, report,
	    sizeof(report)) != KN_AX25_PREPARED_REPLAY_REPORT_OK)
		return 1;

	return strstr(report, "PREPARED-REPLAY name=one result=pass") !=
	    NULL ? 0 : 1;
}
