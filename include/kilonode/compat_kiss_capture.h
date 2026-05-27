/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/compat_kiss_capture.h */

#ifndef KILONODE_COMPAT_KISS_CAPTURE_H
#define KILONODE_COMPAT_KISS_CAPTURE_H

#include <sys/types.h>

#include <stdint.h>

#include "kilonode/compat_packet_capture.h"

#define KN_COMPAT_PACKET_MISMATCH_MAX      8
#define KN_COMPAT_PACKET_MISMATCH_TEXT_MAX 160
#define KN_COMPAT_PACKET_PREVIEW_MAX       256

enum kn_compat_kiss_capture_error {
	KN_COMPAT_KISS_CAPTURE_OK = 0,
	KN_COMPAT_KISS_CAPTURE_ERR_INVALID_ARGUMENT,
	KN_COMPAT_KISS_CAPTURE_ERR_UNSUPPORTED,
	KN_COMPAT_KISS_CAPTURE_ERR_DECODE,
	KN_COMPAT_KISS_CAPTURE_ERR_MISMATCH
};

struct kn_compat_packet_mismatch {
	char text[KN_COMPAT_PACKET_MISMATCH_TEXT_MAX];
};

struct kn_compat_packet_decode {
	char capture_name[KN_COMPAT_CAPTURE_NAME_MAX];
	enum kn_compat_packet_method method;
	uint8_t passed;
	uint8_t kiss_command;
	uint8_t kiss_port;
	char source[KN_CALLSIGN_MAX + 4];
	char destination[KN_CALLSIGN_MAX + 4];
	char kind[32];
	uint8_t pid;
	uint8_t has_pid;
	char payload_preview[KN_COMPAT_PACKET_PREVIEW_MAX];
	size_t payload_len;
	size_t mismatch_count;
	struct kn_compat_packet_mismatch mismatches[KN_COMPAT_PACKET_MISMATCH_MAX];
};

void kn_compat_packet_decode_clear(struct kn_compat_packet_decode *);
const char *kn_compat_kiss_capture_error_name(
	enum kn_compat_kiss_capture_error);
enum kn_compat_kiss_capture_error kn_compat_kiss_capture_decode(
	const struct kn_compat_packet_capture *,
	struct kn_compat_packet_decode *);

#endif
