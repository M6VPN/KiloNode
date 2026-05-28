/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_ax25_scheduler_smoke.c */

#include <sys/types.h>

#include <stdint.h>

#include "kilonode/ax25_runtime.h"

static int configure_runtime(struct kn_ax25_runtime *, uint8_t, uint8_t);
static int test_create_test_connection_disabled_default(void);
static int test_default_disabled(void);
static int test_poll_after_expiry_processes_timer(void);
static int test_poll_before_expiry_no_expired(void);
static int test_reset_counters(void);
static int test_smoke_requires_enabled_ax25(void);
static int test_smoke_requires_scheduler(void);

int
main(void)
{
	if (test_default_disabled() != 0)
		return 1;
	if (test_smoke_requires_enabled_ax25() != 0)
		return 1;
	if (test_smoke_requires_scheduler() != 0)
		return 1;
	if (test_create_test_connection_disabled_default() != 0)
		return 1;
	if (test_poll_before_expiry_no_expired() != 0)
		return 1;
	if (test_poll_after_expiry_processes_timer() != 0)
		return 1;
	if (test_reset_counters() != 0)
		return 1;

	return 0;
}

static int
configure_runtime(struct kn_ax25_runtime *runtime, uint8_t process_expired,
	uint8_t create_test_connection)
{
	struct kn_ax25_params params;
	struct kn_ax25_scheduler_policy policy;
	struct kn_ax25_scheduler_smoke_options smoke;

	kn_ax25_runtime_init(runtime);
	kn_ax25_params_default(&params);
	params.t1_ms = 100;
	params.t2_ms = 50;
	params.t3_ms = 1000;
	if (kn_ax25_runtime_set_params(runtime, &params, 4) !=
	    KN_AX25_RUNTIME_OK)
		return 1;
	if (kn_ax25_runtime_set_enabled(runtime, 1, 0) !=
	    KN_AX25_RUNTIME_OK)
		return 1;
	kn_ax25_scheduler_policy_default(&policy);
	policy.enabled = 1;
	policy.process_expired = process_expired;
	policy.max_expired_per_cycle = 1;
	if (kn_ax25_runtime_set_scheduler_policy(runtime, &policy) !=
	    KN_AX25_RUNTIME_OK)
		return 1;
	kn_ax25_scheduler_smoke_options_default(&smoke);
	smoke.enabled = 1;
	smoke.create_test_connection = create_test_connection;
	if (kn_ax25_runtime_set_scheduler_smoke_options(runtime, &smoke) !=
	    KN_AX25_RUNTIME_OK)
		return 1;

	return 0;
}

static int
test_create_test_connection_disabled_default(void)
{
	struct kn_ax25_scheduler_smoke_options options;

	kn_ax25_scheduler_smoke_options_default(&options);
	return options.create_test_connection == 0 ? 0 : 1;
}

static int
test_default_disabled(void)
{
	struct kn_ax25_runtime runtime;

	kn_ax25_runtime_init(&runtime);
	if (runtime.smoke_options.enabled != 0)
		return 1;
	if (kn_ax25_scheduler_smoke_poll(&runtime, 0) !=
	    KN_AX25_SCHEDULER_SMOKE_OK)
		return 1;

	return runtime.smoke_counters.cycles == 0 ? 0 : 1;
}

static int
test_poll_after_expiry_processes_timer(void)
{
	struct kn_ax25_runtime runtime;

	if (configure_runtime(&runtime, 1, 1) != 0)
		return 1;
	if (kn_ax25_scheduler_smoke_poll(&runtime, 0) !=
	    KN_AX25_SCHEDULER_SMOKE_OK)
		return 1;
	if (kn_ax25_scheduler_smoke_poll(&runtime, 100) !=
	    KN_AX25_SCHEDULER_SMOKE_OK)
		return 1;
	if (runtime.smoke_counters.expired_processed == 0)
		return 1;
	if (runtime.smoke_counters.prepared_frames_generated == 0)
		return 1;
	if (runtime.smoke_counters.prepared_bridge_blocked == 0)
		return 1;
	if (runtime.smoke_counters.tx_writes_attempted != 0)
		return 1;

	return runtime.smoke_counters.dispatch_calls_attempted == 0 ? 0 : 1;
}

static int
test_poll_before_expiry_no_expired(void)
{
	struct kn_ax25_runtime runtime;

	if (configure_runtime(&runtime, 1, 1) != 0)
		return 1;
	if (kn_ax25_scheduler_smoke_poll(&runtime, 0) !=
	    KN_AX25_SCHEDULER_SMOKE_OK)
		return 1;
	if (runtime.smoke_counters.test_connections_created != 1)
		return 1;
	if (runtime.smoke_counters.expired_processed != 0)
		return 1;
	if (kn_ax25_runtime_connection_count(&runtime) != 1)
		return 1;

	return runtime.smoke_counters.tx_writes_attempted == 0 ? 0 : 1;
}

static int
test_reset_counters(void)
{
	struct kn_ax25_scheduler_smoke_counters counters;

	counters.cycles = 4;
	counters.tx_writes_attempted = 3;
	kn_ax25_scheduler_smoke_reset_counters(&counters);
	if (counters.cycles != 0)
		return 1;

	return counters.tx_writes_attempted == 0 ? 0 : 1;
}

static int
test_smoke_requires_enabled_ax25(void)
{
	struct kn_ax25_runtime runtime;
	struct kn_ax25_scheduler_smoke_options smoke;

	kn_ax25_runtime_init(&runtime);
	kn_ax25_scheduler_smoke_options_default(&smoke);
	smoke.enabled = 1;

	return kn_ax25_runtime_set_scheduler_smoke_options(&runtime,
	    &smoke) == KN_AX25_RUNTIME_ERR_INVALID_VALUE ? 0 : 1;
}

static int
test_smoke_requires_scheduler(void)
{
	struct kn_ax25_runtime runtime;
	struct kn_ax25_scheduler_smoke_options smoke;

	kn_ax25_runtime_init(&runtime);
	(void)kn_ax25_runtime_set_enabled(&runtime, 1, 0);
	kn_ax25_scheduler_smoke_options_default(&smoke);
	smoke.enabled = 1;

	return kn_ax25_runtime_set_scheduler_smoke_options(&runtime,
	    &smoke) == KN_AX25_RUNTIME_ERR_INVALID_VALUE ? 0 : 1;
}
