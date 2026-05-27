/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/ax25_prepared_tx_gate.h */

#ifndef KILONODE_AX25_PREPARED_TX_GATE_H
#define KILONODE_AX25_PREPARED_TX_GATE_H

#include <sys/types.h>

#include <stdint.h>

#include "kilonode/ax25_prepared_frame.h"
#include "kilonode/ax25_prepared_tx_policy.h"
#include "kilonode/tx_policy.h"
#include "kilonode/tx_transport_gate.h"

enum kn_ax25_prepared_tx_gate_reason {
	KN_AX25_PREPARED_TX_REASON_ALLOWED_TEST_ONLY = 0,
	KN_AX25_PREPARED_TX_REASON_BRIDGE_DISABLED,
	KN_AX25_PREPARED_TX_REASON_TEST_ONLY_REQUIRED,
	KN_AX25_PREPARED_TX_REASON_CONTROL_FRAMES_DISABLED,
	KN_AX25_PREPARED_TX_REASON_I_FRAMES_DISABLED,
	KN_AX25_PREPARED_TX_REASON_INVALID_PREPARED_FRAME,
	KN_AX25_PREPARED_TX_REASON_RAW_AX25_MISSING,
	KN_AX25_PREPARED_TX_REASON_TX_POLICY_DISABLED,
	KN_AX25_PREPARED_TX_REASON_PORT_TX_DISABLED,
	KN_AX25_PREPARED_TX_REASON_DISPATCH_ENABLED,
	KN_AX25_PREPARED_TX_REASON_AUTO_DISPATCH_NOT_ALLOWED,
	KN_AX25_PREPARED_TX_REASON_FX25_NOT_SUPPORTED
};

struct kn_ax25_prepared_tx_gate_input {
	const struct kn_ax25_prepared_frame *prepared;
	const struct kn_ax25_prepared_tx_policy *policy;
	const struct kn_tx_policy *tx_policy;
	const struct kn_tx_transport_gate_port *port;
};

struct kn_ax25_prepared_tx_gate_decision {
	uint8_t allowed;
	enum kn_ax25_prepared_tx_gate_reason reason;
	uint8_t would_create_tx_frame;
	uint8_t would_require_fx25;
	uint8_t would_require_kiss;
};

void kn_ax25_prepared_tx_gate_decision_clear(
	struct kn_ax25_prepared_tx_gate_decision *);
void kn_ax25_prepared_tx_gate_evaluate(
	const struct kn_ax25_prepared_tx_gate_input *,
	struct kn_ax25_prepared_tx_gate_decision *);
const char *kn_ax25_prepared_tx_gate_reason_name(
	enum kn_ax25_prepared_tx_gate_reason);

#endif
