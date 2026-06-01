/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/ax25_i_frame.h */

#ifndef KILONODE_AX25_I_FRAME_H
#define KILONODE_AX25_I_FRAME_H

#include <sys/types.h>

#include <stdint.h>

#include "kilonode/ax25.h"
#include "kilonode/ax25_params.h"

#define KN_AX25_I_FRAME_PREVIEW_MAX 80

enum kn_ax25_i_frame_error {
	KN_AX25_I_FRAME_OK = 0,
	KN_AX25_I_FRAME_ERR_INVALID_ARGUMENT,
	KN_AX25_I_FRAME_ERR_INVALID_VALUE,
	KN_AX25_I_FRAME_ERR_TOO_MANY_DIGIS,
	KN_AX25_I_FRAME_ERR_TOO_LARGE,
	KN_AX25_I_FRAME_ERR_BUFFER,
	KN_AX25_I_FRAME_ERR_MALFORMED,
	KN_AX25_I_FRAME_ERR_UNSUPPORTED
};

struct kn_ax25_i_frame_request {
	struct kn_callsign source;
	struct kn_callsign destination;
	struct kn_callsign digipeaters[KN_AX25_MAX_DIGIS];
	size_t digipeater_count;
	enum kn_ax25_modulo_mode modulo_mode;
	uint8_t ns;
	uint8_t nr;
	uint8_t poll_final;
	uint8_t pid;
	const uint8_t *payload;
	size_t payload_len;
	size_t max_info_len;
};

struct kn_ax25_i_frame_decoded {
	struct kn_callsign source;
	struct kn_callsign destination;
	struct kn_callsign digipeaters[KN_AX25_MAX_DIGIS];
	size_t digipeater_count;
	uint8_t ns;
	uint8_t nr;
	uint8_t poll_final;
	uint8_t pid;
	const uint8_t *payload;
	size_t payload_len;
	uint8_t payload_is_text;
	char preview[KN_AX25_I_FRAME_PREVIEW_MAX + 1];
};

enum kn_ax25_i_frame_error kn_ax25_i_frame_build(
	const struct kn_ax25_i_frame_request *, uint8_t *, size_t, size_t *);
void kn_ax25_i_frame_decoded_clear(struct kn_ax25_i_frame_decoded *);
enum kn_ax25_i_frame_error kn_ax25_i_frame_decode_raw(const uint8_t *,
	size_t, struct kn_ax25_i_frame_decoded *);
void kn_ax25_i_frame_request_clear(struct kn_ax25_i_frame_request *);

#endif
