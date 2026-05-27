/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/ax25_connection_key.h */

#ifndef KILONODE_AX25_CONNECTION_KEY_H
#define KILONODE_AX25_CONNECTION_KEY_H

#include <sys/types.h>

#include "kilonode/ax25.h"
#include "kilonode/config.h"

enum kn_ax25_connection_key_error {
	KN_AX25_CONNECTION_KEY_OK = 0,
	KN_AX25_CONNECTION_KEY_ERR_INVALID_ARGUMENT,
	KN_AX25_CONNECTION_KEY_ERR_INVALID_VALUE,
	KN_AX25_CONNECTION_KEY_ERR_TOO_MANY_DIGIS,
	KN_AX25_CONNECTION_KEY_ERR_BUFFER
};

struct kn_ax25_connection_key {
	char port_name[KN_CONFIG_PORT_NAME_MAX];
	struct kn_callsign local;
	struct kn_callsign remote;
	struct kn_callsign digipeaters[KN_AX25_MAX_DIGIS];
	size_t digipeater_count;
};

int kn_ax25_connection_key_compare(const struct kn_ax25_connection_key *,
	const struct kn_ax25_connection_key *);
void kn_ax25_connection_key_clear(struct kn_ax25_connection_key *);
uint8_t kn_ax25_connection_key_equal(const struct kn_ax25_connection_key *,
	const struct kn_ax25_connection_key *);
enum kn_ax25_connection_key_error kn_ax25_connection_key_format(
	const struct kn_ax25_connection_key *, char *, size_t);
enum kn_ax25_connection_key_error kn_ax25_connection_key_from_callsigns(
	struct kn_ax25_connection_key *, const char *, const char *,
	const char *, const char *const *, size_t);
enum kn_ax25_connection_key_error kn_ax25_connection_key_from_frame(
	struct kn_ax25_connection_key *, const char *,
	const struct kn_callsign *, const struct kn_ax25_frame *);
enum kn_ax25_connection_key_error kn_ax25_connection_key_validate(
	const struct kn_ax25_connection_key *);

#endif
