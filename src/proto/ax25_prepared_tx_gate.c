/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/proto/ax25_prepared_tx_gate.c */

#include <sys/types.h>

#include <string.h>

#include "kilonode/ax25_prepared_tx_gate.h"

static uint8_t prepared_is_control(
	const struct kn_ax25_prepared_frame *);

static uint8_t
prepared_is_control(const struct kn_ax25_prepared_frame *frame)
{
	if (frame == NULL)
		return 0;

	return frame->type != KN_AX25_FRAME_PLAN_UI &&
	    frame->type != KN_AX25_FRAME_PLAN_UNKNOWN ? 1 : 0;
}

void
kn_ax25_prepared_tx_gate_decision_clear(
	struct kn_ax25_prepared_tx_gate_decision *decision)
{
	if (decision == NULL)
		return;

	memset(decision, 0, sizeof(*decision));
	decision->reason = KN_AX25_PREPARED_TX_REASON_BRIDGE_DISABLED;
}

void
kn_ax25_prepared_tx_gate_evaluate(
	const struct kn_ax25_prepared_tx_gate_input *input,
	struct kn_ax25_prepared_tx_gate_decision *decision)
{
	const struct kn_ax25_prepared_tx_policy *policy;
	const struct kn_ax25_prepared_frame *frame;
	const struct kn_tx_policy *tx_policy;
	const struct kn_tx_transport_gate_port *port;

	if (decision == NULL)
		return;
	kn_ax25_prepared_tx_gate_decision_clear(decision);
	if (input == NULL || input->policy == NULL ||
	    input->prepared == NULL) {
		decision->reason =
		    KN_AX25_PREPARED_TX_REASON_INVALID_PREPARED_FRAME;
		return;
	}

	policy = input->policy;
	frame = input->prepared;
	tx_policy = input->tx_policy;
	port = input->port;
	decision->would_require_fx25 = frame->needs_fx25 != 0 ? 1 : 0;
	decision->would_require_kiss = frame->needs_kiss != 0 ? 1 : 0;

	if (kn_ax25_prepared_tx_policy_validate(policy) !=
	    KN_AX25_PREPARED_TX_POLICY_OK ||
	    kn_ax25_prepared_frame_validate(frame) !=
	    KN_AX25_PREPARED_FRAME_OK ||
	    frame->status != KN_AX25_PREPARED_FRAME_STATUS_PREPARED) {
		decision->reason =
		    KN_AX25_PREPARED_TX_REASON_INVALID_PREPARED_FRAME;
		return;
	}
	if (policy->bridge_enabled == 0) {
		decision->reason =
		    KN_AX25_PREPARED_TX_REASON_BRIDGE_DISABLED;
		return;
	}
	if (policy->test_only == 0) {
		decision->reason =
		    KN_AX25_PREPARED_TX_REASON_TEST_ONLY_REQUIRED;
		return;
	}
	if (frame->needs_fx25 != 0 ||
	    policy->allow_fx25_wrapping != 0) {
		decision->reason =
		    KN_AX25_PREPARED_TX_REASON_FX25_NOT_SUPPORTED;
		return;
	}
	if (frame->raw_len == 0) {
		decision->reason =
		    KN_AX25_PREPARED_TX_REASON_RAW_AX25_MISSING;
		return;
	}
	if (prepared_is_control(frame) != 0 &&
	    policy->allow_control_frames == 0) {
		decision->reason =
		    KN_AX25_PREPARED_TX_REASON_CONTROL_FRAMES_DISABLED;
		return;
	}
	if (prepared_is_control(frame) == 0 &&
	    policy->allow_i_frames == 0) {
		decision->reason =
		    KN_AX25_PREPARED_TX_REASON_I_FRAMES_DISABLED;
		return;
	}
	if (policy->require_tx_policy_enabled != 0 &&
	    (tx_policy == NULL || tx_policy->enabled == 0)) {
		decision->reason =
		    KN_AX25_PREPARED_TX_REASON_TX_POLICY_DISABLED;
		return;
	}
	if (tx_policy != NULL && policy->require_dispatch_disabled != 0 &&
	    tx_policy->dispatch_enabled != 0) {
		decision->reason =
		    KN_AX25_PREPARED_TX_REASON_DISPATCH_ENABLED;
		return;
	}
	if (tx_policy != NULL && policy->require_no_auto_dispatch != 0 &&
	    tx_policy->dispatch_enabled != 0) {
		decision->reason =
		    KN_AX25_PREPARED_TX_REASON_AUTO_DISPATCH_NOT_ALLOWED;
		return;
	}
	if (policy->require_port_tx_enabled != 0 &&
	    (port == NULL || port->enabled == 0 || port->tx_enabled == 0 ||
	    port->open == 0 || port->writable == 0)) {
		decision->reason =
		    KN_AX25_PREPARED_TX_REASON_PORT_TX_DISABLED;
		return;
	}

	decision->allowed = 1;
	decision->would_create_tx_frame = 1;
	decision->reason = KN_AX25_PREPARED_TX_REASON_ALLOWED_TEST_ONLY;
}

const char *
kn_ax25_prepared_tx_gate_reason_name(
	enum kn_ax25_prepared_tx_gate_reason reason)
{
	switch (reason) {
	case KN_AX25_PREPARED_TX_REASON_ALLOWED_TEST_ONLY:
		return "allowed-test-only";
	case KN_AX25_PREPARED_TX_REASON_BRIDGE_DISABLED:
		return "bridge-disabled";
	case KN_AX25_PREPARED_TX_REASON_TEST_ONLY_REQUIRED:
		return "test-only-required";
	case KN_AX25_PREPARED_TX_REASON_CONTROL_FRAMES_DISABLED:
		return "control-frames-disabled";
	case KN_AX25_PREPARED_TX_REASON_I_FRAMES_DISABLED:
		return "i-frames-disabled";
	case KN_AX25_PREPARED_TX_REASON_INVALID_PREPARED_FRAME:
		return "invalid-prepared-frame";
	case KN_AX25_PREPARED_TX_REASON_RAW_AX25_MISSING:
		return "raw-ax25-missing";
	case KN_AX25_PREPARED_TX_REASON_TX_POLICY_DISABLED:
		return "tx-policy-disabled";
	case KN_AX25_PREPARED_TX_REASON_PORT_TX_DISABLED:
		return "port-tx-disabled";
	case KN_AX25_PREPARED_TX_REASON_DISPATCH_ENABLED:
		return "dispatch-enabled";
	case KN_AX25_PREPARED_TX_REASON_AUTO_DISPATCH_NOT_ALLOWED:
		return "auto-dispatch-not-allowed";
	case KN_AX25_PREPARED_TX_REASON_FX25_NOT_SUPPORTED:
		return "fx25-not-supported";
	}

	return "unknown";
}
