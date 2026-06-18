/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/proto/ax25_loopback_window.c */

#include <sys/types.h>

#include <string.h>

#include "kilonode/ax25_loopback_window.h"

static uint8_t is_acked_by_rr(uint8_t, uint8_t);
static void refresh_max_in_flight(struct kn_ax25_loopback_window *);

static uint8_t
is_acked_by_rr(uint8_t ns, uint8_t nr)
{
	uint8_t distance;

	if (ns > 7 || nr > 7)
		return 0;
	distance = (uint8_t)((nr + 8U - ns) & 0x07U);
	return distance > 0 ? 1 : 0;
}

static void
refresh_max_in_flight(struct kn_ax25_loopback_window *window)
{
	size_t in_flight;

	in_flight = kn_ax25_loopback_window_in_flight(window);
	if (in_flight > window->max_in_flight_seen)
		window->max_in_flight_seen = (uint64_t)in_flight;
}

enum kn_ax25_loopback_window_error
kn_ax25_loopback_window_ack_rr(struct kn_ax25_loopback_window *window,
	uint8_t nr, size_t *acked)
{
	size_t i;
	size_t count;

	if (window == NULL || acked == NULL)
		return KN_AX25_LOOPBACK_WINDOW_ERR_INVALID_ARGUMENT;
	if (nr > 7)
		return KN_AX25_LOOPBACK_WINDOW_ERR_INVALID_VALUE;

	count = 0;
	for (i = 0; i < window->count; i++) {
		if (window->frames[i].status !=
		    KN_AX25_LOOPBACK_WINDOW_STATUS_IN_FLIGHT)
			continue;
		if (is_acked_by_rr(window->frames[i].ns, nr) == 0)
			continue;
		window->frames[i].status =
		    KN_AX25_LOOPBACK_WINDOW_STATUS_ACKED;
		count++;
	}
	window->acked += count;
	*acked = count;
	return KN_AX25_LOOPBACK_WINDOW_OK;
}

size_t
kn_ax25_loopback_window_in_flight(
	const struct kn_ax25_loopback_window *window)
{
	size_t i;
	size_t count;

	if (window == NULL)
		return 0;
	count = 0;
	for (i = 0; i < window->count; i++) {
		if (window->frames[i].status ==
		    KN_AX25_LOOPBACK_WINDOW_STATUS_IN_FLIGHT)
			count++;
	}
	return count;
}

void
kn_ax25_loopback_window_init(struct kn_ax25_loopback_window *window)
{
	if (window == NULL)
		return;

	memset(window, 0, sizeof(*window));
	window->next_id = 1;
}

enum kn_ax25_loopback_window_error
kn_ax25_loopback_window_record(struct kn_ax25_loopback_window *window,
	uint8_t ns, uint8_t nr, size_t payload_len, size_t segment_index,
	size_t window_size)
{
	struct kn_ax25_loopback_window_frame *frame;

	if (window == NULL)
		return KN_AX25_LOOPBACK_WINDOW_ERR_INVALID_ARGUMENT;
	if (ns > 7 || nr > 7 || window_size == 0)
		return KN_AX25_LOOPBACK_WINDOW_ERR_INVALID_VALUE;
	if (kn_ax25_loopback_window_in_flight(window) >= window_size) {
		window->blocked++;
		return KN_AX25_LOOPBACK_WINDOW_ERR_BLOCKED;
	}
	if (window->count >= KN_AX25_LOOPBACK_WINDOW_MAX)
		return KN_AX25_LOOPBACK_WINDOW_ERR_FULL;

	frame = &window->frames[window->count++];
	memset(frame, 0, sizeof(*frame));
	frame->id = window->next_id++;
	frame->ns = ns;
	frame->nr = nr;
	frame->payload_len = payload_len;
	frame->segment_index = segment_index;
	frame->status = KN_AX25_LOOPBACK_WINDOW_STATUS_IN_FLIGHT;
	window->recorded++;
	refresh_max_in_flight(window);
	return KN_AX25_LOOPBACK_WINDOW_OK;
}

enum kn_ax25_loopback_window_error
kn_ax25_loopback_window_reject(struct kn_ax25_loopback_window *window,
	uint8_t nr, size_t *rejected)
{
	size_t i;
	size_t count;

	if (window == NULL || rejected == NULL)
		return KN_AX25_LOOPBACK_WINDOW_ERR_INVALID_ARGUMENT;
	if (nr > 7)
		return KN_AX25_LOOPBACK_WINDOW_ERR_INVALID_VALUE;

	count = 0;
	for (i = 0; i < window->count; i++) {
		if (window->frames[i].status !=
		    KN_AX25_LOOPBACK_WINDOW_STATUS_IN_FLIGHT)
			continue;
		if (window->frames[i].ns != nr)
			continue;
		window->frames[i].status =
		    KN_AX25_LOOPBACK_WINDOW_STATUS_REJECTED;
		count++;
	}
	window->rejected += count;
	*rejected = count;
	return KN_AX25_LOOPBACK_WINDOW_OK;
}

void
kn_ax25_loopback_window_reset(struct kn_ax25_loopback_window *window)
{
	kn_ax25_loopback_window_init(window);
}
