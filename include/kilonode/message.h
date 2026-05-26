/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/message.h */

#ifndef KILONODE_MESSAGE_H
#define KILONODE_MESSAGE_H

#include <sys/types.h>

#include <stdint.h>

#include "kilonode/callsign.h"

#define KN_MESSAGE_AREA_MAX    32
#define KN_MESSAGE_BODY_MAX    65536
#define KN_MESSAGE_SUBJECT_MAX 120

enum kn_message_type {
	KN_MESSAGE_TYPE_PRIVATE = 1,
	KN_MESSAGE_TYPE_BULLETIN
};

enum kn_message_error {
	KN_MESSAGE_OK = 0,
	KN_MESSAGE_ERR_INVALID_ARGUMENT,
	KN_MESSAGE_ERR_INVALID_TYPE,
	KN_MESSAGE_ERR_INVALID_CALLSIGN,
	KN_MESSAGE_ERR_INVALID_AREA,
	KN_MESSAGE_ERR_INVALID_SUBJECT,
	KN_MESSAGE_ERR_INVALID_BODY,
	KN_MESSAGE_ERR_BODY_TOO_LARGE,
	KN_MESSAGE_ERR_BUFFER
};

struct kn_message {
	uint64_t id;
	enum kn_message_type type;
	struct kn_callsign from;
	struct kn_callsign to;
	char area[KN_MESSAGE_AREA_MAX + 1];
	char subject[KN_MESSAGE_SUBJECT_MAX + 1];
	uint64_t created;
	uint64_t updated;
	uint8_t read;
	uint8_t deleted;
	size_t body_len;
};

uint8_t kn_message_area_valid(const char *);
enum kn_message_error kn_message_bulletin_init(struct kn_message *,
	const char *, const char *, const char *, size_t, uint64_t);
enum kn_message_error kn_message_private_init(struct kn_message *,
	const char *, const char *, const char *, size_t, uint64_t);
enum kn_message_error kn_message_summary(const struct kn_message *, char *,
	size_t);
const char *kn_message_type_name(enum kn_message_type);

#endif
