/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/compat_axip_capture.h */

#ifndef KILONODE_COMPAT_AXIP_CAPTURE_H
#define KILONODE_COMPAT_AXIP_CAPTURE_H

#include "kilonode/compat_kiss_capture.h"

enum kn_compat_axip_capture_error {
	KN_COMPAT_AXIP_CAPTURE_OK = 0,
	KN_COMPAT_AXIP_CAPTURE_ERR_INVALID_ARGUMENT,
	KN_COMPAT_AXIP_CAPTURE_ERR_UNSUPPORTED,
	KN_COMPAT_AXIP_CAPTURE_ERR_DECODE,
	KN_COMPAT_AXIP_CAPTURE_ERR_MISMATCH
};

const char *kn_compat_axip_capture_error_name(
	enum kn_compat_axip_capture_error);
enum kn_compat_axip_capture_error kn_compat_axip_capture_decode(
	const struct kn_compat_packet_capture *,
	struct kn_compat_packet_decode *);

#endif
