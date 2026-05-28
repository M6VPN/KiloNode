/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_ax25_loopback_link.c */

#include <sys/types.h>

#include <stddef.h>

#include "kilonode/ax25_loopback_link.h"

static int test_empty_link(void);
static int test_malformed_raw_rejected(void);
static int test_oversized_raw_rejected(void);
static int test_transfer_connect_frame(void);

int
main(void)
{
	if (test_empty_link() != 0)
		return 1;
	if (test_malformed_raw_rejected() != 0)
		return 1;
	if (test_oversized_raw_rejected() != 0)
		return 1;
	if (test_transfer_connect_frame() != 0)
		return 1;
	return 0;
}

static int
test_empty_link(void)
{
	struct kn_ax25_loopback_link link;
	struct kn_ax25_loopback_endpoint a;
	struct kn_ax25_loopback_endpoint b;
	size_t moved;

	kn_ax25_loopback_link_init(&link);
	(void)kn_ax25_loopback_endpoint_init(&a, "A", "M6VPN-1",
	    "N0CALL", "kiss0", NULL);
	(void)kn_ax25_loopback_endpoint_init(&b, "B", "N0CALL",
	    "M6VPN-1", "kiss0", NULL);
	if (kn_ax25_loopback_link_transfer(&link, &a, &b, &moved) !=
	    KN_AX25_LOOPBACK_LINK_OK)
		return 1;
	return moved == 0 && link.raw_ax25_frames_transferred == 0 ? 0 : 1;
}

static int
test_malformed_raw_rejected(void)
{
	struct kn_ax25_loopback_link link;
	struct kn_ax25_loopback_endpoint b;
	uint8_t raw[1];

	raw[0] = 0x00;
	kn_ax25_loopback_link_init(&link);
	(void)kn_ax25_loopback_endpoint_init(&b, "B", "N0CALL",
	    "M6VPN-1", "kiss0", NULL);
	if (kn_ax25_loopback_link_transfer_raw(&link, &b, raw,
	    sizeof(raw)) != KN_AX25_LOOPBACK_LINK_ERR_ENDPOINT)
		return 1;
	return link.malformed_frames == 1 &&
	    link.raw_ax25_frames_transferred == 0 ? 0 : 1;
}

static int
test_oversized_raw_rejected(void)
{
	struct kn_ax25_loopback_link link;
	struct kn_ax25_loopback_endpoint b;
	uint8_t raw[KN_AX25_LOOPBACK_LINK_FRAME_MAX + 1];

	kn_ax25_loopback_link_init(&link);
	(void)kn_ax25_loopback_endpoint_init(&b, "B", "N0CALL",
	    "M6VPN-1", "kiss0", NULL);
	if (kn_ax25_loopback_link_transfer_raw(&link, &b, raw,
	    sizeof(raw)) != KN_AX25_LOOPBACK_LINK_ERR_FRAME_TOO_LARGE)
		return 1;
	return link.raw_ax25_frames_transferred == 0 ? 0 : 1;
}

static int
test_transfer_connect_frame(void)
{
	struct kn_ax25_loopback_link link;
	struct kn_ax25_loopback_endpoint a;
	struct kn_ax25_loopback_endpoint b;
	size_t moved;

	kn_ax25_loopback_link_init(&link);
	(void)kn_ax25_loopback_endpoint_init(&a, "A", "M6VPN-1",
	    "N0CALL", "kiss0", NULL);
	(void)kn_ax25_loopback_endpoint_init(&b, "B", "N0CALL",
	    "M6VPN-1", "kiss0", NULL);
	if (kn_ax25_loopback_endpoint_local_connect(&a) !=
	    KN_AX25_LOOPBACK_ENDPOINT_OK)
		return 1;
	if (kn_ax25_loopback_link_transfer(&link, &a, &b, &moved) !=
	    KN_AX25_LOOPBACK_LINK_OK)
		return 1;
	return moved == 1 && b.inbound_frames == 1 &&
	    link.fx25_frames_generated == 0 ? 0 : 1;
}
