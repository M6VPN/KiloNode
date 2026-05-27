/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/rf_command.h */

#ifndef KILONODE_RF_COMMAND_H
#define KILONODE_RF_COMMAND_H

#include <sys/types.h>

#include <stdint.h>

#include "kilonode/config.h"
#include "kilonode/rx_event.h"
#include "kilonode/stats.h"

#define KN_RF_COMMAND_ERROR_MAX 64
#define KN_RF_COMMAND_RAW_MAX   (KN_CONFIG_RF_COMMAND_BYTES_MAX + 1)

enum kn_rf_command_error {
	KN_RF_COMMAND_OK = 0,
	KN_RF_COMMAND_ERR_INVALID_ARGUMENT,
	KN_RF_COMMAND_ERR_IGNORED,
	KN_RF_COMMAND_ERR_BUFFER
};

enum kn_rf_command_name {
	KN_RF_COMMAND_HELP = 0,
	KN_RF_COMMAND_INFO,
	KN_RF_COMMAND_PORTS,
	KN_RF_COMMAND_HEARD,
	KN_RF_COMMAND_STATS,
	KN_RF_COMMAND_PING,
	KN_RF_COMMAND_UNKNOWN
};

enum kn_rf_command_status {
	KN_RF_COMMAND_STATUS_OK = 0,
	KN_RF_COMMAND_STATUS_DISABLED,
	KN_RF_COMMAND_STATUS_IGNORED,
	KN_RF_COMMAND_STATUS_NOT_UI,
	KN_RF_COMMAND_STATUS_BAD_PID,
	KN_RF_COMMAND_STATUS_OVERLONG,
	KN_RF_COMMAND_STATUS_EMPTY,
	KN_RF_COMMAND_STATUS_UNKNOWN,
	KN_RF_COMMAND_STATUS_ARGUMENTS,
	KN_RF_COMMAND_STATUS_CONTROL,
	KN_RF_COMMAND_STATUS_BINARY,
	KN_RF_COMMAND_STATUS_DESTINATION,
	KN_RF_COMMAND_STATUS_INVALID_SOURCE,
	KN_RF_COMMAND_STATUS_INVALID_PORT,
	KN_RF_COMMAND_STATUS_RATE_LIMITED,
	KN_RF_COMMAND_STATUS_REPLY_SUPPRESSED,
	KN_RF_COMMAND_STATUS_TX_GATE_BLOCKED,
	KN_RF_COMMAND_STATUS_MALFORMED
};

struct kn_rf_command_event {
	uint64_t id;
	uint64_t timestamp;
	uint64_t rx_event_id;
	char port_name[KN_CONFIG_PORT_NAME_MAX];
	struct kn_callsign source;
	struct kn_callsign destination;
	enum kn_rf_command_name command;
	char raw[KN_RF_COMMAND_RAW_MAX];
	enum kn_rf_command_status status;
	uint8_t reply_queued;
	uint64_t tx_frame_id;
	char error[KN_RF_COMMAND_ERROR_MAX];
};

void kn_rf_command_event_clear(struct kn_rf_command_event *);
enum kn_rf_command_error kn_rf_command_from_rx(
	struct kn_rf_command_event *, uint64_t, uint64_t,
	const struct kn_config *, const struct kn_port_stats *, size_t,
	const struct kn_rx_event *, const uint8_t *, size_t);
enum kn_rf_command_error kn_rf_command_format_brief(
	const struct kn_rf_command_event *, char *, size_t);
enum kn_rf_command_error kn_rf_command_format_full(
	const struct kn_rf_command_event *, char *, size_t);
const char *kn_rf_command_name_string(enum kn_rf_command_name);
enum kn_rf_command_error kn_rf_command_parse(const uint8_t *, size_t,
	size_t, enum kn_rf_command_name *, char *, size_t,
	enum kn_rf_command_status *);
const char *kn_rf_command_status_string(enum kn_rf_command_status);

#endif
