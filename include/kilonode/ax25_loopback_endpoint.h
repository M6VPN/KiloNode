/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/ax25_loopback_endpoint.h */

#ifndef KILONODE_AX25_LOOPBACK_ENDPOINT_H
#define KILONODE_AX25_LOOPBACK_ENDPOINT_H

#include <sys/types.h>

#include <stdint.h>

#include "kilonode/ax25.h"
#include "kilonode/ax25_connection_table.h"
#include "kilonode/ax25_payload_delivery.h"
#include "kilonode/ax25_prepared_queue.h"
#include "kilonode/ax25_scheduler.h"

#define KN_AX25_LOOPBACK_ENDPOINT_NAME_MAX 32
#define KN_AX25_LOOPBACK_LAST_ERROR_MAX    96
#define KN_AX25_LOOPBACK_I_PAYLOAD_MAX     128
#define KN_AX25_LOOPBACK_ENDPOINT_STEP_MAX 16

enum kn_ax25_loopback_endpoint_error {
	KN_AX25_LOOPBACK_ENDPOINT_OK = 0,
	KN_AX25_LOOPBACK_ENDPOINT_ERR_INVALID_ARGUMENT,
	KN_AX25_LOOPBACK_ENDPOINT_ERR_INVALID_VALUE,
	KN_AX25_LOOPBACK_ENDPOINT_ERR_STATE,
	KN_AX25_LOOPBACK_ENDPOINT_ERR_DECODE,
	KN_AX25_LOOPBACK_ENDPOINT_ERR_TABLE,
	KN_AX25_LOOPBACK_ENDPOINT_ERR_SCHEDULER,
	KN_AX25_LOOPBACK_ENDPOINT_ERR_PREPARED,
	KN_AX25_LOOPBACK_ENDPOINT_ERR_BUFFER,
	KN_AX25_LOOPBACK_ENDPOINT_ERR_UNSUPPORTED
};

struct kn_ax25_loopback_endpoint {
	char name[KN_AX25_LOOPBACK_ENDPOINT_NAME_MAX];
	char port_name[KN_CONFIG_PORT_NAME_MAX];
	struct kn_callsign local;
	struct kn_callsign peer;
	struct kn_ax25_params params;
	struct kn_ax25_connection_table table;
	struct kn_ax25_scheduler scheduler;
	struct kn_ax25_prepared_queue prepared;
	struct kn_ax25_payload_delivery_queue deliveries;
	uint64_t now_ms;
	size_t sent_prepared_count;
	uint64_t inbound_frames;
	uint64_t outbound_prepared_frames;
	uint64_t delivered_payloads;
	uint64_t rejected_payloads;
	uint64_t i_frames_sent;
	uint64_t i_frames_received;
	uint64_t rr_frames_sent;
	uint64_t rr_frames_received;
	uint64_t tx_queue_writes;
	uint64_t dispatch_calls;
	uint64_t fx25_frames;
	enum kn_ax25_connection_state last_state;
	char last_error[KN_AX25_LOOPBACK_LAST_ERROR_MAX];
};

void kn_ax25_loopback_endpoint_free(struct kn_ax25_loopback_endpoint *);
enum kn_ax25_loopback_endpoint_error kn_ax25_loopback_endpoint_format(
	const struct kn_ax25_loopback_endpoint *, char *, size_t);
enum kn_ax25_loopback_endpoint_error kn_ax25_loopback_endpoint_init(
	struct kn_ax25_loopback_endpoint *, const char *, const char *,
	const char *, const char *, const struct kn_ax25_params *);
enum kn_ax25_loopback_endpoint_error kn_ax25_loopback_endpoint_local_connect(
	struct kn_ax25_loopback_endpoint *);
enum kn_ax25_loopback_endpoint_error
kn_ax25_loopback_endpoint_local_disconnect(
	struct kn_ax25_loopback_endpoint *);
enum kn_ax25_loopback_endpoint_error kn_ax25_loopback_endpoint_process_frame(
	struct kn_ax25_loopback_endpoint *, const uint8_t *, size_t);
enum kn_ax25_loopback_endpoint_error
kn_ax25_loopback_endpoint_process_timers(
	struct kn_ax25_loopback_endpoint *, size_t, size_t *);
void kn_ax25_loopback_endpoint_reset(struct kn_ax25_loopback_endpoint *);
enum kn_ax25_loopback_endpoint_error kn_ax25_loopback_endpoint_send_i(
	struct kn_ax25_loopback_endpoint *, const uint8_t *, size_t,
	uint8_t, uint8_t, uint8_t *, size_t, size_t *);
enum kn_ax25_loopback_endpoint_error kn_ax25_loopback_endpoint_state(
	const struct kn_ax25_loopback_endpoint *,
	enum kn_ax25_connection_state *);

#endif
