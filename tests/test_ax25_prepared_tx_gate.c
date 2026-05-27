/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_ax25_prepared_tx_gate.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "kilonode/ax25_prepared_tx_gate.h"
#include "kilonode/callsign.h"

static void make_port(struct kn_tx_transport_gate_port *);
static int make_frame(struct kn_ax25_prepared_frame *);
static void make_input(struct kn_ax25_prepared_tx_gate_input *,
	struct kn_ax25_prepared_frame *,
	struct kn_ax25_prepared_tx_policy *, struct kn_tx_policy *,
	struct kn_tx_transport_gate_port *);
static int test_allows_test_control(void);
static int test_blocks_dispatch(void);
static int test_blocks_disabled(void);
static int test_blocks_fx25(void);
static int test_blocks_missing_raw(void);
static int test_blocks_port_tx(void);
static int test_blocks_tx_policy(void);
static int test_reason_names(void);

int
main(void)
{
	if (test_blocks_disabled() != 0)
		return 1;
	if (test_blocks_missing_raw() != 0)
		return 1;
	if (test_blocks_tx_policy() != 0)
		return 1;
	if (test_blocks_port_tx() != 0)
		return 1;
	if (test_blocks_dispatch() != 0)
		return 1;
	if (test_blocks_fx25() != 0)
		return 1;
	if (test_allows_test_control() != 0)
		return 1;
	if (test_reason_names() != 0)
		return 1;

	return 0;
}

static void
make_port(struct kn_tx_transport_gate_port *port)
{
	memset(port, 0, sizeof(*port));
	(void)snprintf(port->name, sizeof(port->name), "%s", "kiss0");
	port->enabled = 1;
	port->open = 1;
	port->tx_enabled = 1;
	port->writable = 1;
}

static int
make_frame(struct kn_ax25_prepared_frame *frame)
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
	return 0;
}

static void
make_input(struct kn_ax25_prepared_tx_gate_input *input,
	struct kn_ax25_prepared_frame *frame,
	struct kn_ax25_prepared_tx_policy *policy,
	struct kn_tx_policy *tx_policy, struct kn_tx_transport_gate_port *port)
{
	kn_ax25_prepared_tx_policy_default(policy);
	kn_tx_policy_defaults(tx_policy);
	tx_policy->enabled = 1;
	make_port(port);
	policy->bridge_enabled = 1;
	policy->allow_control_frames = 1;
	memset(input, 0, sizeof(*input));
	input->prepared = frame;
	input->policy = policy;
	input->tx_policy = tx_policy;
	input->port = port;
}

static int
test_allows_test_control(void)
{
	struct kn_ax25_prepared_frame frame;
	struct kn_ax25_prepared_tx_policy policy;
	struct kn_tx_policy tx_policy;
	struct kn_tx_transport_gate_port port;
	struct kn_ax25_prepared_tx_gate_input input;
	struct kn_ax25_prepared_tx_gate_decision decision;

	if (make_frame(&frame) != 0)
		return 1;
	make_input(&input, &frame, &policy, &tx_policy, &port);
	kn_ax25_prepared_tx_gate_evaluate(&input, &decision);

	return decision.allowed != 0 &&
	    decision.reason == KN_AX25_PREPARED_TX_REASON_ALLOWED_TEST_ONLY ?
	    0 : 1;
}

static int
test_blocks_dispatch(void)
{
	struct kn_ax25_prepared_frame frame;
	struct kn_ax25_prepared_tx_policy policy;
	struct kn_tx_policy tx_policy;
	struct kn_tx_transport_gate_port port;
	struct kn_ax25_prepared_tx_gate_input input;
	struct kn_ax25_prepared_tx_gate_decision decision;

	if (make_frame(&frame) != 0)
		return 1;
	make_input(&input, &frame, &policy, &tx_policy, &port);
	tx_policy.dispatch_enabled = 1;
	kn_ax25_prepared_tx_gate_evaluate(&input, &decision);

	return decision.reason == KN_AX25_PREPARED_TX_REASON_DISPATCH_ENABLED ?
	    0 : 1;
}

static int
test_blocks_disabled(void)
{
	struct kn_ax25_prepared_frame frame;
	struct kn_ax25_prepared_tx_policy policy;
	struct kn_tx_policy tx_policy;
	struct kn_tx_transport_gate_port port;
	struct kn_ax25_prepared_tx_gate_input input;
	struct kn_ax25_prepared_tx_gate_decision decision;

	if (make_frame(&frame) != 0)
		return 1;
	make_input(&input, &frame, &policy, &tx_policy, &port);
	policy.bridge_enabled = 0;
	kn_ax25_prepared_tx_gate_evaluate(&input, &decision);

	return decision.reason == KN_AX25_PREPARED_TX_REASON_BRIDGE_DISABLED ?
	    0 : 1;
}

static int
test_blocks_fx25(void)
{
	struct kn_ax25_prepared_frame frame;
	struct kn_ax25_prepared_tx_policy policy;
	struct kn_tx_policy tx_policy;
	struct kn_tx_transport_gate_port port;
	struct kn_ax25_prepared_tx_gate_input input;
	struct kn_ax25_prepared_tx_gate_decision decision;

	if (make_frame(&frame) != 0)
		return 1;
	make_input(&input, &frame, &policy, &tx_policy, &port);
	frame.needs_fx25 = 1;
	kn_ax25_prepared_tx_gate_evaluate(&input, &decision);

	return decision.reason == KN_AX25_PREPARED_TX_REASON_FX25_NOT_SUPPORTED ?
	    0 : 1;
}

static int
test_blocks_missing_raw(void)
{
	struct kn_ax25_prepared_frame frame;
	struct kn_ax25_prepared_tx_policy policy;
	struct kn_tx_policy tx_policy;
	struct kn_tx_transport_gate_port port;
	struct kn_ax25_prepared_tx_gate_input input;
	struct kn_ax25_prepared_tx_gate_decision decision;

	if (make_frame(&frame) != 0)
		return 1;
	make_input(&input, &frame, &policy, &tx_policy, &port);
	frame.raw_len = 0;
	kn_ax25_prepared_tx_gate_evaluate(&input, &decision);

	return decision.reason == KN_AX25_PREPARED_TX_REASON_RAW_AX25_MISSING ?
	    0 : 1;
}

static int
test_blocks_port_tx(void)
{
	struct kn_ax25_prepared_frame frame;
	struct kn_ax25_prepared_tx_policy policy;
	struct kn_tx_policy tx_policy;
	struct kn_tx_transport_gate_port port;
	struct kn_ax25_prepared_tx_gate_input input;
	struct kn_ax25_prepared_tx_gate_decision decision;

	if (make_frame(&frame) != 0)
		return 1;
	make_input(&input, &frame, &policy, &tx_policy, &port);
	port.tx_enabled = 0;
	kn_ax25_prepared_tx_gate_evaluate(&input, &decision);

	return decision.reason == KN_AX25_PREPARED_TX_REASON_PORT_TX_DISABLED ?
	    0 : 1;
}

static int
test_blocks_tx_policy(void)
{
	struct kn_ax25_prepared_frame frame;
	struct kn_ax25_prepared_tx_policy policy;
	struct kn_tx_policy tx_policy;
	struct kn_tx_transport_gate_port port;
	struct kn_ax25_prepared_tx_gate_input input;
	struct kn_ax25_prepared_tx_gate_decision decision;

	if (make_frame(&frame) != 0)
		return 1;
	make_input(&input, &frame, &policy, &tx_policy, &port);
	tx_policy.enabled = 0;
	kn_ax25_prepared_tx_gate_evaluate(&input, &decision);

	return decision.reason == KN_AX25_PREPARED_TX_REASON_TX_POLICY_DISABLED ?
	    0 : 1;
}

static int
test_reason_names(void)
{
	return strcmp(kn_ax25_prepared_tx_gate_reason_name(
	    KN_AX25_PREPARED_TX_REASON_BRIDGE_DISABLED),
	    "bridge-disabled") == 0 ? 0 : 1;
}
