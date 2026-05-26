/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/transport_unix.h */

#ifndef KILONODE_TRANSPORT_UNIX_H
#define KILONODE_TRANSPORT_UNIX_H

#include <stdint.h>

#include "kilonode/transport.h"

enum kn_transport_error kn_transport_unix_connect_open(struct kn_transport *,
	const char *);
enum kn_transport_error kn_transport_unix_listen_open(struct kn_transport *,
	const char *);
uint8_t kn_transport_unix_path_valid(const char *);

#endif
