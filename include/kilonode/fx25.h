/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/fx25.h */

#ifndef KILONODE_FX25_H
#define KILONODE_FX25_H

#include <sys/types.h>

#include <stdint.h>

#include "kilonode/fx25_params.h"

enum kn_fx25_error {
	KN_FX25_OK = 0,
	KN_FX25_ERR_INVALID_ARGUMENT,
	KN_FX25_ERR_TOO_LARGE,
	KN_FX25_ERR_NOT_IMPLEMENTED,
	KN_FX25_ERR_BUFFER
};

enum kn_fx25_mode {
	KN_FX25_MODE_DISABLED = 0,
	KN_FX25_MODE_DETECT_ONLY,
	KN_FX25_MODE_ENCODE_PLANNED,
	KN_FX25_MODE_DECODE_PLANNED
};

enum kn_fx25_decode_status {
	KN_FX25_DECODE_NOT_FX25 = 0,
	KN_FX25_DECODE_CANDIDATE,
	KN_FX25_DECODE_VALID,
	KN_FX25_DECODE_CORRECTED,
	KN_FX25_DECODE_UNCORRECTABLE,
	KN_FX25_DECODE_MALFORMED,
	KN_FX25_DECODE_NOT_IMPLEMENTED
};

enum kn_fx25_payload_relation {
	KN_FX25_PAYLOAD_UNKNOWN = 0,
	KN_FX25_PAYLOAD_EMBEDDED_AX25
};

enum kn_fx25_fec_profile {
	KN_FX25_FEC_PROFILE_NONE = 0,
	KN_FX25_FEC_PROFILE_RS_PLANNED
};

struct kn_fx25_decode_result {
	enum kn_fx25_decode_status status;
	enum kn_fx25_payload_relation payload_relation;
	enum kn_fx25_fec_profile fec_profile;
	const uint8_t *ax25_payload;
	size_t ax25_payload_len;
	uint8_t corrected;
};

enum kn_fx25_error kn_fx25_decode_placeholder(const uint8_t *, size_t,
	const struct kn_fx25_params *, struct kn_fx25_decode_result *);
void kn_fx25_decode_result_clear(struct kn_fx25_decode_result *);
enum kn_fx25_error kn_fx25_decode_result_format(
	const struct kn_fx25_decode_result *, char *, size_t);
const char *kn_fx25_decode_status_name(enum kn_fx25_decode_status);
const char *kn_fx25_fec_profile_name(enum kn_fx25_fec_profile);
const char *kn_fx25_mode_name(enum kn_fx25_mode);
const char *kn_fx25_payload_relation_name(enum kn_fx25_payload_relation);

#endif
