/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/ax25_sequence.h */

#ifndef KILONODE_AX25_SEQUENCE_H
#define KILONODE_AX25_SEQUENCE_H

#include <stdint.h>

#include "kilonode/ax25_params.h"

enum kn_ax25_sequence_error {
	KN_AX25_SEQUENCE_OK = 0,
	KN_AX25_SEQUENCE_ERR_INVALID_ARGUMENT,
	KN_AX25_SEQUENCE_ERR_INVALID_VALUE,
	KN_AX25_SEQUENCE_ERR_NOT_IMPLEMENTED
};

struct kn_ax25_sequence_state {
	uint8_t send_state;
	uint8_t receive_state;
	uint8_t acknowledge_state;
	uint8_t remote_busy;
	uint8_t retransmit_needed;
};

enum kn_ax25_sequence_error kn_ax25_sequence_ack_update_mod8(
	struct kn_ax25_sequence_state *, uint8_t);
uint8_t kn_ax25_sequence_expected_mod8(
	const struct kn_ax25_sequence_state *, uint8_t);
uint8_t kn_ax25_sequence_increment_mod8(uint8_t);
enum kn_ax25_sequence_error kn_ax25_sequence_mark_rej_mod8(
	struct kn_ax25_sequence_state *, uint8_t);
enum kn_ax25_sequence_error kn_ax25_sequence_mark_rnr_mod8(
	struct kn_ax25_sequence_state *, uint8_t);
enum kn_ax25_sequence_error kn_ax25_sequence_mod128_planned(void);
uint8_t kn_ax25_sequence_normalize_mod8(uint8_t);
enum kn_ax25_sequence_error kn_ax25_sequence_receive_i_mod8(
	struct kn_ax25_sequence_state *, uint8_t);
enum kn_ax25_sequence_error kn_ax25_sequence_send_i_mod8(
	struct kn_ax25_sequence_state *, uint8_t *);
void kn_ax25_sequence_state_clear(struct kn_ax25_sequence_state *);
enum kn_ax25_sequence_error kn_ax25_sequence_window_valid(
	enum kn_ax25_modulo_mode, uint8_t);

#endif
