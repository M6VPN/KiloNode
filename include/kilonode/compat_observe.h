/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/compat_observe.h */

#ifndef KILONODE_COMPAT_OBSERVE_H
#define KILONODE_COMPAT_OBSERVE_H

#include <sys/types.h>

#include <stdint.h>

#define KN_COMPAT_OBSERVE_BLOCK_MAX 4096
#define KN_COMPAT_OBSERVE_ERROR_MAX 160
#define KN_COMPAT_OBSERVE_FIELD_MAX 256
#define KN_COMPAT_OBSERVE_LINE_MAX  512
#define KN_COMPAT_OBSERVE_NAME_MAX  96
#define KN_COMPAT_OBSERVE_TEXT_MAX  8192

enum kn_compat_observe_error {
	KN_COMPAT_OBSERVE_OK = 0,
	KN_COMPAT_OBSERVE_ERR_INVALID_ARGUMENT,
	KN_COMPAT_OBSERVE_ERR_IO,
	KN_COMPAT_OBSERVE_ERR_LINE_TOO_LONG,
	KN_COMPAT_OBSERVE_ERR_PARSE,
	KN_COMPAT_OBSERVE_ERR_DUPLICATE_KEY,
	KN_COMPAT_OBSERVE_ERR_UNKNOWN_KEY,
	KN_COMPAT_OBSERVE_ERR_MISSING_REQUIRED,
	KN_COMPAT_OBSERVE_ERR_INVALID_VALUE,
	KN_COMPAT_OBSERVE_ERR_TOO_LARGE
};

enum kn_compat_observe_method {
	KN_COMPAT_OBSERVE_METHOD_NONE = 0,
	KN_COMPAT_OBSERVE_METHOD_PROCESS,
	KN_COMPAT_OBSERVE_METHOD_TELNET,
	KN_COMPAT_OBSERVE_METHOD_TCP_LINE,
	KN_COMPAT_OBSERVE_METHOD_PACKET_CAPTURE,
	KN_COMPAT_OBSERVE_METHOD_MAILBOX
};

enum kn_compat_observe_mode {
	KN_COMPAT_OBSERVE_MODE_NONE = 0,
	KN_COMPAT_OBSERVE_MODE_PROCESS_OUTPUT,
	KN_COMPAT_OBSERVE_MODE_TCP_LINE,
	KN_COMPAT_OBSERVE_MODE_NODE_SHELL,
	KN_COMPAT_OBSERVE_MODE_BBS_SHELL
};

struct kn_compat_observation {
	char name[KN_COMPAT_OBSERVE_NAME_MAX];
	char subject[KN_COMPAT_OBSERVE_FIELD_MAX];
	enum kn_compat_observe_method method;
	char date[32];
	char observer[KN_COMPAT_OBSERVE_FIELD_MAX];
	char binary_path[KN_COMPAT_OBSERVE_FIELD_MAX];
	char config_path[KN_COMPAT_OBSERVE_FIELD_MAX];
	enum kn_compat_observe_mode mode;
	char connect_target[KN_COMPAT_OBSERVE_FIELD_MAX];
	char input[KN_COMPAT_OBSERVE_FIELD_MAX];
	char environment[KN_COMPAT_OBSERVE_FIELD_MAX];
	char notes[KN_COMPAT_OBSERVE_FIELD_MAX];
	char packet_capture_path[KN_COMPAT_OBSERVE_FIELD_MAX];
	char mailbox_path[KN_COMPAT_OBSERVE_FIELD_MAX];
	char result[KN_COMPAT_OBSERVE_FIELD_MAX];
	char observed[KN_COMPAT_OBSERVE_BLOCK_MAX];
	size_t observed_len;
};

struct kn_compat_observe_error_info {
	enum kn_compat_observe_error error;
	size_t line;
	char message[KN_COMPAT_OBSERVE_ERROR_MAX];
};

void kn_compat_observation_clear(struct kn_compat_observation *);
const char *kn_compat_observe_error_name(enum kn_compat_observe_error);
const char *kn_compat_observe_method_name(enum kn_compat_observe_method);
const char *kn_compat_observe_mode_name(enum kn_compat_observe_mode);
enum kn_compat_observe_error kn_compat_observation_parse_file(
	const char *, struct kn_compat_observation *,
	struct kn_compat_observe_error_info *);
enum kn_compat_observe_error kn_compat_observation_parse_text(
	const char *, struct kn_compat_observation *,
	struct kn_compat_observe_error_info *);
enum kn_compat_observe_error kn_compat_observation_report(
	const struct kn_compat_observation *, char *, size_t);

#endif
