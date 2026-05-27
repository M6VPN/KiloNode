/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_tx_transport_gate.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "kilonode/tx_transport_gate.h"

static void make_port(struct kn_tx_transport_gate_port *);
static void make_real_policy(struct kn_tx_policy *);
static int test_default_policy_blocks(void);
static int test_memory_rules(void);
static int test_port_gates(void);
static int test_real_policy_gates(void);

int
main(void)
{
	if (test_default_policy_blocks() != 0)
		return 1;
	if (test_real_policy_gates() != 0)
		return 1;
	if (test_port_gates() != 0)
		return 1;
	if (test_memory_rules() != 0)
		return 1;

	return 0;
}

static void
make_port(struct kn_tx_transport_gate_port *port)
{
	memset(port, 0, sizeof(*port));
	(void)snprintf(port->name, sizeof(port->name), "%s", "kiss0");
	port->config_type = KN_CONFIG_PORT_TCP_CONNECT;
	port->transport_kind = KN_TRANSPORT_KIND_TCP_CLIENT;
	port->enabled = 1;
	port->open = 1;
	port->tx_enabled = 1;
	port->writable = 1;
}

static void
make_real_policy(struct kn_tx_policy *policy)
{
	kn_tx_policy_defaults(policy);
	policy->enabled = 1;
	policy->dry_run = 0;
	policy->allow_ui = 1;
	policy->allow_control_enqueue = 1;
	policy->dispatch_enabled = 1;
	policy->dispatch_test_only = 0;
	policy->dispatch_real_kiss = 1;
	policy->require_explicit_port_tx = 1;
}

static int
test_default_policy_blocks(void)
{
	struct kn_tx_policy policy;
	struct kn_tx_transport_gate_port port;

	kn_tx_policy_defaults(&policy);
	make_port(&port);

	return kn_tx_transport_gate_real(&policy, &port) ==
	    KN_TX_TRANSPORT_GATE_ERR_TX_DISABLED ? 0 : 1;
}

static int
test_memory_rules(void)
{
	struct kn_tx_policy policy;
	struct kn_tx_transport_gate_port port;

	kn_tx_policy_defaults(&policy);
	policy.enabled = 1;
	policy.dispatch_enabled = 1;
	policy.dispatch_test_only = 1;
	make_port(&port);
	port.config_type = KN_CONFIG_PORT_MEMORY_TEST;
	port.transport_kind = KN_TRANSPORT_KIND_MEMORY_TEST;
	port.memory_test = 1;

	if (kn_tx_transport_gate_test(&policy, &port) !=
	    KN_TX_TRANSPORT_GATE_OK)
		return 1;

	make_real_policy(&policy);
	return kn_tx_transport_gate_real(&policy, &port) ==
	    KN_TX_TRANSPORT_GATE_ERR_UNSAFE_TRANSPORT ? 0 : 1;
}

static int
test_port_gates(void)
{
	struct kn_tx_policy policy;
	struct kn_tx_transport_gate_port port;

	make_real_policy(&policy);
	make_port(&port);
	port.enabled = 0;
	if (kn_tx_transport_gate_real(&policy, &port) !=
	    KN_TX_TRANSPORT_GATE_ERR_PORT_DISABLED)
		return 1;
	port.enabled = 1;
	port.tx_enabled = 0;
	if (kn_tx_transport_gate_real(&policy, &port) !=
	    KN_TX_TRANSPORT_GATE_ERR_PORT_TX_DISABLED)
		return 1;
	port.tx_enabled = 1;
	port.open = 0;
	if (kn_tx_transport_gate_real(&policy, &port) !=
	    KN_TX_TRANSPORT_GATE_ERR_PORT_NOT_OPEN)
		return 1;
	port.open = 1;
	port.transport_kind = KN_TRANSPORT_KIND_STDIO;

	return kn_tx_transport_gate_real(&policy, &port) ==
	    KN_TX_TRANSPORT_GATE_ERR_UNSAFE_TRANSPORT ? 0 : 1;
}

static int
test_real_policy_gates(void)
{
	struct kn_tx_policy policy;
	struct kn_tx_transport_gate_port port;

	make_real_policy(&policy);
	make_port(&port);
	if (kn_tx_transport_gate_real(&policy, &port) !=
	    KN_TX_TRANSPORT_GATE_OK)
		return 1;
	policy.dry_run = 1;
	if (kn_tx_transport_gate_real(&policy, &port) !=
	    KN_TX_TRANSPORT_GATE_ERR_DRY_RUN_ENABLED)
		return 1;
	policy.dry_run = 0;
	policy.dispatch_test_only = 1;
	if (kn_tx_transport_gate_real(&policy, &port) !=
	    KN_TX_TRANSPORT_GATE_ERR_TEST_ONLY)
		return 1;
	policy.dispatch_test_only = 0;
	policy.dispatch_real_kiss = 0;

	return kn_tx_transport_gate_real(&policy, &port) ==
	    KN_TX_TRANSPORT_GATE_ERR_REAL_KISS_DISABLED ? 0 : 1;
}
