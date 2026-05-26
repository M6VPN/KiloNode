/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/core/callsign.c */

#include <sys/types.h>

#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "kilonode/callsign.h"

static int callsign_char_valid(int);
static int ssid_parse(const char *, uint8_t *);

static int
callsign_char_valid(int ch)
{
	return isupper(ch) || isdigit(ch);
}

int
kn_callsign_format(const struct kn_callsign *call, char *buf, size_t bufsiz)
{
	int needed;

	if (call == NULL || buf == NULL || bufsiz == 0)
		return EINVAL;

	if (call->call[0] == '\0' || call->ssid > 15)
		return EINVAL;

	if (call->ssid == 0)
		needed = snprintf(buf, bufsiz, "%s", call->call);
	else
		needed = snprintf(buf, bufsiz, "%s-%u", call->call, call->ssid);

	if (needed < 0)
		return EINVAL;

	if ((size_t)needed >= bufsiz)
		return ENOSPC;

	return 0;
}

int
kn_callsign_parse(const char *input, struct kn_callsign *out)
{
	const char *dash;
	size_t call_len;
	size_t i;
	int rc;

	if (input == NULL || out == NULL)
		return EINVAL;

	memset(out, 0, sizeof(*out));

	dash = strchr(input, '-');
	call_len = dash == NULL ? strlen(input) : (size_t)(dash - input);

	if (call_len == 0 || call_len > KN_CALLSIGN_MAX)
		return EINVAL;

	for (i = 0; i < call_len; i++) {
		if (!callsign_char_valid((unsigned char)input[i]))
			return EINVAL;
	}

	memcpy(out->call, input, call_len);
	out->call[call_len] = '\0';
	out->ssid = 0;

	if (dash == NULL)
		return 0;

	rc = ssid_parse(dash + 1, &out->ssid);
	if (rc != 0) {
		memset(out, 0, sizeof(*out));
		return rc;
	}

	return 0;
}

static int
ssid_parse(const char *input, uint8_t *ssid)
{
	unsigned int value;
	size_t i;

	if (input == NULL || ssid == NULL || input[0] == '\0')
		return EINVAL;

	value = 0;
	for (i = 0; input[i] != '\0'; i++) {
		if (!isdigit((unsigned char)input[i]))
			return EINVAL;
		value = value * 10 + (unsigned int)(input[i] - '0');
		if (value > 15)
			return EINVAL;
	}

	*ssid = (uint8_t)value;
	return 0;
}
