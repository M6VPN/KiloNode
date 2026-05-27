/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/compat_requirements.h */

#ifndef KILONODE_COMPAT_REQUIREMENTS_H
#define KILONODE_COMPAT_REQUIREMENTS_H

#include <sys/types.h>

#include <stdint.h>

#include "kilonode/compat_observation_pack.h"

#define KN_COMPAT_REQ_COMMAND_MAX 32
#define KN_COMPAT_REQ_ERROR_MAX   160
#define KN_COMPAT_REQ_FIELD_MAX   256
#define KN_COMPAT_REQ_LINE_MAX    512
#define KN_COMPAT_REQ_MAX         32
#define KN_COMPAT_REQ_REPORT_MAX  8192
#define KN_COMPAT_REQ_TEXT_MAX    16384

enum kn_compat_req_error {
	KN_COMPAT_REQ_OK = 0,
	KN_COMPAT_REQ_ERR_INVALID_ARGUMENT,
	KN_COMPAT_REQ_ERR_IO,
	KN_COMPAT_REQ_ERR_LINE_TOO_LONG,
	KN_COMPAT_REQ_ERR_PARSE,
	KN_COMPAT_REQ_ERR_DUPLICATE_KEY,
	KN_COMPAT_REQ_ERR_UNKNOWN_KEY,
	KN_COMPAT_REQ_ERR_MISSING_REQUIRED,
	KN_COMPAT_REQ_ERR_INVALID_VALUE,
	KN_COMPAT_REQ_ERR_TOO_MANY,
	KN_COMPAT_REQ_ERR_REFERENCE,
	KN_COMPAT_REQ_ERR_BUFFER
};

enum kn_compat_req_status {
	KN_COMPAT_REQ_STATUS_PLANNED = 0,
	KN_COMPAT_REQ_STATUS_BLOCKED,
	KN_COMPAT_REQ_STATUS_NEEDS_OBSERVATION,
	KN_COMPAT_REQ_STATUS_READY_FOR_DESIGN,
	KN_COMPAT_REQ_STATUS_READY_FOR_IMPLEMENTATION,
	KN_COMPAT_REQ_STATUS_IMPLEMENTED_NATIVE,
	KN_COMPAT_REQ_STATUS_IMPLEMENTED_COMPATIBLE,
	KN_COMPAT_REQ_STATUS_OUT_OF_SCOPE
};

enum kn_compat_req_priority {
	KN_COMPAT_REQ_PRIORITY_LOW = 0,
	KN_COMPAT_REQ_PRIORITY_MEDIUM,
	KN_COMPAT_REQ_PRIORITY_HIGH,
	KN_COMPAT_REQ_PRIORITY_CRITICAL
};

struct kn_compat_requirement {
	char command[KN_COMPAT_REQ_COMMAND_MAX];
	enum kn_compat_req_status status;
	enum kn_compat_req_priority priority;
	char observed[KN_COMPAT_REQ_FIELD_MAX];
	char mode[KN_COMPAT_REQ_FIELD_MAX];
	char notes[KN_COMPAT_REQ_FIELD_MAX];
	uint8_t has_status;
	uint8_t has_priority;
};

struct kn_compat_requirements {
	char path[KN_COMPAT_REQ_FIELD_MAX];
	char base_dir[KN_COMPAT_REQ_FIELD_MAX];
	char name[KN_COMPAT_REQ_FIELD_MAX];
	char subject[KN_COMPAT_REQ_FIELD_MAX];
	char source_pack[KN_COMPAT_REQ_FIELD_MAX];
	uint8_t clean_room;
	uint8_t source_code_used;
	struct kn_compat_requirement requirements[KN_COMPAT_REQ_MAX];
	size_t requirement_count;
};

struct kn_compat_req_error_info {
	enum kn_compat_req_error error;
	size_t line;
	char message[KN_COMPAT_REQ_ERROR_MAX];
};

void kn_compat_requirements_clear(struct kn_compat_requirements *);
const char *kn_compat_req_error_name(enum kn_compat_req_error);
const char *kn_compat_req_priority_name(enum kn_compat_req_priority);
const char *kn_compat_req_status_name(enum kn_compat_req_status);
const struct kn_compat_requirement *kn_compat_requirements_find(
	const struct kn_compat_requirements *, const char *);
enum kn_compat_req_error kn_compat_requirements_parse_file(
	const char *, struct kn_compat_requirements *,
	struct kn_compat_req_error_info *);
enum kn_compat_req_error kn_compat_requirements_parse_text(
	const char *, struct kn_compat_requirements *,
	struct kn_compat_req_error_info *);
enum kn_compat_req_error kn_compat_requirements_report(
	const struct kn_compat_requirements *, char *, size_t);
enum kn_compat_req_error kn_compat_requirements_coverage_report(
	const struct kn_compat_requirements *,
	const struct kn_compat_observation_pack *, char *, size_t);
enum kn_compat_req_error kn_compat_requirements_generate_from_pack(
	const struct kn_compat_observation_pack *, char *, size_t);

#endif
