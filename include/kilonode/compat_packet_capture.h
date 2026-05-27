/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/compat_packet_capture.h */

#ifndef KILONODE_COMPAT_PACKET_CAPTURE_H
#define KILONODE_COMPAT_PACKET_CAPTURE_H

#include <sys/types.h>

#include <stdint.h>

#include "kilonode/callsign.h"

#define KN_COMPAT_CAPTURE_ERROR_MAX    160
#define KN_COMPAT_CAPTURE_FRAME_MAX    512
#define KN_COMPAT_CAPTURE_LINE_MAX     512
#define KN_COMPAT_CAPTURE_NAME_MAX     96
#define KN_COMPAT_CAPTURE_PAYLOAD_MAX  256
#define KN_COMPAT_CAPTURE_PORT_MAX     32
#define KN_COMPAT_CAPTURE_TEXT_MAX     8192

enum kn_compat_packet_capture_error {
	KN_COMPAT_PACKET_CAPTURE_OK = 0,
	KN_COMPAT_PACKET_CAPTURE_ERR_INVALID_ARGUMENT,
	KN_COMPAT_PACKET_CAPTURE_ERR_IO,
	KN_COMPAT_PACKET_CAPTURE_ERR_LINE_TOO_LONG,
	KN_COMPAT_PACKET_CAPTURE_ERR_PARSE,
	KN_COMPAT_PACKET_CAPTURE_ERR_DUPLICATE_KEY,
	KN_COMPAT_PACKET_CAPTURE_ERR_UNKNOWN_KEY,
	KN_COMPAT_PACKET_CAPTURE_ERR_MISSING_REQUIRED,
	KN_COMPAT_PACKET_CAPTURE_ERR_INVALID_VALUE,
	KN_COMPAT_PACKET_CAPTURE_ERR_TOO_LARGE
};

enum kn_compat_packet_method {
	KN_COMPAT_PACKET_METHOD_NONE = 0,
	KN_COMPAT_PACKET_METHOD_KISS,
	KN_COMPAT_PACKET_METHOD_AXIP,
	KN_COMPAT_PACKET_METHOD_AXUDP
};

enum kn_compat_packet_direction {
	KN_COMPAT_PACKET_DIRECTION_NONE = 0,
	KN_COMPAT_PACKET_DIRECTION_RX,
	KN_COMPAT_PACKET_DIRECTION_TX
};

enum kn_compat_packet_expect_decode {
	KN_COMPAT_PACKET_EXPECT_NONE = 0,
	KN_COMPAT_PACKET_EXPECT_AX25_UI,
	KN_COMPAT_PACKET_EXPECT_MALFORMED
};

struct kn_compat_packet_capture {
	char name[KN_COMPAT_CAPTURE_NAME_MAX];
	char subject[KN_COMPAT_CAPTURE_NAME_MAX];
	enum kn_compat_packet_method method;
	char date[32];
	char observer[KN_COMPAT_CAPTURE_NAME_MAX];
	char mode[KN_COMPAT_CAPTURE_NAME_MAX];
	enum kn_compat_packet_direction direction;
	char port[KN_COMPAT_CAPTURE_PORT_MAX];
	char source_endpoint[KN_COMPAT_CAPTURE_NAME_MAX];
	char destination_endpoint[KN_COMPAT_CAPTURE_NAME_MAX];
	uint64_t timestamp;
	uint8_t frame[KN_COMPAT_CAPTURE_FRAME_MAX];
	size_t frame_len;
	enum kn_compat_packet_expect_decode expect_decode;
	struct kn_callsign expect_source;
	struct kn_callsign expect_destination;
	uint8_t has_expect_source;
	uint8_t has_expect_destination;
	uint8_t expect_pid;
	uint8_t has_expect_pid;
	char expect_payload_text[KN_COMPAT_CAPTURE_PAYLOAD_MAX];
	uint8_t has_expect_payload_text;
	uint8_t expect_payload_hex[KN_COMPAT_CAPTURE_PAYLOAD_MAX];
	size_t expect_payload_hex_len;
	char expect_kind[32];
	uint8_t has_expect_kind;
};

struct kn_compat_packet_capture_error_info {
	enum kn_compat_packet_capture_error error;
	size_t line;
	char message[KN_COMPAT_CAPTURE_ERROR_MAX];
};

void kn_compat_packet_capture_clear(struct kn_compat_packet_capture *);
const char *kn_compat_packet_capture_error_name(
	enum kn_compat_packet_capture_error);
const char *kn_compat_packet_direction_name(
	enum kn_compat_packet_direction);
const char *kn_compat_packet_expect_decode_name(
	enum kn_compat_packet_expect_decode);
const char *kn_compat_packet_method_name(enum kn_compat_packet_method);
enum kn_compat_packet_capture_error kn_compat_packet_capture_parse_file(
	const char *, struct kn_compat_packet_capture *,
	struct kn_compat_packet_capture_error_info *);
enum kn_compat_packet_capture_error kn_compat_packet_capture_parse_text(
	const char *, struct kn_compat_packet_capture *,
	struct kn_compat_packet_capture_error_info *);

#endif
