/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/tx_dispatch.h */

#ifndef KILONODE_TX_DISPATCH_H
#define KILONODE_TX_DISPATCH_H

#include <sys/types.h>

#include <stdint.h>

#include "kilonode/config.h"
#include "kilonode/transport.h"
#include "kilonode/transport_memory.h"
#include "kilonode/tx_queue.h"
#include "kilonode/tx_transport_gate.h"

#define KN_TX_DISPATCH_TARGET_MAX KN_CONFIG_PORT_MAX

enum kn_tx_dispatch_error {
	KN_TX_DISPATCH_OK = 0,
	KN_TX_DISPATCH_ERR_INVALID_ARGUMENT,
	KN_TX_DISPATCH_ERR_DISABLED,
	KN_TX_DISPATCH_ERR_TEST_ONLY_REQUIRED,
	KN_TX_DISPATCH_ERR_NO_SAFE_TARGET,
	KN_TX_DISPATCH_ERR_INVALID_PORT,
	KN_TX_DISPATCH_ERR_GATE_BLOCKED,
	KN_TX_DISPATCH_ERR_WRITE
};

struct kn_tx_dispatch_target {
	char port_name[KN_CONFIG_PORT_NAME_MAX];
	struct kn_transport *transport;
	struct kn_transport_memory *memory;
	enum kn_config_port_type config_type;
	enum kn_transport_kind transport_kind;
	uint8_t enabled;
	uint8_t open;
	uint8_t tx_enabled;
	uint8_t test_safe;
};

struct kn_tx_dispatch_stats {
	uint64_t attempts;
	uint64_t sent;
	uint64_t failed;
	uint64_t blocked;
	uint64_t bytes_written;
};

struct kn_tx_dispatch_result {
	size_t attempted;
	size_t sent;
	size_t failed;
	size_t remaining;
	uint64_t bytes_written;
	enum kn_tx_transport_gate_error gate_error;
};

struct kn_tx_dispatcher {
	struct kn_tx_dispatch_target targets[KN_TX_DISPATCH_TARGET_MAX];
	size_t target_count;
	struct kn_tx_dispatch_stats stats;
};

enum kn_tx_dispatch_error kn_tx_dispatch_add_memory_target(
	struct kn_tx_dispatcher *, const char *, struct kn_transport_memory *,
	uint8_t, uint8_t);
enum kn_tx_dispatch_error kn_tx_dispatch_add_transport_target(
	struct kn_tx_dispatcher *, const char *, struct kn_transport *,
	enum kn_config_port_type, enum kn_transport_kind, uint8_t, uint8_t,
	uint8_t);
void kn_tx_dispatch_clear(struct kn_tx_dispatcher *);
const char *kn_tx_dispatch_error_name(enum kn_tx_dispatch_error);
enum kn_tx_dispatch_error kn_tx_dispatch_run(struct kn_tx_queue *,
	struct kn_tx_dispatcher *, const char *, struct kn_tx_dispatch_result *);
const struct kn_tx_dispatch_target *kn_tx_dispatch_target_find(
	const struct kn_tx_dispatcher *, const char *);

#endif
