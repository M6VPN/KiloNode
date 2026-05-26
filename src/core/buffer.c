/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/core/buffer.c */

#include <sys/types.h>

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "kilonode/buffer.h"

void
kn_buffer_clear(struct kn_buffer *buf)
{
	if (buf == NULL)
		return;

	buf->len = 0;
}

void
kn_buffer_free(struct kn_buffer *buf)
{
	if (buf == NULL)
		return;

	free(buf->data);
	buf->data = NULL;
	buf->len = 0;
	buf->cap = 0;
}

int
kn_buffer_init(struct kn_buffer *buf, size_t initial_cap)
{
	if (buf == NULL)
		return EINVAL;

	buf->data = NULL;
	buf->len = 0;
	buf->cap = 0;

	if (initial_cap == 0)
		return 0;

	buf->data = calloc(initial_cap, sizeof(*buf->data));
	if (buf->data == NULL)
		return ENOMEM;

	buf->cap = initial_cap;
	return 0;
}

int
kn_buffer_append(struct kn_buffer *buf, const uint8_t *data, size_t data_len)
{
	int rc;

	if (buf == NULL || (data == NULL && data_len > 0))
		return EINVAL;

	if (data_len == 0)
		return 0;

	if (data_len > SIZE_MAX - buf->len)
		return EOVERFLOW;

	rc = kn_buffer_reserve(buf, buf->len + data_len);
	if (rc != 0)
		return rc;

	memcpy(buf->data + buf->len, data, data_len);
	buf->len += data_len;

	return 0;
}

int
kn_buffer_append_byte(struct kn_buffer *buf, uint8_t byte)
{
	return kn_buffer_append(buf, &byte, sizeof(byte));
}

int
kn_buffer_reserve(struct kn_buffer *buf, size_t needed)
{
	uint8_t *new_data;
	size_t new_cap;

	if (buf == NULL)
		return EINVAL;

	if (needed <= buf->cap)
		return 0;

	new_cap = buf->cap == 0 ? 32 : buf->cap;
	while (new_cap < needed) {
		if (new_cap > SIZE_MAX / 2)
			return EOVERFLOW;
		new_cap *= 2;
	}

	new_data = realloc(buf->data, new_cap);
	if (new_data == NULL)
		return ENOMEM;

	buf->data = new_data;
	buf->cap = new_cap;

	return 0;
}
