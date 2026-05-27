/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/compat/compat_capture_report.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "kilonode/compat_capture_report.h"
#include "kilonode/rf_command.h"

const char *
kn_compat_capture_report_error_name(
	enum kn_compat_capture_report_error error)
{
	switch (error) {
	case KN_COMPAT_CAPTURE_REPORT_OK:
		return "ok";
	case KN_COMPAT_CAPTURE_REPORT_ERR_INVALID_ARGUMENT:
		return "invalid-argument";
	case KN_COMPAT_CAPTURE_REPORT_ERR_BUFFER:
		return "buffer";
	case KN_COMPAT_CAPTURE_REPORT_ERR_UNSUPPORTED:
		return "unsupported";
	}

	return "unknown";
}

enum kn_compat_capture_report_error
kn_compat_capture_report_format(const struct kn_compat_packet_decode *decode,
	char *buf, size_t bufsiz)
{
	size_t offset;
	size_t i;
	char pid[16];
	int needed;

	if (decode == NULL || buf == NULL || bufsiz == 0)
		return KN_COMPAT_CAPTURE_REPORT_ERR_INVALID_ARGUMENT;

	if (decode->has_pid != 0)
		needed = snprintf(pid, sizeof(pid), "0x%02x",
		    (unsigned int)decode->pid);
	else
		needed = snprintf(pid, sizeof(pid), "none");
	if (needed < 0 || (size_t)needed >= sizeof(pid))
		return KN_COMPAT_CAPTURE_REPORT_ERR_BUFFER;

	needed = snprintf(buf, bufsiz,
	    "CAPTURE name=%s method=%s result=%s frames=1 source=%s "
	    "destination=%s kind=%s pid=%s payload_len=%llu "
	    "preview=\"%s\" mismatches=%llu\n",
	    decode->capture_name, kn_compat_packet_method_name(decode->method),
	    decode->passed != 0 ? "pass" : "fail", decode->source,
	    decode->destination, decode->kind, pid,
	    (unsigned long long)decode->payload_len, decode->payload_preview,
	    (unsigned long long)decode->mismatch_count);
	if (needed < 0 || (size_t)needed >= bufsiz)
		return KN_COMPAT_CAPTURE_REPORT_ERR_BUFFER;
	offset = (size_t)needed;
	for (i = 0; i < decode->mismatch_count; i++) {
		needed = snprintf(buf + offset, bufsiz - offset,
		    "MISMATCH %s\n", decode->mismatches[i].text);
		if (needed < 0 || (size_t)needed >= bufsiz - offset)
			return KN_COMPAT_CAPTURE_REPORT_ERR_BUFFER;
		offset += (size_t)needed;
	}

	return KN_COMPAT_CAPTURE_REPORT_OK;
}

enum kn_compat_capture_report_error
kn_compat_capture_to_transcript(
	const struct kn_compat_packet_capture *capture,
	const struct kn_compat_packet_decode *decode, char *buf, size_t bufsiz)
{
	enum kn_rf_command_name command;
	enum kn_rf_command_status status;
	char raw[KN_RF_COMMAND_RAW_MAX];
	int needed;

	if (capture == NULL || decode == NULL || buf == NULL || bufsiz == 0)
		return KN_COMPAT_CAPTURE_REPORT_ERR_INVALID_ARGUMENT;
	if (strcmp(decode->kind, "UI") != 0 || decode->has_pid == 0 ||
	    decode->pid != 0xf0 || decode->payload_preview[0] == '\0')
		return KN_COMPAT_CAPTURE_REPORT_ERR_UNSUPPORTED;
	if (kn_rf_command_parse((const uint8_t *)decode->payload_preview,
	    strlen(decode->payload_preview), KN_CONFIG_RF_COMMAND_BYTES_MAX,
	    &command, raw, sizeof(raw), &status) != KN_RF_COMMAND_OK)
		return KN_COMPAT_CAPTURE_REPORT_ERR_UNSUPPORTED;

	needed = snprintf(buf, bufsiz,
	    "# KiloNode compatibility transcript candidate v1\n"
	    "# source=packet-boundary-capture\n"
	    "# capture=%s\n"
	    "# implementation-use=false\n"
	    "name %s-candidate\n"
	    "mode rf-ui\n"
	    "node %s\n"
	    "port %s\n"
	    "source %s\n"
	    "destination %s\n"
	    "pid 0x%02x\n"
	    "input %s\n"
	    "expect-event command=%s status=%s\n"
	    "expect-reply none\n"
	    "expect-no-dispatch true\n",
	    capture->name, capture->name, decode->destination, capture->port,
	    decode->source, decode->destination, (unsigned int)decode->pid,
	    decode->payload_preview, kn_rf_command_name_string(command),
	    kn_rf_command_status_string(status));

	return needed >= 0 && (size_t)needed < bufsiz ?
	    KN_COMPAT_CAPTURE_REPORT_OK : KN_COMPAT_CAPTURE_REPORT_ERR_BUFFER;
}
