/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_ax25_prepared_replay_check.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "kilonode/ax25_prepared_replay_check.h"

static int make_block(struct kn_ax25_prepared_expect_block *);
static int make_frame(struct kn_ax25_prepared_frame *);
static int test_extra_fails_when_exact(void);
static int test_match_passes(void);
static int test_missing_fails(void);
static int test_tx_writes_fail(void);
static int test_wrong_kind_fails(void);

int
main(void)
{
	if (test_match_passes() != 0)
		return 1;
	if (test_missing_fails() != 0)
		return 1;
	if (test_extra_fails_when_exact() != 0)
		return 1;
	if (test_wrong_kind_fails() != 0)
		return 1;
	if (test_tx_writes_fail() != 0)
		return 1;

	return 0;
}

static int
make_block(struct kn_ax25_prepared_expect_block *block)
{
	if (block == NULL)
		return 1;
	memset(block, 0, sizeof(*block));
	block->type = KN_AX25_PREPARED_EXPECT_BLOCK_CAPTURE;
	(void)snprintf(block->name, sizeof(block->name), "one.capture");
	block->has_prepared_count = 1;
	block->prepared_count = 1;
	block->has_tx_writes = 1;
	block->tx_writes = 0;
	block->frame_count = 1;
	block->frames[0].line = 3;
	block->frames[0].order = 1;
	block->frames[0].has_kind = 1;
	block->frames[0].kind = KN_AX25_FRAME_PLAN_UA;
	block->frames[0].has_action = 1;
	block->frames[0].action = KN_AX25_ACTION_SEND_UA;
	block->frames[0].has_status = 1;
	block->frames[0].status = KN_AX25_PREPARED_FRAME_STATUS_PREPARED;
	return 0;
}

static int
make_frame(struct kn_ax25_prepared_frame *frame)
{
	if (frame == NULL)
		return 1;
	kn_ax25_prepared_frame_clear(frame);
	frame->type = KN_AX25_FRAME_PLAN_UA;
	frame->action_source = KN_AX25_ACTION_SEND_UA;
	frame->status = KN_AX25_PREPARED_FRAME_STATUS_PREPARED;
	return 0;
}

static int
test_extra_fails_when_exact(void)
{
	struct kn_ax25_prepared_expect_block block;
	struct kn_ax25_prepared_frame frames[2];
	struct kn_ax25_prepared_replay_check_result result;

	if (make_block(&block) != 0 || make_frame(&frames[0]) != 0 ||
	    make_frame(&frames[1]) != 0)
		return 1;

	return kn_ax25_prepared_replay_check_frames(&block, frames, 2, 0,
	    &result) == KN_AX25_PREPARED_REPLAY_CHECK_ERR_MISMATCH ? 0 : 1;
}

static int
test_match_passes(void)
{
	struct kn_ax25_prepared_expect_block block;
	struct kn_ax25_prepared_frame frame;
	struct kn_ax25_prepared_replay_check_result result;

	if (make_block(&block) != 0 || make_frame(&frame) != 0)
		return 1;
	if (kn_ax25_prepared_replay_check_frames(&block, &frame, 1, 0,
	    &result) != KN_AX25_PREPARED_REPLAY_CHECK_OK)
		return 1;

	return result.pass != 0 && result.mismatch_count == 0 ? 0 : 1;
}

static int
test_missing_fails(void)
{
	struct kn_ax25_prepared_expect_block block;
	struct kn_ax25_prepared_replay_check_result result;

	if (make_block(&block) != 0)
		return 1;

	return kn_ax25_prepared_replay_check_frames(&block, NULL, 0, 0,
	    &result) == KN_AX25_PREPARED_REPLAY_CHECK_ERR_MISMATCH ? 0 : 1;
}

static int
test_tx_writes_fail(void)
{
	struct kn_ax25_prepared_expect_block block;
	struct kn_ax25_prepared_frame frame;
	struct kn_ax25_prepared_replay_check_result result;

	if (make_block(&block) != 0 || make_frame(&frame) != 0)
		return 1;

	return kn_ax25_prepared_replay_check_frames(&block, &frame, 1, 1,
	    &result) == KN_AX25_PREPARED_REPLAY_CHECK_ERR_MISMATCH ? 0 : 1;
}

static int
test_wrong_kind_fails(void)
{
	struct kn_ax25_prepared_expect_block block;
	struct kn_ax25_prepared_frame frame;
	struct kn_ax25_prepared_replay_check_result result;

	if (make_block(&block) != 0 || make_frame(&frame) != 0)
		return 1;
	frame.type = KN_AX25_FRAME_PLAN_DM;

	return kn_ax25_prepared_replay_check_frames(&block, &frame, 1, 0,
	    &result) == KN_AX25_PREPARED_REPLAY_CHECK_ERR_MISMATCH ? 0 : 1;
}
