/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/ax25_connect_dry_run.h */

#ifndef KILONODE_AX25_CONNECT_DRY_RUN_H
#define KILONODE_AX25_CONNECT_DRY_RUN_H

#include <sys/types.h>

#include <stdint.h>

#include "kilonode/ax25_connection.h"
#include "kilonode/ax25_params.h"
#include "kilonode/config.h"

#define KN_AX25_CONNECT_DRY_RUN_COMMAND_MAX 32
#define KN_AX25_CONNECT_DRY_RUN_LINE_MAX    256
#define KN_AX25_CONNECT_DRY_RUN_REPORT_MAX  1024
#define KN_AX25_CONNECT_DRY_RUN_TEXT_MAX    128

enum kn_ax25_connect_dry_run_error {
	KN_AX25_CONNECT_DRY_RUN_OK = 0,
	KN_AX25_CONNECT_DRY_RUN_ERR_INVALID_ARGUMENT,
	KN_AX25_CONNECT_DRY_RUN_ERR_PARSE,
	KN_AX25_CONNECT_DRY_RUN_ERR_IO,
	KN_AX25_CONNECT_DRY_RUN_ERR_MISMATCH,
	KN_AX25_CONNECT_DRY_RUN_ERR_BUFFER
};

enum kn_ax25_connect_dry_run_expect {
	KN_AX25_CONNECT_DRY_RUN_EXPECT_NONE = 0,
	KN_AX25_CONNECT_DRY_RUN_EXPECT_PLANNED_STATE,
	KN_AX25_CONNECT_DRY_RUN_EXPECT_FRAME_KIND,
	KN_AX25_CONNECT_DRY_RUN_EXPECT_BRIDGE,
	KN_AX25_CONNECT_DRY_RUN_EXPECT_TX_WRITES,
	KN_AX25_CONNECT_DRY_RUN_EXPECT_DISPATCH_CALLS,
	KN_AX25_CONNECT_DRY_RUN_EXPECT_FX25_FRAMES,
	KN_AX25_CONNECT_DRY_RUN_EXPECT_CONNECTION_CREATED
};

struct kn_ax25_connect_dry_run_command {
	enum kn_ax25_connect_dry_run_expect expect;
	enum kn_ax25_connection_state state;
	uint64_t value;
	size_t line;
	char text[KN_AX25_CONNECT_DRY_RUN_TEXT_MAX];
};

struct kn_ax25_connect_dry_run_error_info {
	enum kn_ax25_connect_dry_run_error error;
	size_t line;
	char message[KN_AX25_CONNECT_DRY_RUN_TEXT_MAX];
};

struct kn_ax25_connect_dry_run_script {
	char name[KN_AX25_CONNECT_DRY_RUN_TEXT_MAX];
	char local[KN_AX25_CONNECT_DRY_RUN_TEXT_MAX];
	char remote[KN_AX25_CONNECT_DRY_RUN_TEXT_MAX];
	char port[KN_CONFIG_PORT_NAME_MAX];
	struct kn_ax25_params params;
	struct kn_ax25_connect_dry_run_command
	    commands[KN_AX25_CONNECT_DRY_RUN_COMMAND_MAX];
	size_t command_count;
};

struct kn_ax25_connect_dry_run_result {
	char name[KN_AX25_CONNECT_DRY_RUN_TEXT_MAX];
	char local[KN_AX25_CONNECT_DRY_RUN_TEXT_MAX];
	char remote[KN_AX25_CONNECT_DRY_RUN_TEXT_MAX];
	char port[KN_CONFIG_PORT_NAME_MAX];
	enum kn_ax25_connection_state planned_state;
	char frame_kind[KN_AX25_CONNECT_DRY_RUN_TEXT_MAX];
	uint8_t pass;
	uint8_t bridge_blocked;
	uint8_t connection_created;
	uint64_t tx_writes;
	uint64_t dispatch_calls;
	uint64_t fx25_frames;
	size_t mismatch_count;
	size_t first_mismatch_line;
	char last_error[KN_AX25_CONNECT_DRY_RUN_TEXT_MAX];
};

const char *kn_ax25_connect_dry_run_error_name(
	enum kn_ax25_connect_dry_run_error);
enum kn_ax25_connect_dry_run_error kn_ax25_connect_dry_run_format_report(
	const struct kn_ax25_connect_dry_run_result *, char *, size_t);
enum kn_ax25_connect_dry_run_error kn_ax25_connect_dry_run_parse_file(
	const char *, struct kn_ax25_connect_dry_run_script *,
	struct kn_ax25_connect_dry_run_error_info *);
enum kn_ax25_connect_dry_run_error kn_ax25_connect_dry_run_run_file(
	const char *, struct kn_ax25_connect_dry_run_result *,
	struct kn_ax25_connect_dry_run_error_info *);
enum kn_ax25_connect_dry_run_error kn_ax25_connect_dry_run_run_script(
	const struct kn_ax25_connect_dry_run_script *,
	struct kn_ax25_connect_dry_run_result *);

#endif
