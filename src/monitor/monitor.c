/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/monitor/monitor.c */

#include <sys/types.h>

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "kilonode/ax25.h"
#include "kilonode/callsign.h"
#include "kilonode/monitor.h"

#define MONITOR_ADDR_BUFSIZ    16
#define MONITOR_HEX_PREVIEW    16
#define MONITOR_PAYLOAD_BUFSIZ 96

static enum kn_monitor_error append_format(char *, size_t, size_t *,
	const char *, ...);
static enum kn_monitor_error format_addr(const struct kn_ax25_addr *, char *,
	size_t);
static enum kn_monitor_error format_binary_payload(char *, size_t,
	const uint8_t *, size_t);
static enum kn_monitor_error format_digipeaters(const struct kn_ax25_frame *,
	char *, size_t);
static enum kn_monitor_error format_text_payload(char *, size_t,
	const uint8_t *, size_t);
static uint8_t payload_printable(const uint8_t *, size_t);

static enum kn_monitor_error
append_format(char *buf, size_t bufsiz, size_t *offset, const char *fmt, ...)
{
	va_list args;
	int needed;

	if (*offset >= bufsiz)
		return KN_MONITOR_ERR_NO_SPACE;

	va_start(args, fmt);
	needed = vsnprintf(buf + *offset, bufsiz - *offset, fmt, args);
	va_end(args);

	if (needed < 0)
		return KN_MONITOR_ERR_NO_SPACE;

	if ((size_t)needed >= bufsiz - *offset)
		return KN_MONITOR_ERR_NO_SPACE;

	*offset += (size_t)needed;
	return KN_MONITOR_OK;
}

static enum kn_monitor_error
format_addr(const struct kn_ax25_addr *addr, char *buf, size_t bufsiz)
{
	if (kn_callsign_format(&addr->callsign, buf, bufsiz) != 0)
		return KN_MONITOR_ERR_NO_SPACE;

	return KN_MONITOR_OK;
}

static enum kn_monitor_error
format_binary_payload(char *buf, size_t bufsiz, const uint8_t *payload,
	size_t payload_len)
{
	size_t i;
	size_t offset;
	size_t preview_len;
	enum kn_monitor_error rc;

	offset = 0;
	preview_len = payload_len < MONITOR_HEX_PREVIEW ?
	    payload_len : MONITOR_HEX_PREVIEW;

	rc = append_format(buf, bufsiz, &offset, "binary len %zu hex",
	    payload_len);
	if (rc != KN_MONITOR_OK)
		return rc;

	for (i = 0; i < preview_len; i++) {
		rc = append_format(buf, bufsiz, &offset, " %02x",
		    (unsigned int)payload[i]);
		if (rc != KN_MONITOR_OK)
			return rc;
	}

	if (preview_len < payload_len) {
		rc = append_format(buf, bufsiz, &offset, " ...");
		if (rc != KN_MONITOR_OK)
			return rc;
	}

	return KN_MONITOR_OK;
}

static enum kn_monitor_error
format_digipeaters(const struct kn_ax25_frame *frame, char *buf, size_t bufsiz)
{
	char addr[MONITOR_ADDR_BUFSIZ];
	size_t i;
	size_t offset;
	enum kn_monitor_error rc;

	offset = 0;
	buf[0] = '\0';

	for (i = 0; i < frame->digipeater_count; i++) {
		rc = format_addr(&frame->digipeaters[i], addr, sizeof(addr));
		if (rc != KN_MONITOR_OK)
			return rc;

		rc = append_format(buf, bufsiz, &offset, "%s%s",
		    i == 0 ? "" : ",", addr);
		if (rc != KN_MONITOR_OK)
			return rc;
	}

	return KN_MONITOR_OK;
}

static enum kn_monitor_error
format_text_payload(char *buf, size_t bufsiz, const uint8_t *payload,
	size_t payload_len)
{
	size_t i;

	if (payload_len + 1 > bufsiz)
		return KN_MONITOR_ERR_NO_SPACE;

	for (i = 0; i < payload_len; i++)
		buf[i] = (char)payload[i];

	buf[payload_len] = '\0';
	return KN_MONITOR_OK;
}

enum kn_monitor_error
kn_monitor_format_kiss(char *buf, size_t bufsiz, uint8_t port,
	uint8_t command, const uint8_t *payload, size_t payload_len)
{
	struct kn_ax25_frame frame;
	char destination[MONITOR_ADDR_BUFSIZ];
	char digipeaters[MONITOR_PAYLOAD_BUFSIZ];
	char payload_text[MONITOR_PAYLOAD_BUFSIZ];
	char source[MONITOR_ADDR_BUFSIZ];
	enum kn_ax25_error ax25_rc;
	enum kn_monitor_error rc;
	size_t offset;

	if (buf == NULL || bufsiz == 0 || (payload == NULL && payload_len > 0))
		return KN_MONITOR_ERR_INVALID_ARGUMENT;

	buf[0] = '\0';
	offset = 0;

	if (command != 0)
		return append_format(buf, bufsiz, &offset,
		    "PORT %u KISS command %u len %zu", (unsigned int)port,
		    (unsigned int)command, payload_len);

	ax25_rc = kn_ax25_frame_decode(payload, payload_len, &frame);
	if (ax25_rc != KN_AX25_OK)
		return append_format(buf, bufsiz, &offset,
		    "PORT %u malformed AX.25 len %zu error %d",
		    (unsigned int)port, payload_len, (int)ax25_rc);

	if (frame.control != KN_AX25_CONTROL_UI)
		return append_format(buf, bufsiz, &offset,
		    "PORT %u AX.25 control 0x%02x len %zu",
		    (unsigned int)port, (unsigned int)frame.control,
		    frame.payload_len);

	rc = format_addr(&frame.source, source, sizeof(source));
	if (rc != KN_MONITOR_OK)
		return rc;

	rc = format_addr(&frame.destination, destination, sizeof(destination));
	if (rc != KN_MONITOR_OK)
		return rc;

	if (payload_printable(frame.payload, frame.payload_len) != 0)
		rc = format_text_payload(payload_text, sizeof(payload_text),
		    frame.payload, frame.payload_len);
	else
		rc = format_binary_payload(payload_text, sizeof(payload_text),
		    frame.payload, frame.payload_len);
	if (rc != KN_MONITOR_OK)
		return rc;

	rc = append_format(buf, bufsiz, &offset, "PORT %u UI %s > %s",
	    (unsigned int)port, source, destination);
	if (rc != KN_MONITOR_OK)
		return rc;

	if (frame.digipeater_count > 0) {
		rc = format_digipeaters(&frame, digipeaters, sizeof(digipeaters));
		if (rc != KN_MONITOR_OK)
			return rc;
		rc = append_format(buf, bufsiz, &offset, " via %s",
		    digipeaters);
		if (rc != KN_MONITOR_OK)
			return rc;
	}

	return append_format(buf, bufsiz, &offset, ": %s", payload_text);
}

static uint8_t
payload_printable(const uint8_t *payload, size_t payload_len)
{
	size_t i;

	if (payload == NULL && payload_len > 0)
		return 0;

	for (i = 0; i < payload_len; i++) {
		if (payload[i] < 0x20 || payload[i] > 0x7e)
			return 0;
	}

	return 1;
}
