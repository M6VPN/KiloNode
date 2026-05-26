/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/transport/transport_stdio.c */

#define _POSIX_C_SOURCE 200809L

#include <unistd.h>

#include "kilonode/transport.h"
#include "kilonode/transport_stdio.h"

enum kn_transport_error
kn_transport_stdio_open(struct kn_transport *transport)
{
	if (transport == NULL)
		return KN_TRANSPORT_ERR_INVALID_ARGUMENT;

	kn_transport_reset(transport);

	transport->kind = KN_TRANSPORT_KIND_STDIO;
	transport->read_fd = STDIN_FILENO;
	transport->write_fd = STDOUT_FILENO;
	transport->listen_fd = -1;
	transport->open = 1;

	return KN_TRANSPORT_OK;
}
