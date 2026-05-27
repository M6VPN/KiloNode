/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/proto/ax25_connection_key.c */

#include <sys/types.h>

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "kilonode/ax25_connection_key.h"

static int callsign_compare(const struct kn_callsign *,
	const struct kn_callsign *);
static enum kn_ax25_connection_key_error callsign_valid(
	const struct kn_callsign *);
static enum kn_ax25_connection_key_error port_copy(char *, size_t,
	const char *);
static enum kn_ax25_connection_key_error port_valid(const char *);

static int
callsign_compare(const struct kn_callsign *a, const struct kn_callsign *b)
{
	int rc;

	rc = strcmp(a->call, b->call);
	if (rc != 0)
		return rc;
	if (a->ssid < b->ssid)
		return -1;
	if (a->ssid > b->ssid)
		return 1;

	return 0;
}

static enum kn_ax25_connection_key_error
callsign_valid(const struct kn_callsign *callsign)
{
	char text[KN_CALLSIGN_MAX + 4];

	if (kn_callsign_format(callsign, text, sizeof(text)) != 0)
		return KN_AX25_CONNECTION_KEY_ERR_INVALID_VALUE;

	return KN_AX25_CONNECTION_KEY_OK;
}

static enum kn_ax25_connection_key_error
port_copy(char *dst, size_t dst_len, const char *src)
{
	size_t len;

	if (dst == NULL || src == NULL || dst_len == 0)
		return KN_AX25_CONNECTION_KEY_ERR_INVALID_ARGUMENT;
	if (port_valid(src) != KN_AX25_CONNECTION_KEY_OK)
		return KN_AX25_CONNECTION_KEY_ERR_INVALID_VALUE;

	len = strlen(src);
	if (len >= dst_len)
		return KN_AX25_CONNECTION_KEY_ERR_INVALID_VALUE;
	memcpy(dst, src, len + 1);
	return KN_AX25_CONNECTION_KEY_OK;
}

static enum kn_ax25_connection_key_error
port_valid(const char *port)
{
	size_t i;

	if (port == NULL || port[0] == '\0')
		return KN_AX25_CONNECTION_KEY_ERR_INVALID_ARGUMENT;
	if (strlen(port) >= KN_CONFIG_PORT_NAME_MAX)
		return KN_AX25_CONNECTION_KEY_ERR_INVALID_VALUE;
	for (i = 0; port[i] != '\0'; i++) {
		if (!isalnum((unsigned char)port[i]) && port[i] != '-' &&
		    port[i] != '_')
			return KN_AX25_CONNECTION_KEY_ERR_INVALID_VALUE;
	}

	return KN_AX25_CONNECTION_KEY_OK;
}

int
kn_ax25_connection_key_compare(const struct kn_ax25_connection_key *a,
	const struct kn_ax25_connection_key *b)
{
	size_t i;
	int rc;

	if (a == NULL && b == NULL)
		return 0;
	if (a == NULL)
		return -1;
	if (b == NULL)
		return 1;

	rc = strcmp(a->port_name, b->port_name);
	if (rc != 0)
		return rc;
	rc = callsign_compare(&a->local, &b->local);
	if (rc != 0)
		return rc;
	rc = callsign_compare(&a->remote, &b->remote);
	if (rc != 0)
		return rc;
	if (a->digipeater_count < b->digipeater_count)
		return -1;
	if (a->digipeater_count > b->digipeater_count)
		return 1;
	for (i = 0; i < a->digipeater_count; i++) {
		rc = callsign_compare(&a->digipeaters[i],
		    &b->digipeaters[i]);
		if (rc != 0)
			return rc;
	}

	return 0;
}

void
kn_ax25_connection_key_clear(struct kn_ax25_connection_key *key)
{
	if (key == NULL)
		return;

	memset(key, 0, sizeof(*key));
}

uint8_t
kn_ax25_connection_key_equal(const struct kn_ax25_connection_key *a,
	const struct kn_ax25_connection_key *b)
{
	return kn_ax25_connection_key_compare(a, b) == 0 ? 1 : 0;
}

enum kn_ax25_connection_key_error
kn_ax25_connection_key_format(const struct kn_ax25_connection_key *key,
	char *buf, size_t bufsiz)
{
	char local[KN_CALLSIGN_MAX + 4];
	char remote[KN_CALLSIGN_MAX + 4];
	char digi[KN_CALLSIGN_MAX + 4];
	size_t offset;
	size_t i;
	int needed;

	if (key == NULL || buf == NULL || bufsiz == 0)
		return KN_AX25_CONNECTION_KEY_ERR_INVALID_ARGUMENT;
	if (kn_ax25_connection_key_validate(key) !=
	    KN_AX25_CONNECTION_KEY_OK)
		return KN_AX25_CONNECTION_KEY_ERR_INVALID_VALUE;
	if (kn_callsign_format(&key->local, local, sizeof(local)) != 0 ||
	    kn_callsign_format(&key->remote, remote, sizeof(remote)) != 0)
		return KN_AX25_CONNECTION_KEY_ERR_INVALID_VALUE;

	needed = snprintf(buf, bufsiz, "port=%s local=%s remote=%s via=",
	    key->port_name, local, remote);
	if (needed < 0 || (size_t)needed >= bufsiz)
		return KN_AX25_CONNECTION_KEY_ERR_BUFFER;
	offset = (size_t)needed;

	if (key->digipeater_count == 0) {
		needed = snprintf(buf + offset, bufsiz - offset, "-");
		if (needed < 0 || (size_t)needed >= bufsiz - offset)
			return KN_AX25_CONNECTION_KEY_ERR_BUFFER;
		return KN_AX25_CONNECTION_KEY_OK;
	}

	for (i = 0; i < key->digipeater_count; i++) {
		if (kn_callsign_format(&key->digipeaters[i], digi,
		    sizeof(digi)) != 0)
			return KN_AX25_CONNECTION_KEY_ERR_INVALID_VALUE;
		needed = snprintf(buf + offset, bufsiz - offset, "%s%s",
		    i == 0 ? "" : ",", digi);
		if (needed < 0 || (size_t)needed >= bufsiz - offset)
			return KN_AX25_CONNECTION_KEY_ERR_BUFFER;
		offset += (size_t)needed;
	}

	return KN_AX25_CONNECTION_KEY_OK;
}

enum kn_ax25_connection_key_error
kn_ax25_connection_key_from_callsigns(struct kn_ax25_connection_key *key,
	const char *port_name, const char *local, const char *remote,
	const char *const *digipeaters, size_t digipeater_count)
{
	size_t i;

	if (key == NULL || port_name == NULL || local == NULL ||
	    remote == NULL)
		return KN_AX25_CONNECTION_KEY_ERR_INVALID_ARGUMENT;
	if (digipeater_count > KN_AX25_MAX_DIGIS)
		return KN_AX25_CONNECTION_KEY_ERR_TOO_MANY_DIGIS;
	if (digipeater_count > 0 && digipeaters == NULL)
		return KN_AX25_CONNECTION_KEY_ERR_INVALID_ARGUMENT;

	kn_ax25_connection_key_clear(key);
	if (port_copy(key->port_name, sizeof(key->port_name), port_name) !=
	    KN_AX25_CONNECTION_KEY_OK)
		return KN_AX25_CONNECTION_KEY_ERR_INVALID_VALUE;
	if (kn_callsign_parse(local, &key->local) != 0 ||
	    kn_callsign_parse(remote, &key->remote) != 0)
		return KN_AX25_CONNECTION_KEY_ERR_INVALID_VALUE;
	key->digipeater_count = digipeater_count;
	for (i = 0; i < digipeater_count; i++) {
		if (digipeaters[i] == NULL ||
		    kn_callsign_parse(digipeaters[i],
		    &key->digipeaters[i]) != 0)
			return KN_AX25_CONNECTION_KEY_ERR_INVALID_VALUE;
	}

	return KN_AX25_CONNECTION_KEY_OK;
}

enum kn_ax25_connection_key_error
kn_ax25_connection_key_from_frame(struct kn_ax25_connection_key *key,
	const char *port_name, const struct kn_callsign *local,
	const struct kn_ax25_frame *frame)
{
	size_t i;

	if (key == NULL || port_name == NULL || local == NULL ||
	    frame == NULL)
		return KN_AX25_CONNECTION_KEY_ERR_INVALID_ARGUMENT;
	if (frame->digipeater_count > KN_AX25_MAX_DIGIS)
		return KN_AX25_CONNECTION_KEY_ERR_TOO_MANY_DIGIS;
	if (callsign_valid(local) != KN_AX25_CONNECTION_KEY_OK ||
	    callsign_valid(&frame->source.callsign) !=
	    KN_AX25_CONNECTION_KEY_OK)
		return KN_AX25_CONNECTION_KEY_ERR_INVALID_VALUE;

	kn_ax25_connection_key_clear(key);
	if (port_copy(key->port_name, sizeof(key->port_name), port_name) !=
	    KN_AX25_CONNECTION_KEY_OK)
		return KN_AX25_CONNECTION_KEY_ERR_INVALID_VALUE;
	key->local = *local;
	key->remote = frame->source.callsign;
	key->digipeater_count = frame->digipeater_count;
	for (i = 0; i < frame->digipeater_count; i++)
		key->digipeaters[i] = frame->digipeaters[i].callsign;

	return KN_AX25_CONNECTION_KEY_OK;
}

enum kn_ax25_connection_key_error
kn_ax25_connection_key_validate(const struct kn_ax25_connection_key *key)
{
	size_t i;

	if (key == NULL)
		return KN_AX25_CONNECTION_KEY_ERR_INVALID_ARGUMENT;
	if (port_valid(key->port_name) != KN_AX25_CONNECTION_KEY_OK)
		return KN_AX25_CONNECTION_KEY_ERR_INVALID_VALUE;
	if (callsign_valid(&key->local) != KN_AX25_CONNECTION_KEY_OK ||
	    callsign_valid(&key->remote) != KN_AX25_CONNECTION_KEY_OK)
		return KN_AX25_CONNECTION_KEY_ERR_INVALID_VALUE;
	if (key->digipeater_count > KN_AX25_MAX_DIGIS)
		return KN_AX25_CONNECTION_KEY_ERR_TOO_MANY_DIGIS;
	for (i = 0; i < key->digipeater_count; i++) {
		if (callsign_valid(&key->digipeaters[i]) !=
		    KN_AX25_CONNECTION_KEY_OK)
			return KN_AX25_CONNECTION_KEY_ERR_INVALID_VALUE;
	}

	return KN_AX25_CONNECTION_KEY_OK;
}
