/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_ax25_control_plane.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "kilonode/ax25_control.h"
#include "kilonode/ax25_control_plane.h"
#include "kilonode/ax25_runtime.h"
#include "kilonode/control.h"

static int populate_runtime(struct kn_ax25_runtime *);
static int test_connection_detail_response(void);
static int test_counters_response(void);
static int test_empty_connections_response(void);
static int test_invalid_connection_id_rejected(void);
static int test_invalid_port_filter_rejected(void);
static int test_one_connection_response(void);
static int test_output_truncation(void);
static int test_params_response(void);
static int test_port_filtered_connections_response(void);
static int test_status_disabled_response(void);

int
main(void)
{
	if (test_status_disabled_response() != 0)
		return 1;
	if (test_params_response() != 0)
		return 1;
	if (test_empty_connections_response() != 0)
		return 1;
	if (test_one_connection_response() != 0)
		return 1;
	if (test_port_filtered_connections_response() != 0)
		return 1;
	if (test_connection_detail_response() != 0)
		return 1;
	if (test_counters_response() != 0)
		return 1;
	if (test_invalid_port_filter_rejected() != 0)
		return 1;
	if (test_invalid_connection_id_rejected() != 0)
		return 1;
	if (test_output_truncation() != 0)
		return 1;

	return 0;
}

static int
populate_runtime(struct kn_ax25_runtime *runtime)
{
	struct kn_ax25_connection_key key;
	struct kn_ax25_connection_event_record event;
	struct kn_ax25_connection_table_result result;

	kn_ax25_runtime_init(runtime);
	(void)kn_ax25_runtime_set_enabled(runtime, 1, 1);
	if (kn_ax25_connection_key_from_callsigns(&key, "kiss0", "M6VPN-1",
	    "N0CALL", NULL, 0) != KN_AX25_CONNECTION_KEY_OK)
		return 1;
	if (kn_ax25_connection_event_local_connect(&event, 1710000000,
	    &key) != KN_AX25_CONNECTION_EVENT_OK)
		return 1;
	event.kind = KN_AX25_CONNECTION_EVENT_RX_SABM;
	event.control.class = KN_AX25_CONTROL_CLASS_U;
	event.control.u_subtype = KN_AX25_U_SUBTYPE_SABM;
	return kn_ax25_runtime_inject_event(runtime, &event, &result) ==
	    KN_AX25_RUNTIME_OK ? 0 : 1;
}

static int
test_connection_detail_response(void)
{
	struct kn_ax25_runtime runtime;
	char out[KN_CONTROL_QUEUE_MAX];

	if (populate_runtime(&runtime) != 0)
		return 1;
	if (kn_ax25_control_plane_format_connection(&runtime, 1, out,
	    sizeof(out)) != KN_AX25_CONTROL_PLANE_OK)
		return 1;
	if (strstr(out, "OK AX25 CONNECTION id=1\n") == NULL)
		return 1;
	if (strstr(out, "AX25 PLAN index=0 kind=UA") == NULL)
		return 1;

	return strstr(out, "END\n") != NULL ? 0 : 1;
}

static int
test_counters_response(void)
{
	struct kn_ax25_runtime runtime;
	char out[KN_CONTROL_QUEUE_MAX];

	if (populate_runtime(&runtime) != 0)
		return 1;
	if (kn_ax25_control_plane_format_counters(&runtime, out,
	    sizeof(out)) != KN_AX25_CONTROL_PLANE_OK)
		return 1;
	if (strstr(out, "OK AX25 COUNTERS events=1") == NULL)
		return 1;

	return strstr(out, "created=1") != NULL ? 0 : 1;
}

static int
test_empty_connections_response(void)
{
	struct kn_ax25_runtime runtime;
	char out[KN_CONTROL_QUEUE_MAX];

	kn_ax25_runtime_init(&runtime);
	if (kn_ax25_control_plane_format_connections(&runtime, NULL, out,
	    sizeof(out)) != KN_AX25_CONTROL_PLANE_OK)
		return 1;

	return strcmp(out, "OK AX25 CONNECTIONS count=0\nEND\n") == 0 ?
	    0 : 1;
}

static int
test_invalid_connection_id_rejected(void)
{
	struct kn_ax25_runtime runtime;
	char out[KN_CONTROL_QUEUE_MAX];

	kn_ax25_runtime_init(&runtime);
	if (kn_ax25_control_plane_format("CONNECTION bad", &runtime, out,
	    sizeof(out)) != KN_AX25_CONTROL_PLANE_ERR_INVALID_COMMAND)
		return 1;

	return strcmp(out, "ERR invalid-connection-id\n") == 0 ? 0 : 1;
}

static int
test_invalid_port_filter_rejected(void)
{
	struct kn_ax25_runtime runtime;
	char out[KN_CONTROL_QUEUE_MAX];

	kn_ax25_runtime_init(&runtime);
	if (kn_ax25_control_plane_format_connections(&runtime, "bad port",
	    out, sizeof(out)) != KN_AX25_CONTROL_PLANE_ERR_INVALID_COMMAND)
		return 1;

	return strcmp(out, "ERR invalid-port\n") == 0 ? 0 : 1;
}

static int
test_one_connection_response(void)
{
	struct kn_ax25_runtime runtime;
	char out[KN_CONTROL_QUEUE_MAX];

	if (populate_runtime(&runtime) != 0)
		return 1;
	if (kn_ax25_control_plane_format_connections(&runtime, NULL, out,
	    sizeof(out)) != KN_AX25_CONTROL_PLANE_OK)
		return 1;
	if (strstr(out, "OK AX25 CONNECTIONS count=1\n") == NULL)
		return 1;
	if (strstr(out, "port=kiss0 local=M6VPN-1 remote=N0CALL") == NULL)
		return 1;

	return strstr(out, "plans=1") != NULL ? 0 : 1;
}

static int
test_output_truncation(void)
{
	struct kn_ax25_runtime runtime;
	char out[8];

	if (populate_runtime(&runtime) != 0)
		return 1;

	return kn_ax25_control_plane_format_connections(&runtime, NULL, out,
	    sizeof(out)) == KN_AX25_CONTROL_PLANE_ERR_BUFFER ? 0 : 1;
}

static int
test_params_response(void)
{
	struct kn_ax25_runtime runtime;
	char out[KN_CONTROL_QUEUE_MAX];

	kn_ax25_runtime_init(&runtime);
	if (kn_ax25_control_plane_format_params(&runtime, out,
	    sizeof(out)) != KN_AX25_CONTROL_PLANE_OK)
		return 1;
	if (strstr(out, "OK AX25 PARAMS modulo=8 window=1") == NULL)
		return 1;

	return strstr(out, "n2=10\nEND\n") != NULL ? 0 : 1;
}

static int
test_port_filtered_connections_response(void)
{
	struct kn_ax25_runtime runtime;
	char out[KN_CONTROL_QUEUE_MAX];

	if (populate_runtime(&runtime) != 0)
		return 1;
	if (kn_ax25_control_plane_format_connections(&runtime, "kiss1", out,
	    sizeof(out)) != KN_AX25_CONTROL_PLANE_OK)
		return 1;

	return strcmp(out, "OK AX25 CONNECTIONS count=0\nEND\n") == 0 ?
	    0 : 1;
}

static int
test_status_disabled_response(void)
{
	struct kn_ax25_runtime runtime;
	char out[KN_CONTROL_QUEUE_MAX];

	kn_ax25_runtime_init(&runtime);
	if (kn_ax25_control_plane_format_status(&runtime, out,
	    sizeof(out)) != KN_AX25_CONTROL_PLANE_OK)
		return 1;

	return strcmp(out,
	    "OK AX25 STATUS enabled=false connected_mode=false "
	    "connections=0 max_connections=32 diagnostics=true\nEND\n") == 0 ?
	    0 : 1;
}
