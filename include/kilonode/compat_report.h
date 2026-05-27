/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/compat_report.h */

#ifndef KILONODE_COMPAT_REPORT_H
#define KILONODE_COMPAT_REPORT_H

#include <sys/types.h>

#include "kilonode/compat_replay.h"

enum kn_compat_report_error {
	KN_COMPAT_REPORT_OK = 0,
	KN_COMPAT_REPORT_ERR_INVALID_ARGUMENT,
	KN_COMPAT_REPORT_ERR_BUFFER
};

const char *kn_compat_report_error_name(enum kn_compat_report_error);
enum kn_compat_report_error kn_compat_report_format_text(
	const struct kn_compat_replay_result *, char *, size_t);

#endif
