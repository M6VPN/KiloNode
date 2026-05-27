/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/ax25_action.h */

#ifndef KILONODE_AX25_ACTION_H
#define KILONODE_AX25_ACTION_H

#include <sys/types.h>

#include <stdint.h>

#define KN_AX25_ACTION_LIST_MAX 16

enum kn_ax25_action_error {
	KN_AX25_ACTION_OK = 0,
	KN_AX25_ACTION_ERR_INVALID_ARGUMENT,
	KN_AX25_ACTION_ERR_FULL,
	KN_AX25_ACTION_ERR_BUFFER
};

enum kn_ax25_action_intent {
	KN_AX25_ACTION_NONE = 0,
	KN_AX25_ACTION_SEND_SABM,
	KN_AX25_ACTION_SEND_SABME,
	KN_AX25_ACTION_SEND_UA,
	KN_AX25_ACTION_SEND_DM,
	KN_AX25_ACTION_SEND_DISC,
	KN_AX25_ACTION_SEND_RR,
	KN_AX25_ACTION_SEND_RNR,
	KN_AX25_ACTION_SEND_REJ,
	KN_AX25_ACTION_SEND_FRMR,
	KN_AX25_ACTION_DELIVER_I_PAYLOAD,
	KN_AX25_ACTION_START_T1,
	KN_AX25_ACTION_STOP_T1,
	KN_AX25_ACTION_START_T3,
	KN_AX25_ACTION_STOP_T3,
	KN_AX25_ACTION_RESET_RETRY_COUNT,
	KN_AX25_ACTION_INCREMENT_RETRY_COUNT,
	KN_AX25_ACTION_ENTER_CONNECTED,
	KN_AX25_ACTION_ENTER_DISCONNECTED,
	KN_AX25_ACTION_PROTOCOL_ERROR,
	KN_AX25_ACTION_RETRANSMIT_NEEDED
};

struct kn_ax25_action {
	enum kn_ax25_action_intent intent;
	uint8_t sequence;
};

struct kn_ax25_action_list {
	struct kn_ax25_action actions[KN_AX25_ACTION_LIST_MAX];
	size_t count;
};

enum kn_ax25_action_error kn_ax25_action_list_append(
	struct kn_ax25_action_list *, enum kn_ax25_action_intent);
enum kn_ax25_action_error kn_ax25_action_list_append_sequence(
	struct kn_ax25_action_list *, enum kn_ax25_action_intent, uint8_t);
void kn_ax25_action_list_clear(struct kn_ax25_action_list *);
enum kn_ax25_action_error kn_ax25_action_list_format(
	const struct kn_ax25_action_list *, char *, size_t);
const char *kn_ax25_action_intent_name(enum kn_ax25_action_intent);

#endif
