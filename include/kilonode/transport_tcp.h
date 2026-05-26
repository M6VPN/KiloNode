/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/transport_tcp.h */

#ifndef KILONODE_TRANSPORT_TCP_H
#define KILONODE_TRANSPORT_TCP_H

#include <stdint.h>

#include "kilonode/transport.h"

enum kn_transport_error kn_transport_tcp_connect_open(struct kn_transport *,
	const char *, const char *);
enum kn_transport_error kn_transport_tcp_listen_open(struct kn_transport *,
	const char *, const char *);
uint8_t kn_transport_tcp_port_valid(const char *);

#endif
