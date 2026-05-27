/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/tx_policy.h */

#ifndef KILONODE_TX_POLICY_H
#define KILONODE_TX_POLICY_H

#include <sys/types.h>

#include <stdint.h>

#define KN_TX_POLICY_MAX_QUEUED_DEFAULT 128
#define KN_TX_POLICY_MAX_QUEUED_MAX     1024
#define KN_TX_POLICY_MAX_QUEUED_MIN     1
#define KN_TX_POLICY_PAYLOAD_DEFAULT    256
#define KN_TX_POLICY_PAYLOAD_MAX        4096
#define KN_TX_POLICY_PAYLOAD_MIN        0
#define KN_TX_POLICY_PREVIEW_DEFAULT    80
#define KN_TX_POLICY_PREVIEW_MAX        256
#define KN_TX_POLICY_PREVIEW_MIN        1
#define KN_TX_POLICY_DISPATCH_MAX_DEFAULT 4
#define KN_TX_POLICY_DISPATCH_MAX_MAX     64
#define KN_TX_POLICY_DISPATCH_MAX_MIN     1

enum kn_tx_policy_error {
	KN_TX_POLICY_OK = 0,
	KN_TX_POLICY_ERR_INVALID_ARGUMENT,
	KN_TX_POLICY_ERR_DISABLED,
	KN_TX_POLICY_ERR_UI_NOT_ALLOWED,
	KN_TX_POLICY_ERR_TOO_LARGE,
	KN_TX_POLICY_ERR_DISPATCH_DISABLED,
	KN_TX_POLICY_ERR_DISPATCH_TEST_ONLY_REQUIRED,
	KN_TX_POLICY_ERR_CONTROL_DISABLED,
	KN_TX_POLICY_ERR_SHELL_DISABLED
};

struct kn_tx_policy {
	size_t max_queued;
	size_t max_payload_bytes;
	size_t payload_preview_bytes;
	uint8_t enabled;
	uint8_t dry_run;
	uint8_t allow_ui;
	uint8_t allow_control_enqueue;
	uint8_t allow_shell_enqueue;
	uint8_t dispatch_enabled;
	uint8_t dispatch_test_only;
	size_t dispatch_max_per_cycle;
};

enum kn_tx_policy_error kn_tx_policy_allow_dispatch(
	const struct kn_tx_policy *);
enum kn_tx_policy_error kn_tx_policy_allow_control_enqueue(
	const struct kn_tx_policy *, size_t);
enum kn_tx_policy_error kn_tx_policy_allow_enqueue(
	const struct kn_tx_policy *, size_t);
enum kn_tx_policy_error kn_tx_policy_allow_shell_enqueue(
	const struct kn_tx_policy *, size_t);
enum kn_tx_policy_error kn_tx_policy_allow_ui(
	const struct kn_tx_policy *, size_t);
void kn_tx_policy_defaults(struct kn_tx_policy *);
enum kn_tx_policy_error kn_tx_policy_validate(
	const struct kn_tx_policy *);

#endif
