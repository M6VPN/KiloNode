/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/compat_observation_pack.h */

#ifndef KILONODE_COMPAT_OBSERVATION_PACK_H
#define KILONODE_COMPAT_OBSERVATION_PACK_H

#include <sys/types.h>

#include <stdint.h>

#define KN_COMPAT_PACK_ERROR_MAX       160
#define KN_COMPAT_PACK_FIELD_MAX       256
#define KN_COMPAT_PACK_FIXTURE_MAX     32
#define KN_COMPAT_PACK_LINE_MAX        512
#define KN_COMPAT_PACK_NAME_MAX        96
#define KN_COMPAT_PACK_REPORT_MAX      4096
#define KN_COMPAT_PACK_TEXT_MAX        8192

enum kn_compat_pack_error {
	KN_COMPAT_PACK_OK = 0,
	KN_COMPAT_PACK_ERR_INVALID_ARGUMENT,
	KN_COMPAT_PACK_ERR_IO,
	KN_COMPAT_PACK_ERR_LINE_TOO_LONG,
	KN_COMPAT_PACK_ERR_PARSE,
	KN_COMPAT_PACK_ERR_DUPLICATE_KEY,
	KN_COMPAT_PACK_ERR_UNKNOWN_KEY,
	KN_COMPAT_PACK_ERR_MISSING_REQUIRED,
	KN_COMPAT_PACK_ERR_INVALID_VALUE,
	KN_COMPAT_PACK_ERR_TOO_MANY,
	KN_COMPAT_PACK_ERR_REFERENCE
};

struct kn_compat_pack_entry {
	char path[KN_COMPAT_PACK_FIELD_MAX];
};

struct kn_compat_observation_pack {
	char manifest_path[KN_COMPAT_PACK_FIELD_MAX];
	char base_dir[KN_COMPAT_PACK_FIELD_MAX];
	char name[KN_COMPAT_PACK_NAME_MAX];
	char subject[KN_COMPAT_PACK_FIELD_MAX];
	char type[KN_COMPAT_PACK_FIELD_MAX];
	char source[KN_COMPAT_PACK_FIELD_MAX];
	char created[32];
	uint8_t clean_room;
	uint8_t source_code_used;
	char notes[KN_COMPAT_PACK_FIELD_MAX];
	struct kn_compat_pack_entry fixtures[KN_COMPAT_PACK_FIXTURE_MAX];
	size_t fixture_count;
	struct kn_compat_pack_entry transcripts[KN_COMPAT_PACK_FIXTURE_MAX];
	size_t transcript_count;
};

struct kn_compat_pack_error_info {
	enum kn_compat_pack_error error;
	size_t line;
	char message[KN_COMPAT_PACK_ERROR_MAX];
};

void kn_compat_observation_pack_clear(struct kn_compat_observation_pack *);
const char *kn_compat_pack_error_name(enum kn_compat_pack_error);
enum kn_compat_pack_error kn_compat_observation_pack_parse_file(
	const char *, struct kn_compat_observation_pack *,
	struct kn_compat_pack_error_info *);
enum kn_compat_pack_error kn_compat_observation_pack_parse_text(
	const char *, struct kn_compat_observation_pack *,
	struct kn_compat_pack_error_info *);
enum kn_compat_pack_error kn_compat_observation_pack_report(
	const struct kn_compat_observation_pack *, char *, size_t);
enum kn_compat_pack_error kn_compat_observation_pack_validate_refs(
	const struct kn_compat_observation_pack *,
	struct kn_compat_pack_error_info *);
enum kn_compat_pack_error kn_compat_observation_pack_join_path(
	const struct kn_compat_observation_pack *, const char *, char *, size_t);

#endif
