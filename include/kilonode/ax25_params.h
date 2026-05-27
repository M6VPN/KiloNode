/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/ax25_params.h */

#ifndef KILONODE_AX25_PARAMS_H
#define KILONODE_AX25_PARAMS_H

#include <sys/types.h>

#include <stdint.h>

enum kn_ax25_params_error {
	KN_AX25_PARAMS_OK = 0,
	KN_AX25_PARAMS_ERR_INVALID_ARGUMENT,
	KN_AX25_PARAMS_ERR_INVALID_VALUE,
	KN_AX25_PARAMS_ERR_BUFFER
};

enum kn_ax25_modulo_mode {
	KN_AX25_MODULO_8 = 0,
	KN_AX25_MODULO_128
};

struct kn_ax25_params {
	uint8_t allow_connected_mode;
	enum kn_ax25_modulo_mode modulo_mode;
	uint32_t t1_ms;
	uint32_t t2_ms;
	uint32_t t3_ms;
	uint8_t n2_retry_count;
	uint8_t window_size;
	size_t max_info_len;
	size_t paclen;
};

void kn_ax25_params_default(struct kn_ax25_params *);
enum kn_ax25_params_error kn_ax25_params_format(
	const struct kn_ax25_params *, char *, size_t);
const char *kn_ax25_params_modulo_name(enum kn_ax25_modulo_mode);
enum kn_ax25_params_error kn_ax25_params_validate(
	const struct kn_ax25_params *);

#endif
