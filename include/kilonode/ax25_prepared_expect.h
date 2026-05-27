/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/ax25_prepared_expect.h */

#ifndef KILONODE_AX25_PREPARED_EXPECT_H
#define KILONODE_AX25_PREPARED_EXPECT_H

#include <sys/types.h>

#include <stdint.h>

#include "kilonode/ax25_prepared_frame.h"

#define KN_AX25_PREPARED_EXPECT_BLOCK_MAX    32
#define KN_AX25_PREPARED_EXPECT_FRAME_MAX    16
#define KN_AX25_PREPARED_EXPECT_NAME_MAX     128
#define KN_AX25_PREPARED_EXPECT_TEXT_MAX     160
#define KN_AX25_PREPARED_EXPECT_LINE_MAX     512
#define KN_AX25_PREPARED_EXPECT_HEX_MAX      128

enum kn_ax25_prepared_expect_error {
	KN_AX25_PREPARED_EXPECT_OK = 0,
	KN_AX25_PREPARED_EXPECT_ERR_INVALID_ARGUMENT,
	KN_AX25_PREPARED_EXPECT_ERR_IO,
	KN_AX25_PREPARED_EXPECT_ERR_PARSE,
	KN_AX25_PREPARED_EXPECT_ERR_FULL,
	KN_AX25_PREPARED_EXPECT_ERR_BUFFER
};

enum kn_ax25_prepared_expect_block_type {
	KN_AX25_PREPARED_EXPECT_BLOCK_CAPTURE = 0,
	KN_AX25_PREPARED_EXPECT_BLOCK_REPLAY
};

struct kn_ax25_prepared_expect_frame {
	size_t line;
	size_t order;
	uint8_t has_kind;
	enum kn_ax25_frame_plan_type kind;
	uint8_t has_action;
	enum kn_ax25_action_intent action;
	uint8_t has_status;
	enum kn_ax25_prepared_frame_status status;
	uint8_t has_local;
	char local[KN_CALLSIGN_MAX + 4];
	uint8_t has_remote;
	char remote[KN_CALLSIGN_MAX + 4];
	uint8_t has_port;
	char port[KN_AX25_PREPARED_FRAME_PORT_MAX];
	uint8_t has_ax25_len;
	size_t ax25_len;
	uint8_t has_hex_prefix;
	char hex_prefix[KN_AX25_PREPARED_EXPECT_HEX_MAX + 1U];
	uint8_t has_tx_writes;
	uint64_t tx_writes;
};

struct kn_ax25_prepared_expect_block {
	size_t line;
	enum kn_ax25_prepared_expect_block_type type;
	char name[KN_AX25_PREPARED_EXPECT_NAME_MAX];
	uint8_t has_prepared_count;
	size_t prepared_count;
	uint8_t has_tx_writes;
	uint64_t tx_writes;
	struct kn_ax25_prepared_expect_frame
	    frames[KN_AX25_PREPARED_EXPECT_FRAME_MAX];
	size_t frame_count;
};

struct kn_ax25_prepared_expect_file {
	struct kn_ax25_prepared_expect_block
	    blocks[KN_AX25_PREPARED_EXPECT_BLOCK_MAX];
	size_t block_count;
};

struct kn_ax25_prepared_expect_error_info {
	enum kn_ax25_prepared_expect_error error;
	size_t line;
	char message[KN_AX25_PREPARED_EXPECT_TEXT_MAX];
};

void kn_ax25_prepared_expect_clear(struct kn_ax25_prepared_expect_file *);
void kn_ax25_prepared_expect_error_clear(
	struct kn_ax25_prepared_expect_error_info *);
const char *kn_ax25_prepared_expect_error_name(
	enum kn_ax25_prepared_expect_error);
const struct kn_ax25_prepared_expect_block *
kn_ax25_prepared_expect_find(const struct kn_ax25_prepared_expect_file *,
	enum kn_ax25_prepared_expect_block_type, const char *);
enum kn_ax25_prepared_expect_error kn_ax25_prepared_expect_parse_file(
	const char *, struct kn_ax25_prepared_expect_file *,
	struct kn_ax25_prepared_expect_error_info *);
int kn_ax25_prepared_expect_action_from_text(const char *,
	enum kn_ax25_action_intent *);
int kn_ax25_prepared_expect_kind_from_text(const char *,
	enum kn_ax25_frame_plan_type *);
int kn_ax25_prepared_expect_status_from_text(const char *,
	enum kn_ax25_prepared_frame_status *);

#endif
