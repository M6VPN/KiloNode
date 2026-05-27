/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/proto/ax25_prepared_replay_check.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "kilonode/ax25_prepared_replay_check.h"

static void add_mismatch(struct kn_ax25_prepared_replay_check_result *,
	size_t, const char *);
static void check_frame(const struct kn_ax25_prepared_expect_frame *,
	const struct kn_ax25_prepared_frame *,
	struct kn_ax25_prepared_replay_check_result *);

static void
add_mismatch(struct kn_ax25_prepared_replay_check_result *result,
	size_t line, const char *detail)
{
	if (result == NULL || detail == NULL)
		return;
	if (result->mismatch_count >= KN_AX25_PREPARED_REPLAY_MISMATCH_MAX)
		return;
	result->mismatches[result->mismatch_count].line = line;
	(void)snprintf(result->mismatches[result->mismatch_count].detail,
	    sizeof(result->mismatches[result->mismatch_count].detail), "%s",
	    detail);
	result->mismatch_count++;
	result->pass = 0;
}

static void
check_frame(const struct kn_ax25_prepared_expect_frame *expect,
	const struct kn_ax25_prepared_frame *frame,
	struct kn_ax25_prepared_replay_check_result *result)
{
	char local[KN_CALLSIGN_MAX + 4];
	char remote[KN_CALLSIGN_MAX + 4];
	char hex[KN_AX25_PREPARED_FRAME_HEX_PREVIEW * 2U + 1U];

	if (expect == NULL || frame == NULL || result == NULL)
		return;
	if (expect->has_kind != 0 && frame->type != expect->kind)
		add_mismatch(result, expect->line, "prepared-kind");
	if (expect->has_action != 0 &&
	    frame->action_source != expect->action)
		add_mismatch(result, expect->line, "prepared-action");
	if (expect->has_status != 0 && frame->status != expect->status)
		add_mismatch(result, expect->line, "prepared-status");
	if (expect->has_local != 0) {
		if (kn_callsign_format(&frame->local, local, sizeof(local)) !=
		    0 || strcmp(local, expect->local) != 0)
			add_mismatch(result, expect->line, "prepared-local");
	}
	if (expect->has_remote != 0) {
		if (kn_callsign_format(&frame->remote, remote,
		    sizeof(remote)) != 0 ||
		    strcmp(remote, expect->remote) != 0)
			add_mismatch(result, expect->line, "prepared-remote");
	}
	if (expect->has_port != 0 &&
	    strcmp(frame->port_name, expect->port) != 0)
		add_mismatch(result, expect->line, "prepared-port");
	if (expect->has_ax25_len != 0 && frame->raw_len != expect->ax25_len)
		add_mismatch(result, expect->line, "prepared-ax25-len");
	if (expect->has_hex_prefix != 0) {
		if (kn_ax25_prepared_frame_hex_preview(frame, hex,
		    sizeof(hex)) != KN_AX25_PREPARED_FRAME_OK ||
		    strncmp(hex, expect->hex_prefix,
		    strlen(expect->hex_prefix)) != 0)
			add_mismatch(result, expect->line, "prepared-hex");
	}
}

void
kn_ax25_prepared_replay_check_result_clear(
	struct kn_ax25_prepared_replay_check_result *result)
{
	if (result == NULL)
		return;

	memset(result, 0, sizeof(*result));
}

enum kn_ax25_prepared_replay_check_error
kn_ax25_prepared_replay_check_frames(
	const struct kn_ax25_prepared_expect_block *expect,
	const struct kn_ax25_prepared_frame *frames, size_t frame_count,
	uint64_t tx_writes,
	struct kn_ax25_prepared_replay_check_result *result)
{
	size_t i;
	size_t index;

	if (expect == NULL || (frames == NULL && frame_count > 0) ||
	    result == NULL)
		return KN_AX25_PREPARED_REPLAY_CHECK_ERR_INVALID_ARGUMENT;
	kn_ax25_prepared_replay_check_result_clear(result);
	result->pass = 1;
	result->prepared_count = frame_count;
	result->tx_writes = tx_writes;
	if (tx_writes != 0)
		add_mismatch(result, expect->line, "tx-writes-nonzero");
	if (expect->has_tx_writes != 0 && tx_writes != expect->tx_writes)
		add_mismatch(result, expect->line, "tx-writes");
	if (expect->has_prepared_count != 0 &&
	    frame_count != expect->prepared_count)
		add_mismatch(result, expect->line, "prepared-count");
	for (i = 0; i < expect->frame_count; i++) {
		index = expect->frames[i].order - 1U;
		if (index >= frame_count) {
			add_mismatch(result, expect->frames[i].line,
			    "prepared-missing");
			continue;
		}
		check_frame(&expect->frames[i], &frames[index], result);
	}

	return result->mismatch_count == 0 ?
	    KN_AX25_PREPARED_REPLAY_CHECK_OK :
	    KN_AX25_PREPARED_REPLAY_CHECK_ERR_MISMATCH;
}
