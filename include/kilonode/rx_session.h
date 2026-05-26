/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/rx_session.h */

#ifndef KILONODE_RX_SESSION_H
#define KILONODE_RX_SESSION_H

#include <sys/types.h>

#include <stdint.h>

#include "kilonode/callsign.h"
#include "kilonode/config.h"
#include "kilonode/rx_event.h"

#define KN_RX_SESSION_DEFAULT_MAX 128
#define KN_RX_SESSION_MAX         128

enum kn_rx_session_error {
	KN_RX_SESSION_OK = 0,
	KN_RX_SESSION_ERR_INVALID_ARGUMENT,
	KN_RX_SESSION_ERR_INVALID_VALUE
};

struct kn_rx_session_entry {
	struct kn_callsign source;
	struct kn_callsign destination;
	char port_name[KN_CONFIG_PORT_NAME_MAX];
	uint64_t first_seen;
	uint64_t last_seen;
	uint64_t frame_count;
	uint64_t ui_count;
	uint64_t i_count;
	uint64_t s_count;
	uint64_t u_count;
	uint64_t malformed_count;
	uint8_t last_control;
	uint8_t last_pid;
	uint8_t has_pid;
	uint64_t last_event_id;
};

struct kn_rx_session_table {
	struct kn_rx_session_entry entries[KN_RX_SESSION_MAX];
	size_t max_sessions;
	size_t count;
};

void kn_rx_session_clear(struct kn_rx_session_table *);
size_t kn_rx_session_count(const struct kn_rx_session_table *);
const struct kn_rx_session_entry *kn_rx_session_entries(
	const struct kn_rx_session_table *);
void kn_rx_session_init(struct kn_rx_session_table *, size_t);
enum kn_rx_session_error kn_rx_session_list_by_port(
	const struct kn_rx_session_table *, const char *,
	const struct kn_rx_session_entry **, size_t, size_t *);
enum kn_rx_session_error kn_rx_session_list_by_source(
	const struct kn_rx_session_table *, const struct kn_callsign *,
	const struct kn_rx_session_entry **, size_t, size_t *);
enum kn_rx_session_error kn_rx_session_update(struct kn_rx_session_table *,
	const struct kn_rx_event *);

#endif
