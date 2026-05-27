/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/compat/compat_report.c */

#include <sys/types.h>

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "kilonode/compat_report.h"

static enum kn_compat_report_error append_format(char *, size_t, size_t *,
	const char *, ...);
static void escape_preview(const char *, char *, size_t);

const char *
kn_compat_report_error_name(enum kn_compat_report_error error)
{
	switch (error) {
	case KN_COMPAT_REPORT_OK:
		return "ok";
	case KN_COMPAT_REPORT_ERR_INVALID_ARGUMENT:
		return "invalid-argument";
	case KN_COMPAT_REPORT_ERR_BUFFER:
		return "buffer";
	}

	return "unknown";
}

enum kn_compat_report_error
kn_compat_report_format_text(const struct kn_compat_replay_result *result,
	char *buf, size_t bufsiz)
{
	char reply[KN_COMPAT_REPLY_PREVIEW_MAX * 4];
	size_t offset;
	size_t i;
	enum kn_compat_report_error rc;

	if (result == NULL || buf == NULL || bufsiz == 0)
		return KN_COMPAT_REPORT_ERR_INVALID_ARGUMENT;

	escape_preview(result->observed_reply_preview, reply, sizeof(reply));

	offset = 0;
	rc = append_format(buf, bufsiz, &offset,
	    "COMPAT transcript=%s mode=%s result=%s mismatches=%llu\n",
	    result->transcript_name, kn_compat_mode_name(result->mode),
	    result->passed != 0 ? "pass" : "fail",
	    (unsigned long long)result->mismatch_count);
	if (rc != KN_COMPAT_REPORT_OK)
		return rc;
	rc = append_format(buf, bufsiz, &offset,
	    "OBSERVED command=%s status=%s reply_queued=%s tx=%llu "
	    "reply=\"%s\"\n",
	    kn_rf_command_name_string(result->observed_command),
	    kn_rf_command_status_string(result->observed_status),
	    result->observed_reply_queued != 0 ? "true" : "false",
	    (unsigned long long)result->observed_tx_frame_id,
	    reply);
	if (rc != KN_COMPAT_REPORT_OK)
		return rc;
	for (i = 0; i < result->mismatch_count; i++) {
		rc = append_format(buf, bufsiz, &offset,
		    "MISMATCH %s\n", result->mismatches[i].text);
		if (rc != KN_COMPAT_REPORT_OK)
			return rc;
	}

	return KN_COMPAT_REPORT_OK;
}

static enum kn_compat_report_error
append_format(char *buf, size_t bufsiz, size_t *offset, const char *fmt, ...)
{
	va_list args;
	int needed;

	if (*offset >= bufsiz)
		return KN_COMPAT_REPORT_ERR_BUFFER;

	va_start(args, fmt);
	needed = vsnprintf(buf + *offset, bufsiz - *offset, fmt, args);
	va_end(args);
	if (needed < 0 || (size_t)needed >= bufsiz - *offset)
		return KN_COMPAT_REPORT_ERR_BUFFER;
	*offset += (size_t)needed;
	return KN_COMPAT_REPORT_OK;
}

static void
escape_preview(const char *src, char *dst, size_t dst_len)
{
	size_t i;
	size_t off;
	int needed;
	unsigned char ch;

	if (dst == NULL || dst_len == 0)
		return;
	dst[0] = '\0';
	if (src == NULL)
		return;

	off = 0;
	for (i = 0; src[i] != '\0'; i++) {
		ch = (unsigned char)src[i];
		if (ch == '"' || ch == '\\') {
			needed = snprintf(dst + off, dst_len - off, "\\%c",
			    (int)ch);
		} else if (ch >= 0x20 && ch <= 0x7e) {
			needed = snprintf(dst + off, dst_len - off, "%c",
			    (int)ch);
		} else {
			needed = snprintf(dst + off, dst_len - off, "\\x%02x",
			    (unsigned int)ch);
		}
		if (needed < 0 || (size_t)needed >= dst_len - off) {
			dst[off] = '\0';
			return;
		}
		off += (size_t)needed;
	}
}
