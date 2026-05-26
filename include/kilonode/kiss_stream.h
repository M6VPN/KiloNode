/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/kiss_stream.h */

#ifndef KILONODE_KISS_STREAM_H
#define KILONODE_KISS_STREAM_H

#include <sys/types.h>

#include <stdint.h>

#include "kilonode/buffer.h"

#define KN_KISS_STREAM_DEFAULT_MAX_FRAME 4096
#define KN_KISS_STREAM_QUEUE_MAX         8

enum kn_kiss_stream_error {
	KN_KISS_STREAM_OK = 0,
	KN_KISS_STREAM_ERR_INVALID_ARGUMENT,
	KN_KISS_STREAM_ERR_BUFFER,
	KN_KISS_STREAM_ERR_INVALID_ESCAPE,
	KN_KISS_STREAM_ERR_OVERSIZED_FRAME,
	KN_KISS_STREAM_ERR_NO_FRAME
};

struct kn_kiss_stream_frame {
	uint8_t type;
	uint8_t command;
	uint8_t port;
	const uint8_t *payload;
	size_t payload_len;
};

struct kn_kiss_stream_parser {
	struct kn_buffer current;
	struct kn_buffer frames[KN_KISS_STREAM_QUEUE_MAX];
	uint8_t frame_types[KN_KISS_STREAM_QUEUE_MAX];
	size_t head;
	size_t count;
	size_t max_frame;
	uint8_t in_frame;
	uint8_t escaping;
	uint8_t dropping;
	enum kn_kiss_stream_error last_error;
};

void kn_kiss_stream_free(struct kn_kiss_stream_parser *);
enum kn_kiss_stream_error kn_kiss_stream_feed(struct kn_kiss_stream_parser *,
	const uint8_t *, size_t);
uint8_t kn_kiss_stream_has_frame(const struct kn_kiss_stream_parser *);
enum kn_kiss_stream_error kn_kiss_stream_init(struct kn_kiss_stream_parser *,
	size_t);
enum kn_kiss_stream_error kn_kiss_stream_pop_frame(
	struct kn_kiss_stream_parser *, struct kn_kiss_stream_frame *,
	struct kn_buffer *);
void kn_kiss_stream_reset(struct kn_kiss_stream_parser *);

#endif
