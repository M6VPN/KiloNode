/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/proto/ax25_prepared_tx_bridge.c */

#include <sys/types.h>

#include <stddef.h>

#include "kilonode/ax25_prepared_tx_bridge.h"

enum kn_ax25_prepared_tx_bridge_error
kn_ax25_prepared_tx_bridge_prepare_frame(
	const struct kn_ax25_prepared_tx_gate_input *input,
	const struct kn_ax25_prepared_tx_gate_decision *decision,
	uint64_t tx_id, struct kn_tx_frame *out)
{
	const struct kn_ax25_prepared_frame *prepared;

	if (input == NULL || decision == NULL || out == NULL ||
	    input->prepared == NULL)
		return KN_AX25_PREPARED_TX_BRIDGE_ERR_INVALID_ARGUMENT;
	if (decision->allowed == 0 || decision->would_create_tx_frame == 0)
		return KN_AX25_PREPARED_TX_BRIDGE_ERR_BLOCKED;

	prepared = input->prepared;
	if (kn_tx_frame_build_raw_ax25(out, tx_id, prepared->created_ms,
	    prepared->port_name, 0, 0, prepared->raw, prepared->raw_len,
	    KN_AX25_PREPARED_FRAME_HEX_PREVIEW) != KN_TX_FRAME_OK)
		return KN_AX25_PREPARED_TX_BRIDGE_ERR_BUILD;
	out->status = KN_TX_FRAME_DRY_RUN;

	return KN_AX25_PREPARED_TX_BRIDGE_OK;
}
