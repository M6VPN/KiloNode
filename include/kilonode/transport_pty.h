/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/transport_pty.h */

#ifndef KILONODE_TRANSPORT_PTY_H
#define KILONODE_TRANSPORT_PTY_H

#include <sys/types.h>

#include "kilonode/transport.h"

#define KN_TRANSPORT_PTY_PATH_MAX 128

enum kn_transport_error kn_transport_pty_open(struct kn_transport *, char *,
	size_t);

#endif
