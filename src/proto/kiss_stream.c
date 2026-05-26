/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/proto/kiss_stream.c */

#include <sys/types.h>

#include <stdint.h>
#include <string.h>

#include "kilonode/kiss.h"
#include "kilonode/kiss_stream.h"

static enum kn_kiss_stream_error append_byte(struct kn_kiss_stream_parser *,
	uint8_t);
static enum kn_kiss_stream_error enqueue_frame(struct kn_kiss_stream_parser *);
static enum kn_kiss_stream_error rc_from_buffer(int);
static void start_new_frame(struct kn_kiss_stream_parser *);

static enum kn_kiss_stream_error
append_byte(struct kn_kiss_stream_parser *parser, uint8_t byte)
{
	enum kn_kiss_stream_error rc;

	if (parser->current.len >= parser->max_frame) {
		parser->dropping = 1;
		parser->escaping = 0;
		kn_buffer_clear(&parser->current);
		return KN_KISS_STREAM_ERR_OVERSIZED_FRAME;
	}

	rc = rc_from_buffer(kn_buffer_append_byte(&parser->current, byte));
	if (rc != KN_KISS_STREAM_OK)
		return rc;

	return KN_KISS_STREAM_OK;
}

static enum kn_kiss_stream_error
enqueue_frame(struct kn_kiss_stream_parser *parser)
{
	size_t idx;
	enum kn_kiss_stream_error rc;

	if (parser->current.len == 0)
		return KN_KISS_STREAM_OK;

	if (parser->count >= KN_KISS_STREAM_QUEUE_MAX)
		return KN_KISS_STREAM_ERR_BUFFER;

	idx = (parser->head + parser->count) % KN_KISS_STREAM_QUEUE_MAX;
	kn_buffer_clear(&parser->frames[idx]);

	rc = rc_from_buffer(kn_buffer_append(&parser->frames[idx],
	    parser->current.data, parser->current.len));
	if (rc != KN_KISS_STREAM_OK)
		return rc;

	parser->frame_types[idx] = parser->frames[idx].data[0];
	parser->count++;

	return KN_KISS_STREAM_OK;
}

void
kn_kiss_stream_free(struct kn_kiss_stream_parser *parser)
{
	size_t i;

	if (parser == NULL)
		return;

	kn_buffer_free(&parser->current);
	for (i = 0; i < KN_KISS_STREAM_QUEUE_MAX; i++)
		kn_buffer_free(&parser->frames[i]);

	memset(parser, 0, sizeof(*parser));
}

enum kn_kiss_stream_error
kn_kiss_stream_feed(struct kn_kiss_stream_parser *parser, const uint8_t *data,
	size_t data_len)
{
	enum kn_kiss_stream_error first_error;
	enum kn_kiss_stream_error rc;
	size_t i;
	uint8_t byte;

	if (parser == NULL || (data == NULL && data_len > 0))
		return KN_KISS_STREAM_ERR_INVALID_ARGUMENT;

	first_error = KN_KISS_STREAM_OK;

	for (i = 0; i < data_len; i++) {
		byte = data[i];

		if (parser->escaping != 0) {
			parser->escaping = 0;

			if (byte == KN_KISS_TFEND) {
				rc = append_byte(parser, KN_KISS_FEND);
			} else if (byte == KN_KISS_TFESC) {
				rc = append_byte(parser, KN_KISS_FESC);
			} else {
				parser->dropping = 1;
				kn_buffer_clear(&parser->current);
				rc = KN_KISS_STREAM_ERR_INVALID_ESCAPE;
				if (byte == KN_KISS_FEND)
					start_new_frame(parser);
			}

			if (rc != KN_KISS_STREAM_OK &&
			    first_error == KN_KISS_STREAM_OK)
				first_error = rc;
			continue;
		}

		if (byte == KN_KISS_FEND) {
			if (parser->dropping != 0) {
				start_new_frame(parser);
				continue;
			}

			if (parser->in_frame == 0) {
				start_new_frame(parser);
				continue;
			}

			rc = enqueue_frame(parser);
			if (rc != KN_KISS_STREAM_OK &&
			    first_error == KN_KISS_STREAM_OK)
				first_error = rc;
			start_new_frame(parser);
			continue;
		}

		if (parser->in_frame == 0 || parser->dropping != 0)
			continue;

		if (byte == KN_KISS_FESC) {
			parser->escaping = 1;
			continue;
		}

		rc = append_byte(parser, byte);
		if (rc != KN_KISS_STREAM_OK &&
		    first_error == KN_KISS_STREAM_OK)
			first_error = rc;
	}

	parser->last_error = first_error;
	return first_error;
}

uint8_t
kn_kiss_stream_has_frame(const struct kn_kiss_stream_parser *parser)
{
	if (parser == NULL)
		return 0;

	return parser->count > 0 ? 1 : 0;
}

enum kn_kiss_stream_error
kn_kiss_stream_init(struct kn_kiss_stream_parser *parser, size_t max_frame)
{
	size_t i;
	enum kn_kiss_stream_error rc;

	if (parser == NULL)
		return KN_KISS_STREAM_ERR_INVALID_ARGUMENT;

	memset(parser, 0, sizeof(*parser));
	parser->max_frame = max_frame == 0 ?
	    KN_KISS_STREAM_DEFAULT_MAX_FRAME : max_frame;

	rc = rc_from_buffer(kn_buffer_init(&parser->current, 0));
	if (rc != KN_KISS_STREAM_OK)
		return rc;

	for (i = 0; i < KN_KISS_STREAM_QUEUE_MAX; i++) {
		rc = rc_from_buffer(kn_buffer_init(&parser->frames[i], 0));
		if (rc != KN_KISS_STREAM_OK) {
			kn_kiss_stream_free(parser);
			return rc;
		}
	}

	return KN_KISS_STREAM_OK;
}

enum kn_kiss_stream_error
kn_kiss_stream_pop_frame(struct kn_kiss_stream_parser *parser,
	struct kn_kiss_stream_frame *frame, struct kn_buffer *out)
{
	struct kn_buffer *stored;
	enum kn_kiss_stream_error rc;
	uint8_t type;

	if (parser == NULL || frame == NULL || out == NULL)
		return KN_KISS_STREAM_ERR_INVALID_ARGUMENT;

	if (parser->count == 0)
		return KN_KISS_STREAM_ERR_NO_FRAME;

	stored = &parser->frames[parser->head];
	if (stored->len == 0)
		return KN_KISS_STREAM_ERR_NO_FRAME;

	kn_buffer_clear(out);
	rc = rc_from_buffer(kn_buffer_append(out, stored->data, stored->len));
	if (rc != KN_KISS_STREAM_OK)
		return rc;

	type = parser->frame_types[parser->head];
	memset(frame, 0, sizeof(*frame));
	frame->type = type;
	frame->command = (uint8_t)(type & 0x0fu);
	frame->port = (uint8_t)((type >> 4) & 0x0fu);
	frame->payload = out->len > 1 ? out->data + 1 : NULL;
	frame->payload_len = out->len > 0 ? out->len - 1 : 0;

	kn_buffer_clear(stored);
	parser->head = (parser->head + 1) % KN_KISS_STREAM_QUEUE_MAX;
	parser->count--;

	if (parser->count == 0)
		parser->head = 0;

	return KN_KISS_STREAM_OK;
}

void
kn_kiss_stream_reset(struct kn_kiss_stream_parser *parser)
{
	size_t i;

	if (parser == NULL)
		return;

	kn_buffer_clear(&parser->current);
	for (i = 0; i < KN_KISS_STREAM_QUEUE_MAX; i++)
		kn_buffer_clear(&parser->frames[i]);

	parser->head = 0;
	parser->count = 0;
	parser->in_frame = 0;
	parser->escaping = 0;
	parser->dropping = 0;
	parser->last_error = KN_KISS_STREAM_OK;
}

static enum kn_kiss_stream_error
rc_from_buffer(int rc)
{
	if (rc == 0)
		return KN_KISS_STREAM_OK;

	return KN_KISS_STREAM_ERR_BUFFER;
}

static void
start_new_frame(struct kn_kiss_stream_parser *parser)
{
	kn_buffer_clear(&parser->current);
	parser->in_frame = 1;
	parser->escaping = 0;
	parser->dropping = 0;
}
