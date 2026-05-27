/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/fx25_params.h */

#ifndef KILONODE_FX25_PARAMS_H
#define KILONODE_FX25_PARAMS_H

#include <sys/types.h>

#include <stdint.h>

enum kn_fx25_params_error {
	KN_FX25_PARAMS_OK = 0,
	KN_FX25_PARAMS_ERR_INVALID_ARGUMENT,
	KN_FX25_PARAMS_ERR_INVALID_VALUE,
	KN_FX25_PARAMS_ERR_BUFFER
};

struct kn_fx25_params {
	uint8_t enabled;
	uint8_t detect_only;
	size_t max_frame_bytes;
	uint8_t allow_fallback_ax25;
	uint8_t strict_mode;
};

void kn_fx25_params_default(struct kn_fx25_params *);
enum kn_fx25_params_error kn_fx25_params_format(
	const struct kn_fx25_params *, char *, size_t);
enum kn_fx25_params_error kn_fx25_params_validate(
	const struct kn_fx25_params *);

#endif
