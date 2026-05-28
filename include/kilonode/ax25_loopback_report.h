/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/ax25_loopback_report.h */

#ifndef KILONODE_AX25_LOOPBACK_REPORT_H
#define KILONODE_AX25_LOOPBACK_REPORT_H

#include <sys/types.h>

#include "kilonode/ax25_loopback.h"

#define KN_AX25_LOOPBACK_REPORT_MAX 4096

enum kn_ax25_loopback_report_error {
	KN_AX25_LOOPBACK_REPORT_OK = 0,
	KN_AX25_LOOPBACK_REPORT_ERR_INVALID_ARGUMENT,
	KN_AX25_LOOPBACK_REPORT_ERR_BUFFER
};

enum kn_ax25_loopback_report_error kn_ax25_loopback_report_format(
	const struct kn_ax25_loopback_result *, char *, size_t);

#endif
