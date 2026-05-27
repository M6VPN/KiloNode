/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/ax25_retry.h */

#ifndef KILONODE_AX25_RETRY_H
#define KILONODE_AX25_RETRY_H

#include <sys/types.h>

#include <stdint.h>

#define KN_AX25_RETRY_MAX 16

enum kn_ax25_retry_error {
	KN_AX25_RETRY_OK = 0,
	KN_AX25_RETRY_ERR_INVALID_ARGUMENT,
	KN_AX25_RETRY_ERR_INVALID_VALUE,
	KN_AX25_RETRY_ERR_BUFFER
};

struct kn_ax25_retry {
	uint8_t count;
	uint8_t max_retries;
};

uint8_t kn_ax25_retry_exhausted(const struct kn_ax25_retry *);
enum kn_ax25_retry_error kn_ax25_retry_format(
	const struct kn_ax25_retry *, char *, size_t);
enum kn_ax25_retry_error kn_ax25_retry_increment(struct kn_ax25_retry *);
enum kn_ax25_retry_error kn_ax25_retry_init(struct kn_ax25_retry *, uint8_t);
void kn_ax25_retry_reset(struct kn_ax25_retry *);
uint8_t kn_ax25_retry_under_limit(const struct kn_ax25_retry *);
enum kn_ax25_retry_error kn_ax25_retry_validate_max(uint8_t);

#endif
