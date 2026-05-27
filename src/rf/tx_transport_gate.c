/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/rf/tx_transport_gate.c */

#include <sys/types.h>

#include <stddef.h>

#include "kilonode/tx_transport_gate.h"

const char *
kn_tx_transport_gate_error_name(enum kn_tx_transport_gate_error error)
{
	switch (error) {
	case KN_TX_TRANSPORT_GATE_OK:
		return "ok";
	case KN_TX_TRANSPORT_GATE_ERR_INVALID_ARGUMENT:
		return "invalid-argument";
	case KN_TX_TRANSPORT_GATE_ERR_TX_DISABLED:
		return "tx-disabled";
	case KN_TX_TRANSPORT_GATE_ERR_DRY_RUN_ENABLED:
		return "dry-run-enabled";
	case KN_TX_TRANSPORT_GATE_ERR_DISPATCH_DISABLED:
		return "tx-dispatch-disabled";
	case KN_TX_TRANSPORT_GATE_ERR_TEST_ONLY:
		return "tx-dispatch-test-only";
	case KN_TX_TRANSPORT_GATE_ERR_REAL_KISS_DISABLED:
		return "tx-real-kiss-disabled";
	case KN_TX_TRANSPORT_GATE_ERR_EXPLICIT_PORT_TX_REQUIRED:
		return "explicit-port-tx-required";
	case KN_TX_TRANSPORT_GATE_ERR_PORT_MISSING:
		return "port-missing";
	case KN_TX_TRANSPORT_GATE_ERR_PORT_DISABLED:
		return "port-disabled";
	case KN_TX_TRANSPORT_GATE_ERR_PORT_TX_DISABLED:
		return "port-tx-disabled";
	case KN_TX_TRANSPORT_GATE_ERR_PORT_NOT_OPEN:
		return "port-not-open";
	case KN_TX_TRANSPORT_GATE_ERR_UNSAFE_TRANSPORT:
		return "unsafe-transport";
	case KN_TX_TRANSPORT_GATE_ERR_NO_TRANSPORT:
		return "no-transport";
	}

	return "tx-gate-error";
}

uint8_t
kn_tx_transport_gate_kind_real_allowed(enum kn_transport_kind kind)
{
	switch (kind) {
	case KN_TRANSPORT_KIND_TCP_CLIENT:
	case KN_TRANSPORT_KIND_TCP_SERVER:
	case KN_TRANSPORT_KIND_SERIAL:
	case KN_TRANSPORT_KIND_PTY:
	case KN_TRANSPORT_KIND_UNIX_CLIENT:
	case KN_TRANSPORT_KIND_UNIX_SERVER:
		return 1;
	case KN_TRANSPORT_KIND_NONE:
	case KN_TRANSPORT_KIND_STDIO:
	case KN_TRANSPORT_KIND_MEMORY_TEST:
		return 0;
	}

	return 0;
}

uint8_t
kn_tx_transport_gate_kind_test_allowed(enum kn_transport_kind kind)
{
	if (kind == KN_TRANSPORT_KIND_MEMORY_TEST)
		return 1;

	return 0;
}

enum kn_tx_transport_gate_error
kn_tx_transport_gate_real(const struct kn_tx_policy *policy,
	const struct kn_tx_transport_gate_port *port)
{
	if (policy == NULL)
		return KN_TX_TRANSPORT_GATE_ERR_INVALID_ARGUMENT;
	if (policy->enabled == 0)
		return KN_TX_TRANSPORT_GATE_ERR_TX_DISABLED;
	if (policy->dispatch_enabled == 0)
		return KN_TX_TRANSPORT_GATE_ERR_DISPATCH_DISABLED;
	if (policy->dry_run != 0)
		return KN_TX_TRANSPORT_GATE_ERR_DRY_RUN_ENABLED;
	if (policy->dispatch_test_only != 0)
		return KN_TX_TRANSPORT_GATE_ERR_TEST_ONLY;
	if (policy->dispatch_real_kiss == 0)
		return KN_TX_TRANSPORT_GATE_ERR_REAL_KISS_DISABLED;
	if (policy->require_explicit_port_tx == 0)
		return KN_TX_TRANSPORT_GATE_ERR_EXPLICIT_PORT_TX_REQUIRED;
	if (port == NULL)
		return KN_TX_TRANSPORT_GATE_ERR_PORT_MISSING;
	if (port->enabled == 0)
		return KN_TX_TRANSPORT_GATE_ERR_PORT_DISABLED;
	if (port->tx_enabled == 0)
		return KN_TX_TRANSPORT_GATE_ERR_PORT_TX_DISABLED;
	if (port->open == 0)
		return KN_TX_TRANSPORT_GATE_ERR_PORT_NOT_OPEN;
	if (port->writable == 0)
		return KN_TX_TRANSPORT_GATE_ERR_NO_TRANSPORT;
	if (kn_tx_transport_gate_kind_real_allowed(port->transport_kind) == 0)
		return KN_TX_TRANSPORT_GATE_ERR_UNSAFE_TRANSPORT;

	return KN_TX_TRANSPORT_GATE_OK;
}

enum kn_tx_transport_gate_error
kn_tx_transport_gate_test(const struct kn_tx_policy *policy,
	const struct kn_tx_transport_gate_port *port)
{
	if (policy == NULL)
		return KN_TX_TRANSPORT_GATE_ERR_INVALID_ARGUMENT;
	if (policy->enabled == 0)
		return KN_TX_TRANSPORT_GATE_ERR_TX_DISABLED;
	if (policy->dispatch_enabled == 0)
		return KN_TX_TRANSPORT_GATE_ERR_DISPATCH_DISABLED;
	if (policy->dispatch_test_only == 0)
		return KN_TX_TRANSPORT_GATE_ERR_TEST_ONLY;
	if (port == NULL)
		return KN_TX_TRANSPORT_GATE_ERR_PORT_MISSING;
	if (port->enabled == 0)
		return KN_TX_TRANSPORT_GATE_ERR_PORT_DISABLED;
	if (port->open == 0)
		return KN_TX_TRANSPORT_GATE_ERR_PORT_NOT_OPEN;
	if (port->writable == 0)
		return KN_TX_TRANSPORT_GATE_ERR_NO_TRANSPORT;
	if (kn_tx_transport_gate_kind_test_allowed(port->transport_kind) == 0)
		return KN_TX_TRANSPORT_GATE_ERR_UNSAFE_TRANSPORT;

	return KN_TX_TRANSPORT_GATE_OK;
}
