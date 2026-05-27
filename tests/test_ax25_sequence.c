/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_ax25_sequence.c */

#include <sys/types.h>

#include <stdint.h>

#include "kilonode/ax25_sequence.h"

static int test_ack_update(void);
static int test_expected_match(void);
static int test_expected_mismatch(void);
static int test_increment(void);
static int test_invalid_sequence(void);
static int test_invalid_window(void);
static int test_mod128_planned(void);
static int test_receive_update(void);
static int test_rej_marks_retransmit(void);
static int test_rnr_marks_busy(void);
static int test_wrap(void);

int
main(void)
{
	if (test_increment() != 0)
		return 1;
	if (test_wrap() != 0)
		return 1;
	if (test_expected_match() != 0)
		return 1;
	if (test_expected_mismatch() != 0)
		return 1;
	if (test_ack_update() != 0)
		return 1;
	if (test_rnr_marks_busy() != 0)
		return 1;
	if (test_rej_marks_retransmit() != 0)
		return 1;
	if (test_invalid_sequence() != 0)
		return 1;
	if (test_invalid_window() != 0)
		return 1;
	if (test_mod128_planned() != 0)
		return 1;
	if (test_receive_update() != 0)
		return 1;

	return 0;
}

static int
test_ack_update(void)
{
	struct kn_ax25_sequence_state state;

	kn_ax25_sequence_state_clear(&state);
	if (kn_ax25_sequence_ack_update_mod8(&state, 4) !=
	    KN_AX25_SEQUENCE_OK)
		return 1;

	return state.acknowledge_state == 4 && state.remote_busy == 0 ? 0 : 1;
}

static int
test_expected_match(void)
{
	struct kn_ax25_sequence_state state;

	kn_ax25_sequence_state_clear(&state);
	state.receive_state = 3;

	return kn_ax25_sequence_expected_mod8(&state, 3) != 0 ? 0 : 1;
}

static int
test_expected_mismatch(void)
{
	struct kn_ax25_sequence_state state;

	kn_ax25_sequence_state_clear(&state);
	state.receive_state = 3;

	return kn_ax25_sequence_expected_mod8(&state, 4) == 0 ? 0 : 1;
}

static int
test_increment(void)
{
	uint8_t i;

	for (i = 0; i < 7; i++) {
		if (kn_ax25_sequence_increment_mod8(i) != i + 1)
			return 1;
	}

	return 0;
}

static int
test_invalid_sequence(void)
{
	struct kn_ax25_sequence_state state;

	kn_ax25_sequence_state_clear(&state);

	return kn_ax25_sequence_ack_update_mod8(&state, 8) ==
	    KN_AX25_SEQUENCE_ERR_INVALID_VALUE ? 0 : 1;
}

static int
test_invalid_window(void)
{
	if (kn_ax25_sequence_window_valid(KN_AX25_MODULO_8, 8) !=
	    KN_AX25_SEQUENCE_ERR_INVALID_VALUE)
		return 1;
	if (kn_ax25_sequence_window_valid(KN_AX25_MODULO_128, 64) !=
	    KN_AX25_SEQUENCE_ERR_INVALID_VALUE)
		return 1;

	return kn_ax25_sequence_window_valid(KN_AX25_MODULO_8, 7) ==
	    KN_AX25_SEQUENCE_OK ? 0 : 1;
}

static int
test_mod128_planned(void)
{
	return kn_ax25_sequence_mod128_planned() ==
	    KN_AX25_SEQUENCE_ERR_NOT_IMPLEMENTED ? 0 : 1;
}

static int
test_receive_update(void)
{
	struct kn_ax25_sequence_state state;

	kn_ax25_sequence_state_clear(&state);
	state.receive_state = 7;
	if (kn_ax25_sequence_receive_i_mod8(&state, 7) !=
	    KN_AX25_SEQUENCE_OK)
		return 1;

	return state.receive_state == 0 ? 0 : 1;
}

static int
test_rej_marks_retransmit(void)
{
	struct kn_ax25_sequence_state state;

	kn_ax25_sequence_state_clear(&state);
	if (kn_ax25_sequence_mark_rej_mod8(&state, 2) !=
	    KN_AX25_SEQUENCE_OK)
		return 1;

	return state.retransmit_needed == 1 &&
	    state.acknowledge_state == 2 ? 0 : 1;
}

static int
test_rnr_marks_busy(void)
{
	struct kn_ax25_sequence_state state;

	kn_ax25_sequence_state_clear(&state);
	if (kn_ax25_sequence_mark_rnr_mod8(&state, 5) !=
	    KN_AX25_SEQUENCE_OK)
		return 1;

	return state.remote_busy == 1 && state.acknowledge_state == 5 ? 0 : 1;
}

static int
test_wrap(void)
{
	if (kn_ax25_sequence_increment_mod8(7) != 0)
		return 1;

	return kn_ax25_sequence_normalize_mod8(15) == 7 ? 0 : 1;
}
