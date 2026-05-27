/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/tx_transport_gate.h */

#ifndef KILONODE_TX_TRANSPORT_GATE_H
#define KILONODE_TX_TRANSPORT_GATE_H

#include <sys/types.h>

#include <stdint.h>

#include "kilonode/config.h"
#include "kilonode/transport.h"
#include "kilonode/tx_policy.h"

enum kn_tx_transport_gate_error {
	KN_TX_TRANSPORT_GATE_OK = 0,
	KN_TX_TRANSPORT_GATE_ERR_INVALID_ARGUMENT,
	KN_TX_TRANSPORT_GATE_ERR_TX_DISABLED,
	KN_TX_TRANSPORT_GATE_ERR_DRY_RUN_ENABLED,
	KN_TX_TRANSPORT_GATE_ERR_DISPATCH_DISABLED,
	KN_TX_TRANSPORT_GATE_ERR_TEST_ONLY,
	KN_TX_TRANSPORT_GATE_ERR_REAL_KISS_DISABLED,
	KN_TX_TRANSPORT_GATE_ERR_EXPLICIT_PORT_TX_REQUIRED,
	KN_TX_TRANSPORT_GATE_ERR_PORT_MISSING,
	KN_TX_TRANSPORT_GATE_ERR_PORT_DISABLED,
	KN_TX_TRANSPORT_GATE_ERR_PORT_TX_DISABLED,
	KN_TX_TRANSPORT_GATE_ERR_PORT_NOT_OPEN,
	KN_TX_TRANSPORT_GATE_ERR_UNSAFE_TRANSPORT,
	KN_TX_TRANSPORT_GATE_ERR_NO_TRANSPORT
};

struct kn_tx_transport_gate_port {
	char name[KN_CONFIG_PORT_NAME_MAX];
	enum kn_config_port_type config_type;
	enum kn_transport_kind transport_kind;
	uint8_t enabled;
	uint8_t open;
	uint8_t tx_enabled;
	uint8_t writable;
	uint8_t memory_test;
};

const char *kn_tx_transport_gate_error_name(
	enum kn_tx_transport_gate_error);
uint8_t kn_tx_transport_gate_kind_real_allowed(enum kn_transport_kind);
uint8_t kn_tx_transport_gate_kind_test_allowed(enum kn_transport_kind);
enum kn_tx_transport_gate_error kn_tx_transport_gate_real(
	const struct kn_tx_policy *,
	const struct kn_tx_transport_gate_port *);
enum kn_tx_transport_gate_error kn_tx_transport_gate_test(
	const struct kn_tx_policy *,
	const struct kn_tx_transport_gate_port *);

#endif
