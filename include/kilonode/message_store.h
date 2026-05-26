/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/message_store.h */

#ifndef KILONODE_MESSAGE_STORE_H
#define KILONODE_MESSAGE_STORE_H

#include <sys/types.h>

#include <stdint.h>

#include "kilonode/message.h"

#define KN_MESSAGE_STORE_PATH_MAX 512

enum kn_message_store_error {
	KN_MESSAGE_STORE_OK = 0,
	KN_MESSAGE_STORE_ERR_INVALID_ARGUMENT,
	KN_MESSAGE_STORE_ERR_INVALID_MESSAGE,
	KN_MESSAGE_STORE_ERR_IO,
	KN_MESSAGE_STORE_ERR_CORRUPT,
	KN_MESSAGE_STORE_ERR_NOT_FOUND,
	KN_MESSAGE_STORE_ERR_DELETED,
	KN_MESSAGE_STORE_ERR_BODY_TOO_LARGE,
	KN_MESSAGE_STORE_ERR_BUFFER
};

struct kn_message_store {
	char path[KN_MESSAGE_STORE_PATH_MAX];
	size_t max_body_bytes;
	uint64_t next_id;
	uint8_t open;
};

void kn_message_store_close(struct kn_message_store *);
enum kn_message_store_error kn_message_store_create_bulletin(
	struct kn_message_store *, const char *, const char *, const char *,
	const uint8_t *, size_t, uint64_t *);
enum kn_message_store_error kn_message_store_create_private(
	struct kn_message_store *, const char *, const char *, const char *,
	const uint8_t *, size_t, uint64_t *);
enum kn_message_store_error kn_message_store_delete(struct kn_message_store *,
	uint64_t);
void kn_message_store_init(struct kn_message_store *);
enum kn_message_store_error kn_message_store_list(struct kn_message_store *,
	struct kn_message *, size_t, size_t *);
enum kn_message_store_error kn_message_store_mark_read(
	struct kn_message_store *, uint64_t);
enum kn_message_store_error kn_message_store_open(struct kn_message_store *,
	const char *, size_t);
enum kn_message_store_error kn_message_store_read_body(
	struct kn_message_store *, uint64_t, uint8_t *, size_t, size_t *);
enum kn_message_store_error kn_message_store_read_metadata(
	struct kn_message_store *, uint64_t, struct kn_message *);
const char *kn_message_store_error_name(enum kn_message_store_error);

#endif
