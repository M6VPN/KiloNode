/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_ax25_scheduler_smoke_diag.c */

#include <sys/types.h>

#include <string.h>

#include "kilonode/ax25_scheduler_smoke_diag.h"

static int test_format_disabled(void);
static int test_format_enabled(void);
static int test_null_runtime_fallback(void);

int
main(void)
{
	if (test_format_disabled() != 0)
		return 1;
	if (test_format_enabled() != 0)
		return 1;
	if (test_null_runtime_fallback() != 0)
		return 1;

	return 0;
}

static int
test_format_disabled(void)
{
	struct kn_ax25_runtime runtime;
	char buf[256];

	kn_ax25_runtime_init(&runtime);
	if (kn_ax25_scheduler_smoke_diag_format(&runtime, buf,
	    sizeof(buf)) != KN_AX25_SCHEDULER_SMOKE_DIAG_OK)
		return 1;
	if (strstr(buf, "OK AX25 SCHEDULER SMOKE enabled=false") == NULL)
		return 1;

	return strstr(buf, "tx_writes=0 dispatch_calls=0") != NULL ? 0 : 1;
}

static int
test_format_enabled(void)
{
	struct kn_ax25_runtime runtime;
	char buf[256];

	kn_ax25_runtime_init(&runtime);
	runtime.smoke_options.enabled = 1;
	runtime.smoke_options.create_test_connection = 1;
	runtime.smoke_counters.cycles = 2;
	runtime.smoke_counters.test_connections_created = 1;
	if (kn_ax25_scheduler_smoke_diag_format(&runtime, buf,
	    sizeof(buf)) != KN_AX25_SCHEDULER_SMOKE_DIAG_OK)
		return 1;
	if (strstr(buf, "enabled=true create_test_connection=true") == NULL)
		return 1;

	return strstr(buf, "cycles=2 test_connections=1") != NULL ? 0 : 1;
}

static int
test_null_runtime_fallback(void)
{
	char buf[256];

	if (kn_ax25_scheduler_smoke_diag_format(NULL, buf, sizeof(buf)) !=
	    KN_AX25_SCHEDULER_SMOKE_DIAG_OK)
		return 1;

	return strstr(buf, "END\n") != NULL ? 0 : 1;
}
