/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_compat_risk.c */

#include <sys/types.h>

#include <string.h>

#include "kilonode/compat_risk.h"

static int test_default_register(void);
static int test_names(void);
static int test_report(void);

int
main(void)
{
	if (test_default_register() != 0)
		return 1;
	if (test_names() != 0)
		return 1;
	if (test_report() != 0)
		return 1;
	return 0;
}

static int
test_default_register(void)
{
	struct kn_compat_risk_register reg;

	if (kn_compat_risk_register_default(&reg) != KN_COMPAT_RISK_OK)
		return 1;
	return reg.risk_count == 10 ? 0 : 1;
}

static int
test_names(void)
{
	if (strcmp(kn_compat_risk_severity_name(KN_COMPAT_RISK_CRITICAL),
	    "critical") != 0)
		return 1;
	if (strcmp(kn_compat_risk_status_name(KN_COMPAT_RISK_DEFERRED),
	    "deferred") != 0)
		return 1;
	return strcmp(kn_compat_risk_error_name(KN_COMPAT_RISK_ERR_BUFFER),
	    "buffer") == 0 ? 0 : 1;
}

static int
test_report(void)
{
	struct kn_compat_risk_register reg;
	char report[KN_COMPAT_RISK_REPORT_MAX];

	if (kn_compat_risk_register_default(&reg) != KN_COMPAT_RISK_OK)
		return 1;
	if (kn_compat_risk_register_report(&reg, report, sizeof(report)) !=
	    KN_COMPAT_RISK_OK)
		return 1;
	if (strstr(report, "RISK-REGISTER count=10") == NULL)
		return 1;
	return strstr(report, "GPL contamination risk") != NULL ? 0 : 1;
}
