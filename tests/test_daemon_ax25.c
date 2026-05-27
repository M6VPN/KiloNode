/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_daemon_ax25.c */

#include <sys/types.h>

#include "kilonode/ax25_runtime.h"

static int test_daemon_ax25_runtime_defaults(void);
static int test_daemon_ax25_runtime_no_live_processing_default(void);

int
main(void)
{
	if (test_daemon_ax25_runtime_defaults() != 0)
		return 1;
	if (test_daemon_ax25_runtime_no_live_processing_default() != 0)
		return 1;

	return 0;
}

static int
test_daemon_ax25_runtime_defaults(void)
{
	struct kn_ax25_runtime runtime;

	kn_ax25_runtime_init(&runtime);
	if (runtime.enabled != 0)
		return 1;
	if (runtime.connected_mode_enabled != 0)
		return 1;

	return runtime.diagnostics_enabled == 1 ? 0 : 1;
}

static int
test_daemon_ax25_runtime_no_live_processing_default(void)
{
	struct kn_ax25_runtime runtime;

	kn_ax25_runtime_init(&runtime);

	return kn_ax25_runtime_connection_count(&runtime) == 0 ? 0 : 1;
}
