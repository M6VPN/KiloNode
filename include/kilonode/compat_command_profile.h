/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/compat_command_profile.h */

#ifndef KILONODE_COMPAT_COMMAND_PROFILE_H
#define KILONODE_COMPAT_COMMAND_PROFILE_H

#include <sys/types.h>

#include <stdint.h>

#include "kilonode/compat_observation_pack.h"

#define KN_COMPAT_PROFILE_COMMAND_MAX 32
#define KN_COMPAT_PROFILE_ERROR_MAX   160
#define KN_COMPAT_PROFILE_FIELD_MAX   256
#define KN_COMPAT_PROFILE_LINE_MAX    512
#define KN_COMPAT_PROFILE_MAX         32
#define KN_COMPAT_PROFILE_REPORT_MAX  8192
#define KN_COMPAT_PROFILE_TEXT_MAX    16384

enum kn_compat_profile_error {
	KN_COMPAT_PROFILE_OK = 0,
	KN_COMPAT_PROFILE_ERR_INVALID_ARGUMENT,
	KN_COMPAT_PROFILE_ERR_IO,
	KN_COMPAT_PROFILE_ERR_LINE_TOO_LONG,
	KN_COMPAT_PROFILE_ERR_PARSE,
	KN_COMPAT_PROFILE_ERR_DUPLICATE_KEY,
	KN_COMPAT_PROFILE_ERR_UNKNOWN_KEY,
	KN_COMPAT_PROFILE_ERR_MISSING_REQUIRED,
	KN_COMPAT_PROFILE_ERR_INVALID_VALUE,
	KN_COMPAT_PROFILE_ERR_TOO_MANY,
	KN_COMPAT_PROFILE_ERR_BUFFER
};

enum kn_compat_profile_category {
	KN_COMPAT_PROFILE_CATEGORY_INFORMATIONAL = 0,
	KN_COMPAT_PROFILE_CATEGORY_SESSION,
	KN_COMPAT_PROFILE_CATEGORY_BBS,
	KN_COMPAT_PROFILE_CATEGORY_ROUTING,
	KN_COMPAT_PROFILE_CATEGORY_SYSOP,
	KN_COMPAT_PROFILE_CATEGORY_UNKNOWN_HANDLING
};

enum kn_compat_profile_transport {
	KN_COMPAT_PROFILE_TRANSPORT_LOCAL_SHELL = 0,
	KN_COMPAT_PROFILE_TRANSPORT_RF_UI,
	KN_COMPAT_PROFILE_TRANSPORT_CONNECTED_AX25,
	KN_COMPAT_PROFILE_TRANSPORT_NETROM,
	KN_COMPAT_PROFILE_TRANSPORT_TELNET
};

enum kn_compat_profile_args {
	KN_COMPAT_PROFILE_ARGS_NONE = 0,
	KN_COMPAT_PROFILE_ARGS_OPTIONAL,
	KN_COMPAT_PROFILE_ARGS_REQUIRED,
	KN_COMPAT_PROFILE_ARGS_FREE_TEXT
};

enum kn_compat_profile_reply {
	KN_COMPAT_PROFILE_REPLY_NONE = 0,
	KN_COMPAT_PROFILE_REPLY_ONE_LINE,
	KN_COMPAT_PROFILE_REPLY_ONE_OR_MORE_LINES,
	KN_COMPAT_PROFILE_REPLY_SESSION_TRANSITION
};

enum kn_compat_profile_status {
	KN_COMPAT_PROFILE_STATUS_NEEDS_OBSERVATION = 0,
	KN_COMPAT_PROFILE_STATUS_PLANNED,
	KN_COMPAT_PROFILE_STATUS_BLOCKED,
	KN_COMPAT_PROFILE_STATUS_NATIVE_ONLY,
	KN_COMPAT_PROFILE_STATUS_READY_FOR_DESIGN
};

struct kn_compat_command_profile {
	char command[KN_COMPAT_PROFILE_COMMAND_MAX];
	enum kn_compat_profile_category category;
	enum kn_compat_profile_transport transport;
	enum kn_compat_profile_args args;
	enum kn_compat_profile_reply reply;
	enum kn_compat_profile_status compat_status;
	uint8_t stateful;
	uint8_t requires_connected_mode;
	uint8_t has_category;
	uint8_t has_transport;
	uint8_t has_args;
	uint8_t has_reply;
	uint8_t has_status;
};

struct kn_compat_command_profiles {
	char name[KN_COMPAT_PROFILE_FIELD_MAX];
	char subject[KN_COMPAT_PROFILE_FIELD_MAX];
	uint8_t clean_room;
	uint8_t source_code_used;
	struct kn_compat_command_profile profiles[KN_COMPAT_PROFILE_MAX];
	size_t profile_count;
};

struct kn_compat_profile_error_info {
	enum kn_compat_profile_error error;
	size_t line;
	char message[KN_COMPAT_PROFILE_ERROR_MAX];
};

void kn_compat_command_profiles_clear(
	struct kn_compat_command_profiles *);
const char *kn_compat_profile_error_name(enum kn_compat_profile_error);
const char *kn_compat_profile_category_name(
	enum kn_compat_profile_category);
const char *kn_compat_profile_transport_name(
	enum kn_compat_profile_transport);
const char *kn_compat_profile_args_name(enum kn_compat_profile_args);
const char *kn_compat_profile_reply_name(enum kn_compat_profile_reply);
const char *kn_compat_profile_status_name(enum kn_compat_profile_status);
const struct kn_compat_command_profile *kn_compat_command_profile_find(
	const struct kn_compat_command_profiles *, const char *);
enum kn_compat_profile_error kn_compat_command_profiles_parse_file(
	const char *, struct kn_compat_command_profiles *,
	struct kn_compat_profile_error_info *);
enum kn_compat_profile_error kn_compat_command_profiles_parse_text(
	const char *, struct kn_compat_command_profiles *,
	struct kn_compat_profile_error_info *);
enum kn_compat_profile_error kn_compat_command_profiles_report(
	const struct kn_compat_command_profiles *, char *, size_t);
enum kn_compat_profile_error kn_compat_command_profiles_generate_from_pack(
	const struct kn_compat_observation_pack *, char *, size_t);

#endif
