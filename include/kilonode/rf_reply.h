/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/rf_reply.h */

#ifndef KILONODE_RF_REPLY_H
#define KILONODE_RF_REPLY_H

#include <sys/types.h>

#include <stdint.h>

#include "kilonode/config.h"
#include "kilonode/heard.h"
#include "kilonode/rf_command.h"
#include "kilonode/stats.h"
#include "kilonode/tx_queue.h"

#define KN_RF_REPLY_REASON_MAX 64

enum kn_rf_reply_error {
	KN_RF_REPLY_OK = 0,
	KN_RF_REPLY_ERR_INVALID_ARGUMENT,
	KN_RF_REPLY_ERR_DISABLED,
	KN_RF_REPLY_ERR_TX_BLOCKED,
	KN_RF_REPLY_ERR_BUFFER
};

enum kn_rf_reply_error kn_rf_reply_format(
	const struct kn_rf_command_event *, const struct kn_config *,
	const struct kn_daemon_stats *, const struct kn_port_stats *, size_t,
	const struct kn_heard_entry *, size_t, char *, size_t);
enum kn_rf_reply_error kn_rf_reply_try_queue(
	struct kn_tx_queue *, const struct kn_config *,
	const struct kn_daemon_stats *, const struct kn_port_stats *, size_t,
	const struct kn_heard_entry *, size_t,
	const struct kn_rf_command_event *, uint64_t, uint64_t *, char *,
	size_t);

#endif
