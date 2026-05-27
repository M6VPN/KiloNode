/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/compat/bench_replay_report.c */

#include <sys/types.h>

#include <stdio.h>

#include "kilonode/bench_replay_report.h"

enum kn_bench_replay_report_error
kn_bench_diag_result_format(const struct kn_bench_diag_result *result,
	char *buf, size_t bufsiz)
{
	size_t off;
	size_t i;
	int needed;

	if (result == NULL || buf == NULL || bufsiz == 0)
		return KN_BENCH_REPLAY_REPORT_ERR_INVALID_ARGUMENT;

	needed = snprintf(buf, bufsiz,
	    "BENCH-DIAG capture=%s method=%s result=%s unsupported=%s "
	    "frames=%llu decoded=%llu ui_ignored=%llu accepted=%llu "
	    "ignored=%llu malformed=%llu created=%llu final=%llu "
	    "state=%s plans=%llu tx_writes=%llu mismatches=%llu\n",
	    result->capture_name,
	    kn_compat_packet_method_name(result->method),
	    result->pass != 0 ? "pass" : "fail",
	    result->unsupported != 0 ? "true" : "false",
	    (unsigned long long)result->frames_parsed,
	    (unsigned long long)result->frames_decoded,
	    (unsigned long long)result->ui_ignored,
	    (unsigned long long)result->connected_frames_accepted,
	    (unsigned long long)result->frames_ignored,
	    (unsigned long long)result->malformed_frames,
	    (unsigned long long)result->connections_created,
	    (unsigned long long)result->final_connections,
	    result->final_state,
	    (unsigned long long)result->frame_plans_retained,
	    (unsigned long long)result->tx_writes_attempted,
	    (unsigned long long)result->mismatch_count);
	if (needed < 0 || (size_t)needed >= bufsiz)
		return KN_BENCH_REPLAY_REPORT_ERR_BUFFER;
	off = (size_t)needed;
	for (i = 0; i < result->mismatch_count; i++) {
		needed = snprintf(buf + off, bufsiz - off,
		    "BENCH-DIAG-MISMATCH %s\n", result->mismatches[i].text);
		if (needed < 0 || (size_t)needed >= bufsiz - off)
			return KN_BENCH_REPLAY_REPORT_ERR_BUFFER;
		off += (size_t)needed;
	}

	return KN_BENCH_REPLAY_REPORT_OK;
}

enum kn_bench_replay_report_error
kn_bench_pack_diag_result_format(
	const struct kn_bench_pack_diag_result *result, char *buf,
	size_t bufsiz)
{
	size_t off;
	size_t i;
	int needed;

	if (result == NULL || buf == NULL || bufsiz == 0)
		return KN_BENCH_REPLAY_REPORT_ERR_INVALID_ARGUMENT;

	needed = snprintf(buf, bufsiz,
	    "BENCH-DIAG-PACK results=%llu pass=%llu unsupported=%llu "
	    "fail=%llu frames=%llu tx_writes=%llu\n",
	    (unsigned long long)result->result_count,
	    (unsigned long long)result->pass_count,
	    (unsigned long long)result->unsupported_count,
	    (unsigned long long)result->fail_count,
	    (unsigned long long)result->total_frames,
	    (unsigned long long)result->total_tx_writes);
	if (needed < 0 || (size_t)needed >= bufsiz)
		return KN_BENCH_REPLAY_REPORT_ERR_BUFFER;
	off = (size_t)needed;
	for (i = 0; i < result->result_count; i++) {
		needed = snprintf(buf + off, bufsiz - off,
		    "BENCH-DIAG-ITEM capture=%s result=%s unsupported=%s "
		    "state=%s tx_writes=%llu\n",
		    result->results[i].capture_name,
		    result->results[i].pass != 0 ? "pass" : "fail",
		    result->results[i].unsupported != 0 ? "true" : "false",
		    result->results[i].final_state,
		    (unsigned long long)
		    result->results[i].tx_writes_attempted);
		if (needed < 0 || (size_t)needed >= bufsiz - off)
			return KN_BENCH_REPLAY_REPORT_ERR_BUFFER;
		off += (size_t)needed;
	}

	return KN_BENCH_REPLAY_REPORT_OK;
}
