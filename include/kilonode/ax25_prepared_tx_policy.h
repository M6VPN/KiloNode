/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/ax25_prepared_tx_policy.h */

#ifndef KILONODE_AX25_PREPARED_TX_POLICY_H
#define KILONODE_AX25_PREPARED_TX_POLICY_H

#include <sys/types.h>

#include <stdint.h>

#define KN_AX25_PREPARED_TX_MAX_PER_CALL_DEFAULT 4
#define KN_AX25_PREPARED_TX_MAX_PER_CALL_MAX     64

enum kn_ax25_prepared_tx_policy_error {
	KN_AX25_PREPARED_TX_POLICY_OK = 0,
	KN_AX25_PREPARED_TX_POLICY_ERR_INVALID_ARGUMENT,
	KN_AX25_PREPARED_TX_POLICY_ERR_INVALID_VALUE,
	KN_AX25_PREPARED_TX_POLICY_ERR_BUFFER
};

struct kn_ax25_prepared_tx_policy {
	uint8_t bridge_enabled;
	uint8_t test_only;
	uint8_t allow_control_frames;
	uint8_t allow_i_frames;
	uint8_t require_tx_policy_enabled;
	uint8_t require_port_tx_enabled;
	uint8_t require_dispatch_disabled;
	uint8_t require_no_auto_dispatch;
	size_t max_bridge_per_call;
	uint8_t allow_fx25_wrapping;
};

void kn_ax25_prepared_tx_policy_default(
	struct kn_ax25_prepared_tx_policy *);
enum kn_ax25_prepared_tx_policy_error
kn_ax25_prepared_tx_policy_format(
	const struct kn_ax25_prepared_tx_policy *, char *, size_t);
enum kn_ax25_prepared_tx_policy_error
kn_ax25_prepared_tx_policy_validate(
	const struct kn_ax25_prepared_tx_policy *);

#endif
