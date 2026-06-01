/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_ax25_payload_delivery.c */

#include <sys/types.h>

#include <string.h>

#include "kilonode/ax25_payload_delivery.h"

static int record_payload(struct kn_ax25_payload_delivery_queue *,
	const uint8_t *, size_t, uint8_t);
static int test_binary_payload(void);
static int test_queue_full(void);
static int test_rejected_payload(void);
static int test_text_payload(void);

int
main(void)
{
	if (test_text_payload() != 0)
		return 1;
	if (test_binary_payload() != 0)
		return 1;
	if (test_rejected_payload() != 0)
		return 1;
	if (test_queue_full() != 0)
		return 1;
	return 0;
}

static int
record_payload(struct kn_ax25_payload_delivery_queue *queue,
	const uint8_t *payload, size_t payload_len, uint8_t accepted)
{
	struct kn_callsign source;
	struct kn_callsign destination;

	if (kn_callsign_parse("M6VPN-1", &source) != 0 ||
	    kn_callsign_parse("N0CALL", &destination) != 0)
		return 1;
	return kn_ax25_payload_delivery_queue_record(queue, "A", "kiss0",
	    &source, &destination, 0, 0, payload, payload_len, accepted,
	    accepted != 0 ? "accepted" : "sequence", NULL) ==
	    KN_AX25_PAYLOAD_DELIVERY_OK ? 0 : 1;
}

static int
test_binary_payload(void)
{
	struct kn_ax25_payload_delivery_queue queue;
	const struct kn_ax25_payload_delivery_record *record;
	uint8_t payload[2];
	char hex[8];

	payload[0] = 0x00;
	payload[1] = 0xff;
	kn_ax25_payload_delivery_queue_init(&queue);
	if (record_payload(&queue, payload, sizeof(payload), 1) != 0)
		return 1;
	record = kn_ax25_payload_delivery_last_accepted(&queue);
	if (record == NULL || record->payload_is_text != 0)
		return 1;
	if (kn_ax25_payload_delivery_preview_hex(record, hex,
	    sizeof(hex)) != KN_AX25_PAYLOAD_DELIVERY_OK)
		return 1;
	return strcmp(hex, "00ff") == 0 ? 0 : 1;
}

static int
test_queue_full(void)
{
	struct kn_ax25_payload_delivery_queue queue;
	uint8_t payload;
	size_t i;

	payload = 'x';
	kn_ax25_payload_delivery_queue_init(&queue);
	for (i = 0; i < KN_AX25_PAYLOAD_DELIVERY_MAX; i++) {
		if (record_payload(&queue, &payload, 1, 1) != 0)
			return 1;
	}
	return record_payload(&queue, &payload, 1, 1) != 0 ? 0 : 1;
}

static int
test_rejected_payload(void)
{
	struct kn_ax25_payload_delivery_queue queue;
	const uint8_t payload[] = "bad";

	kn_ax25_payload_delivery_queue_init(&queue);
	if (record_payload(&queue, payload, sizeof(payload) - 1, 0) != 0)
		return 1;
	return queue.rejected_count == 1 &&
	    kn_ax25_payload_delivery_last_rejected(&queue) != NULL ? 0 : 1;
}

static int
test_text_payload(void)
{
	struct kn_ax25_payload_delivery_queue queue;
	const struct kn_ax25_payload_delivery_record *record;
	const uint8_t payload[] = "hello";

	kn_ax25_payload_delivery_queue_init(&queue);
	if (record_payload(&queue, payload, sizeof(payload) - 1, 1) != 0)
		return 1;
	record = kn_ax25_payload_delivery_last_accepted(&queue);
	return record != NULL && record->payload_is_text != 0 &&
	    record->preview_len == sizeof(payload) - 1 ? 0 : 1;
}
