/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/compat_bench_pack.h */

#ifndef KILONODE_COMPAT_BENCH_PACK_H
#define KILONODE_COMPAT_BENCH_PACK_H

#include <sys/types.h>

#include <stdint.h>

#define KN_COMPAT_BENCH_ERROR_MAX    160
#define KN_COMPAT_BENCH_FIELD_MAX    256
#define KN_COMPAT_BENCH_FIXTURE_MAX  32
#define KN_COMPAT_BENCH_LINE_MAX     512
#define KN_COMPAT_BENCH_NAME_MAX     96
#define KN_COMPAT_BENCH_REPORT_MAX   4096
#define KN_COMPAT_BENCH_TEXT_MAX     8192

enum kn_compat_bench_error {
	KN_COMPAT_BENCH_OK = 0,
	KN_COMPAT_BENCH_ERR_INVALID_ARGUMENT,
	KN_COMPAT_BENCH_ERR_IO,
	KN_COMPAT_BENCH_ERR_LINE_TOO_LONG,
	KN_COMPAT_BENCH_ERR_PARSE,
	KN_COMPAT_BENCH_ERR_DUPLICATE_KEY,
	KN_COMPAT_BENCH_ERR_UNKNOWN_KEY,
	KN_COMPAT_BENCH_ERR_MISSING_REQUIRED,
	KN_COMPAT_BENCH_ERR_INVALID_VALUE,
	KN_COMPAT_BENCH_ERR_TOO_MANY,
	KN_COMPAT_BENCH_ERR_REFERENCE,
	KN_COMPAT_BENCH_ERR_REPLAY,
	KN_COMPAT_BENCH_ERR_BUFFER
};

struct kn_compat_bench_entry {
	char path[KN_COMPAT_BENCH_FIELD_MAX];
};

struct kn_compat_bench_pack {
	char manifest_path[KN_COMPAT_BENCH_FIELD_MAX];
	char base_dir[KN_COMPAT_BENCH_FIELD_MAX];
	char name[KN_COMPAT_BENCH_NAME_MAX];
	char type[KN_COMPAT_BENCH_FIELD_MAX];
	char source[KN_COMPAT_BENCH_FIELD_MAX];
	uint8_t clean_room;
	uint8_t source_code_used;
	uint8_t hardware_required;
	uint8_t transmit_required;
	struct kn_compat_bench_entry fixtures[KN_COMPAT_BENCH_FIXTURE_MAX];
	size_t fixture_count;
};

struct kn_compat_bench_coverage {
	size_t fixture_count;
	size_t kiss_count;
	size_t raw_ax25_count;
	size_t ui_count;
	size_t setup_count;
	size_t supervisory_count;
	size_t disconnect_count;
	size_t fx25_placeholder_count;
};

struct kn_compat_bench_error_info {
	enum kn_compat_bench_error error;
	size_t line;
	char message[KN_COMPAT_BENCH_ERROR_MAX];
};

void kn_compat_bench_pack_clear(struct kn_compat_bench_pack *);
const char *kn_compat_bench_error_name(enum kn_compat_bench_error);
enum kn_compat_bench_error kn_compat_bench_pack_parse_file(
	const char *, struct kn_compat_bench_pack *,
	struct kn_compat_bench_error_info *);
enum kn_compat_bench_error kn_compat_bench_pack_parse_text(
	const char *, struct kn_compat_bench_pack *,
	struct kn_compat_bench_error_info *);
enum kn_compat_bench_error kn_compat_bench_pack_validate_refs(
	const struct kn_compat_bench_pack *,
	struct kn_compat_bench_error_info *);
enum kn_compat_bench_error kn_compat_bench_pack_join_path(
	const struct kn_compat_bench_pack *, const char *, char *, size_t);
enum kn_compat_bench_error kn_compat_bench_pack_coverage(
	const struct kn_compat_bench_pack *,
	struct kn_compat_bench_coverage *);
enum kn_compat_bench_error kn_compat_bench_pack_coverage_report(
	const struct kn_compat_bench_pack *, char *, size_t);
enum kn_compat_bench_error kn_compat_bench_pack_report(
	const struct kn_compat_bench_pack *, char *, size_t);
enum kn_compat_bench_error kn_compat_bench_pack_replay_report(
	const struct kn_compat_bench_pack *, char *, size_t);

#endif
