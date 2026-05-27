/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/ax25_prepared_tx_bridge.h */

#ifndef KILONODE_AX25_PREPARED_TX_BRIDGE_H
#define KILONODE_AX25_PREPARED_TX_BRIDGE_H

#include <sys/types.h>

#include <stdint.h>

#include "kilonode/ax25_prepared_tx_gate.h"
#include "kilonode/tx_frame.h"

enum kn_ax25_prepared_tx_bridge_error {
	KN_AX25_PREPARED_TX_BRIDGE_OK = 0,
	KN_AX25_PREPARED_TX_BRIDGE_ERR_INVALID_ARGUMENT,
	KN_AX25_PREPARED_TX_BRIDGE_ERR_BLOCKED,
	KN_AX25_PREPARED_TX_BRIDGE_ERR_BUILD
};

enum kn_ax25_prepared_tx_bridge_error
kn_ax25_prepared_tx_bridge_prepare_frame(
	const struct kn_ax25_prepared_tx_gate_input *,
	const struct kn_ax25_prepared_tx_gate_decision *, uint64_t,
	struct kn_tx_frame *);

#endif
