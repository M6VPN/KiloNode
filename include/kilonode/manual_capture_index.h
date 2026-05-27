/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/manual_capture_index.h */

#ifndef KILONODE_MANUAL_CAPTURE_INDEX_H
#define KILONODE_MANUAL_CAPTURE_INDEX_H

#include <sys/types.h>

#include <stdint.h>

#include "kilonode/manual_capture_workspace.h"

#define KN_MANUAL_CAPTURE_INDEX_MAX 64
#define KN_MANUAL_CAPTURE_NOTES_MAX 128
#define KN_MANUAL_CAPTURE_REPORT_MAX 8192

enum kn_manual_capture_source {
	KN_MANUAL_CAPTURE_SOURCE_MANUAL = 0,
	KN_MANUAL_CAPTURE_SOURCE_SYNTHETIC,
	KN_MANUAL_CAPTURE_SOURCE_BLACK_BOX
};

enum kn_manual_capture_status {
	KN_MANUAL_CAPTURE_STATUS_UNCHECKED = 0,
	KN_MANUAL_CAPTURE_STATUS_VALID,
	KN_MANUAL_CAPTURE_STATUS_INVALID,
	KN_MANUAL_CAPTURE_STATUS_UNSUPPORTED
};

enum kn_manual_capture_replay_status {
	KN_MANUAL_CAPTURE_REPLAY_NOT_RUN = 0,
	KN_MANUAL_CAPTURE_REPLAY_PASS,
	KN_MANUAL_CAPTURE_REPLAY_FAIL,
	KN_MANUAL_CAPTURE_REPLAY_UNSUPPORTED
};

struct kn_manual_capture_entry {
	uint32_t id;
	char file[KN_MANUAL_CAPTURE_FIELD_MAX];
	char method[KN_MANUAL_CAPTURE_FIELD_MAX];
	enum kn_manual_capture_source source;
	enum kn_manual_capture_status status;
	enum kn_manual_capture_replay_status replay;
	uint64_t added;
	char notes[KN_MANUAL_CAPTURE_NOTES_MAX];
};

struct kn_manual_capture_index {
	struct kn_manual_capture_entry entries[KN_MANUAL_CAPTURE_INDEX_MAX];
	size_t entry_count;
	uint32_t next_id;
};

void kn_manual_capture_index_clear(struct kn_manual_capture_index *);
const char *kn_manual_capture_source_name(enum kn_manual_capture_source);
const char *kn_manual_capture_status_name(enum kn_manual_capture_status);
const char *kn_manual_capture_replay_status_name(
	enum kn_manual_capture_replay_status);
enum kn_manual_capture_error kn_manual_capture_source_from_text(
	const char *, enum kn_manual_capture_source *);
enum kn_manual_capture_error kn_manual_capture_status_from_text(
	const char *, enum kn_manual_capture_status *);
enum kn_manual_capture_error kn_manual_capture_replay_status_from_text(
	const char *, enum kn_manual_capture_replay_status *);
enum kn_manual_capture_error kn_manual_capture_index_load(
	const char *, struct kn_manual_capture_index *,
	struct kn_manual_capture_error_info *);
enum kn_manual_capture_error kn_manual_capture_index_save(
	const char *, const struct kn_manual_capture_index *,
	struct kn_manual_capture_error_info *);
enum kn_manual_capture_error kn_manual_capture_index_add(
	struct kn_manual_capture_index *, const struct kn_manual_capture_entry *);
struct kn_manual_capture_entry *kn_manual_capture_index_find(
	struct kn_manual_capture_index *, uint32_t);
enum kn_manual_capture_error kn_manual_capture_index_update_status(
	struct kn_manual_capture_index *, uint32_t,
	enum kn_manual_capture_status);
enum kn_manual_capture_error kn_manual_capture_index_update_replay(
	struct kn_manual_capture_index *, uint32_t,
	enum kn_manual_capture_replay_status);
enum kn_manual_capture_error kn_manual_capture_index_validate_refs(
	const char *, const struct kn_manual_capture_index *,
	struct kn_manual_capture_error_info *);
enum kn_manual_capture_error kn_manual_capture_index_format(
	const struct kn_manual_capture_index *, char *, size_t);

#endif
