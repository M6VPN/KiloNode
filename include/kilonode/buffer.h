/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/buffer.h */

#ifndef KILONODE_BUFFER_H
#define KILONODE_BUFFER_H

#include <sys/types.h>

#include <stdint.h>

struct kn_buffer {
	uint8_t *data;
	size_t len;
	size_t cap;
};

void kn_buffer_clear(struct kn_buffer *);
void kn_buffer_free(struct kn_buffer *);
int kn_buffer_init(struct kn_buffer *, size_t);
int kn_buffer_append(struct kn_buffer *, const uint8_t *, size_t);
int kn_buffer_append_byte(struct kn_buffer *, uint8_t);
int kn_buffer_reserve(struct kn_buffer *, size_t);

#endif
