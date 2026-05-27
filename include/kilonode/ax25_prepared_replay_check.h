/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/ax25_prepared_replay_check.h */

#ifndef KILONODE_AX25_PREPARED_REPLAY_CHECK_H
#define KILONODE_AX25_PREPARED_REPLAY_CHECK_H

#include <sys/types.h>

#include <stdint.h>

#include "kilonode/ax25_prepared_expect.h"

#define KN_AX25_PREPARED_REPLAY_MISMATCH_MAX  16
#define KN_AX25_PREPARED_REPLAY_MISMATCH_TEXT 160

enum kn_ax25_prepared_replay_check_error {
	KN_AX25_PREPARED_REPLAY_CHECK_OK = 0,
	KN_AX25_PREPARED_REPLAY_CHECK_ERR_INVALID_ARGUMENT,
	KN_AX25_PREPARED_REPLAY_CHECK_ERR_MISMATCH,
	KN_AX25_PREPARED_REPLAY_CHECK_ERR_BUFFER
};

struct kn_ax25_prepared_replay_mismatch {
	size_t line;
	char detail[KN_AX25_PREPARED_REPLAY_MISMATCH_TEXT];
};

struct kn_ax25_prepared_replay_check_result {
	uint8_t pass;
	size_t prepared_count;
	uint64_t tx_writes;
	size_t mismatch_count;
	struct kn_ax25_prepared_replay_mismatch
	    mismatches[KN_AX25_PREPARED_REPLAY_MISMATCH_MAX];
};

void kn_ax25_prepared_replay_check_result_clear(
	struct kn_ax25_prepared_replay_check_result *);
enum kn_ax25_prepared_replay_check_error
kn_ax25_prepared_replay_check_frames(
	const struct kn_ax25_prepared_expect_block *,
	const struct kn_ax25_prepared_frame *, size_t, uint64_t,
	struct kn_ax25_prepared_replay_check_result *);

#endif
