/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/ax25_paclen.h */

#ifndef KILONODE_AX25_PACLEN_H
#define KILONODE_AX25_PACLEN_H

#include <sys/types.h>

#include "kilonode/ax25_params.h"

#define KN_AX25_PACLEN_MIN 1
#define KN_AX25_PACLEN_MAX 2048

enum kn_ax25_paclen_error {
	KN_AX25_PACLEN_OK = 0,
	KN_AX25_PACLEN_ERR_INVALID_ARGUMENT,
	KN_AX25_PACLEN_ERR_INVALID_VALUE,
	KN_AX25_PACLEN_ERR_BUFFER
};

enum kn_ax25_paclen_error kn_ax25_paclen_derive(
	const struct kn_ax25_params *, size_t *);
enum kn_ax25_paclen_error kn_ax25_paclen_format(
	const struct kn_ax25_params *, char *, size_t);
enum kn_ax25_paclen_error kn_ax25_paclen_validate(
	size_t, size_t);

#endif
