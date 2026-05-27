/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/proto/ax25_prepared_replay_report.c */

#include <sys/types.h>

#include <stdio.h>

#include "kilonode/ax25_prepared_replay_report.h"

enum kn_ax25_prepared_replay_report_error
kn_ax25_prepared_replay_report_format(const char *name,
	const struct kn_ax25_prepared_replay_check_result *result, char *buf,
	size_t bufsiz)
{
	size_t off;
	size_t i;
	int needed;

	if (name == NULL || result == NULL || buf == NULL || bufsiz == 0)
		return KN_AX25_PREPARED_REPLAY_REPORT_ERR_INVALID_ARGUMENT;

	needed = snprintf(buf, bufsiz,
	    "PREPARED-REPLAY name=%s result=%s prepared=%llu "
	    "tx_writes=%llu mismatches=%llu\n",
	    name, result->pass != 0 ? "pass" : "fail",
	    (unsigned long long)result->prepared_count,
	    (unsigned long long)result->tx_writes,
	    (unsigned long long)result->mismatch_count);
	if (needed < 0 || (size_t)needed >= bufsiz)
		return KN_AX25_PREPARED_REPLAY_REPORT_ERR_BUFFER;
	off = (size_t)needed;
	for (i = 0; i < result->mismatch_count; i++) {
		needed = snprintf(buf + off, bufsiz - off,
		    "PREPARED-REPLAY-MISMATCH line=%llu detail=%s\n",
		    (unsigned long long)result->mismatches[i].line,
		    result->mismatches[i].detail);
		if (needed < 0 || (size_t)needed >= bufsiz - off)
			return KN_AX25_PREPARED_REPLAY_REPORT_ERR_BUFFER;
		off += (size_t)needed;
	}

	return KN_AX25_PREPARED_REPLAY_REPORT_OK;
}
