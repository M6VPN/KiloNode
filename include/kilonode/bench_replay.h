/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/bench_replay.h */

#ifndef KILONODE_BENCH_REPLAY_H
#define KILONODE_BENCH_REPLAY_H

#include <sys/types.h>

#include <stdint.h>

#include "kilonode/bench_ax25_diag_replay.h"
#include "kilonode/compat_bench_pack.h"

#define KN_BENCH_EXPECT_CAPTURE_MAX 32
#define KN_BENCH_EXPECT_LINE_MAX    512
#define KN_BENCH_EXPECT_TEXT_MAX    8192

enum kn_bench_replay_error {
	KN_BENCH_REPLAY_OK = 0,
	KN_BENCH_REPLAY_ERR_INVALID_ARGUMENT,
	KN_BENCH_REPLAY_ERR_IO,
	KN_BENCH_REPLAY_ERR_PARSE,
	KN_BENCH_REPLAY_ERR_LINE_TOO_LONG,
	KN_BENCH_REPLAY_ERR_UNKNOWN_KEY,
	KN_BENCH_REPLAY_ERR_INVALID_VALUE,
	KN_BENCH_REPLAY_ERR_TOO_MANY,
	KN_BENCH_REPLAY_ERR_REPLAY,
	KN_BENCH_REPLAY_ERR_BUFFER
};

struct kn_bench_expected_capture {
	char path[KN_COMPAT_BENCH_FIELD_MAX];
	uint8_t has_frames_parsed;
	uint64_t frames_parsed;
	uint8_t has_connections_created;
	uint64_t connections_created;
	uint8_t has_final_connections;
	uint64_t final_connections;
	uint8_t has_state;
	char state[32];
	uint8_t has_tx_writes;
	uint64_t tx_writes;
	uint8_t has_ui_ignored;
	uint64_t ui_ignored;
	uint8_t has_frame_plans_retained;
	uint64_t frame_plans_retained;
};

struct kn_bench_expected_file {
	struct kn_bench_expected_capture captures[KN_BENCH_EXPECT_CAPTURE_MAX];
	size_t capture_count;
};

struct kn_bench_replay_error_info {
	enum kn_bench_replay_error error;
	size_t line;
	char message[KN_COMPAT_BENCH_ERROR_MAX];
};

struct kn_bench_pack_diag_result {
	size_t result_count;
	size_t pass_count;
	size_t unsupported_count;
	size_t fail_count;
	uint64_t total_frames;
	uint64_t total_tx_writes;
	struct kn_bench_diag_result results[KN_COMPAT_BENCH_FIXTURE_MAX];
};

const char *kn_bench_replay_error_name(enum kn_bench_replay_error);
void kn_bench_expected_file_clear(struct kn_bench_expected_file *);
enum kn_bench_replay_error kn_bench_expected_parse_file(const char *,
	struct kn_bench_expected_file *, struct kn_bench_replay_error_info *);
enum kn_bench_replay_error kn_bench_expected_parse_text(const char *,
	struct kn_bench_expected_file *, struct kn_bench_replay_error_info *);
enum kn_bench_replay_error kn_bench_expected_check_result(
	const struct kn_bench_expected_file *,
	const struct kn_bench_diag_result *);
enum kn_bench_replay_error kn_bench_replay_pack_diagnostics(
	const struct kn_compat_bench_pack *, const struct kn_bench_expected_file *,
	struct kn_bench_pack_diag_result *);

#endif
