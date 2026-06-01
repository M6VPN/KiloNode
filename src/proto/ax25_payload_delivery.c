/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/proto/ax25_payload_delivery.c */

#include <sys/types.h>

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "kilonode/ax25_payload_delivery.h"

static uint8_t payload_is_text(const uint8_t *, size_t);

static uint8_t
payload_is_text(const uint8_t *payload, size_t payload_len)
{
	size_t i;

	if (payload == NULL && payload_len > 0)
		return 0;
	for (i = 0; i < payload_len; i++) {
		if (payload[i] == '\0')
			return 0;
		if (payload[i] != '\n' && payload[i] != '\r' &&
		    payload[i] != '\t' && isprint(payload[i]) == 0)
			return 0;
	}
	return 1;
}

enum kn_ax25_payload_delivery_error
kn_ax25_payload_delivery_format(
	const struct kn_ax25_payload_delivery_record *record, char *buf,
	size_t bufsiz)
{
	char source[KN_CALLSIGN_MAX + 4];
	char destination[KN_CALLSIGN_MAX + 4];
	char hex[KN_AX25_PAYLOAD_DELIVERY_PREVIEW_MAX * 2 + 1];
	int needed;

	if (record == NULL || buf == NULL || bufsiz == 0)
		return KN_AX25_PAYLOAD_DELIVERY_ERR_INVALID_ARGUMENT;
	if (kn_callsign_format(&record->source, source, sizeof(source)) != 0 ||
	    kn_callsign_format(&record->destination, destination,
	    sizeof(destination)) != 0)
		return KN_AX25_PAYLOAD_DELIVERY_ERR_INVALID_VALUE;
	if (kn_ax25_payload_delivery_preview_hex(record, hex,
	    sizeof(hex)) != KN_AX25_PAYLOAD_DELIVERY_OK)
		return KN_AX25_PAYLOAD_DELIVERY_ERR_BUFFER;
	needed = snprintf(buf, bufsiz,
	    "delivery=%llu endpoint=%s port=%s source=%s destination=%s "
	    "ns=%u nr=%u len=%llu text=%s accepted=%s reason=%s hex=%s",
	    (unsigned long long)record->id, record->endpoint,
	    record->port_name, source, destination, record->ns, record->nr,
	    (unsigned long long)record->payload_len,
	    record->payload_is_text != 0 ? "true" : "false",
	    record->accepted != 0 ? "true" : "false",
	    record->reason[0] == '\0' ? "-" : record->reason, hex);
	if (needed < 0 || (size_t)needed >= bufsiz)
		return KN_AX25_PAYLOAD_DELIVERY_ERR_BUFFER;
	return KN_AX25_PAYLOAD_DELIVERY_OK;
}

const struct kn_ax25_payload_delivery_record *
kn_ax25_payload_delivery_last_accepted(
	const struct kn_ax25_payload_delivery_queue *queue)
{
	size_t i;

	if (queue == NULL)
		return NULL;
	for (i = queue->count; i > 0; i--) {
		if (queue->records[i - 1].accepted != 0)
			return &queue->records[i - 1];
	}
	return NULL;
}

const struct kn_ax25_payload_delivery_record *
kn_ax25_payload_delivery_last_rejected(
	const struct kn_ax25_payload_delivery_queue *queue)
{
	size_t i;

	if (queue == NULL)
		return NULL;
	for (i = queue->count; i > 0; i--) {
		if (queue->records[i - 1].accepted == 0)
			return &queue->records[i - 1];
	}
	return NULL;
}

void
kn_ax25_payload_delivery_queue_init(
	struct kn_ax25_payload_delivery_queue *queue)
{
	if (queue == NULL)
		return;
	memset(queue, 0, sizeof(*queue));
	queue->next_id = 1;
}

enum kn_ax25_payload_delivery_error
kn_ax25_payload_delivery_queue_record(
	struct kn_ax25_payload_delivery_queue *queue, const char *endpoint,
	const char *port_name, const struct kn_callsign *source,
	const struct kn_callsign *destination, uint8_t ns, uint8_t nr,
	const uint8_t *payload, size_t payload_len, uint8_t accepted,
	const char *reason, uint64_t *id)
{
	struct kn_ax25_payload_delivery_record *record;
	size_t preview_len;
	int needed;

	if (queue == NULL || endpoint == NULL || port_name == NULL ||
	    source == NULL || destination == NULL)
		return KN_AX25_PAYLOAD_DELIVERY_ERR_INVALID_ARGUMENT;
	if (payload == NULL && payload_len > 0)
		return KN_AX25_PAYLOAD_DELIVERY_ERR_INVALID_ARGUMENT;
	if (ns > 7 || nr > 7)
		return KN_AX25_PAYLOAD_DELIVERY_ERR_INVALID_VALUE;
	if (queue->count >= KN_AX25_PAYLOAD_DELIVERY_MAX)
		return KN_AX25_PAYLOAD_DELIVERY_ERR_FULL;

	record = &queue->records[queue->count];
	memset(record, 0, sizeof(*record));
	record->id = queue->next_id++;
	needed = snprintf(record->endpoint, sizeof(record->endpoint), "%s",
	    endpoint);
	if (needed < 0 || (size_t)needed >= sizeof(record->endpoint))
		return KN_AX25_PAYLOAD_DELIVERY_ERR_INVALID_VALUE;
	needed = snprintf(record->port_name, sizeof(record->port_name), "%s",
	    port_name);
	if (needed < 0 || (size_t)needed >= sizeof(record->port_name))
		return KN_AX25_PAYLOAD_DELIVERY_ERR_INVALID_VALUE;
	record->source = *source;
	record->destination = *destination;
	record->ns = ns;
	record->nr = nr;
	record->payload_len = payload_len;
	preview_len = payload_len;
	if (preview_len > KN_AX25_PAYLOAD_DELIVERY_PREVIEW_MAX)
		preview_len = KN_AX25_PAYLOAD_DELIVERY_PREVIEW_MAX;
	record->preview_len = preview_len;
	if (preview_len > 0)
		memcpy(record->preview, payload, preview_len);
	record->payload_is_text = payload_is_text(payload, payload_len);
	record->accepted = accepted != 0 ? 1 : 0;
	needed = snprintf(record->reason, sizeof(record->reason), "%s",
	    reason == NULL ? "" : reason);
	if (needed < 0 || (size_t)needed >= sizeof(record->reason))
		return KN_AX25_PAYLOAD_DELIVERY_ERR_INVALID_VALUE;

	if (record->accepted != 0)
		queue->accepted_count++;
	else
		queue->rejected_count++;
	queue->count++;
	if (id != NULL)
		*id = record->id;
	return KN_AX25_PAYLOAD_DELIVERY_OK;
}

void
kn_ax25_payload_delivery_queue_reset(
	struct kn_ax25_payload_delivery_queue *queue)
{
	kn_ax25_payload_delivery_queue_init(queue);
}

enum kn_ax25_payload_delivery_error
kn_ax25_payload_delivery_preview_hex(
	const struct kn_ax25_payload_delivery_record *record, char *buf,
	size_t bufsiz)
{
	size_t i;
	int needed;
	size_t used;

	if (record == NULL || buf == NULL || bufsiz == 0)
		return KN_AX25_PAYLOAD_DELIVERY_ERR_INVALID_ARGUMENT;
	buf[0] = '\0';
	used = 0;
	for (i = 0; i < record->preview_len; i++) {
		needed = snprintf(buf + used, bufsiz - used, "%02x",
		    record->preview[i]);
		if (needed < 0 || (size_t)needed >= bufsiz - used)
			return KN_AX25_PAYLOAD_DELIVERY_ERR_BUFFER;
		used += (size_t)needed;
	}
	return KN_AX25_PAYLOAD_DELIVERY_OK;
}
