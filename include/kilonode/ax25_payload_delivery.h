/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/ax25_payload_delivery.h */

#ifndef KILONODE_AX25_PAYLOAD_DELIVERY_H
#define KILONODE_AX25_PAYLOAD_DELIVERY_H

#include <sys/types.h>

#include <stdint.h>

#include "kilonode/callsign.h"
#include "kilonode/config.h"

#define KN_AX25_PAYLOAD_DELIVERY_MAX         64
#define KN_AX25_PAYLOAD_DELIVERY_PREVIEW_MAX 80
#define KN_AX25_PAYLOAD_DELIVERY_REASON_MAX  48
#define KN_AX25_PAYLOAD_DELIVERY_NAME_MAX    32

enum kn_ax25_payload_delivery_error {
	KN_AX25_PAYLOAD_DELIVERY_OK = 0,
	KN_AX25_PAYLOAD_DELIVERY_ERR_INVALID_ARGUMENT,
	KN_AX25_PAYLOAD_DELIVERY_ERR_INVALID_VALUE,
	KN_AX25_PAYLOAD_DELIVERY_ERR_FULL,
	KN_AX25_PAYLOAD_DELIVERY_ERR_BUFFER
};

struct kn_ax25_payload_delivery_record {
	uint64_t id;
	char endpoint[KN_AX25_PAYLOAD_DELIVERY_NAME_MAX];
	char port_name[KN_CONFIG_PORT_NAME_MAX];
	struct kn_callsign source;
	struct kn_callsign destination;
	uint8_t ns;
	uint8_t nr;
	size_t payload_len;
	size_t preview_len;
	uint8_t preview[KN_AX25_PAYLOAD_DELIVERY_PREVIEW_MAX];
	uint8_t payload_is_text;
	uint8_t accepted;
	char reason[KN_AX25_PAYLOAD_DELIVERY_REASON_MAX];
};

struct kn_ax25_payload_delivery_queue {
	struct kn_ax25_payload_delivery_record
	    records[KN_AX25_PAYLOAD_DELIVERY_MAX];
	size_t count;
	uint64_t next_id;
	uint64_t accepted_count;
	uint64_t rejected_count;
};

enum kn_ax25_payload_delivery_error kn_ax25_payload_delivery_format(
	const struct kn_ax25_payload_delivery_record *, char *, size_t);
const struct kn_ax25_payload_delivery_record *
kn_ax25_payload_delivery_last_accepted(
	const struct kn_ax25_payload_delivery_queue *);
const struct kn_ax25_payload_delivery_record *
kn_ax25_payload_delivery_last_rejected(
	const struct kn_ax25_payload_delivery_queue *);
void kn_ax25_payload_delivery_queue_init(
	struct kn_ax25_payload_delivery_queue *);
enum kn_ax25_payload_delivery_error kn_ax25_payload_delivery_queue_record(
	struct kn_ax25_payload_delivery_queue *, const char *, const char *,
	const struct kn_callsign *, const struct kn_callsign *, uint8_t,
	uint8_t, const uint8_t *, size_t, uint8_t, const char *, uint64_t *);
void kn_ax25_payload_delivery_queue_reset(
	struct kn_ax25_payload_delivery_queue *);
enum kn_ax25_payload_delivery_error kn_ax25_payload_delivery_preview_hex(
	const struct kn_ax25_payload_delivery_record *, char *, size_t);

#endif
