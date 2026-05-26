/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/message_index.h */

#ifndef KILONODE_MESSAGE_INDEX_H
#define KILONODE_MESSAGE_INDEX_H

#include <sys/types.h>

#include <stdint.h>

#include "kilonode/message_store.h"

#define KN_MESSAGE_INDEX_AREA_MAX 128

enum kn_message_index_error {
	KN_MESSAGE_INDEX_OK = 0,
	KN_MESSAGE_INDEX_ERR_INVALID_ARGUMENT,
	KN_MESSAGE_INDEX_ERR_IO,
	KN_MESSAGE_INDEX_ERR_CORRUPT,
	KN_MESSAGE_INDEX_ERR_BUFFER,
	KN_MESSAGE_INDEX_ERR_STORE
};

enum kn_message_index_filter {
	KN_MESSAGE_INDEX_ALL = 0,
	KN_MESSAGE_INDEX_PRIVATE,
	KN_MESSAGE_INDEX_BULLETIN,
	KN_MESSAGE_INDEX_AREA,
	KN_MESSAGE_INDEX_TO,
	KN_MESSAGE_INDEX_FROM
};

struct kn_message_index_area {
	char name[KN_MESSAGE_AREA_MAX + 1];
	size_t count;
	uint64_t newest_id;
	uint64_t newest_created;
};

enum kn_message_index_error kn_message_index_add(
	struct kn_message_store *, const struct kn_message *);
enum kn_message_index_error kn_message_index_areas(
	struct kn_message_store *, struct kn_message_index_area *, size_t,
	size_t *);
enum kn_message_index_error kn_message_index_count(
	struct kn_message_store *, enum kn_message_index_filter, const char *,
	size_t *);
enum kn_message_index_error kn_message_index_init(
	struct kn_message_store *);
enum kn_message_index_error kn_message_index_list(
	struct kn_message_store *, enum kn_message_index_filter, const char *,
	struct kn_message *, size_t, size_t *);
enum kn_message_index_error kn_message_index_rebuild(
	struct kn_message_store *);
const char *kn_message_index_error_name(enum kn_message_index_error);

#endif
