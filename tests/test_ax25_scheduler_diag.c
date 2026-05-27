/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_ax25_scheduler_diag.c */

#include <sys/types.h>

#include <string.h>

#include "kilonode/ax25_scheduler_diag.h"
#include "kilonode/ax25_runtime.h"

static int test_counters_format(void);
static int test_status_format(void);
static int test_timers_empty_format(void);
static int test_timers_populated_format(void);
static int test_truncation(void);

int
main(void)
{
	if (test_status_format() != 0)
		return 1;
	if (test_timers_empty_format() != 0)
		return 1;
	if (test_timers_populated_format() != 0)
		return 1;
	if (test_counters_format() != 0)
		return 1;
	if (test_truncation() != 0)
		return 1;

	return 0;
}

static int
test_counters_format(void)
{
	struct kn_ax25_runtime runtime;
	char buf[256];

	kn_ax25_runtime_init(&runtime);
	runtime.live_scheduler.cycles_run = 2;
	runtime.live_scheduler.expired_processed = 1;
	if (kn_ax25_scheduler_diag_format_counters(&runtime, buf,
	    sizeof(buf)) != KN_AX25_SCHEDULER_DIAG_OK)
		return 1;

	return strstr(buf, "cycles=2 expired=1") != NULL &&
	    strstr(buf, "tx_writes=0") != NULL ? 0 : 1;
}

static int
test_status_format(void)
{
	struct kn_ax25_runtime runtime;
	char buf[192];

	kn_ax25_runtime_init(&runtime);
	if (kn_ax25_scheduler_diag_format_status(&runtime, buf,
	    sizeof(buf)) != KN_AX25_SCHEDULER_DIAG_OK)
		return 1;

	return strstr(buf, "OK AX25 SCHEDULER enabled=false") != NULL ?
	    0 : 1;
}

static int
test_timers_empty_format(void)
{
	struct kn_ax25_runtime runtime;
	char buf[128];

	kn_ax25_runtime_init(&runtime);
	if (kn_ax25_scheduler_diag_format_timers(&runtime, buf,
	    sizeof(buf)) != KN_AX25_SCHEDULER_DIAG_OK)
		return 1;

	return strstr(buf, "OK AX25 SCHEDULER TIMERS count=0") != NULL ?
	    0 : 1;
}

static int
test_timers_populated_format(void)
{
	struct kn_ax25_runtime runtime;
	char buf[512];

	kn_ax25_runtime_init(&runtime);
	if (kn_ax25_timer_queue_start(&runtime.scheduler.queue, 1,
	    KN_AX25_TIMER_T1, 10, 100) != KN_AX25_TIMER_QUEUE_OK)
		return 1;
	if (kn_ax25_scheduler_diag_format_timers(&runtime, buf,
	    sizeof(buf)) != KN_AX25_SCHEDULER_DIAG_OK)
		return 1;

	return strstr(buf, "AX25 TIMER index=0 conn=1 kind=T1") != NULL ?
	    0 : 1;
}

static int
test_truncation(void)
{
	struct kn_ax25_runtime runtime;
	char buf[8];

	kn_ax25_runtime_init(&runtime);

	return kn_ax25_scheduler_diag_format_status(&runtime, buf,
	    sizeof(buf)) == KN_AX25_SCHEDULER_DIAG_ERR_BUFFER ? 0 : 1;
}
