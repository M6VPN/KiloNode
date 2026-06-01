/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_ax25_reassembly.c */

#include <sys/types.h>

#include <stdint.h>
#include <string.h>

#include "kilonode/ax25_reassembly.h"

static void callsign_set(struct kn_callsign *, const char *);
static int test_binary_reassembly(void);
static int test_format(void);
static int test_queue_full(void);
static int test_record_complete(void);
static int test_record_invalid_order(void);
static int test_reset(void);
static int test_truncated_preview(void);

int
main(void)
{
	if (test_record_complete() != 0)
		return 1;
	if (test_binary_reassembly() != 0)
		return 1;
	if (test_record_invalid_order() != 0)
		return 1;
	if (test_truncated_preview() != 0)
		return 1;
	if (test_queue_full() != 0)
		return 1;
	if (test_reset() != 0)
		return 1;
	if (test_format() != 0)
		return 1;
	return 0;
}

static void
callsign_set(struct kn_callsign *callsign, const char *text)
{
	(void)kn_callsign_parse(text, callsign);
}

static int
test_binary_reassembly(void)
{
	struct kn_ax25_reassembly_queue queue;
	struct kn_callsign source;
	struct kn_callsign destination;
	const struct kn_ax25_reassembly_record *record;
	const uint8_t payload[] = { 0x00, 0xff, 0x41 };

	kn_ax25_reassembly_queue_init(&queue);
	callsign_set(&source, "M6VPN-1");
	callsign_set(&destination, "N0CALL");
	if (kn_ax25_reassembly_queue_record(&queue, "B", "kiss0",
	    &source, &destination, 2, 2, payload, sizeof(payload), 1,
	    "complete", NULL) != KN_AX25_REASSEMBLY_OK)
		return 1;
	record = kn_ax25_reassembly_last_complete(&queue);
	return record != NULL && record->payload_is_text == 0 &&
	    record->preview[1] == 0xff ? 0 : 1;
}

static int
test_format(void)
{
	struct kn_ax25_reassembly_queue queue;
	struct kn_callsign source;
	struct kn_callsign destination;
	char buf[256];
	const uint8_t payload[] = { 'o', 'k' };

	kn_ax25_reassembly_queue_init(&queue);
	callsign_set(&source, "M6VPN-1");
	callsign_set(&destination, "N0CALL");
	if (kn_ax25_reassembly_queue_record(&queue, "B", "kiss0",
	    &source, &destination, 1, 1, payload, sizeof(payload), 1,
	    "complete", NULL) != KN_AX25_REASSEMBLY_OK)
		return 1;
	if (kn_ax25_reassembly_format(&queue.records[0], buf, sizeof(buf)) !=
	    KN_AX25_REASSEMBLY_OK)
		return 1;
	return strstr(buf, "complete=true") != NULL ? 0 : 1;
}

static int
test_queue_full(void)
{
	struct kn_ax25_reassembly_queue queue;
	struct kn_callsign source;
	struct kn_callsign destination;
	uint8_t payload;
	size_t i;

	kn_ax25_reassembly_queue_init(&queue);
	callsign_set(&source, "M6VPN-1");
	callsign_set(&destination, "N0CALL");
	payload = 'x';
	for (i = 0; i < KN_AX25_REASSEMBLY_MAX; i++) {
		if (kn_ax25_reassembly_queue_record(&queue, "B", "kiss0",
		    &source, &destination, 1, 1, &payload, 1, 1,
		    "complete", NULL) != KN_AX25_REASSEMBLY_OK)
			return 1;
	}
	return kn_ax25_reassembly_queue_record(&queue, "B", "kiss0",
	    &source, &destination, 1, 1, &payload, 1, 1, "complete",
	    NULL) == KN_AX25_REASSEMBLY_ERR_FULL ? 0 : 1;
}

static int
test_record_complete(void)
{
	struct kn_ax25_reassembly_queue queue;
	struct kn_callsign source;
	struct kn_callsign destination;
	const uint8_t payload[] = { 'h', 'e', 'l', 'l', 'o' };

	kn_ax25_reassembly_queue_init(&queue);
	callsign_set(&source, "M6VPN-1");
	callsign_set(&destination, "N0CALL");
	if (kn_ax25_reassembly_queue_record(&queue, "B", "kiss0",
	    &source, &destination, 2, 2, payload, sizeof(payload), 1,
	    "complete", NULL) != KN_AX25_REASSEMBLY_OK)
		return 1;
	return queue.complete_count == 1 && queue.records[0].preview_len == 5 ?
	    0 : 1;
}

static int
test_record_invalid_order(void)
{
	struct kn_ax25_reassembly_queue queue;
	struct kn_callsign source;
	struct kn_callsign destination;
	const uint8_t payload[] = { 'x' };

	kn_ax25_reassembly_queue_init(&queue);
	callsign_set(&source, "M6VPN-1");
	callsign_set(&destination, "N0CALL");
	return kn_ax25_reassembly_queue_record(&queue, "B", "kiss0",
	    &source, &destination, 1, 2, payload, sizeof(payload), 0,
	    "order", NULL) == KN_AX25_REASSEMBLY_ERR_INVALID_VALUE ? 0 : 1;
}

static int
test_reset(void)
{
	struct kn_ax25_reassembly_queue queue;

	kn_ax25_reassembly_queue_init(&queue);
	kn_ax25_reassembly_queue_reset(&queue);
	return queue.count == 0 && queue.next_id == 1 ? 0 : 1;
}

static int
test_truncated_preview(void)
{
	struct kn_ax25_reassembly_queue queue;
	struct kn_callsign source;
	struct kn_callsign destination;
	uint8_t payload[KN_AX25_REASSEMBLY_PREVIEW_MAX + 5];

	memset(payload, 'a', sizeof(payload));
	kn_ax25_reassembly_queue_init(&queue);
	callsign_set(&source, "M6VPN-1");
	callsign_set(&destination, "N0CALL");
	if (kn_ax25_reassembly_queue_record(&queue, "B", "kiss0",
	    &source, &destination, 2, 2, payload, sizeof(payload), 1,
	    "complete", NULL) != KN_AX25_REASSEMBLY_OK)
		return 1;
	return queue.records[0].preview_len ==
	    KN_AX25_REASSEMBLY_PREVIEW_MAX ? 0 : 1;
}
