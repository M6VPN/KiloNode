/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_ax25_prepared_diag.c */

#include <sys/types.h>

#include <string.h>

#include "kilonode/ax25_connection_event.h"
#include "kilonode/ax25_prepared_diag.h"
#include "kilonode/ax25_runtime.h"

static int make_event(struct kn_ax25_connection_event_record *);
static int populate(struct kn_ax25_runtime *);
static int test_counters_format(void);
static int test_frame_format(void);
static int test_list_empty(void);
static int test_list_populated(void);

int
main(void)
{
	if (test_list_empty() != 0)
		return 1;
	if (test_list_populated() != 0)
		return 1;
	if (test_frame_format() != 0)
		return 1;
	if (test_counters_format() != 0)
		return 1;

	return 0;
}

static int
make_event(struct kn_ax25_connection_event_record *event)
{
	struct kn_ax25_connection_key key;

	if (event == NULL)
		return 1;
	if (kn_ax25_connection_key_from_callsigns(&key, "kiss0",
	    "M6VPN-1", "N0CALL", NULL, 0) != KN_AX25_CONNECTION_KEY_OK)
		return 1;
	if (kn_ax25_connection_event_local_connect(event, 1000, &key) !=
	    KN_AX25_CONNECTION_EVENT_OK)
		return 1;
	event->kind = KN_AX25_CONNECTION_EVENT_RX_SABM;
	return 0;
}

static int
populate(struct kn_ax25_runtime *runtime)
{
	struct kn_ax25_connection_event_record event;
	struct kn_ax25_connection_table_result result;

	kn_ax25_runtime_init(runtime);
	(void)kn_ax25_runtime_set_enabled(runtime, 1, 1);
	if (make_event(&event) != 0)
		return 1;
	if (kn_ax25_runtime_inject_event(runtime, &event, &result) !=
	    KN_AX25_RUNTIME_OK)
		return 1;

	return runtime->prepared_queue.count == 1 ? 0 : 1;
}

static int
test_counters_format(void)
{
	struct kn_ax25_runtime runtime;
	char buf[240];

	if (populate(&runtime) != 0)
		return 1;
	if (kn_ax25_prepared_diag_format_counters(&runtime, buf,
	    sizeof(buf)) != KN_AX25_PREPARED_DIAG_OK)
		return 1;

	return strstr(buf, "stored=1") != NULL &&
	    strstr(buf, "tx_writes=0") != NULL ? 0 : 1;
}

static int
test_frame_format(void)
{
	struct kn_ax25_runtime runtime;
	char buf[512];

	if (populate(&runtime) != 0)
		return 1;
	if (kn_ax25_prepared_diag_format_frame(&runtime, 1, buf,
	    sizeof(buf)) != KN_AX25_PREPARED_DIAG_OK)
		return 1;

	return strstr(buf, "hex=") != NULL &&
	    strstr(buf, "needs_fx25=false") != NULL ? 0 : 1;
}

static int
test_list_empty(void)
{
	struct kn_ax25_runtime runtime;
	char buf[128];

	kn_ax25_runtime_init(&runtime);
	if (kn_ax25_prepared_diag_format_list(&runtime, NULL, 0, buf,
	    sizeof(buf)) != KN_AX25_PREPARED_DIAG_OK)
		return 1;

	return strstr(buf, "count=0") != NULL ? 0 : 1;
}

static int
test_list_populated(void)
{
	struct kn_ax25_runtime runtime;
	char buf[512];

	if (populate(&runtime) != 0)
		return 1;
	if (kn_ax25_prepared_diag_format_list(&runtime, "kiss0", 0, buf,
	    sizeof(buf)) != KN_AX25_PREPARED_DIAG_OK)
		return 1;

	return strstr(buf, "AX25 PREPARED id=1") != NULL ? 0 : 1;
}
