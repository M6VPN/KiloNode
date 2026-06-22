/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/external_modem_control.h */

#ifndef KILONODE_EXTERNAL_MODEM_CONTROL_H
#define KILONODE_EXTERNAL_MODEM_CONTROL_H

#include <sys/types.h>

#include "kilonode/external_modem_status.h"

enum kn_external_modem_control_error {
	KN_EXTERNAL_MODEM_CONTROL_OK = 0,
	KN_EXTERNAL_MODEM_CONTROL_ERR_INVALID_ARGUMENT,
	KN_EXTERNAL_MODEM_CONTROL_ERR_UNKNOWN_COMMAND,
	KN_EXTERNAL_MODEM_CONTROL_ERR_BUFFER
};

enum kn_external_modem_control_error kn_external_modem_control_format(
	const char *, const struct kn_external_modem_status_table *, char *,
	size_t);

#endif
