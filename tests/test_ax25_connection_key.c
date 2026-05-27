/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_ax25_connection_key.c */

#include <sys/types.h>

#include <string.h>

#include "kilonode/ax25_connection_key.h"

static void decoded_frame(struct kn_ax25_frame *);
static int test_accept_multiple_digipeaters(void);
static int test_accept_no_digipeaters(void);
static int test_accept_one_digipeater(void);
static int test_equal_same_key(void);
static int test_format_deterministic(void);
static int test_from_inbound_frame(void);
static int test_inequality_different_local(void);
static int test_inequality_different_port(void);
static int test_inequality_different_remote(void);
static int test_invalid_local(void);
static int test_invalid_port(void);
static int test_invalid_remote(void);
static int test_normalize_callsigns(void);
static int test_reject_too_many_digipeaters(void);
static int test_valid_key(void);

int
main(void)
{
	if (test_valid_key() != 0)
		return 1;
	if (test_normalize_callsigns() != 0)
		return 1;
	if (test_invalid_local() != 0)
		return 1;
	if (test_invalid_remote() != 0)
		return 1;
	if (test_invalid_port() != 0)
		return 1;
	if (test_accept_no_digipeaters() != 0)
		return 1;
	if (test_accept_one_digipeater() != 0)
		return 1;
	if (test_accept_multiple_digipeaters() != 0)
		return 1;
	if (test_reject_too_many_digipeaters() != 0)
		return 1;
	if (test_equal_same_key() != 0)
		return 1;
	if (test_inequality_different_port() != 0)
		return 1;
	if (test_inequality_different_local() != 0)
		return 1;
	if (test_inequality_different_remote() != 0)
		return 1;
	if (test_format_deterministic() != 0)
		return 1;
	if (test_from_inbound_frame() != 0)
		return 1;

	return 0;
}

static void
decoded_frame(struct kn_ax25_frame *frame)
{
	kn_ax25_frame_reset(frame);
	(void)kn_callsign_parse("M6VPN-1", &frame->destination.callsign);
	(void)kn_callsign_parse("N0CALL-2", &frame->source.callsign);
	(void)kn_callsign_parse("WIDE1-1",
	    &frame->digipeaters[0].callsign);
	frame->digipeater_count = 1;
}

static int
test_accept_multiple_digipeaters(void)
{
	const char *digis[] = { "WIDE1-1", "WIDE2-2" };
	struct kn_ax25_connection_key key;

	if (kn_ax25_connection_key_from_callsigns(&key, "kiss0",
	    "M6VPN-1", "N0CALL-1", digis, 2) !=
	    KN_AX25_CONNECTION_KEY_OK)
		return 1;

	return key.digipeater_count == 2 ? 0 : 1;
}

static int
test_accept_no_digipeaters(void)
{
	struct kn_ax25_connection_key key;

	if (kn_ax25_connection_key_from_callsigns(&key, "kiss0",
	    "M6VPN-1", "N0CALL-1", NULL, 0) !=
	    KN_AX25_CONNECTION_KEY_OK)
		return 1;

	return key.digipeater_count == 0 ? 0 : 1;
}

static int
test_accept_one_digipeater(void)
{
	const char *digis[] = { "WIDE1-1" };
	struct kn_ax25_connection_key key;

	if (kn_ax25_connection_key_from_callsigns(&key, "kiss0",
	    "M6VPN-1", "N0CALL-1", digis, 1) !=
	    KN_AX25_CONNECTION_KEY_OK)
		return 1;

	return key.digipeaters[0].ssid == 1 ? 0 : 1;
}

static int
test_equal_same_key(void)
{
	struct kn_ax25_connection_key a;
	struct kn_ax25_connection_key b;

	(void)kn_ax25_connection_key_from_callsigns(&a, "kiss0",
	    "M6VPN-1", "N0CALL-1", NULL, 0);
	(void)kn_ax25_connection_key_from_callsigns(&b, "kiss0",
	    "M6VPN-1", "N0CALL-1", NULL, 0);

	return kn_ax25_connection_key_equal(&a, &b) != 0 ? 0 : 1;
}

static int
test_format_deterministic(void)
{
	const char *digis[] = { "WIDE1-1" };
	struct kn_ax25_connection_key key;
	char out[192];

	(void)kn_ax25_connection_key_from_callsigns(&key, "kiss0",
	    "M6VPN-1", "N0CALL-1", digis, 1);
	if (kn_ax25_connection_key_format(&key, out, sizeof(out)) !=
	    KN_AX25_CONNECTION_KEY_OK)
		return 1;

	return strcmp(out, "port=kiss0 local=M6VPN-1 remote=N0CALL-1 "
	    "via=WIDE1-1") == 0 ? 0 : 1;
}

static int
test_from_inbound_frame(void)
{
	struct kn_ax25_frame frame;
	struct kn_ax25_connection_key key;
	struct kn_callsign local;

	decoded_frame(&frame);
	(void)kn_callsign_parse("M6VPN-1", &local);
	if (kn_ax25_connection_key_from_frame(&key, "kiss0", &local,
	    &frame) != KN_AX25_CONNECTION_KEY_OK)
		return 1;

	return strcmp(key.local.call, "M6VPN") == 0 &&
	    strcmp(key.remote.call, "N0CALL") == 0 &&
	    key.remote.ssid == 2 &&
	    key.digipeater_count == 1 ? 0 : 1;
}

static int
test_inequality_different_local(void)
{
	struct kn_ax25_connection_key a;
	struct kn_ax25_connection_key b;

	(void)kn_ax25_connection_key_from_callsigns(&a, "kiss0",
	    "M6VPN-1", "N0CALL-1", NULL, 0);
	(void)kn_ax25_connection_key_from_callsigns(&b, "kiss0",
	    "OTHER-1", "N0CALL-1", NULL, 0);

	return kn_ax25_connection_key_equal(&a, &b) == 0 ? 0 : 1;
}

static int
test_inequality_different_port(void)
{
	struct kn_ax25_connection_key a;
	struct kn_ax25_connection_key b;

	(void)kn_ax25_connection_key_from_callsigns(&a, "kiss0",
	    "M6VPN-1", "N0CALL-1", NULL, 0);
	(void)kn_ax25_connection_key_from_callsigns(&b, "kiss1",
	    "M6VPN-1", "N0CALL-1", NULL, 0);

	return kn_ax25_connection_key_equal(&a, &b) == 0 ? 0 : 1;
}

static int
test_inequality_different_remote(void)
{
	struct kn_ax25_connection_key a;
	struct kn_ax25_connection_key b;

	(void)kn_ax25_connection_key_from_callsigns(&a, "kiss0",
	    "M6VPN-1", "N0CALL-1", NULL, 0);
	(void)kn_ax25_connection_key_from_callsigns(&b, "kiss0",
	    "M6VPN-1", "OTHER-1", NULL, 0);

	return kn_ax25_connection_key_equal(&a, &b) == 0 ? 0 : 1;
}

static int
test_invalid_local(void)
{
	struct kn_ax25_connection_key key;

	return kn_ax25_connection_key_from_callsigns(&key, "kiss0",
	    "bad*", "N0CALL-1", NULL, 0) ==
	    KN_AX25_CONNECTION_KEY_ERR_INVALID_VALUE ? 0 : 1;
}

static int
test_invalid_port(void)
{
	struct kn_ax25_connection_key key;

	return kn_ax25_connection_key_from_callsigns(&key, "bad port",
	    "M6VPN-1", "N0CALL-1", NULL, 0) ==
	    KN_AX25_CONNECTION_KEY_ERR_INVALID_VALUE ? 0 : 1;
}

static int
test_invalid_remote(void)
{
	struct kn_ax25_connection_key key;

	return kn_ax25_connection_key_from_callsigns(&key, "kiss0",
	    "M6VPN-1", "bad*", NULL, 0) ==
	    KN_AX25_CONNECTION_KEY_ERR_INVALID_VALUE ? 0 : 1;
}

static int
test_normalize_callsigns(void)
{
	struct kn_ax25_connection_key key;

	if (kn_ax25_connection_key_from_callsigns(&key, "kiss0",
	    "M6VPN-1", "N0CALL-7", NULL, 0) !=
	    KN_AX25_CONNECTION_KEY_OK)
		return 1;

	return strcmp(key.local.call, "M6VPN") == 0 &&
	    key.local.ssid == 1 && key.remote.ssid == 7 ? 0 : 1;
}

static int
test_reject_too_many_digipeaters(void)
{
	const char *digis[KN_AX25_MAX_DIGIS + 1];
	struct kn_ax25_connection_key key;
	size_t i;

	for (i = 0; i < KN_AX25_MAX_DIGIS + 1; i++)
		digis[i] = "WIDE1-1";

	return kn_ax25_connection_key_from_callsigns(&key, "kiss0",
	    "M6VPN-1", "N0CALL-1", digis, KN_AX25_MAX_DIGIS + 1) ==
	    KN_AX25_CONNECTION_KEY_ERR_TOO_MANY_DIGIS ? 0 : 1;
}

static int
test_valid_key(void)
{
	struct kn_ax25_connection_key key;

	return kn_ax25_connection_key_from_callsigns(&key, "kiss0",
	    "M6VPN-1", "N0CALL-1", NULL, 0) ==
	    KN_AX25_CONNECTION_KEY_OK ? 0 : 1;
}
