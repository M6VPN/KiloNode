/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/external_modem_status.h */

#ifndef KILONODE_EXTERNAL_MODEM_STATUS_H
#define KILONODE_EXTERNAL_MODEM_STATUS_H

#include <sys/types.h>

#include "kilonode/external_modem.h"
#include "kilonode/external_modem_profile.h"

struct kn_external_modem_status_entry {
	char name[KN_EXTERNAL_MODEM_NAME_MAX];
	enum kn_external_modem_type type;
	enum kn_external_modem_mode mode;
	enum kn_external_modem_state state;
	char host[KN_EXTERNAL_MODEM_HOST_MAX];
	uint16_t port;
	char reason[KN_EXTERNAL_MODEM_REASON_MAX];
	uint8_t configured;
	uint8_t enabled;
	uint8_t auto_start;
	uint8_t tx_enabled;
	uint8_t connect_enabled;
	const struct kn_external_modem_profile *profile;
};

struct kn_external_modem_status_table {
	struct kn_external_modem_status_entry entries[KN_EXTERNAL_MODEM_MAX];
	size_t count;
};

enum kn_external_modem_error kn_external_modem_status_format(
	const struct kn_external_modem_status_entry *, char *, size_t);
const struct kn_external_modem_status_entry *kn_external_modem_status_lookup(
	const struct kn_external_modem_status_table *, const char *);
enum kn_external_modem_error kn_external_modem_status_table_init(
	struct kn_external_modem_status_table *,
	const struct kn_external_modem_config *, size_t);

#endif
