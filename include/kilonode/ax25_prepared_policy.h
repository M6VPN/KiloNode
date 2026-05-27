/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/ax25_prepared_policy.h */

#ifndef KILONODE_AX25_PREPARED_POLICY_H
#define KILONODE_AX25_PREPARED_POLICY_H

#include <sys/types.h>

#include <stdint.h>

#include "kilonode/ax25_prepared_frame.h"

#define KN_AX25_PREPARED_QUEUE_DEFAULT_MAX 128
#define KN_AX25_PREPARED_QUEUE_MAX         128

enum kn_ax25_prepared_policy_error {
	KN_AX25_PREPARED_POLICY_OK = 0,
	KN_AX25_PREPARED_POLICY_ERR_INVALID_ARGUMENT,
	KN_AX25_PREPARED_POLICY_ERR_INVALID_VALUE,
	KN_AX25_PREPARED_POLICY_ERR_BRIDGE_BLOCKED,
	KN_AX25_PREPARED_POLICY_ERR_BUFFER
};

struct kn_ax25_prepared_policy {
	uint8_t enabled;
	uint8_t build_raw;
	uint8_t bridge_to_tx;
	size_t max_frames;
};

enum kn_ax25_prepared_policy_error kn_ax25_prepared_bridge_to_tx(
	const struct kn_ax25_prepared_frame *);
void kn_ax25_prepared_policy_default(struct kn_ax25_prepared_policy *);
enum kn_ax25_prepared_policy_error kn_ax25_prepared_policy_format(
	const struct kn_ax25_prepared_policy *, char *, size_t);
enum kn_ax25_prepared_policy_error kn_ax25_prepared_policy_validate(
	const struct kn_ax25_prepared_policy *);

#endif
