/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/compat_risk.h */

#ifndef KILONODE_COMPAT_RISK_H
#define KILONODE_COMPAT_RISK_H

#include <sys/types.h>

#include <stdint.h>

#define KN_COMPAT_RISK_FIELD_MAX  160
#define KN_COMPAT_RISK_MAX        16
#define KN_COMPAT_RISK_REPORT_MAX 8192

enum kn_compat_risk_error {
	KN_COMPAT_RISK_OK = 0,
	KN_COMPAT_RISK_ERR_INVALID_ARGUMENT,
	KN_COMPAT_RISK_ERR_BUFFER,
	KN_COMPAT_RISK_ERR_INVALID_VALUE
};

enum kn_compat_risk_severity {
	KN_COMPAT_RISK_LOW = 0,
	KN_COMPAT_RISK_MEDIUM,
	KN_COMPAT_RISK_HIGH,
	KN_COMPAT_RISK_CRITICAL
};

enum kn_compat_risk_status {
	KN_COMPAT_RISK_OPEN = 0,
	KN_COMPAT_RISK_MITIGATED,
	KN_COMPAT_RISK_ACCEPTED,
	KN_COMPAT_RISK_DEFERRED
};

struct kn_compat_risk {
	char id[32];
	char title[KN_COMPAT_RISK_FIELD_MAX];
	enum kn_compat_risk_severity severity;
	char area[64];
	char mitigation[KN_COMPAT_RISK_FIELD_MAX];
	enum kn_compat_risk_status status;
};

struct kn_compat_risk_register {
	struct kn_compat_risk risks[KN_COMPAT_RISK_MAX];
	size_t risk_count;
};

void kn_compat_risk_register_clear(struct kn_compat_risk_register *);
const char *kn_compat_risk_error_name(enum kn_compat_risk_error);
const char *kn_compat_risk_severity_name(enum kn_compat_risk_severity);
const char *kn_compat_risk_status_name(enum kn_compat_risk_status);
enum kn_compat_risk_error kn_compat_risk_register_default(
	struct kn_compat_risk_register *);
enum kn_compat_risk_error kn_compat_risk_register_report(
	const struct kn_compat_risk_register *, char *, size_t);

#endif
