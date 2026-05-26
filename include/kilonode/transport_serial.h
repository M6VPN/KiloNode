/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/transport_serial.h */

#ifndef KILONODE_TRANSPORT_SERIAL_H
#define KILONODE_TRANSPORT_SERIAL_H

#include <stdint.h>

#include "kilonode/transport.h"

uint8_t kn_transport_serial_baud_valid(unsigned int);
uint8_t kn_transport_serial_flow_control_parse(const char *, uint8_t *);
enum kn_transport_error kn_transport_serial_open(struct kn_transport *,
	const char *, unsigned int, uint8_t);

#endif
