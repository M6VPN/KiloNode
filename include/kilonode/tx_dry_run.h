/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/tx_dry_run.h */

#ifndef KILONODE_TX_DRY_RUN_H
#define KILONODE_TX_DRY_RUN_H

#include <sys/types.h>

#include <stdint.h>

#include "kilonode/stats.h"
#include "kilonode/tx_queue.h"

enum kn_tx_dry_run_error {
	KN_TX_DRY_RUN_OK = 0,
	KN_TX_DRY_RUN_ERR_INVALID_ARGUMENT,
	KN_TX_DRY_RUN_ERR_DISABLED,
	KN_TX_DRY_RUN_ERR_DRY_RUN_REQUIRED,
	KN_TX_DRY_RUN_ERR_CONTROL_DISABLED,
	KN_TX_DRY_RUN_ERR_SHELL_DISABLED,
	KN_TX_DRY_RUN_ERR_UI_DISABLED,
	KN_TX_DRY_RUN_ERR_INVALID_PORT,
	KN_TX_DRY_RUN_ERR_INVALID_CALLSIGN,
	KN_TX_DRY_RUN_ERR_INVALID_VIA,
	KN_TX_DRY_RUN_ERR_PAYLOAD_TOO_LARGE,
	KN_TX_DRY_RUN_ERR_QUEUE_FULL,
	KN_TX_DRY_RUN_ERR_BUILD
};

enum kn_tx_dry_run_origin {
	KN_TX_DRY_RUN_ORIGIN_CONTROL = 0,
	KN_TX_DRY_RUN_ORIGIN_SHELL
};

enum kn_tx_dry_run_error kn_tx_dry_run_enqueue_ui(struct kn_tx_queue *,
	const struct kn_port_stats *, size_t, enum kn_tx_dry_run_origin,
	uint64_t, const char *, const char *, const char *, const char *,
	uint8_t, const uint8_t *, size_t, uint64_t *);
const char *kn_tx_dry_run_error_name(enum kn_tx_dry_run_error);

#endif
