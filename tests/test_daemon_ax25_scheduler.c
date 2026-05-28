/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_daemon_ax25_scheduler.c */

#include <sys/types.h>

#include <stdint.h>

#include "kilonode/daemon_ax25_scheduler.h"

static int configure_runtime(struct kn_ax25_runtime *, uint8_t);
static int test_poll_default_noop(void);
static int test_poll_scheduler_without_smoke(void);
static int test_poll_smoke_with_injected_time(void);

int
main(void)
{
	if (test_poll_default_noop() != 0)
		return 1;
	if (test_poll_scheduler_without_smoke() != 0)
		return 1;
	if (test_poll_smoke_with_injected_time() != 0)
		return 1;

	return 0;
}

static int
configure_runtime(struct kn_ax25_runtime *runtime, uint8_t smoke_enabled)
{
	struct kn_ax25_scheduler_policy policy;
	struct kn_ax25_scheduler_smoke_options smoke;

	kn_ax25_runtime_init(runtime);
	if (kn_ax25_runtime_set_enabled(runtime, 1, 0) !=
	    KN_AX25_RUNTIME_OK)
		return 1;
	kn_ax25_scheduler_policy_default(&policy);
	policy.enabled = 1;
	policy.process_expired = 1;
	policy.max_expired_per_cycle = 1;
	if (kn_ax25_runtime_set_scheduler_policy(runtime, &policy) !=
	    KN_AX25_RUNTIME_OK)
		return 1;
	kn_ax25_scheduler_smoke_options_default(&smoke);
	smoke.enabled = smoke_enabled;
	smoke.create_test_connection = smoke_enabled;
	if (kn_ax25_runtime_set_scheduler_smoke_options(runtime, &smoke) !=
	    KN_AX25_RUNTIME_OK)
		return 1;

	return 0;
}

static int
test_poll_default_noop(void)
{
	struct kn_ax25_runtime runtime;

	kn_ax25_runtime_init(&runtime);
	if (kn_daemon_ax25_scheduler_poll(&runtime, 0) !=
	    KN_DAEMON_AX25_SCHEDULER_OK)
		return 1;

	return runtime.live_scheduler.cycles_run == 0 ? 0 : 1;
}

static int
test_poll_scheduler_without_smoke(void)
{
	struct kn_ax25_runtime runtime;

	if (configure_runtime(&runtime, 0) != 0)
		return 1;
	if (kn_daemon_ax25_scheduler_poll(&runtime, 0) !=
	    KN_DAEMON_AX25_SCHEDULER_OK)
		return 1;
	if (runtime.smoke_counters.cycles != 0)
		return 1;

	return runtime.live_scheduler.cycles_run == 1 ? 0 : 1;
}

static int
test_poll_smoke_with_injected_time(void)
{
	struct kn_ax25_runtime runtime;

	if (configure_runtime(&runtime, 1) != 0)
		return 1;
	if (kn_daemon_ax25_scheduler_poll(&runtime, 0) !=
	    KN_DAEMON_AX25_SCHEDULER_OK)
		return 1;
	if (runtime.smoke_counters.cycles != 1)
		return 1;
	if (runtime.smoke_counters.test_connections_created != 1)
		return 1;
	if (runtime.smoke_counters.tx_writes_attempted != 0)
		return 1;

	return runtime.smoke_counters.dispatch_calls_attempted == 0 ? 0 : 1;
}
