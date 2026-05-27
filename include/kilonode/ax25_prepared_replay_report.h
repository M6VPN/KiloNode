/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/ax25_prepared_replay_report.h */

#ifndef KILONODE_AX25_PREPARED_REPLAY_REPORT_H
#define KILONODE_AX25_PREPARED_REPLAY_REPORT_H

#include <sys/types.h>

#include "kilonode/ax25_prepared_replay_check.h"

enum kn_ax25_prepared_replay_report_error {
	KN_AX25_PREPARED_REPLAY_REPORT_OK = 0,
	KN_AX25_PREPARED_REPLAY_REPORT_ERR_INVALID_ARGUMENT,
	KN_AX25_PREPARED_REPLAY_REPORT_ERR_BUFFER
};

enum kn_ax25_prepared_replay_report_error
kn_ax25_prepared_replay_report_format(const char *,
	const struct kn_ax25_prepared_replay_check_result *, char *, size_t);

#endif
