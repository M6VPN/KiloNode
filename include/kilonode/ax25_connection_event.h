/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/ax25_connection_event.h */

#ifndef KILONODE_AX25_CONNECTION_EVENT_H
#define KILONODE_AX25_CONNECTION_EVENT_H

#include <sys/types.h>

#include <stdint.h>

#include "kilonode/ax25_connection_key.h"
#include "kilonode/ax25_state.h"

enum kn_ax25_connection_event_error {
	KN_AX25_CONNECTION_EVENT_OK = 0,
	KN_AX25_CONNECTION_EVENT_ERR_INVALID_ARGUMENT,
	KN_AX25_CONNECTION_EVENT_ERR_INVALID_VALUE,
	KN_AX25_CONNECTION_EVENT_ERR_IGNORED,
	KN_AX25_CONNECTION_EVENT_ERR_NOT_FOR_LOCAL
};

struct kn_ax25_connection_event_record {
	enum kn_ax25_connection_event kind;
	uint64_t timestamp;
	struct kn_ax25_connection_key key;
	struct kn_ax25_control_info control;
	uint8_t ns;
	uint8_t nr;
	uint8_t poll_final;
	size_t payload_len;
};

void kn_ax25_connection_event_clear(struct kn_ax25_connection_event_record *);
enum kn_ax25_connection_event_error kn_ax25_connection_event_from_frame(
	struct kn_ax25_connection_event_record *, uint64_t, const char *,
	const struct kn_callsign *, const struct kn_ax25_frame *);
enum kn_ax25_connection_event_error
kn_ax25_connection_event_local_connect(struct kn_ax25_connection_event_record *,
	uint64_t, const struct kn_ax25_connection_key *);
enum kn_ax25_connection_event_error
kn_ax25_connection_event_local_disconnect(
	struct kn_ax25_connection_event_record *, uint64_t,
	const struct kn_ax25_connection_key *);
enum kn_ax25_connection_event_error kn_ax25_connection_event_timeout_t1(
	struct kn_ax25_connection_event_record *, uint64_t,
	const struct kn_ax25_connection_key *);
enum kn_ax25_connection_event_error kn_ax25_connection_event_timeout_t3(
	struct kn_ax25_connection_event_record *, uint64_t,
	const struct kn_ax25_connection_key *);
enum kn_ax25_connection_event_error kn_ax25_connection_event_to_state_input(
	const struct kn_ax25_connection_event_record *,
	struct kn_ax25_state_input *);

#endif
