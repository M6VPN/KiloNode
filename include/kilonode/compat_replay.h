/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/compat_replay.h */

#ifndef KILONODE_COMPAT_REPLAY_H
#define KILONODE_COMPAT_REPLAY_H

#include <sys/types.h>

#include <stdint.h>

#include "kilonode/compat_transcript.h"
#include "kilonode/rf_command.h"

#define KN_COMPAT_MISMATCH_MAX       8
#define KN_COMPAT_MISMATCH_TEXT_MAX  160
#define KN_COMPAT_REPLY_PREVIEW_MAX  256

enum kn_compat_replay_error {
	KN_COMPAT_REPLAY_OK = 0,
	KN_COMPAT_REPLAY_ERR_INVALID_ARGUMENT,
	KN_COMPAT_REPLAY_ERR_UNSUPPORTED_MODE,
	KN_COMPAT_REPLAY_ERR_INTERNAL,
	KN_COMPAT_REPLAY_ERR_MISMATCH
};

struct kn_compat_mismatch {
	char text[KN_COMPAT_MISMATCH_TEXT_MAX];
};

struct kn_compat_replay_result {
	char transcript_name[KN_COMPAT_NAME_MAX];
	enum kn_compat_mode mode;
	uint8_t passed;
	enum kn_rf_command_name observed_command;
	enum kn_rf_command_status observed_status;
	uint8_t observed_reply_queued;
	uint64_t observed_tx_frame_id;
	char observed_reply_preview[KN_COMPAT_REPLY_PREVIEW_MAX];
	size_t mismatch_count;
	struct kn_compat_mismatch mismatches[KN_COMPAT_MISMATCH_MAX];
};

void kn_compat_replay_result_clear(struct kn_compat_replay_result *);
const char *kn_compat_replay_error_name(enum kn_compat_replay_error);
enum kn_compat_replay_error kn_compat_replay_transcript(
	const struct kn_compat_transcript *, struct kn_compat_replay_result *);

#endif
