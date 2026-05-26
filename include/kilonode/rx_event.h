/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/rx_event.h */

#ifndef KILONODE_RX_EVENT_H
#define KILONODE_RX_EVENT_H

#include <sys/types.h>

#include <stdint.h>

#include "kilonode/ax25.h"
#include "kilonode/callsign.h"
#include "kilonode/config.h"

#define KN_RX_EVENT_PATH_MAX            128
#define KN_RX_EVENT_PREVIEW_DEFAULT     80
#define KN_RX_EVENT_PREVIEW_MAX         256
#define KN_RX_EVENT_PREVIEW_TEXT_MAX    3 + (KN_RX_EVENT_PREVIEW_MAX * 4)

enum kn_rx_event_error {
	KN_RX_EVENT_OK = 0,
	KN_RX_EVENT_ERR_INVALID_ARGUMENT,
	KN_RX_EVENT_ERR_INVALID_VALUE,
	KN_RX_EVENT_ERR_BUFFER
};

enum kn_rx_frame_kind {
	KN_RX_FRAME_UI = 0,
	KN_RX_FRAME_I,
	KN_RX_FRAME_S,
	KN_RX_FRAME_U,
	KN_RX_FRAME_UNKNOWN,
	KN_RX_FRAME_MALFORMED
};

struct kn_rx_event {
	uint64_t id;
	uint64_t timestamp;
	char port_name[KN_CONFIG_PORT_NAME_MAX];
	uint8_t kiss_port;
	uint8_t kiss_command;
	struct kn_callsign source;
	struct kn_callsign destination;
	char path[KN_RX_EVENT_PATH_MAX];
	uint8_t control;
	uint8_t pid;
	uint8_t has_pid;
	enum kn_rx_frame_kind kind;
	size_t payload_len;
	size_t preview_len;
	uint8_t preview_binary;
	char preview[KN_RX_EVENT_PREVIEW_TEXT_MAX];
	int decode_status;
	uint8_t malformed;
};

enum kn_rx_frame_kind kn_rx_event_classify_control(uint8_t);
void kn_rx_event_clear(struct kn_rx_event *);
const char *kn_rx_event_kind_name(enum kn_rx_frame_kind);
enum kn_rx_event_error kn_rx_event_from_ax25(struct kn_rx_event *,
	uint64_t, uint64_t, const char *, uint8_t, uint8_t,
	const struct kn_ax25_frame *, size_t);
enum kn_rx_event_error kn_rx_event_from_malformed(struct kn_rx_event *,
	uint64_t, uint64_t, const char *, uint8_t, uint8_t, int, size_t);
enum kn_rx_event_error kn_rx_event_format_brief(const struct kn_rx_event *,
	char *, size_t);
enum kn_rx_event_error kn_rx_event_format_full(const struct kn_rx_event *,
	char *, size_t);

#endif
