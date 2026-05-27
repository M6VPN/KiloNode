/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/compat_coverage.h */

#ifndef KILONODE_COMPAT_COVERAGE_H
#define KILONODE_COMPAT_COVERAGE_H

#include <sys/types.h>

#include <stdint.h>

#include "kilonode/compat_observation_pack.h"

#define KN_COMPAT_COVERAGE_COMMANDS 12
#define KN_COMPAT_COVERAGE_REPORT_MAX 4096

enum kn_compat_coverage_error {
	KN_COMPAT_COVERAGE_OK = 0,
	KN_COMPAT_COVERAGE_ERR_INVALID_ARGUMENT,
	KN_COMPAT_COVERAGE_ERR_BUFFER,
	KN_COMPAT_COVERAGE_ERR_REFERENCE
};

enum kn_compat_coverage_status {
	KN_COMPAT_COVERAGE_NOT_STARTED = 0,
	KN_COMPAT_COVERAGE_SYNTHETIC,
	KN_COMPAT_COVERAGE_OBSERVED,
	KN_COMPAT_COVERAGE_TRANSCRIPT_CANDIDATE,
	KN_COMPAT_COVERAGE_IMPLEMENTED_KILONODE_NATIVE,
	KN_COMPAT_COVERAGE_COMPATIBILITY_PLANNED,
	KN_COMPAT_COVERAGE_OUT_OF_SCOPE_THIS_MILESTONE
};

struct kn_compat_coverage_entry {
	char command[16];
	uint8_t observed;
	uint8_t synthetic;
	uint8_t manual_black_box;
	uint8_t transcript_candidate;
	uint8_t replayable;
	enum kn_compat_coverage_status status;
	char notes[96];
};

struct kn_compat_coverage {
	struct kn_compat_coverage_entry entries[KN_COMPAT_COVERAGE_COMMANDS];
	size_t count;
};

void kn_compat_coverage_clear(struct kn_compat_coverage *);
const char *kn_compat_coverage_error_name(enum kn_compat_coverage_error);
const char *kn_compat_coverage_status_name(
	enum kn_compat_coverage_status);
enum kn_compat_coverage_error kn_compat_coverage_from_pack(
	const struct kn_compat_observation_pack *, struct kn_compat_coverage *);
enum kn_compat_coverage_error kn_compat_coverage_report(
	const struct kn_compat_observation_pack *,
	const struct kn_compat_coverage *, char *, size_t);

#endif
