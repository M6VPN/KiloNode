/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_daemon_heard.c */

#include <sys/types.h>

#include <stdint.h>
#include <string.h>

#include "kilonode/ax25.h"
#include "kilonode/buffer.h"
#include "kilonode/callsign.h"
#include "kilonode/config.h"
#include "kilonode/heard.h"
#include "kilonode/stats.h"

static int addr_set(struct kn_ax25_addr *, const char *);
static int decoded_frame(struct kn_ax25_frame *, struct kn_buffer *);
static int test_malformed_ax25_not_heard(void);
static int test_per_port_heard_separate(void);
static int test_stats_and_heard_same_frame(void);
static int test_valid_ui_updates_heard(void);

int
main(void)
{
	if (test_valid_ui_updates_heard() != 0)
		return 1;
	if (test_malformed_ax25_not_heard() != 0)
		return 1;
	if (test_per_port_heard_separate() != 0)
		return 1;
	if (test_stats_and_heard_same_frame() != 0)
		return 1;

	return 0;
}

static int
addr_set(struct kn_ax25_addr *addr, const char *input)
{
	memset(addr, 0, sizeof(*addr));
	return kn_callsign_parse(input, &addr->callsign);
}

static int
decoded_frame(struct kn_ax25_frame *decoded, struct kn_buffer *buf)
{
	struct kn_ax25_frame frame;
	static const uint8_t payload[] = { 'h', 'i' };

	if (kn_buffer_init(buf, 0) != 0)
		return 1;

	kn_ax25_frame_reset(&frame);
	if (addr_set(&frame.source, "M6VPN-1") != 0)
		return 1;
	if (addr_set(&frame.destination, "CQ") != 0)
		return 1;
	frame.pid = KN_AX25_PID_NO_LAYER_3;
	frame.payload = payload;
	frame.payload_len = sizeof(payload);
	if (kn_ax25_ui_frame_encode(&frame, buf) != KN_AX25_OK)
		return 1;

	return kn_ax25_frame_decode(buf->data, buf->len, decoded) ==
	    KN_AX25_OK ? 0 : 1;
}

static int
test_malformed_ax25_not_heard(void)
{
	struct kn_heard_table heard;
	struct kn_ax25_frame decoded;
	const uint8_t bad[] = { 0x00, 0x01 };

	kn_heard_init(&heard, 4);
	if (kn_ax25_frame_decode(bad, sizeof(bad), &decoded) == KN_AX25_OK)
		return 1;

	return kn_heard_count(&heard) == 0 ? 0 : 1;
}

static int
test_per_port_heard_separate(void)
{
	struct kn_heard_table heard;
	struct kn_ax25_frame decoded;
	struct kn_buffer buf;
	int rc;

	kn_heard_init(&heard, 4);
	if (decoded_frame(&decoded, &buf) != 0)
		return 1;
	if (kn_heard_update(&heard, "kiss0", &decoded, 1) != KN_HEARD_OK)
		return 1;
	if (kn_heard_update(&heard, "kiss1", &decoded, 2) != KN_HEARD_OK)
		return 1;
	rc = kn_heard_count(&heard) == 2 ? 0 : 1;
	kn_buffer_free(&buf);
	return rc;
}

static int
test_stats_and_heard_same_frame(void)
{
	struct kn_heard_table heard;
	struct kn_daemon_stats daemon_stats;
	struct kn_port_stats port_stats;
	struct kn_config_port port;
	struct kn_ax25_frame decoded;
	struct kn_buffer buf;
	int rc;

	memset(&port, 0, sizeof(port));
	memcpy(port.name, "kiss0", 6);
	port.type = KN_CONFIG_PORT_STDIO;
	port.enabled = 1;
	kn_daemon_stats_init(&daemon_stats, 1, 1);
	kn_port_stats_init(&port_stats, &port);
	kn_heard_init(&heard, 4);
	if (decoded_frame(&decoded, &buf) != 0)
		return 1;

	kn_stats_add_ax25_frame(&daemon_stats, &port_stats);
	if (kn_heard_update(&heard, "kiss0", &decoded, 1) != KN_HEARD_OK)
		return 1;
	rc = daemon_stats.ax25_frames_decoded == 1 &&
	    kn_heard_count(&heard) == 1 ? 0 : 1;
	kn_buffer_free(&buf);
	return rc;
}

static int
test_valid_ui_updates_heard(void)
{
	struct kn_heard_table heard;
	struct kn_ax25_frame decoded;
	struct kn_buffer buf;
	int rc;

	kn_heard_init(&heard, 4);
	if (decoded_frame(&decoded, &buf) != 0)
		return 1;
	if (kn_heard_update(&heard, "kiss0", &decoded, 1) != KN_HEARD_OK)
		return 1;

	rc = kn_heard_count(&heard) == 1 &&
	    strcmp(heard.entries[0].source.call, "M6VPN") == 0 ? 0 : 1;
	kn_buffer_free(&buf);
	return rc;
}
