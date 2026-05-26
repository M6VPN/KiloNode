/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/bbs_store_maintenance.h */

#ifndef KILONODE_BBS_STORE_MAINTENANCE_H
#define KILONODE_BBS_STORE_MAINTENANCE_H

#include <sys/types.h>

#include <stdint.h>

#include "kilonode/message_store.h"

#define KN_BBS_STORE_FINDING_CODE_MAX 32
#define KN_BBS_STORE_FINDING_MSG_MAX 160
#define KN_BBS_STORE_FINDING_PATH_MAX KN_MESSAGE_STORE_PATH_MAX

enum kn_bbs_store_maintenance_error {
	KN_BBS_STORE_MAINTENANCE_OK = 0,
	KN_BBS_STORE_MAINTENANCE_ERR_INVALID_ARGUMENT,
	KN_BBS_STORE_MAINTENANCE_ERR_IO,
	KN_BBS_STORE_MAINTENANCE_ERR_LOCK,
	KN_BBS_STORE_MAINTENANCE_ERR_BUFFER,
	KN_BBS_STORE_MAINTENANCE_ERR_CORRUPT
};

enum kn_bbs_store_finding_severity {
	KN_BBS_STORE_FINDING_INFO = 0,
	KN_BBS_STORE_FINDING_WARNING,
	KN_BBS_STORE_FINDING_ERROR
};

struct kn_bbs_store_finding {
	enum kn_bbs_store_finding_severity severity;
	char code[KN_BBS_STORE_FINDING_CODE_MAX];
	char path[KN_BBS_STORE_FINDING_PATH_MAX];
	char message[KN_BBS_STORE_FINDING_MSG_MAX];
	uint64_t message_id;
};

struct kn_bbs_store_stats {
	uint64_t total_messages;
	uint64_t private_messages;
	uint64_t bulletins;
	uint64_t deleted_messages;
	uint64_t users;
	uint64_t read_state_files;
	uint64_t bulletin_areas;
	uint64_t total_body_bytes;
	uint64_t newest_message_id;
	uint64_t next_id;
};

enum kn_bbs_store_maintenance_error kn_bbs_store_check(const char *,
	struct kn_bbs_store_finding *, size_t, size_t *, size_t *);
const char *kn_bbs_store_maintenance_error_name(
	enum kn_bbs_store_maintenance_error);
const char *kn_bbs_store_finding_severity_name(
	enum kn_bbs_store_finding_severity);
enum kn_bbs_store_maintenance_error kn_bbs_store_export(const char *,
	const char *);
enum kn_bbs_store_maintenance_error kn_bbs_store_purge_deleted(const char *,
	size_t *);
enum kn_bbs_store_maintenance_error kn_bbs_store_repair(const char *,
	struct kn_bbs_store_finding *, size_t, size_t *);
enum kn_bbs_store_maintenance_error kn_bbs_store_stats(const char *,
	struct kn_bbs_store_stats *);

#endif
