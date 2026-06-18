/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/proto/ax25_loopback_report.c */

#include <sys/types.h>

#include <stdio.h>

#include "kilonode/ax25_loopback_report.h"

enum kn_ax25_loopback_report_error
kn_ax25_loopback_report_format(const struct kn_ax25_loopback_result *result,
	char *buf, size_t bufsiz)
{
	int needed;

	if (result == NULL || buf == NULL || bufsiz == 0)
		return KN_AX25_LOOPBACK_REPORT_ERR_INVALID_ARGUMENT;
	needed = snprintf(buf, bufsiz,
	    "AX25-LOOPBACK name=%s result=%s A=%s B=%s "
	    "prepared=%llu transferred=%llu A_delivered=%llu "
	    "B_delivered=%llu A_rejected=%llu B_rejected=%llu "
	    "A_reassembled=%llu B_reassembled=%llu segments_sent=%llu "
	    "segments_received=%llu "
	    "i_sent=%llu i_received=%llu rr_sent=%llu rr_received=%llu "
	    "outstanding=%llu outstanding_max=%llu acked=%llu "
	    "outstanding_rejected=%llu window_blocked=%llu "
	    "tx_writes=%llu dispatch=%llu fx25=%llu "
	    "mismatches=%llu last=%s\n",
	    result->name, result->pass != 0 ? "pass" : "fail",
	    kn_ax25_connection_state_name(result->endpoint_a_state),
	    kn_ax25_connection_state_name(result->endpoint_b_state),
	    (unsigned long long)result->prepared_frames_generated,
	    (unsigned long long)result->raw_ax25_frames_transferred,
	    (unsigned long long)result->endpoint_a_delivered,
	    (unsigned long long)result->endpoint_b_delivered,
	    (unsigned long long)result->endpoint_a_rejected,
	    (unsigned long long)result->endpoint_b_rejected,
	    (unsigned long long)result->endpoint_a_reassembled,
	    (unsigned long long)result->endpoint_b_reassembled,
	    (unsigned long long)result->segments_sent,
	    (unsigned long long)result->segments_received,
	    (unsigned long long)result->i_frames_sent,
	    (unsigned long long)result->i_frames_received,
	    (unsigned long long)result->rr_frames_sent,
	    (unsigned long long)result->rr_frames_received,
	    (unsigned long long)result->outstanding_frames,
	    (unsigned long long)result->outstanding_max_seen,
	    (unsigned long long)result->outstanding_acked,
	    (unsigned long long)result->outstanding_rejected,
	    (unsigned long long)result->window_blocked,
	    (unsigned long long)result->real_tx_queue_writes,
	    (unsigned long long)result->dispatch_calls,
	    (unsigned long long)result->fx25_frames_generated,
	    (unsigned long long)result->mismatch_count,
	    result->last_mismatch[0] == '\0' ? "-" : result->last_mismatch);
	if (needed < 0 || (size_t)needed >= bufsiz)
		return KN_AX25_LOOPBACK_REPORT_ERR_BUFFER;

	return KN_AX25_LOOPBACK_REPORT_OK;
}
