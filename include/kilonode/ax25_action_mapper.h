/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/ax25_action_mapper.h */

#ifndef KILONODE_AX25_ACTION_MAPPER_H
#define KILONODE_AX25_ACTION_MAPPER_H

#include <sys/types.h>

#include <stdint.h>

#include "kilonode/ax25_action.h"
#include "kilonode/ax25_frame_plan.h"
#include "kilonode/ax25_params.h"

enum kn_ax25_action_mapper_error {
	KN_AX25_ACTION_MAPPER_OK = 0,
	KN_AX25_ACTION_MAPPER_ERR_INVALID_ARGUMENT,
	KN_AX25_ACTION_MAPPER_ERR_INVALID_VALUE,
	KN_AX25_ACTION_MAPPER_ERR_PLAN_OVERFLOW
};

struct kn_ax25_action_mapper_context {
	struct kn_callsign local;
	struct kn_callsign remote;
	struct kn_callsign digipeaters[KN_AX25_MAX_DIGIS];
	size_t digipeater_count;
	uint8_t receive_state;
	uint8_t send_state;
	enum kn_ax25_modulo_mode modulo_mode;
	uint8_t poll_final;
};

void kn_ax25_action_mapper_context_clear(
	struct kn_ax25_action_mapper_context *);
enum kn_ax25_action_mapper_error kn_ax25_action_mapper_context_init(
	struct kn_ax25_action_mapper_context *, const char *, const char *);
enum kn_ax25_action_mapper_error kn_ax25_action_mapper_map_action(
	const struct kn_ax25_action_mapper_context *,
	const struct kn_ax25_action *, struct kn_ax25_frame_plan_list *);
enum kn_ax25_action_mapper_error kn_ax25_action_mapper_map_list(
	const struct kn_ax25_action_mapper_context *,
	const struct kn_ax25_action_list *, struct kn_ax25_frame_plan_list *);

#endif
