/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/manual_capture_workspace.h */

#ifndef KILONODE_MANUAL_CAPTURE_WORKSPACE_H
#define KILONODE_MANUAL_CAPTURE_WORKSPACE_H

#include <sys/types.h>

#include <stdint.h>

#define KN_MANUAL_CAPTURE_FIELD_MAX 256
#define KN_MANUAL_CAPTURE_LINE_MAX 512
#define KN_MANUAL_CAPTURE_TEXT_MAX 8192
#define KN_MANUAL_CAPTURE_ERROR_MAX 160
#define KN_MANUAL_CAPTURE_PATH_MAX 512

enum kn_manual_capture_error {
	KN_MANUAL_CAPTURE_OK = 0,
	KN_MANUAL_CAPTURE_ERR_INVALID_ARGUMENT,
	KN_MANUAL_CAPTURE_ERR_IO,
	KN_MANUAL_CAPTURE_ERR_PARSE,
	KN_MANUAL_CAPTURE_ERR_LINE_TOO_LONG,
	KN_MANUAL_CAPTURE_ERR_UNKNOWN_KEY,
	KN_MANUAL_CAPTURE_ERR_MISSING_REQUIRED,
	KN_MANUAL_CAPTURE_ERR_INVALID_VALUE,
	KN_MANUAL_CAPTURE_ERR_UNSAFE_PATH,
	KN_MANUAL_CAPTURE_ERR_TOO_MANY,
	KN_MANUAL_CAPTURE_ERR_CAPTURE,
	KN_MANUAL_CAPTURE_ERR_REPLAY,
	KN_MANUAL_CAPTURE_ERR_BUFFER
};

struct kn_manual_capture_error_info {
	enum kn_manual_capture_error error;
	size_t line;
	char message[KN_MANUAL_CAPTURE_ERROR_MAX];
};

struct kn_manual_capture_workspace {
	char root[KN_MANUAL_CAPTURE_PATH_MAX];
	char name[KN_MANUAL_CAPTURE_FIELD_MAX];
	char type[KN_MANUAL_CAPTURE_FIELD_MAX];
	char created[KN_MANUAL_CAPTURE_FIELD_MAX];
	uint8_t clean_room;
	uint8_t source_code_used;
	uint8_t transmit_required;
	char hardware_required[KN_MANUAL_CAPTURE_FIELD_MAX];
	char notes[KN_MANUAL_CAPTURE_FIELD_MAX];
};

void kn_manual_capture_workspace_clear(
	struct kn_manual_capture_workspace *);
const char *kn_manual_capture_error_name(enum kn_manual_capture_error);
enum kn_manual_capture_error kn_manual_capture_workspace_init(
	const char *, struct kn_manual_capture_error_info *);
enum kn_manual_capture_error kn_manual_capture_workspace_check(
	const char *, struct kn_manual_capture_workspace *,
	struct kn_manual_capture_error_info *);
enum kn_manual_capture_error kn_manual_capture_workspace_parse_text(
	const char *, struct kn_manual_capture_workspace *,
	struct kn_manual_capture_error_info *);
enum kn_manual_capture_error kn_manual_capture_workspace_parse_file(
	const char *, struct kn_manual_capture_workspace *,
	struct kn_manual_capture_error_info *);
enum kn_manual_capture_error kn_manual_capture_workspace_join(
	const char *, const char *, char *, size_t);
int kn_manual_capture_path_safe(const char *);
int kn_manual_capture_relative_path_safe(const char *);

#endif
