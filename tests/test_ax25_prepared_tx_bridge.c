/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_ax25_prepared_tx_bridge.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "kilonode/ax25_prepared_tx_bridge.h"
#include "kilonode/callsign.h"

static int make_allowed_input(struct kn_ax25_prepared_tx_gate_input *,
	struct kn_ax25_prepared_frame *,
	struct kn_ax25_prepared_tx_policy *, struct kn_tx_policy *,
	struct kn_tx_transport_gate_port *);
static int test_default_blocked(void);
static int test_test_only_conversion(void);

int
main(void)
{
	if (test_default_blocked() != 0)
		return 1;
	if (test_test_only_conversion() != 0)
		return 1;

	return 0;
}

static int
make_allowed_input(struct kn_ax25_prepared_tx_gate_input *input,
	struct kn_ax25_prepared_frame *frame,
	struct kn_ax25_prepared_tx_policy *policy,
	struct kn_tx_policy *tx_policy, struct kn_tx_transport_gate_port *port)
{
	kn_ax25_prepared_frame_clear(frame);
	frame->connection_id = 1;
	(void)snprintf(frame->port_name, sizeof(frame->port_name), "%s",
	    "kiss0");
	if (kn_callsign_parse("M6VPN-1", &frame->local) != 0 ||
	    kn_callsign_parse("N0CALL", &frame->remote) != 0)
		return 1;
	frame->type = KN_AX25_FRAME_PLAN_UA;
	frame->action_source = KN_AX25_ACTION_SEND_UA;
	frame->status = KN_AX25_PREPARED_FRAME_STATUS_PREPARED;
	frame->raw[0] = 0x01;
	frame->raw_len = 1;

	kn_ax25_prepared_tx_policy_default(policy);
	policy->bridge_enabled = 1;
	policy->allow_control_frames = 1;
	kn_tx_policy_defaults(tx_policy);
	tx_policy->enabled = 1;
	memset(port, 0, sizeof(*port));
	(void)snprintf(port->name, sizeof(port->name), "%s", "kiss0");
	port->enabled = 1;
	port->open = 1;
	port->tx_enabled = 1;
	port->writable = 1;
	memset(input, 0, sizeof(*input));
	input->prepared = frame;
	input->policy = policy;
	input->tx_policy = tx_policy;
	input->port = port;
	return 0;
}

static int
test_default_blocked(void)
{
	struct kn_ax25_prepared_tx_gate_input input;
	struct kn_ax25_prepared_tx_gate_decision decision;
	struct kn_tx_frame frame;

	memset(&input, 0, sizeof(input));
	kn_ax25_prepared_tx_gate_decision_clear(&decision);

	return kn_ax25_prepared_tx_bridge_prepare_frame(&input, &decision, 1,
	    &frame) == KN_AX25_PREPARED_TX_BRIDGE_ERR_INVALID_ARGUMENT ?
	    0 : 1;
}

static int
test_test_only_conversion(void)
{
	struct kn_ax25_prepared_frame prepared;
	struct kn_ax25_prepared_tx_policy policy;
	struct kn_tx_policy tx_policy;
	struct kn_tx_transport_gate_port port;
	struct kn_ax25_prepared_tx_gate_input input;
	struct kn_ax25_prepared_tx_gate_decision decision;
	struct kn_tx_frame frame;

	if (make_allowed_input(&input, &prepared, &policy, &tx_policy,
	    &port) != 0)
		return 1;
	kn_ax25_prepared_tx_gate_evaluate(&input, &decision);
	if (decision.allowed == 0)
		return 1;
	if (kn_ax25_prepared_tx_bridge_prepare_frame(&input, &decision, 7,
	    &frame) != KN_AX25_PREPARED_TX_BRIDGE_OK)
		return 1;
	if (frame.id != 7 || frame.kind != KN_TX_FRAME_RAW_AX25 ||
	    frame.status != KN_TX_FRAME_DRY_RUN)
		return 1;

	return frame.kiss_len > frame.ax25_len ? 0 : 1;
}
