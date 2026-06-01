/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/ax25_reassembly.h */

#ifndef KILONODE_AX25_REASSEMBLY_H
#define KILONODE_AX25_REASSEMBLY_H

#include <sys/types.h>

#include <stdint.h>

#include "kilonode/callsign.h"
#include "kilonode/config.h"

#define KN_AX25_REASSEMBLY_MAX         32
#define KN_AX25_REASSEMBLY_PREVIEW_MAX 80
#define KN_AX25_REASSEMBLY_REASON_MAX  48
#define KN_AX25_REASSEMBLY_NAME_MAX    32

enum kn_ax25_reassembly_error {
	KN_AX25_REASSEMBLY_OK = 0,
	KN_AX25_REASSEMBLY_ERR_INVALID_ARGUMENT,
	KN_AX25_REASSEMBLY_ERR_INVALID_VALUE,
	KN_AX25_REASSEMBLY_ERR_FULL,
	KN_AX25_REASSEMBLY_ERR_BUFFER
};

struct kn_ax25_reassembly_record {
	uint64_t id;
	char endpoint[KN_AX25_REASSEMBLY_NAME_MAX];
	char port_name[KN_CONFIG_PORT_NAME_MAX];
	struct kn_callsign source;
	struct kn_callsign destination;
	size_t expected_segments;
	size_t received_segments;
	size_t total_payload_len;
	size_t preview_len;
	uint8_t preview[KN_AX25_REASSEMBLY_PREVIEW_MAX];
	uint8_t payload_is_text;
	uint8_t complete;
	char reason[KN_AX25_REASSEMBLY_REASON_MAX];
};

struct kn_ax25_reassembly_queue {
	struct kn_ax25_reassembly_record records[KN_AX25_REASSEMBLY_MAX];
	size_t count;
	uint64_t next_id;
	uint64_t complete_count;
	uint64_t rejected_count;
};

enum kn_ax25_reassembly_error kn_ax25_reassembly_format(
	const struct kn_ax25_reassembly_record *, char *, size_t);
const struct kn_ax25_reassembly_record *kn_ax25_reassembly_last_complete(
	const struct kn_ax25_reassembly_queue *);
enum kn_ax25_reassembly_error kn_ax25_reassembly_preview_hex(
	const struct kn_ax25_reassembly_record *, char *, size_t);
void kn_ax25_reassembly_queue_init(struct kn_ax25_reassembly_queue *);
enum kn_ax25_reassembly_error kn_ax25_reassembly_queue_record(
	struct kn_ax25_reassembly_queue *, const char *, const char *,
	const struct kn_callsign *, const struct kn_callsign *, size_t,
	size_t, const uint8_t *, size_t, uint8_t, const char *, uint64_t *);
void kn_ax25_reassembly_queue_reset(struct kn_ax25_reassembly_queue *);

#endif
