/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/heard.h */

#ifndef KILONODE_HEARD_H
#define KILONODE_HEARD_H

#include <sys/types.h>

#include <stdint.h>

#include "kilonode/ax25.h"
#include "kilonode/callsign.h"

#define KN_HEARD_DEFAULT_MAX 256
#define KN_HEARD_MAX_DIGIS  KN_AX25_MAX_DIGIS
#define KN_HEARD_PORT_MAX   32

enum kn_heard_error {
	KN_HEARD_OK = 0,
	KN_HEARD_ERR_INVALID_ARGUMENT
};

struct kn_heard_entry {
	struct kn_callsign source;
	char port_name[KN_HEARD_PORT_MAX];
	uint64_t first_heard;
	uint64_t last_heard;
	uint64_t frame_count;
	struct kn_callsign last_destination;
	struct kn_ax25_addr digipeaters[KN_HEARD_MAX_DIGIS];
	size_t digipeater_count;
	uint8_t last_control;
	uint8_t last_pid;
	uint8_t has_pid;
	size_t last_payload_len;
	uint8_t last_ui;
};

struct kn_heard_table {
	struct kn_heard_entry entries[KN_HEARD_DEFAULT_MAX];
	size_t count;
	size_t max_entries;
};

void kn_heard_clear(struct kn_heard_table *);
size_t kn_heard_count(const struct kn_heard_table *);
const struct kn_heard_entry *kn_heard_entries(const struct kn_heard_table *);
int kn_heard_format_callsign(const struct kn_callsign *, char *, size_t);
void kn_heard_init(struct kn_heard_table *, size_t);
enum kn_heard_error kn_heard_update(struct kn_heard_table *, const char *,
	const struct kn_ax25_frame *, uint64_t);

#endif
