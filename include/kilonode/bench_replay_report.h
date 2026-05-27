/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/bench_replay_report.h */

#ifndef KILONODE_BENCH_REPLAY_REPORT_H
#define KILONODE_BENCH_REPLAY_REPORT_H

#include <sys/types.h>

#include "kilonode/bench_replay.h"

enum kn_bench_replay_report_error {
	KN_BENCH_REPLAY_REPORT_OK = 0,
	KN_BENCH_REPLAY_REPORT_ERR_INVALID_ARGUMENT,
	KN_BENCH_REPLAY_REPORT_ERR_BUFFER
};

enum kn_bench_replay_report_error kn_bench_diag_result_format(
	const struct kn_bench_diag_result *, char *, size_t);
enum kn_bench_replay_report_error kn_bench_pack_diag_result_format(
	const struct kn_bench_pack_diag_result *, char *, size_t);

#endif
