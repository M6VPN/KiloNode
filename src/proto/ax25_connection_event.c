/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/proto/ax25_connection_event.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "kilonode/ax25_connection_event.h"

static uint8_t callsign_equal(const struct kn_callsign *,
	const struct kn_callsign *);
static enum kn_ax25_connection_event_error event_from_key(
	struct kn_ax25_connection_event_record *, uint64_t,
	const struct kn_ax25_connection_key *, enum kn_ax25_connection_event);

static uint8_t
callsign_equal(const struct kn_callsign *a, const struct kn_callsign *b)
{
	if (a == NULL || b == NULL)
		return 0;
	if (a->ssid != b->ssid)
		return 0;
	return strcmp(a->call, b->call) == 0 ? 1 : 0;
}

static enum kn_ax25_connection_event_error
event_from_key(struct kn_ax25_connection_event_record *event, uint64_t timestamp,
	const struct kn_ax25_connection_key *key,
	enum kn_ax25_connection_event kind)
{
	if (event == NULL || key == NULL)
		return KN_AX25_CONNECTION_EVENT_ERR_INVALID_ARGUMENT;
	if (kn_ax25_connection_key_validate(key) !=
	    KN_AX25_CONNECTION_KEY_OK)
		return KN_AX25_CONNECTION_EVENT_ERR_INVALID_VALUE;

	kn_ax25_connection_event_clear(event);
	event->timestamp = timestamp;
	event->kind = kind;
	event->key = *key;
	return KN_AX25_CONNECTION_EVENT_OK;
}

void
kn_ax25_connection_event_clear(struct kn_ax25_connection_event_record *event)
{
	if (event == NULL)
		return;

	memset(event, 0, sizeof(*event));
	event->control.class = KN_AX25_CONTROL_CLASS_UNKNOWN;
	event->control.s_subtype = KN_AX25_S_SUBTYPE_UNKNOWN;
	event->control.u_subtype = KN_AX25_U_SUBTYPE_UNKNOWN;
}

enum kn_ax25_connection_event_error
kn_ax25_connection_event_from_frame(struct kn_ax25_connection_event_record *event,
	uint64_t timestamp, const char *port_name,
	const struct kn_callsign *local, const struct kn_ax25_frame *frame)
{
	enum kn_ax25_connection_event kind;

	if (event == NULL || port_name == NULL || local == NULL ||
	    frame == NULL)
		return KN_AX25_CONNECTION_EVENT_ERR_INVALID_ARGUMENT;
	if (callsign_equal(&frame->destination.callsign, local) == 0)
		return KN_AX25_CONNECTION_EVENT_ERR_NOT_FOR_LOCAL;

	kn_ax25_connection_event_clear(event);
	kn_ax25_control_decode(frame->control, &event->control);
	kind = kn_ax25_connection_event_from_control(frame->control);
	if (kind == KN_AX25_CONNECTION_EVENT_NONE)
		return KN_AX25_CONNECTION_EVENT_ERR_IGNORED;
	if (kn_ax25_connection_key_from_frame(&event->key, port_name, local,
	    frame) != KN_AX25_CONNECTION_KEY_OK)
		return KN_AX25_CONNECTION_EVENT_ERR_INVALID_VALUE;

	event->timestamp = timestamp;
	event->kind = kind;
	event->ns = event->control.ns;
	event->nr = event->control.nr;
	event->poll_final = event->control.poll_final;
	event->payload_len = frame->payload_len;
	return KN_AX25_CONNECTION_EVENT_OK;
}

enum kn_ax25_connection_event_error
kn_ax25_connection_event_from_frame_pair(
	struct kn_ax25_connection_event_record *event, uint64_t timestamp,
	const char *port_name, const struct kn_callsign *local,
	const struct kn_ax25_frame *frame)
{
	const struct kn_callsign *remote;
	enum kn_ax25_connection_event kind;
	size_t i;
	int needed;

	if (event == NULL || port_name == NULL || local == NULL ||
	    frame == NULL)
		return KN_AX25_CONNECTION_EVENT_ERR_INVALID_ARGUMENT;
	if (callsign_equal(&frame->destination.callsign, local) != 0)
		remote = &frame->source.callsign;
	else if (callsign_equal(&frame->source.callsign, local) != 0)
		remote = &frame->destination.callsign;
	else
		return KN_AX25_CONNECTION_EVENT_ERR_NOT_FOR_LOCAL;

	kn_ax25_connection_event_clear(event);
	kn_ax25_control_decode(frame->control, &event->control);
	kind = kn_ax25_connection_event_from_control(frame->control);
	if (kind == KN_AX25_CONNECTION_EVENT_NONE)
		return KN_AX25_CONNECTION_EVENT_ERR_IGNORED;

	kn_ax25_connection_key_clear(&event->key);
	needed = snprintf(event->key.port_name,
	    sizeof(event->key.port_name), "%s", port_name);
	if (needed < 0 || (size_t)needed >= sizeof(event->key.port_name))
		return KN_AX25_CONNECTION_EVENT_ERR_INVALID_VALUE;
	event->key.local = *local;
	event->key.remote = *remote;
	event->key.digipeater_count = frame->digipeater_count;
	if (event->key.digipeater_count > KN_AX25_MAX_DIGIS)
		return KN_AX25_CONNECTION_EVENT_ERR_INVALID_VALUE;
	for (i = 0; i < event->key.digipeater_count; i++)
		event->key.digipeaters[i] = frame->digipeaters[i].callsign;
	if (kn_ax25_connection_key_validate(&event->key) !=
	    KN_AX25_CONNECTION_KEY_OK)
		return KN_AX25_CONNECTION_EVENT_ERR_INVALID_VALUE;

	event->timestamp = timestamp;
	event->kind = kind;
	event->ns = event->control.ns;
	event->nr = event->control.nr;
	event->poll_final = event->control.poll_final;
	event->payload_len = frame->payload_len;
	return KN_AX25_CONNECTION_EVENT_OK;
}

enum kn_ax25_connection_event_error
kn_ax25_connection_event_local_connect(struct kn_ax25_connection_event_record *event,
	uint64_t timestamp, const struct kn_ax25_connection_key *key)
{
	return event_from_key(event, timestamp, key,
	    KN_AX25_CONNECTION_EVENT_LOCAL_CONNECT_REQUEST);
}

enum kn_ax25_connection_event_error
kn_ax25_connection_event_local_disconnect(
	struct kn_ax25_connection_event_record *event, uint64_t timestamp,
	const struct kn_ax25_connection_key *key)
{
	return event_from_key(event, timestamp, key,
	    KN_AX25_CONNECTION_EVENT_LOCAL_DISCONNECT_REQUEST);
}

enum kn_ax25_connection_event_error
kn_ax25_connection_event_timeout_t1(struct kn_ax25_connection_event_record *event,
	uint64_t timestamp, const struct kn_ax25_connection_key *key)
{
	return event_from_key(event, timestamp, key,
	    KN_AX25_CONNECTION_EVENT_TIMEOUT_T1);
}

enum kn_ax25_connection_event_error
kn_ax25_connection_event_timeout_t3(struct kn_ax25_connection_event_record *event,
	uint64_t timestamp, const struct kn_ax25_connection_key *key)
{
	return event_from_key(event, timestamp, key,
	    KN_AX25_CONNECTION_EVENT_TIMEOUT_T3);
}

enum kn_ax25_connection_event_error
kn_ax25_connection_event_to_state_input(
	const struct kn_ax25_connection_event_record *event,
	struct kn_ax25_state_input *input)
{
	if (event == NULL || input == NULL)
		return KN_AX25_CONNECTION_EVENT_ERR_INVALID_ARGUMENT;
	if (event->kind == KN_AX25_CONNECTION_EVENT_NONE)
		return KN_AX25_CONNECTION_EVENT_ERR_INVALID_VALUE;

	kn_ax25_state_input_clear(input);
	input->event = event->kind;
	input->ns = event->ns;
	input->nr = event->nr;
	input->poll_final = event->poll_final;
	input->payload_len = event->payload_len;
	return KN_AX25_CONNECTION_EVENT_OK;
}
