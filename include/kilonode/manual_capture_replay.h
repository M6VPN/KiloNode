/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/manual_capture_replay.h */

#ifndef KILONODE_MANUAL_CAPTURE_REPLAY_H
#define KILONODE_MANUAL_CAPTURE_REPLAY_H

#include <sys/types.h>

#include <stdint.h>

#include "kilonode/bench_ax25_diag_replay.h"
#include "kilonode/manual_capture_index.h"

#define KN_MANUAL_CAPTURE_FILE_MAX (64 * 1024)

struct kn_manual_capture_import_request {
	const char *source_path;
	const char *workspace_root;
	enum kn_manual_capture_source source;
	const char *notes;
};

struct kn_manual_capture_import_result {
	uint32_t id;
	char file[KN_MANUAL_CAPTURE_FIELD_MAX];
	enum kn_manual_capture_status status;
};

struct kn_manual_capture_replay_result {
	uint32_t id;
	struct kn_bench_diag_result diag;
	enum kn_manual_capture_replay_status replay;
};

struct kn_manual_capture_replay_all_result {
	size_t count;
	size_t pass_count;
	size_t fail_count;
	size_t unsupported_count;
	uint64_t tx_writes;
};

enum kn_manual_capture_error kn_manual_capture_import(
	const struct kn_manual_capture_import_request *,
	struct kn_manual_capture_import_result *,
	struct kn_manual_capture_error_info *);
enum kn_manual_capture_error kn_manual_capture_validate_all(
	const char *, struct kn_manual_capture_index *,
	struct kn_manual_capture_error_info *);
enum kn_manual_capture_error kn_manual_capture_replay_one(
	const char *, uint32_t, struct kn_manual_capture_replay_result *,
	struct kn_manual_capture_error_info *);
enum kn_manual_capture_error kn_manual_capture_replay_all(
	const char *, struct kn_manual_capture_replay_all_result *,
	struct kn_manual_capture_error_info *);

#endif
