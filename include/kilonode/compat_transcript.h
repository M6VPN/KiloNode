/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/compat_transcript.h */

#ifndef KILONODE_COMPAT_TRANSCRIPT_H
#define KILONODE_COMPAT_TRANSCRIPT_H

#include <sys/types.h>

#include <stdint.h>

#include "kilonode/callsign.h"
#include "kilonode/config.h"
#include "kilonode/rf_command.h"

#define KN_COMPAT_ERROR_MAX      160
#define KN_COMPAT_EXPECT_MAX     160
#define KN_COMPAT_INPUT_MAX      KN_CONFIG_RF_COMMAND_BYTES_MAX
#define KN_COMPAT_LINE_MAX       512
#define KN_COMPAT_NAME_MAX       64
#define KN_COMPAT_TRANSCRIPT_MAX 4096

enum kn_compat_transcript_error {
	KN_COMPAT_TRANSCRIPT_OK = 0,
	KN_COMPAT_TRANSCRIPT_ERR_INVALID_ARGUMENT,
	KN_COMPAT_TRANSCRIPT_ERR_IO,
	KN_COMPAT_TRANSCRIPT_ERR_LINE_TOO_LONG,
	KN_COMPAT_TRANSCRIPT_ERR_PARSE,
	KN_COMPAT_TRANSCRIPT_ERR_DUPLICATE_KEY,
	KN_COMPAT_TRANSCRIPT_ERR_UNKNOWN_KEY,
	KN_COMPAT_TRANSCRIPT_ERR_MISSING_REQUIRED,
	KN_COMPAT_TRANSCRIPT_ERR_INVALID_VALUE
};

enum kn_compat_mode {
	KN_COMPAT_MODE_NONE = 0,
	KN_COMPAT_MODE_RF_UI
};

enum kn_compat_reply_expect {
	KN_COMPAT_REPLY_UNSET = 0,
	KN_COMPAT_REPLY_NONE,
	KN_COMPAT_REPLY_CONTAINS,
	KN_COMPAT_REPLY_EXACT
};

struct kn_compat_transcript {
	char name[KN_COMPAT_NAME_MAX];
	enum kn_compat_mode mode;
	struct kn_callsign node;
	char port_name[KN_CONFIG_PORT_NAME_MAX];
	struct kn_callsign source;
	struct kn_callsign destination;
	uint8_t pid;
	uint8_t input[KN_COMPAT_INPUT_MAX];
	size_t input_len;
	enum kn_rf_command_name expect_command;
	enum kn_rf_command_status expect_status;
	enum kn_compat_reply_expect expect_reply;
	char expect_reply_text[KN_COMPAT_EXPECT_MAX];
	uint8_t expect_reply_queued;
	uint8_t has_expect_reply_queued;
	uint8_t expect_no_dispatch;
	char expect_error[KN_RF_COMMAND_ERROR_MAX];
	uint8_t has_expect_error;
};

struct kn_compat_transcript_error_info {
	enum kn_compat_transcript_error error;
	size_t line;
	char message[KN_COMPAT_ERROR_MAX];
};

void kn_compat_transcript_clear(struct kn_compat_transcript *);
const char *kn_compat_transcript_error_name(
	enum kn_compat_transcript_error);
const char *kn_compat_mode_name(enum kn_compat_mode);
enum kn_compat_transcript_error kn_compat_transcript_parse_file(
	const char *, struct kn_compat_transcript *,
	struct kn_compat_transcript_error_info *);
enum kn_compat_transcript_error kn_compat_transcript_parse_text(
	const char *, struct kn_compat_transcript *,
	struct kn_compat_transcript_error_info *);
enum kn_compat_transcript_error kn_compat_transcript_report(
	const struct kn_compat_transcript *, char *, size_t);

#endif
