/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/proto/ax25_control_plane.c */

#include <sys/types.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kilonode/ax25_control_plane.h"
#include "kilonode/ax25_connection_diag.h"
#include "kilonode/config.h"

static enum kn_ax25_control_plane_error append_format(char *, size_t,
	size_t *, const char *, ...);
static uint64_t record_frame_count(const struct kn_ax25_connection_record *);
static void runtime_default(struct kn_ax25_runtime *);
static const char *state_name(
	const struct kn_ax25_connection_record *);
static uint8_t port_filter_valid(const char *);

static enum kn_ax25_control_plane_error
append_format(char *buf, size_t bufsiz, size_t *offset, const char *fmt, ...)
{
	va_list args;
	int needed;

	if (buf == NULL || offset == NULL || fmt == NULL || bufsiz == 0)
		return KN_AX25_CONTROL_PLANE_ERR_INVALID_ARGUMENT;
	if (*offset >= bufsiz)
		return KN_AX25_CONTROL_PLANE_ERR_BUFFER;

	va_start(args, fmt);
	needed = vsnprintf(buf + *offset, bufsiz - *offset, fmt, args);
	va_end(args);
	if (needed < 0 || (size_t)needed >= bufsiz - *offset)
		return KN_AX25_CONTROL_PLANE_ERR_BUFFER;

	*offset += (size_t)needed;
	return KN_AX25_CONTROL_PLANE_OK;
}

static uint8_t
port_filter_valid(const char *port)
{
	size_t i;
	unsigned char ch;

	if (port == NULL || port[0] == '\0' ||
	    strlen(port) >= KN_CONFIG_PORT_NAME_MAX)
		return 0;

	for (i = 0; port[i] != '\0'; i++) {
		ch = (unsigned char)port[i];
		if ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') ||
		    (ch >= '0' && ch <= '9') || ch == '_' || ch == '-')
			continue;
		return 0;
	}

	return 1;
}

static uint64_t
record_frame_count(const struct kn_ax25_connection_record *record)
{
	if (record == NULL)
		return 0;

	return record->counters.rx_sabm_sabme + record->counters.rx_ua +
	    record->counters.rx_disc + record->counters.rx_dm +
	    record->counters.rx_i + record->counters.rx_s_frames;
}

static void
runtime_default(struct kn_ax25_runtime *runtime)
{
	kn_ax25_runtime_init(runtime);
}

static const char *
state_name(const struct kn_ax25_connection_record *record)
{
	if (record == NULL)
		return "unknown";

	return kn_ax25_connection_state_name(record->connection.state);
}

enum kn_ax25_control_plane_error
kn_ax25_control_plane_format_status(const struct kn_ax25_runtime *runtime,
	char *buf, size_t bufsiz)
{
	struct kn_ax25_runtime fallback;
	const struct kn_ax25_runtime *rt;
	int needed;

	if (buf == NULL || bufsiz == 0)
		return KN_AX25_CONTROL_PLANE_ERR_INVALID_ARGUMENT;

	rt = runtime;
	if (rt == NULL) {
		runtime_default(&fallback);
		rt = &fallback;
	}
	needed = snprintf(buf, bufsiz,
	    "OK AX25 STATUS enabled=%s connected_mode=%s connections=%llu "
	    "max_connections=%llu diagnostics=%s\nEND\n",
	    rt->enabled != 0 ? "true" : "false",
	    rt->connected_mode_enabled != 0 ? "true" : "false",
	    (unsigned long long)kn_ax25_runtime_connection_count(rt),
	    (unsigned long long)rt->max_connections,
	    rt->diagnostics_enabled != 0 ? "true" : "false");
	if (needed < 0 || (size_t)needed >= bufsiz)
		return KN_AX25_CONTROL_PLANE_ERR_BUFFER;

	return KN_AX25_CONTROL_PLANE_OK;
}

enum kn_ax25_control_plane_error
kn_ax25_control_plane_format_params(const struct kn_ax25_runtime *runtime,
	char *buf, size_t bufsiz)
{
	struct kn_ax25_runtime fallback;
	const struct kn_ax25_runtime *rt;
	const char *modulo;
	int needed;

	if (buf == NULL || bufsiz == 0)
		return KN_AX25_CONTROL_PLANE_ERR_INVALID_ARGUMENT;

	rt = runtime;
	if (rt == NULL) {
		runtime_default(&fallback);
		rt = &fallback;
	}
	modulo = rt->params.modulo_mode == KN_AX25_MODULO_8 ? "8" : "128";
	needed = snprintf(buf, bufsiz,
	    "OK AX25 PARAMS modulo=%s window=%u t1_ms=%u t2_ms=%u "
	    "t3_ms=%u n2=%u\nEND\n",
	    modulo, (unsigned int)rt->params.window_size,
	    (unsigned int)rt->params.t1_ms,
	    (unsigned int)rt->params.t2_ms,
	    (unsigned int)rt->params.t3_ms,
	    (unsigned int)rt->params.n2_retry_count);
	if (needed < 0 || (size_t)needed >= bufsiz)
		return KN_AX25_CONTROL_PLANE_ERR_BUFFER;

	return KN_AX25_CONTROL_PLANE_OK;
}

enum kn_ax25_control_plane_error
kn_ax25_control_plane_format_connections(const struct kn_ax25_runtime *runtime,
	const char *port_filter, char *buf, size_t bufsiz)
{
	struct kn_ax25_runtime fallback;
	const struct kn_ax25_runtime *rt;
	const struct kn_ax25_connection_record *record;
	char local[KN_CALLSIGN_MAX + 4];
	char remote[KN_CALLSIGN_MAX + 4];
	size_t i;
	size_t count;
	size_t offset;
	enum kn_ax25_control_plane_error rc;

	if (buf == NULL || bufsiz == 0)
		return KN_AX25_CONTROL_PLANE_ERR_INVALID_ARGUMENT;
	if (port_filter != NULL && port_filter_valid(port_filter) == 0) {
		(void)snprintf(buf, bufsiz, "ERR invalid-port\n");
		return KN_AX25_CONTROL_PLANE_ERR_INVALID_COMMAND;
	}

	rt = runtime;
	if (rt == NULL) {
		runtime_default(&fallback);
		rt = &fallback;
	}

	count = 0;
	for (i = 0; i < rt->table.count; i++) {
		record = kn_ax25_runtime_get_connection(rt, i);
		if (record == NULL)
			continue;
		if (port_filter != NULL &&
		    strcmp(record->key.port_name, port_filter) != 0)
			continue;
		count++;
	}

	offset = 0;
	rc = append_format(buf, bufsiz, &offset,
	    "OK AX25 CONNECTIONS count=%llu\n", (unsigned long long)count);
	if (rc != KN_AX25_CONTROL_PLANE_OK)
		return rc;
	for (i = 0; i < rt->table.count; i++) {
		record = kn_ax25_runtime_get_connection(rt, i);
		if (record == NULL)
			continue;
		if (port_filter != NULL &&
		    strcmp(record->key.port_name, port_filter) != 0)
			continue;
		if (kn_callsign_format(&record->key.local, local,
		    sizeof(local)) != 0 ||
		    kn_callsign_format(&record->key.remote, remote,
		    sizeof(remote)) != 0)
			return KN_AX25_CONTROL_PLANE_ERR_BUFFER;
		rc = append_format(buf, bufsiz, &offset,
		    "AX25 CONN id=%llu port=%s local=%s remote=%s "
		    "state=%s frames=%llu plans=%llu last=%s\n",
		    (unsigned long long)(i + 1U), record->key.port_name,
		    local, remote, state_name(record),
		    (unsigned long long)record_frame_count(record),
		    (unsigned long long)record->last_plans.count,
		    kn_ax25_connection_event_name(record->last_event_kind));
		if (rc != KN_AX25_CONTROL_PLANE_OK)
			return rc;
	}

	return append_format(buf, bufsiz, &offset, "END\n");
}

enum kn_ax25_control_plane_error
kn_ax25_control_plane_format_connection(const struct kn_ax25_runtime *runtime,
	size_t id, char *buf, size_t bufsiz)
{
	struct kn_ax25_runtime fallback;
	const struct kn_ax25_runtime *rt;
	const struct kn_ax25_connection_record *record;
	const struct kn_ax25_frame_plan *plan;
	char local[KN_CALLSIGN_MAX + 4];
	char remote[KN_CALLSIGN_MAX + 4];
	char source[KN_CALLSIGN_MAX + 4];
	char dest[KN_CALLSIGN_MAX + 4];
	size_t i;
	size_t offset;
	enum kn_ax25_control_plane_error rc;

	if (buf == NULL || bufsiz == 0 || id == 0)
		return KN_AX25_CONTROL_PLANE_ERR_INVALID_ARGUMENT;

	rt = runtime;
	if (rt == NULL) {
		runtime_default(&fallback);
		rt = &fallback;
	}
	record = kn_ax25_runtime_get_connection(rt, id - 1U);
	if (record == NULL) {
		(void)snprintf(buf, bufsiz, "ERR connection-not-found\n");
		return KN_AX25_CONTROL_PLANE_ERR_NOT_FOUND;
	}
	if (kn_callsign_format(&record->key.local, local, sizeof(local)) != 0 ||
	    kn_callsign_format(&record->key.remote, remote,
	    sizeof(remote)) != 0)
		return KN_AX25_CONTROL_PLANE_ERR_BUFFER;

	offset = 0;
	rc = append_format(buf, bufsiz, &offset,
	    "OK AX25 CONNECTION id=%llu\n",
	    (unsigned long long)id);
	if (rc != KN_AX25_CONTROL_PLANE_OK)
		return rc;
	rc = append_format(buf, bufsiz, &offset,
	    "AX25 CONN id=%llu port=%s local=%s remote=%s state=%s "
	    "created=%llu last=%llu rx_sabm=%llu rx_i=%llu rx_s=%llu "
	    "protocol_errors=%llu plans=%llu\n",
	    (unsigned long long)id, record->key.port_name, local, remote,
	    state_name(record), (unsigned long long)record->created,
	    (unsigned long long)record->last_event,
	    (unsigned long long)record->counters.rx_sabm_sabme,
	    (unsigned long long)record->counters.rx_i,
	    (unsigned long long)record->counters.rx_s_frames,
	    (unsigned long long)record->counters.protocol_errors,
	    (unsigned long long)record->last_plans.count);
	if (rc != KN_AX25_CONTROL_PLANE_OK)
		return rc;
	for (i = 0; i < record->last_plans.count; i++) {
		plan = &record->last_plans.plans[i];
		if (kn_callsign_format(&plan->source, source,
		    sizeof(source)) != 0 ||
		    kn_callsign_format(&plan->destination, dest,
		    sizeof(dest)) != 0)
			return KN_AX25_CONTROL_PLANE_ERR_BUFFER;
		rc = append_format(buf, bufsiz, &offset,
		    "AX25 PLAN index=%llu kind=%s source=%s dest=%s\n",
		    (unsigned long long)i,
		    kn_ax25_frame_plan_type_name(plan->type), source, dest);
		if (rc != KN_AX25_CONTROL_PLANE_OK)
			return rc;
	}

	return append_format(buf, bufsiz, &offset, "END\n");
}

enum kn_ax25_control_plane_error
kn_ax25_control_plane_format_counters(const struct kn_ax25_runtime *runtime,
	char *buf, size_t bufsiz)
{
	struct kn_ax25_runtime fallback;
	const struct kn_ax25_runtime *rt;
	int needed;

	if (buf == NULL || bufsiz == 0)
		return KN_AX25_CONTROL_PLANE_ERR_INVALID_ARGUMENT;

	rt = runtime;
	if (rt == NULL) {
		runtime_default(&fallback);
		rt = &fallback;
	}
	needed = snprintf(buf, bufsiz,
	    "OK AX25 COUNTERS events=%llu rejected=%llu created=%llu "
	    "removed=%llu plans=%llu unsupported=%llu errors=%llu\nEND\n",
	    (unsigned long long)rt->counters.events_accepted,
	    (unsigned long long)rt->counters.events_rejected,
	    (unsigned long long)rt->counters.connections_created,
	    (unsigned long long)rt->counters.connections_removed,
	    (unsigned long long)rt->counters.frame_plans_generated,
	    (unsigned long long)rt->counters.unsupported_frame_events,
	    (unsigned long long)rt->counters.protocol_errors);
	if (needed < 0 || (size_t)needed >= bufsiz)
		return KN_AX25_CONTROL_PLANE_ERR_BUFFER;

	return KN_AX25_CONTROL_PLANE_OK;
}

enum kn_ax25_control_plane_error
kn_ax25_control_plane_format(const char *command,
	const struct kn_ax25_runtime *runtime, char *buf, size_t bufsiz)
{
	char *end;
	unsigned long id;

	if (command == NULL || buf == NULL || bufsiz == 0)
		return KN_AX25_CONTROL_PLANE_ERR_INVALID_ARGUMENT;

	if (strcmp(command, "STATUS") == 0)
		return kn_ax25_control_plane_format_status(runtime, buf,
		    bufsiz);
	if (strcmp(command, "PARAMS") == 0)
		return kn_ax25_control_plane_format_params(runtime, buf,
		    bufsiz);
	if (strcmp(command, "CONNECTIONS") == 0)
		return kn_ax25_control_plane_format_connections(runtime, NULL,
		    buf, bufsiz);
	if (strncmp(command, "CONNECTIONS PORT ", 17) == 0)
		return kn_ax25_control_plane_format_connections(runtime,
		    command + 17, buf, bufsiz);
	if (strncmp(command, "CONNECTION ", 11) == 0) {
		id = strtoul(command + 11, &end, 10);
		if (*end != '\0' || id == 0) {
			(void)snprintf(buf, bufsiz,
			    "ERR invalid-connection-id\n");
			return KN_AX25_CONTROL_PLANE_ERR_INVALID_COMMAND;
		}
		return kn_ax25_control_plane_format_connection(runtime,
		    (size_t)id, buf, bufsiz);
	}
	if (strcmp(command, "COUNTERS") == 0)
		return kn_ax25_control_plane_format_counters(runtime, buf,
		    bufsiz);

	(void)snprintf(buf, bufsiz, "ERR invalid-ax25-command\n");
	return KN_AX25_CONTROL_PLANE_ERR_INVALID_COMMAND;
}
