/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/compat_capture_report.h */

#ifndef KILONODE_COMPAT_CAPTURE_REPORT_H
#define KILONODE_COMPAT_CAPTURE_REPORT_H

#include <sys/types.h>

#include "kilonode/compat_kiss_capture.h"

#define KN_COMPAT_CAPTURE_REPORT_MAX 4096

enum kn_compat_capture_report_error {
	KN_COMPAT_CAPTURE_REPORT_OK = 0,
	KN_COMPAT_CAPTURE_REPORT_ERR_INVALID_ARGUMENT,
	KN_COMPAT_CAPTURE_REPORT_ERR_BUFFER,
	KN_COMPAT_CAPTURE_REPORT_ERR_UNSUPPORTED
};

const char *kn_compat_capture_report_error_name(
	enum kn_compat_capture_report_error);
enum kn_compat_capture_report_error kn_compat_capture_report_format(
	const struct kn_compat_packet_decode *, char *, size_t);
enum kn_compat_capture_report_error kn_compat_capture_to_transcript(
	const struct kn_compat_packet_capture *,
	const struct kn_compat_packet_decode *, char *, size_t);

#endif
