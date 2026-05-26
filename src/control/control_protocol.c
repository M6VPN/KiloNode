/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/control/control_protocol.c */

#include <sys/types.h>

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "kilonode/bbs_control.h"
#include "kilonode/control.h"
#include "kilonode/heard.h"
#include "kilonode/stats.h"

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
static enum kn_control_error format_stats(const struct kn_control_snapshot *,
	char *, size_t);
static enum kn_control_error format_status(const struct kn_control_snapshot *,
	char *, size_t);
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
	    "OK HELP PING VERSION STATUS PORTS STATS HEARD BBS HELP QUIT\n");
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
