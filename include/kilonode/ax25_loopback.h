/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/ax25_loopback.h */

#ifndef KILONODE_AX25_LOOPBACK_H
#define KILONODE_AX25_LOOPBACK_H

#include <sys/types.h>

#include <stdint.h>

#include "kilonode/ax25_loopback_endpoint.h"
#include "kilonode/ax25_loopback_link.h"
#include "kilonode/ax25_loopback_script.h"

struct kn_ax25_loopback {
	struct kn_ax25_loopback_endpoint a;
	struct kn_ax25_loopback_endpoint b;
	struct kn_ax25_loopback_link link;
	uint64_t now_ms;
	uint64_t mismatch_count;
	uint64_t prepared_frames_generated;
	uint64_t raw_ax25_frames_transferred;
	uint64_t rejected_payloads;
	uint64_t i_frames_sent;
	uint64_t i_frames_received;
	uint64_t rr_frames_sent;
	uint64_t rr_frames_received;
	uint64_t real_tx_queue_writes;
	uint64_t dispatch_calls;
	uint64_t fx25_frames_generated;
	char name[KN_AX25_LOOPBACK_SCRIPT_TEXT_MAX];
	char last_mismatch[KN_AX25_LOOPBACK_SCRIPT_TEXT_MAX];
};

struct kn_ax25_loopback_result {
	uint8_t pass;
	char name[KN_AX25_LOOPBACK_SCRIPT_TEXT_MAX];
	enum kn_ax25_connection_state endpoint_a_state;
	enum kn_ax25_connection_state endpoint_b_state;
	uint64_t endpoint_a_delivered;
	uint64_t endpoint_b_delivered;
	uint64_t prepared_frames_generated;
	uint64_t raw_ax25_frames_transferred;
	uint64_t endpoint_a_rejected;
	uint64_t endpoint_b_rejected;
	uint64_t i_frames_sent;
	uint64_t i_frames_received;
	uint64_t rr_frames_sent;
	uint64_t rr_frames_received;
	uint64_t real_tx_queue_writes;
	uint64_t dispatch_calls;
	uint64_t fx25_frames_generated;
	uint64_t mismatch_count;
	char last_mismatch[KN_AX25_LOOPBACK_SCRIPT_TEXT_MAX];
};

void kn_ax25_loopback_init(struct kn_ax25_loopback *);
void kn_ax25_loopback_reset(struct kn_ax25_loopback *);
enum kn_ax25_loopback_error kn_ax25_loopback_run_file(const char *,
	struct kn_ax25_loopback_result *,
	struct kn_ax25_loopback_error_info *);
enum kn_ax25_loopback_error kn_ax25_loopback_run_script(
	const struct kn_ax25_loopback_script *,
	struct kn_ax25_loopback_result *,
	struct kn_ax25_loopback_error_info *);

#endif
