/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/control/control_protocol.c */

#include <sys/types.h>

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "kilonode/bbs_control.h"
#include "kilonode/callsign.h"
#include "kilonode/control.h"
#include "kilonode/heard.h"
#include "kilonode/rx_event.h"
#include "kilonode/rx_queue.h"
#include "kilonode/rx_session.h"
#include "kilonode/stats.h"
#include "kilonode/tx_frame.h"
#include "kilonode/tx_dry_run.h"
#include "kilonode/tx_queue.h"

#define KILONODE_VERSION "0.1.0"

static enum kn_control_error append_format(char *, size_t, size_t *,
	const char *, ...);
static enum kn_control_error cap_response_lines(char *, size_t, size_t);
static uint8_t command_clean(const char *);
static void format_digipeaters(const struct kn_heard_entry *, char *, size_t);
static enum kn_control_error format_bbs(const struct kn_control_snapshot *,
	const char *, char *, size_t);
static enum kn_control_error format_heard(const struct kn_control_snapshot *,
	const char *, char *, size_t);
static enum kn_control_error format_help(char *, size_t);
static enum kn_control_error format_ports(const struct kn_control_snapshot *,
	char *, size_t);
static enum kn_control_error format_rx(const struct kn_control_snapshot *,
	const char *, char *, size_t);
static enum kn_control_error format_stats(const struct kn_control_snapshot *,
	char *, size_t);
static enum kn_control_error format_status(const struct kn_control_snapshot *,
	char *, size_t);
static enum kn_control_error format_tx(const struct kn_control_snapshot *,
	const char *, char *, size_t);
static enum kn_control_error format_tx_dryrun_ui(
	const struct kn_control_snapshot *, const char *, char *, size_t);
static enum kn_control_error read_word(const char **, char *, size_t);
static enum kn_control_error return_with_cap(
	const struct kn_control_snapshot *, char *, size_t,
	enum kn_control_error);

static enum kn_control_error
append_format(char *buf, size_t bufsiz, size_t *offset, const char *fmt, ...)
{
	va_list args;
	int needed;

	if (*offset >= bufsiz)
		return KN_CONTROL_ERR_IO;

	va_start(args, fmt);
	needed = vsnprintf(buf + *offset, bufsiz - *offset, fmt, args);
	va_end(args);

	if (needed < 0 || (size_t)needed >= bufsiz - *offset)
		return KN_CONTROL_ERR_IO;

	*offset += (size_t)needed;
	return KN_CONTROL_OK;
}

static enum kn_control_error
cap_response_lines(char *buf, size_t bufsiz, size_t max_lines)
{
	size_t i;
	size_t lines;

	if (max_lines == 0)
		return KN_CONTROL_OK;
	lines = 0;
	for (i = 0; buf[i] != '\0'; i++) {
		if (buf[i] == '\n')
			lines++;
	}
	if (lines <= max_lines)
		return KN_CONTROL_OK;
	if (snprintf(buf, bufsiz, "ERR response-line-limit\n") < 0 ||
	    strlen("ERR response-line-limit\n") >= bufsiz)
		return KN_CONTROL_ERR_IO;
	return KN_CONTROL_ERR_UNKNOWN_COMMAND;
}

static uint8_t
command_clean(const char *command)
{
	size_t i;

	for (i = 0; command[i] != '\0'; i++) {
		if ((unsigned char)command[i] < 0x20 ||
		    (unsigned char)command[i] > 0x7e)
			return 0;
	}

	return 1;
}

static enum kn_control_error
return_with_cap(const struct kn_control_snapshot *snapshot, char *out,
	size_t out_len, enum kn_control_error rc)
{
	if (rc != KN_CONTROL_OK)
		return rc;
	return cap_response_lines(out, out_len,
	    snapshot->control_max_response_lines);
}

static void
format_digipeaters(const struct kn_heard_entry *entry, char *buf,
	size_t bufsiz)
{
	char call[KN_CALLSIGN_MAX + 4];
	size_t i;
	size_t offset;
	int needed;

	if (bufsiz == 0)
		return;

	if (entry->digipeater_count == 0) {
		(void)snprintf(buf, bufsiz, "-");
		return;
	}

	offset = 0;
	buf[0] = '\0';
	for (i = 0; i < entry->digipeater_count; i++) {
		if (kn_heard_format_callsign(&entry->digipeaters[i].callsign,
		    call, sizeof(call)) != 0)
			(void)snprintf(call, sizeof(call), "-");
		needed = snprintf(buf + offset, bufsiz - offset, "%s%s",
		    i == 0 ? "" : ",", call);
		if (needed < 0 || (size_t)needed >= bufsiz - offset) {
			buf[bufsiz - 1] = '\0';
			return;
		}
		offset += (size_t)needed;
	}
}

static enum kn_control_error
format_bbs(const struct kn_control_snapshot *snapshot, const char *command,
	char *buf, size_t bufsiz)
{
	enum kn_bbs_control_error rc;

	rc = kn_bbs_control_format(command, snapshot->bbs_enabled,
	    snapshot->bbs_store, buf, bufsiz);
	return rc == KN_BBS_CONTROL_OK ? KN_CONTROL_OK :
	    KN_CONTROL_ERR_UNKNOWN_COMMAND;
}

static enum kn_control_error
format_heard(const struct kn_control_snapshot *snapshot, const char *port_name,
	char *buf, size_t bufsiz)
{
	char call[KN_CALLSIGN_MAX + 4];
	char dest[KN_CALLSIGN_MAX + 4];
	char path[128];
	char pid[16];
	const struct kn_heard_entry *entry;
	size_t count;
	size_t i;
	size_t offset;
	enum kn_control_error rc;

	count = 0;
	for (i = 0; i < snapshot->heard_count; i++) {
		entry = &snapshot->heard[i];
		if (port_name == NULL || strcmp(entry->port_name, port_name) == 0)
			count++;
	}

	offset = 0;
	rc = append_format(buf, bufsiz, &offset, "OK HEARD count=%llu\n",
	    (unsigned long long)count);
	if (rc != KN_CONTROL_OK)
		return rc;

	for (i = 0; i < snapshot->heard_count; i++) {
		entry = &snapshot->heard[i];
		if (port_name != NULL && strcmp(entry->port_name, port_name) != 0)
			continue;
		if (kn_heard_format_callsign(&entry->source, call,
		    sizeof(call)) != 0)
			(void)snprintf(call, sizeof(call), "-");
		if (kn_heard_format_callsign(&entry->last_destination, dest,
		    sizeof(dest)) != 0)
			(void)snprintf(dest, sizeof(dest), "-");
		format_digipeaters(entry, path, sizeof(path));
		if (entry->has_pid != 0)
			(void)snprintf(pid, sizeof(pid), "0x%02x",
			    (unsigned int)entry->last_pid);
		else
			(void)snprintf(pid, sizeof(pid), "none");
		rc = append_format(buf, bufsiz, &offset,
		    "HEARD port=%s call=%s last_dest=%s frames=%llu "
		    "via=%s last_ui=%s last_pid=%s last_payload=%llu "
		    "first=%llu last=%llu\n",
		    entry->port_name, call, dest,
		    (unsigned long long)entry->frame_count,
		    path,
		    entry->last_ui != 0 ? "true" : "false",
		    pid,
		    (unsigned long long)entry->last_payload_len,
		    (unsigned long long)entry->first_heard,
		    (unsigned long long)entry->last_heard);
		if (rc != KN_CONTROL_OK)
			return rc;
	}

	return append_format(buf, bufsiz, &offset, "END\n");
}

static enum kn_control_error
format_help(char *buf, size_t bufsiz)
{
	size_t offset;
	enum kn_control_error rc;

	offset = 0;
	rc = append_format(buf, bufsiz, &offset,
	    "OK HELP PING VERSION STATUS PORTS STATS HEARD BBS RX TX HELP "
	    "QUIT\n");
	if (rc != KN_CONTROL_OK)
		return rc;
	return append_format(buf, bufsiz, &offset, "END\n");
}

static enum kn_control_error
format_ports(const struct kn_control_snapshot *snapshot, char *buf,
	size_t bufsiz)
{
	size_t i;
	size_t offset;
	const struct kn_port_stats *port;
	enum kn_control_error rc;

	offset = 0;

	for (i = 0; i < snapshot->port_count; i++) {
		port = &snapshot->ports[i];
		rc = append_format(buf, bufsiz, &offset,
		    "OK PORT name=%s type=%s enabled=%s open=%s "
		    "rx_bytes=%llu frames=%llu ax25=%llu malformed=%llu "
		    "last_error=%s last_frame=%llu\n",
		    port->name, kn_stats_port_type_name(port->type),
		    port->enabled != 0 ? "true" : "false",
		    port->open != 0 ? "true" : "false",
		    (unsigned long long)port->bytes_received,
		    (unsigned long long)port->kiss_frames_received,
		    (unsigned long long)port->ax25_frames_decoded,
		    (unsigned long long)port->malformed_frames,
		    port->last_error[0] == '\0' ? "none" : port->last_error,
		    (unsigned long long)port->last_frame_time);
		if (rc != KN_CONTROL_OK)
			return rc;
	}

	return append_format(buf, bufsiz, &offset, "END\n");
}

static enum kn_control_error
format_rx_event_list(const struct kn_rx_event **events, size_t count,
	char *buf, size_t bufsiz)
{
	char line[KN_CONTROL_LINE_MAX];
	size_t i;
	size_t offset;
	enum kn_control_error rc;

	offset = 0;
	rc = append_format(buf, bufsiz, &offset, "OK RX EVENTS count=%llu\n",
	    (unsigned long long)count);
	if (rc != KN_CONTROL_OK)
		return rc;

	for (i = 0; i < count; i++) {
		if (kn_rx_event_format_brief(events[i], line,
		    sizeof(line)) != KN_RX_EVENT_OK)
			return KN_CONTROL_ERR_IO;
		rc = append_format(buf, bufsiz, &offset, "%s\n", line);
		if (rc != KN_CONTROL_OK)
			return rc;
	}

	return append_format(buf, bufsiz, &offset, "END\n");
}

static enum kn_control_error
format_rx_sessions(const struct kn_rx_session_entry **entries, size_t count,
	char *buf, size_t bufsiz)
{
	char source[KN_CALLSIGN_MAX + 4];
	char destination[KN_CALLSIGN_MAX + 4];
	size_t i;
	size_t offset;
	enum kn_control_error rc;

	offset = 0;
	rc = append_format(buf, bufsiz, &offset,
	    "OK RX SESSIONS count=%llu\n", (unsigned long long)count);
	if (rc != KN_CONTROL_OK)
		return rc;

	for (i = 0; i < count; i++) {
		if (kn_callsign_format(&entries[i]->source, source,
		    sizeof(source)) != 0 ||
		    kn_callsign_format(&entries[i]->destination, destination,
		    sizeof(destination)) != 0)
			return KN_CONTROL_ERR_IO;
		rc = append_format(buf, bufsiz, &offset,
		    "RX SESSION port=%s from=%s to=%s frames=%llu "
		    "ui=%llu i=%llu s=%llu u=%llu malformed=%llu "
		    "first=%llu last=%llu last_event=%llu\n",
		    entries[i]->port_name, source, destination,
		    (unsigned long long)entries[i]->frame_count,
		    (unsigned long long)entries[i]->ui_count,
		    (unsigned long long)entries[i]->i_count,
		    (unsigned long long)entries[i]->s_count,
		    (unsigned long long)entries[i]->u_count,
		    (unsigned long long)entries[i]->malformed_count,
		    (unsigned long long)entries[i]->first_seen,
		    (unsigned long long)entries[i]->last_seen,
		    (unsigned long long)entries[i]->last_event_id);
		if (rc != KN_CONTROL_OK)
			return rc;
	}

	return append_format(buf, bufsiz, &offset, "END\n");
}

static enum kn_control_error
format_rx(const struct kn_control_snapshot *snapshot, const char *command,
	char *buf, size_t bufsiz)
{
	const struct kn_rx_event *events[KN_RX_QUEUE_MAX];
	const struct kn_rx_session_entry *sessions[KN_RX_SESSION_MAX];
	const struct kn_rx_event *event;
	struct kn_callsign callsign;
	char line[KN_CONTROL_LINE_MAX];
	char *end;
	unsigned long id;
	unsigned long limit;
	size_t count;
	size_t i;

	if (snapshot->rx_enabled == 0 || snapshot->rx_events == NULL ||
	    snapshot->rx_sessions == NULL) {
		(void)snprintf(buf, bufsiz, "ERR rx-disabled\n");
		return KN_CONTROL_ERR_UNKNOWN_COMMAND;
	}

	if (strcmp(command, "STATUS") == 0) {
		if (snprintf(buf, bufsiz,
		    "OK RX STATUS events_enabled=true events=%llu "
		    "max_events=%llu sessions=%llu max_sessions=%llu "
		    "preview_bytes=%llu\nEND\n",
		    (unsigned long long)kn_rx_queue_count(snapshot->rx_events),
		    (unsigned long long)snapshot->rx_events->max_events,
		    (unsigned long long)kn_rx_session_count(
		    snapshot->rx_sessions),
		    (unsigned long long)snapshot->rx_sessions->max_sessions,
		    (unsigned long long)snapshot->rx_events->preview_bytes) >=
		    (int)bufsiz)
			return KN_CONTROL_ERR_IO;
		return KN_CONTROL_OK;
	}

	if (strcmp(command, "EVENTS") == 0) {
		if (kn_rx_queue_list(snapshot->rx_events, events, 100,
		    &count) != KN_RX_QUEUE_OK)
			return KN_CONTROL_ERR_IO;
		return format_rx_event_list(events, count, buf, bufsiz);
	}

	if (strncmp(command, "EVENTS LIMIT ", 13) == 0) {
		limit = strtoul(command + 13, &end, 10);
		if (*end != '\0' || limit == 0 || limit > 100) {
			(void)snprintf(buf, bufsiz, "ERR invalid-rx-command\n");
			return KN_CONTROL_ERR_UNKNOWN_COMMAND;
		}
		if (kn_rx_queue_list(snapshot->rx_events, events,
		    (size_t)limit, &count) != KN_RX_QUEUE_OK)
			return KN_CONTROL_ERR_IO;
		return format_rx_event_list(events, count, buf, bufsiz);
	}

	if (strncmp(command, "EVENTS PORT ", 12) == 0) {
		if (command[12] == '\0' ||
		    strlen(command + 12) >= KN_CONFIG_PORT_NAME_MAX ||
		    strchr(command + 12, ' ') != NULL) {
			(void)snprintf(buf, bufsiz, "ERR invalid-port\n");
			return KN_CONTROL_ERR_UNKNOWN_COMMAND;
		}
		if (kn_rx_queue_list_by_port(snapshot->rx_events,
		    command + 12, events, 100, &count) != KN_RX_QUEUE_OK)
			return KN_CONTROL_ERR_IO;
		return format_rx_event_list(events, count, buf, bufsiz);
	}

	if (strncmp(command, "EVENTS FROM ", 12) == 0 ||
	    strncmp(command, "EVENTS TO ", 10) == 0) {
		const char *value;

		value = strncmp(command, "EVENTS FROM ", 12) == 0 ?
		    command + 12 : command + 10;
		if (kn_callsign_parse(value, &callsign) != 0) {
			(void)snprintf(buf, bufsiz, "ERR invalid-callsign\n");
			return KN_CONTROL_ERR_UNKNOWN_COMMAND;
		}
		if (strncmp(command, "EVENTS FROM ", 12) == 0) {
			if (kn_rx_queue_list_by_source(snapshot->rx_events,
			    &callsign, events, 100, &count) != KN_RX_QUEUE_OK)
				return KN_CONTROL_ERR_IO;
		} else {
			if (kn_rx_queue_list_by_destination(snapshot->rx_events,
			    &callsign, events, 100, &count) !=
			    KN_RX_QUEUE_OK)
				return KN_CONTROL_ERR_IO;
		}
		return format_rx_event_list(events, count, buf, bufsiz);
	}

	if (strncmp(command, "EVENT ", 6) == 0) {
		id = strtoul(command + 6, &end, 10);
		if (*end != '\0' || id == 0) {
			(void)snprintf(buf, bufsiz, "ERR invalid-event-id\n");
			return KN_CONTROL_ERR_UNKNOWN_COMMAND;
		}
		event = kn_rx_queue_get(snapshot->rx_events, (uint64_t)id);
		if (event == NULL) {
			(void)snprintf(buf, bufsiz, "ERR event-not-found\n");
			return KN_CONTROL_ERR_UNKNOWN_COMMAND;
		}
		if (kn_rx_event_format_full(event, line, sizeof(line)) !=
		    KN_RX_EVENT_OK)
			return KN_CONTROL_ERR_IO;
		if (snprintf(buf, bufsiz, "OK RX EVENT id=%llu\n%s\nEND\n",
		    (unsigned long long)event->id, line) >= (int)bufsiz)
			return KN_CONTROL_ERR_IO;
		return KN_CONTROL_OK;
	}

	if (strcmp(command, "SESSIONS") == 0) {
		count = kn_rx_session_count(snapshot->rx_sessions);
		if (count > KN_RX_SESSION_MAX)
			count = KN_RX_SESSION_MAX;
		for (i = 0; i < count; i++)
			sessions[i] = &kn_rx_session_entries(
			    snapshot->rx_sessions)[i];
		return format_rx_sessions(sessions, count, buf, bufsiz);
	}

	if (strncmp(command, "SESSIONS PORT ", 14) == 0) {
		if (command[14] == '\0' ||
		    strlen(command + 14) >= KN_CONFIG_PORT_NAME_MAX ||
		    strchr(command + 14, ' ') != NULL) {
			(void)snprintf(buf, bufsiz, "ERR invalid-port\n");
			return KN_CONTROL_ERR_UNKNOWN_COMMAND;
		}
		if (kn_rx_session_list_by_port(snapshot->rx_sessions,
		    command + 14, sessions, 100, &count) !=
		    KN_RX_SESSION_OK)
			return KN_CONTROL_ERR_IO;
		return format_rx_sessions(sessions, count, buf, bufsiz);
	}

	if (strncmp(command, "SESSIONS FROM ", 14) == 0) {
		if (kn_callsign_parse(command + 14, &callsign) != 0) {
			(void)snprintf(buf, bufsiz, "ERR invalid-callsign\n");
			return KN_CONTROL_ERR_UNKNOWN_COMMAND;
		}
		if (kn_rx_session_list_by_source(snapshot->rx_sessions,
		    &callsign, sessions, 100, &count) != KN_RX_SESSION_OK)
			return KN_CONTROL_ERR_IO;
		return format_rx_sessions(sessions, count, buf, bufsiz);
	}

	(void)snprintf(buf, bufsiz, "ERR invalid-rx-command\n");
	return KN_CONTROL_ERR_UNKNOWN_COMMAND;
}

static enum kn_control_error
format_tx_frame_list(const struct kn_tx_frame **frames, size_t count,
	char *buf, size_t bufsiz)
{
	char line[KN_CONTROL_LINE_MAX];
	size_t i;
	size_t offset;
	enum kn_control_error rc;

	offset = 0;
	rc = append_format(buf, bufsiz, &offset, "OK TX QUEUE count=%llu\n",
	    (unsigned long long)count);
	if (rc != KN_CONTROL_OK)
		return rc;

	for (i = 0; i < count; i++) {
		if (kn_tx_frame_format_brief(frames[i], line,
		    sizeof(line)) != KN_TX_FRAME_OK)
			return KN_CONTROL_ERR_IO;
		rc = append_format(buf, bufsiz, &offset, "%s\n", line);
		if (rc != KN_CONTROL_OK)
			return rc;
	}

	return append_format(buf, bufsiz, &offset, "END\n");
}

static enum kn_control_error
format_tx(const struct kn_control_snapshot *snapshot, const char *command,
	char *buf, size_t bufsiz)
{
	const struct kn_tx_frame *frames[100];
	const struct kn_tx_frame *frame;
	char line[KN_CONTROL_LINE_MAX];
	char *end;
	unsigned long id;
	size_t count;

	if (snapshot->tx_queue == NULL) {
		(void)snprintf(buf, bufsiz, "ERR tx-unavailable\n");
		return KN_CONTROL_ERR_UNKNOWN_COMMAND;
	}

	if (strncmp(command, "DRYRUN UI ", 10) == 0)
		return format_tx_dryrun_ui(snapshot, command + 10, buf,
		    bufsiz);

	if (strcmp(command, "STATUS") == 0) {
		if (snprintf(buf, bufsiz,
		    "OK TX STATUS enabled=%s dry_run=%s allow_ui=%s "
		    "allow_control_enqueue=%s allow_shell_enqueue=%s "
		    "queued=%llu max_queued=%llu max_payload=%llu\nEND\n",
		    snapshot->tx_queue->policy.enabled != 0 ? "true" :
		    "false",
		    snapshot->tx_queue->policy.dry_run != 0 ? "true" :
		    "false",
		    snapshot->tx_queue->policy.allow_ui != 0 ? "true" :
		    "false",
		    snapshot->tx_queue->policy.allow_control_enqueue != 0 ?
		    "true" : "false",
		    snapshot->tx_queue->policy.allow_shell_enqueue != 0 ?
		    "true" : "false",
		    (unsigned long long)kn_tx_queue_count(
		    snapshot->tx_queue),
		    (unsigned long long)snapshot->tx_queue->max_frames,
		    (unsigned long long)
		    snapshot->tx_queue->policy.max_payload_bytes) >=
		    (int)bufsiz)
			return KN_CONTROL_ERR_IO;
		return KN_CONTROL_OK;
	}

	if (strcmp(command, "QUEUE") == 0) {
		if (kn_tx_queue_list(snapshot->tx_queue, frames, 100,
		    &count) != KN_TX_QUEUE_OK)
			return KN_CONTROL_ERR_IO;
		return format_tx_frame_list(frames, count, buf, bufsiz);
	}

	if (strncmp(command, "QUEUE PORT ", 11) == 0) {
		if (command[11] == '\0' ||
		    strlen(command + 11) >= KN_CONFIG_PORT_NAME_MAX ||
		    strchr(command + 11, ' ') != NULL) {
			(void)snprintf(buf, bufsiz, "ERR invalid-port\n");
			return KN_CONTROL_ERR_UNKNOWN_COMMAND;
		}
		if (kn_tx_queue_list_by_port(snapshot->tx_queue,
		    command + 11, frames, 100, &count) != KN_TX_QUEUE_OK)
			return KN_CONTROL_ERR_IO;
		return format_tx_frame_list(frames, count, buf, bufsiz);
	}

	if (strncmp(command, "FRAME ", 6) == 0) {
		id = strtoul(command + 6, &end, 10);
		if (*end != '\0' || id == 0) {
			(void)snprintf(buf, bufsiz, "ERR invalid-frame-id\n");
			return KN_CONTROL_ERR_UNKNOWN_COMMAND;
		}
		frame = kn_tx_queue_get(snapshot->tx_queue, (uint64_t)id);
		if (frame == NULL) {
			(void)snprintf(buf, bufsiz, "ERR frame-not-found\n");
			return KN_CONTROL_ERR_UNKNOWN_COMMAND;
		}
		if (kn_tx_frame_format_full(frame, line, sizeof(line)) !=
		    KN_TX_FRAME_OK)
			return KN_CONTROL_ERR_IO;
		if (snprintf(buf, bufsiz, "OK TX FRAME id=%llu\n%s\nEND\n",
		    (unsigned long long)frame->id, line) >= (int)bufsiz)
			return KN_CONTROL_ERR_IO;
		return KN_CONTROL_OK;
	}

	(void)snprintf(buf, bufsiz, "ERR invalid-tx-command\n");
	return KN_CONTROL_ERR_UNKNOWN_COMMAND;
}

static enum kn_control_error
format_tx_dryrun_ui(const struct kn_control_snapshot *snapshot,
	const char *command, char *buf, size_t bufsiz)
{
	char port[KN_CONFIG_PORT_NAME_MAX];
	char source[KN_CALLSIGN_MAX + 4];
	char destination[KN_CALLSIGN_MAX + 4];
	char via[KN_TX_FRAME_PATH_MAX];
	const char *p;
	const char *payload;
	const char *via_arg;
	const struct kn_tx_frame *frame;
	uint64_t id;
	enum kn_tx_dry_run_error tx_rc;

	p = command;
	via[0] = '\0';
	via_arg = NULL;

	if (strncmp(p, "PORT ", 5) != 0)
		goto invalid;
	p += 5;
	if (read_word(&p, port, sizeof(port)) != KN_CONTROL_OK)
		goto invalid;
	if (strncmp(p, " FROM ", 6) != 0)
		goto invalid;
	p += 6;
	if (read_word(&p, source, sizeof(source)) != KN_CONTROL_OK)
		goto invalid;
	if (strncmp(p, " TO ", 4) != 0)
		goto invalid;
	p += 4;
	if (read_word(&p, destination, sizeof(destination)) != KN_CONTROL_OK)
		goto invalid;
	if (strncmp(p, " VIA ", 5) == 0) {
		p += 5;
		if (read_word(&p, via, sizeof(via)) != KN_CONTROL_OK)
			goto invalid;
		via_arg = via;
	}
	if (strncmp(p, " TEXT ", 6) != 0)
		goto invalid;
	payload = p + 6;
	if (payload[0] == '\0')
		goto invalid;

	tx_rc = kn_tx_dry_run_enqueue_ui(snapshot->tx_queue, snapshot->ports,
	    snapshot->port_count, KN_TX_DRY_RUN_ORIGIN_CONTROL,
	    (uint64_t)time(NULL), port, source, destination, via_arg,
	    KN_AX25_PID_NO_LAYER_3, (const uint8_t *)payload,
	    strlen(payload), &id);
	if (tx_rc != KN_TX_DRY_RUN_OK) {
		if (snprintf(buf, bufsiz, "ERR %s\n",
		    kn_tx_dry_run_error_name(tx_rc)) >= (int)bufsiz)
			return KN_CONTROL_ERR_IO;
		return KN_CONTROL_ERR_UNKNOWN_COMMAND;
	}

	frame = kn_tx_queue_get(snapshot->tx_queue, id);
	if (frame == NULL)
		return KN_CONTROL_ERR_IO;
	if (snprintf(buf, bufsiz,
	    "OK TX DRYRUN queued id=%llu port=%s kind=%s ax25_len=%llu "
	    "kiss_len=%llu\nEND\n",
	    (unsigned long long)frame->id, frame->port_name,
	    kn_tx_frame_kind_name(frame->kind),
	    (unsigned long long)frame->ax25_len,
	    (unsigned long long)frame->kiss_len) >= (int)bufsiz)
		return KN_CONTROL_ERR_IO;
	return KN_CONTROL_OK;

invalid:
	(void)snprintf(buf, bufsiz, "ERR invalid-tx-command\n");
	return KN_CONTROL_ERR_UNKNOWN_COMMAND;
}

static enum kn_control_error
read_word(const char **input, char *buf, size_t bufsiz)
{
	const char *p;
	size_t len;

	if (input == NULL || *input == NULL || buf == NULL || bufsiz == 0)
		return KN_CONTROL_ERR_INVALID_ARGUMENT;

	p = *input;
	len = 0;
	while (p[len] != '\0' && p[len] != ' ')
		len++;
	if (len == 0 || len >= bufsiz)
		return KN_CONTROL_ERR_UNKNOWN_COMMAND;
	memcpy(buf, p, len);
	buf[len] = '\0';
	*input = p + len;
	return KN_CONTROL_OK;
}

static enum kn_control_error
format_stats(const struct kn_control_snapshot *snapshot, char *buf,
	size_t bufsiz)
{
	const struct kn_daemon_stats *stats;

	stats = snapshot->daemon;
	return (snprintf(buf, bufsiz,
	    "OK STATS rx_bytes=%llu kiss_frames=%llu ax25_frames=%llu "
	    "malformed_kiss=%llu malformed_ax25=%llu start_time=%llu "
	    "configured_ports=%llu enabled_ports=%llu open_ports=%llu\n",
	    (unsigned long long)stats->bytes_received,
	    (unsigned long long)stats->kiss_frames_received,
	    (unsigned long long)stats->ax25_frames_decoded,
	    (unsigned long long)stats->malformed_kiss_frames,
	    (unsigned long long)stats->malformed_ax25_frames,
	    (unsigned long long)stats->start_time,
	    (unsigned long long)stats->configured_ports,
	    (unsigned long long)stats->enabled_ports,
	    (unsigned long long)stats->open_ports) < (int)bufsiz) ?
	    KN_CONTROL_OK : KN_CONTROL_ERR_IO;
}

static enum kn_control_error
format_status(const struct kn_control_snapshot *snapshot, char *buf,
	size_t bufsiz)
{
	const struct kn_daemon_stats *stats;

	stats = snapshot->daemon;
	return (snprintf(buf, bufsiz,
	    "OK STATUS running ports=%llu open=%llu frames=%llu malformed=%llu\n",
	    (unsigned long long)stats->configured_ports,
	    (unsigned long long)stats->open_ports,
	    (unsigned long long)stats->kiss_frames_received,
	    (unsigned long long)(stats->malformed_kiss_frames +
	    stats->malformed_ax25_frames)) < (int)bufsiz) ?
	    KN_CONTROL_OK : KN_CONTROL_ERR_IO;
}

enum kn_control_error
kn_control_protocol_handle(const char *command,
	const struct kn_control_snapshot *snapshot, char *out, size_t out_len)
{
	if (command == NULL || snapshot == NULL || snapshot->daemon == NULL ||
	    out == NULL || out_len == 0)
		return KN_CONTROL_ERR_INVALID_ARGUMENT;

	out[0] = '\0';

	if (strlen(command) >= KN_CONTROL_COMMAND_MAX ||
	    (snapshot->control_max_command_bytes != 0 &&
	    strlen(command) > snapshot->control_max_command_bytes)) {
		(void)snprintf(out, out_len, "ERR overlong-command\n");
		return KN_CONTROL_ERR_OVERLONG_COMMAND;
	}

	if (command_clean(command) == 0) {
		(void)snprintf(out, out_len, "ERR invalid-command\n");
		return KN_CONTROL_ERR_UNKNOWN_COMMAND;
	}

	if (command[0] == '\0') {
		(void)snprintf(out, out_len, "ERR empty-command\n");
		return KN_CONTROL_ERR_UNKNOWN_COMMAND;
	}

	if (strcmp(command, "PING") == 0)
		return return_with_cap(snapshot, out, out_len,
		    (snprintf(out, out_len, "OK PONG\n") < (int)out_len) ?
		    KN_CONTROL_OK : KN_CONTROL_ERR_IO);
	if (strcmp(command, "VERSION") == 0)
		return return_with_cap(snapshot, out, out_len,
		    (snprintf(out, out_len, "OK VERSION KiloNode %s\n",
		    KILONODE_VERSION) < (int)out_len) ?
		    KN_CONTROL_OK : KN_CONTROL_ERR_IO);
	if (strcmp(command, "STATUS") == 0)
		return return_with_cap(snapshot, out, out_len,
		    format_status(snapshot, out, out_len));
	if (strcmp(command, "PORTS") == 0)
		return return_with_cap(snapshot, out, out_len,
		    format_ports(snapshot, out, out_len));
	if (strcmp(command, "STATS") == 0)
		return return_with_cap(snapshot, out, out_len,
		    format_stats(snapshot, out, out_len));
	if (strcmp(command, "HEARD") == 0)
		return return_with_cap(snapshot, out, out_len,
		    format_heard(snapshot, NULL, out, out_len));
	if (strncmp(command, "HEARD PORT ", 11) == 0) {
		if (command[11] == '\0' ||
		    strlen(command + 11) >= KN_HEARD_PORT_MAX ||
		    strchr(command + 11, ' ') != NULL) {
			(void)snprintf(out, out_len,
			    "ERR invalid-heard-command\n");
			return KN_CONTROL_ERR_UNKNOWN_COMMAND;
		}
		return return_with_cap(snapshot, out, out_len,
		    format_heard(snapshot, command + 11, out, out_len));
	}
	if (strcmp(command, "HEARD CLEAR") == 0) {
		(void)snprintf(out, out_len, "ERR not-implemented\n");
		return KN_CONTROL_ERR_UNKNOWN_COMMAND;
	}
	if (strncmp(command, "HEARD ", 6) == 0) {
		(void)snprintf(out, out_len, "ERR invalid-heard-command\n");
		return KN_CONTROL_ERR_UNKNOWN_COMMAND;
	}
	if (strcmp(command, "BBS") == 0) {
		(void)snprintf(out, out_len, "ERR invalid-bbs-command\n");
		return KN_CONTROL_ERR_UNKNOWN_COMMAND;
	}
	if (strncmp(command, "BBS ", 4) == 0)
		return return_with_cap(snapshot, out, out_len,
		    format_bbs(snapshot, command + 4, out, out_len));
	if (strcmp(command, "RX") == 0) {
		(void)snprintf(out, out_len, "ERR invalid-rx-command\n");
		return KN_CONTROL_ERR_UNKNOWN_COMMAND;
	}
	if (strncmp(command, "RX ", 3) == 0)
		return return_with_cap(snapshot, out, out_len,
		    format_rx(snapshot, command + 3, out, out_len));
	if (strcmp(command, "TX") == 0) {
		(void)snprintf(out, out_len, "ERR invalid-tx-command\n");
		return KN_CONTROL_ERR_UNKNOWN_COMMAND;
	}
	if (strncmp(command, "TX ", 3) == 0)
		return return_with_cap(snapshot, out, out_len,
		    format_tx(snapshot, command + 3, out, out_len));
	if (strcmp(command, "HELP") == 0)
		return return_with_cap(snapshot, out, out_len,
		    format_help(out, out_len));
	if (strcmp(command, "QUIT") == 0)
		return return_with_cap(snapshot, out, out_len,
		    (snprintf(out, out_len, "OK BYE\n") < (int)out_len) ?
		    KN_CONTROL_OK : KN_CONTROL_ERR_IO);

	(void)snprintf(out, out_len, "ERR unknown-command\n");
	return KN_CONTROL_ERR_UNKNOWN_COMMAND;
}
