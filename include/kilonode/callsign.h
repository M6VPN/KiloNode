/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/callsign.h */

#ifndef KILONODE_CALLSIGN_H
#define KILONODE_CALLSIGN_H

#include <sys/types.h>

#include <stdint.h>

#define KN_CALLSIGN_MAX 6

struct kn_callsign {
	char call[KN_CALLSIGN_MAX + 1];
	uint8_t ssid;
};

int kn_callsign_format(const struct kn_callsign *, char *, size_t);
int kn_callsign_parse(const char *, struct kn_callsign *);

#endif
