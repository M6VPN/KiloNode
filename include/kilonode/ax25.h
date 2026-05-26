/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/ax25.h */

#ifndef KILONODE_AX25_H
#define KILONODE_AX25_H

#include <sys/types.h>

#include <stdint.h>

#include "kilonode/buffer.h"
#include "kilonode/callsign.h"

#define KN_AX25_ADDR_LEN  7
#define KN_AX25_MAX_DIGIS 8

#define KN_AX25_CONTROL_UI     0x03
#define KN_AX25_PID_NO_LAYER_3 0xf0

enum kn_ax25_error {
	KN_AX25_OK = 0,
	KN_AX25_ERR_INVALID_ARGUMENT,
	KN_AX25_ERR_SHORT_FRAME,
	KN_AX25_ERR_UNTERMINATED_ADDRESS,
	KN_AX25_ERR_MALFORMED_ADDRESS,
	KN_AX25_ERR_TOO_MANY_DIGIS,
	KN_AX25_ERR_INVALID_SSID,
	KN_AX25_ERR_BUFFER,
	KN_AX25_ERR_MISSING_PID
};

struct kn_ax25_addr {
	struct kn_callsign callsign;
	uint8_t repeated;
};

struct kn_ax25_frame {
	struct kn_ax25_addr destination;
	struct kn_ax25_addr source;
	struct kn_ax25_addr digipeaters[KN_AX25_MAX_DIGIS];
	size_t digipeater_count;
	uint8_t control;
	uint8_t pid;
	uint8_t has_pid;
	const uint8_t *payload;
	size_t payload_len;
};

enum kn_ax25_error kn_ax25_address_decode(const uint8_t *, size_t,
	struct kn_ax25_addr *, uint8_t *);
enum kn_ax25_error kn_ax25_address_encode(const struct kn_ax25_addr *,
	uint8_t, struct kn_buffer *);
enum kn_ax25_error kn_ax25_frame_decode(const uint8_t *, size_t,
	struct kn_ax25_frame *);
void kn_ax25_frame_reset(struct kn_ax25_frame *);
enum kn_ax25_error kn_ax25_ui_frame_encode(const struct kn_ax25_frame *,
	struct kn_buffer *);

#endif
