/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/ax25_frame_plan.h */

#ifndef KILONODE_AX25_FRAME_PLAN_H
#define KILONODE_AX25_FRAME_PLAN_H

#include <sys/types.h>

#include <stdint.h>

#include "kilonode/ax25.h"
#include "kilonode/ax25_action.h"
#include "kilonode/ax25_control.h"
#include "kilonode/callsign.h"

#define KN_AX25_FRAME_PLAN_LIST_MAX 16

enum kn_ax25_frame_plan_error {
	KN_AX25_FRAME_PLAN_OK = 0,
	KN_AX25_FRAME_PLAN_ERR_INVALID_ARGUMENT,
	KN_AX25_FRAME_PLAN_ERR_INVALID_VALUE,
	KN_AX25_FRAME_PLAN_ERR_FULL,
	KN_AX25_FRAME_PLAN_ERR_BUFFER
};

enum kn_ax25_frame_plan_type {
	KN_AX25_FRAME_PLAN_SABM = 0,
	KN_AX25_FRAME_PLAN_SABME,
	KN_AX25_FRAME_PLAN_UA,
	KN_AX25_FRAME_PLAN_DM,
	KN_AX25_FRAME_PLAN_DISC,
	KN_AX25_FRAME_PLAN_RR,
	KN_AX25_FRAME_PLAN_RNR,
	KN_AX25_FRAME_PLAN_REJ,
	KN_AX25_FRAME_PLAN_FRMR,
	KN_AX25_FRAME_PLAN_UI,
	KN_AX25_FRAME_PLAN_UNKNOWN
};

struct kn_ax25_frame_plan {
	uint64_t id;
	struct kn_callsign source;
	struct kn_callsign destination;
	struct kn_callsign digipeaters[KN_AX25_MAX_DIGIS];
	size_t digipeater_count;
	enum kn_ax25_frame_plan_type type;
	uint8_t poll_final;
	uint8_t nr;
	uint8_t ns;
	uint8_t pid;
	const uint8_t *payload;
	size_t payload_len;
	enum kn_ax25_action_intent action_source;
	uint8_t needs_kiss;
	uint8_t needs_fx25;
};

struct kn_ax25_frame_plan_list {
	struct kn_ax25_frame_plan plans[KN_AX25_FRAME_PLAN_LIST_MAX];
	size_t count;
};

enum kn_ax25_frame_plan_error kn_ax25_frame_plan_format(
	const struct kn_ax25_frame_plan *, char *, size_t);
void kn_ax25_frame_plan_list_clear(struct kn_ax25_frame_plan_list *);
enum kn_ax25_frame_plan_error kn_ax25_frame_plan_list_append(
	struct kn_ax25_frame_plan_list *, const struct kn_ax25_frame_plan *);
const char *kn_ax25_frame_plan_type_name(enum kn_ax25_frame_plan_type);
enum kn_ax25_frame_plan_error kn_ax25_frame_plan_validate(
	const struct kn_ax25_frame_plan *);

#endif
