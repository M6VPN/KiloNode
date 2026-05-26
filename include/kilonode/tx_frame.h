/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/tx_frame.h */

#ifndef KILONODE_TX_FRAME_H
#define KILONODE_TX_FRAME_H

#include <sys/types.h>

#include <stdint.h>

#include "kilonode/ax25.h"
#include "kilonode/callsign.h"
#include "kilonode/config.h"
#include "kilonode/tx_policy.h"

#define KN_TX_FRAME_AX25_MAX         330
#define KN_TX_FRAME_KISS_MAX         ((KN_TX_FRAME_AX25_MAX * 2) + 4)
#define KN_TX_FRAME_PATH_MAX         128
#define KN_TX_FRAME_PREVIEW_TEXT_MAX (3 + (KN_TX_POLICY_PREVIEW_MAX * 4))

enum kn_tx_frame_error {
	KN_TX_FRAME_OK = 0,
	KN_TX_FRAME_ERR_INVALID_ARGUMENT,
	KN_TX_FRAME_ERR_INVALID_VALUE,
	KN_TX_FRAME_ERR_TOO_LARGE,
	KN_TX_FRAME_ERR_BUFFER,
	KN_TX_FRAME_ERR_POLICY
};

enum kn_tx_frame_kind {
	KN_TX_FRAME_UI = 0,
	KN_TX_FRAME_RAW_AX25,
	KN_TX_FRAME_UNKNOWN
};

enum kn_tx_frame_status {
	KN_TX_FRAME_QUEUED = 0,
	KN_TX_FRAME_SENT,
	KN_TX_FRAME_DROPPED,
	KN_TX_FRAME_FAILED,
	KN_TX_FRAME_DRY_RUN
};

struct kn_tx_frame {
	uint64_t id;
	uint64_t created;
	char port_name[KN_CONFIG_PORT_NAME_MAX];
	uint8_t kiss_port;
	uint8_t kiss_command;
	struct kn_callsign destination;
	struct kn_callsign source;
	struct kn_callsign digipeaters[KN_AX25_MAX_DIGIS];
	size_t digipeater_count;
	char path[KN_TX_FRAME_PATH_MAX];
	uint8_t control;
	uint8_t pid;
	uint8_t has_pid;
	enum kn_tx_frame_kind kind;
	enum kn_tx_frame_status status;
	size_t payload_len;
	size_t preview_len;
	uint8_t preview_binary;
	char preview[KN_TX_FRAME_PREVIEW_TEXT_MAX];
	uint8_t ax25[KN_TX_FRAME_AX25_MAX];
	size_t ax25_len;
	uint8_t kiss[KN_TX_FRAME_KISS_MAX];
	size_t kiss_len;
	int last_error;
};

enum kn_tx_frame_error kn_tx_frame_build_raw_ax25(struct kn_tx_frame *,
	uint64_t, uint64_t, const char *, uint8_t, uint8_t,
	const uint8_t *, size_t, size_t);
enum kn_tx_frame_error kn_tx_frame_build_ui(struct kn_tx_frame *,
	uint64_t, uint64_t, const char *, uint8_t, const char *,
	const char *, const char *const *, size_t, uint8_t,
	const uint8_t *, size_t, const struct kn_tx_policy *);
void kn_tx_frame_clear(struct kn_tx_frame *);
enum kn_tx_frame_error kn_tx_frame_format_brief(
	const struct kn_tx_frame *, char *, size_t);
enum kn_tx_frame_error kn_tx_frame_format_full(
	const struct kn_tx_frame *, char *, size_t);
const char *kn_tx_frame_kind_name(enum kn_tx_frame_kind);
const char *kn_tx_frame_status_name(enum kn_tx_frame_status);

#endif
