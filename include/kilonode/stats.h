/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/stats.h */

#ifndef KILONODE_STATS_H
#define KILONODE_STATS_H

#include <sys/types.h>

#include <stdint.h>
#include <time.h>

#include "kilonode/config.h"

#define KN_STATS_ERROR_MAX 64

struct kn_port_stats {
	char name[KN_CONFIG_PORT_NAME_MAX];
	enum kn_config_port_type type;
	uint8_t enabled;
	uint8_t open;
	uint64_t bytes_received;
	uint64_t kiss_frames_received;
	uint64_t ax25_frames_decoded;
	uint64_t malformed_frames;
	uint64_t last_frame_time;
	char last_error[KN_STATS_ERROR_MAX];
};

struct kn_daemon_stats {
	uint64_t start_time;
	uint64_t configured_ports;
	uint64_t enabled_ports;
	uint64_t open_ports;
	uint64_t bytes_received;
	uint64_t kiss_frames_received;
	uint64_t ax25_frames_decoded;
	uint64_t malformed_kiss_frames;
	uint64_t malformed_ax25_frames;
};

void kn_daemon_stats_init(struct kn_daemon_stats *, size_t, size_t);
void kn_port_stats_init(struct kn_port_stats *, const struct kn_config_port *);
void kn_stats_add_ax25_frame(struct kn_daemon_stats *, struct kn_port_stats *);
void kn_stats_add_bytes(struct kn_daemon_stats *, struct kn_port_stats *,
	size_t);
void kn_stats_add_kiss_frame(struct kn_daemon_stats *, struct kn_port_stats *);
void kn_stats_add_malformed_ax25(struct kn_daemon_stats *,
	struct kn_port_stats *, const char *);
void kn_stats_add_malformed_kiss(struct kn_daemon_stats *,
	struct kn_port_stats *, const char *);
const char *kn_stats_port_type_name(enum kn_config_port_type);

#endif
