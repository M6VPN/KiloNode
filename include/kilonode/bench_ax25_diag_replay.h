/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/bench_ax25_diag_replay.h */

#ifndef KILONODE_BENCH_AX25_DIAG_REPLAY_H
#define KILONODE_BENCH_AX25_DIAG_REPLAY_H

#include <sys/types.h>

#include <stdint.h>

#include "kilonode/ax25_runtime.h"
#include "kilonode/compat_packet_capture.h"

#define KN_BENCH_DIAG_MISMATCH_MAX      8
#define KN_BENCH_DIAG_MISMATCH_TEXT_MAX 160
#define KN_BENCH_DIAG_NAME_MAX          96
#define KN_BENCH_DIAG_REPORT_MAX        4096

enum kn_bench_diag_replay_error {
	KN_BENCH_DIAG_REPLAY_OK = 0,
	KN_BENCH_DIAG_REPLAY_ERR_INVALID_ARGUMENT,
	KN_BENCH_DIAG_REPLAY_ERR_UNSUPPORTED,
	KN_BENCH_DIAG_REPLAY_ERR_PARSE,
	KN_BENCH_DIAG_REPLAY_ERR_DECODE,
	KN_BENCH_DIAG_REPLAY_ERR_RUNTIME,
	KN_BENCH_DIAG_REPLAY_ERR_MISMATCH
};

struct kn_bench_diag_mismatch {
	char text[KN_BENCH_DIAG_MISMATCH_TEXT_MAX];
};

struct kn_bench_diag_result {
	char capture_name[KN_BENCH_DIAG_NAME_MAX];
	char capture_path[KN_BENCH_DIAG_NAME_MAX * 4];
	enum kn_compat_packet_method method;
	uint8_t pass;
	uint8_t unsupported;
	uint64_t frames_parsed;
	uint64_t frames_decoded;
	uint64_t ui_ignored;
	uint64_t connected_frames_accepted;
	uint64_t frames_ignored;
	uint64_t malformed_frames;
	uint64_t connections_created;
	uint64_t final_connections;
	uint64_t frame_plans_retained;
	uint64_t tx_writes_attempted;
	char final_state[32];
	size_t mismatch_count;
	struct kn_bench_diag_mismatch mismatches[KN_BENCH_DIAG_MISMATCH_MAX];
};

const char *kn_bench_diag_replay_error_name(
	enum kn_bench_diag_replay_error);
void kn_bench_diag_result_clear(struct kn_bench_diag_result *);
enum kn_bench_diag_replay_error kn_bench_diag_replay_capture(
	const char *, struct kn_bench_diag_result *);
enum kn_bench_diag_replay_error kn_bench_diag_replay_capture_parsed(
	const char *, const struct kn_compat_packet_capture *,
	struct kn_bench_diag_result *);

#endif
