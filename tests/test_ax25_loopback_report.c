/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_ax25_loopback_report.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "kilonode/ax25_loopback_report.h"

int
main(void)
{
	struct kn_ax25_loopback_result result;
	char report[KN_AX25_LOOPBACK_REPORT_MAX];

	memset(&result, 0, sizeof(result));
	result.pass = 1;
	(void)snprintf(result.name, sizeof(result.name), "%s", "report");
	result.endpoint_a_state = KN_AX25_CONNECTION_CONNECTED;
	result.endpoint_b_state = KN_AX25_CONNECTION_CONNECTED;
	if (kn_ax25_loopback_report_format(&result, report,
	    sizeof(report)) != KN_AX25_LOOPBACK_REPORT_OK)
		return 1;
	return strstr(report, "AX25-LOOPBACK name=report result=pass") !=
	    NULL ? 0 : 1;
}
