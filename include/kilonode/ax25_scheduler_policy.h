/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/ax25_scheduler_policy.h */

#ifndef KILONODE_AX25_SCHEDULER_POLICY_H
#define KILONODE_AX25_SCHEDULER_POLICY_H

#include <sys/types.h>

#include <stdint.h>

#define KN_AX25_LIVE_SCHEDULER_EXPIRED_MAX 32
#define KN_AX25_LIVE_SCHEDULER_EXPIRED_DEFAULT 4

enum kn_ax25_scheduler_policy_error {
	KN_AX25_SCHEDULER_POLICY_OK = 0,
	KN_AX25_SCHEDULER_POLICY_ERR_INVALID_ARGUMENT,
	KN_AX25_SCHEDULER_POLICY_ERR_INVALID_VALUE,
	KN_AX25_SCHEDULER_POLICY_ERR_BUFFER
};

struct kn_ax25_scheduler_policy {
	uint8_t enabled;
	uint8_t process_expired;
	size_t max_expired_per_cycle;
	uint8_t tx_actions_enabled;
	uint8_t diagnostics_enabled;
};

void kn_ax25_scheduler_policy_default(struct kn_ax25_scheduler_policy *);
enum kn_ax25_scheduler_policy_error kn_ax25_scheduler_policy_format(
	const struct kn_ax25_scheduler_policy *, char *, size_t);
enum kn_ax25_scheduler_policy_error kn_ax25_scheduler_policy_validate(
	const struct kn_ax25_scheduler_policy *);

#endif
