/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_ax25_frame_builder.c */

#include <sys/types.h>

#include <stdint.h>
#include <string.h>

#include "kilonode/ax25.h"
#include "kilonode/ax25_control.h"
#include "kilonode/ax25_frame_builder.h"

static void base_request(struct kn_ax25_frame_builder_request *,
	enum kn_ax25_frame_plan_type);
static int build_and_decode(enum kn_ax25_frame_plan_type,
	struct kn_ax25_frame *, struct kn_ax25_control_info *);
static int test_build_disc(void);
static int test_build_dm(void);
static int test_build_rej(void);
static int test_build_rnr(void);
static int test_build_rr(void);
static int test_build_sabm(void);
static int test_build_sabme(void);
static int test_build_ua(void);
static int test_invalid_destination(void);
static int test_invalid_digipeater(void);
static int test_invalid_nr(void);
static int test_invalid_source(void);
static int test_no_hdlc_or_fcs(void);
static int test_pf_bit_changes_control(void);
static int test_too_many_digipeaters(void);
static int test_too_small_output(void);

int
main(void)
{
	if (test_build_sabm() != 0)
		return 1;
	if (test_build_sabme() != 0)
		return 1;
	if (test_build_ua() != 0)
		return 1;
	if (test_build_dm() != 0)
		return 1;
	if (test_build_disc() != 0)
		return 1;
	if (test_build_rr() != 0)
		return 1;
	if (test_build_rnr() != 0)
		return 1;
	if (test_build_rej() != 0)
		return 1;
	if (test_pf_bit_changes_control() != 0)
		return 1;
	if (test_invalid_source() != 0)
		return 1;
	if (test_invalid_destination() != 0)
		return 1;
	if (test_invalid_digipeater() != 0)
		return 1;
	if (test_too_many_digipeaters() != 0)
		return 1;
	if (test_invalid_nr() != 0)
		return 1;
	if (test_too_small_output() != 0)
		return 1;
	if (test_no_hdlc_or_fcs() != 0)
		return 1;

	return 0;
}

static void
base_request(struct kn_ax25_frame_builder_request *request,
	enum kn_ax25_frame_plan_type type)
{
	kn_ax25_frame_builder_request_clear(request);
	(void)kn_callsign_parse("M6VPN-1", &request->source);
	(void)kn_callsign_parse("N0CALL-1", &request->destination);
	request->type = type;
	request->nr = 3;
}

static int
build_and_decode(enum kn_ax25_frame_plan_type type,
	struct kn_ax25_frame *frame, struct kn_ax25_control_info *info)
{
	struct kn_ax25_frame_builder_request request;
	uint8_t out[64];
	size_t out_len;

	base_request(&request, type);
	if (kn_ax25_frame_builder_build(&request, out, sizeof(out),
	    &out_len) != KN_AX25_FRAME_BUILDER_OK)
		return 1;
	if (kn_ax25_frame_decode(out, out_len, frame) != KN_AX25_OK)
		return 1;
	kn_ax25_control_decode(frame->control, info);
	return 0;
}

static int
test_build_disc(void)
{
	struct kn_ax25_frame frame;
	struct kn_ax25_control_info info;

	if (build_and_decode(KN_AX25_FRAME_PLAN_DISC, &frame, &info) != 0)
		return 1;

	return info.u_subtype == KN_AX25_U_SUBTYPE_DISC &&
	    frame.has_pid == 0 ? 0 : 1;
}

static int
test_build_dm(void)
{
	struct kn_ax25_frame frame;
	struct kn_ax25_control_info info;

	if (build_and_decode(KN_AX25_FRAME_PLAN_DM, &frame, &info) != 0)
		return 1;

	return info.u_subtype == KN_AX25_U_SUBTYPE_DM &&
	    frame.has_pid == 0 ? 0 : 1;
}

static int
test_build_rej(void)
{
	struct kn_ax25_frame frame;
	struct kn_ax25_control_info info;

	if (build_and_decode(KN_AX25_FRAME_PLAN_REJ, &frame, &info) != 0)
		return 1;

	return info.s_subtype == KN_AX25_S_SUBTYPE_REJ &&
	    info.nr == 3 ? 0 : 1;
}

static int
test_build_rnr(void)
{
	struct kn_ax25_frame frame;
	struct kn_ax25_control_info info;

	if (build_and_decode(KN_AX25_FRAME_PLAN_RNR, &frame, &info) != 0)
		return 1;

	return info.s_subtype == KN_AX25_S_SUBTYPE_RNR &&
	    info.nr == 3 ? 0 : 1;
}

static int
test_build_rr(void)
{
	struct kn_ax25_frame frame;
	struct kn_ax25_control_info info;

	if (build_and_decode(KN_AX25_FRAME_PLAN_RR, &frame, &info) != 0)
		return 1;

	return info.s_subtype == KN_AX25_S_SUBTYPE_RR &&
	    info.nr == 3 ? 0 : 1;
}

static int
test_build_sabm(void)
{
	struct kn_ax25_frame frame;
	struct kn_ax25_control_info info;

	if (build_and_decode(KN_AX25_FRAME_PLAN_SABM, &frame, &info) != 0)
		return 1;

	return info.u_subtype == KN_AX25_U_SUBTYPE_SABM &&
	    strcmp(frame.source.callsign.call, "M6VPN") == 0 &&
	    strcmp(frame.destination.callsign.call, "N0CALL") == 0 ? 0 : 1;
}

static int
test_build_sabme(void)
{
	struct kn_ax25_frame frame;
	struct kn_ax25_control_info info;

	if (build_and_decode(KN_AX25_FRAME_PLAN_SABME, &frame, &info) != 0)
		return 1;

	return info.u_subtype == KN_AX25_U_SUBTYPE_SABME ? 0 : 1;
}

static int
test_build_ua(void)
{
	struct kn_ax25_frame frame;
	struct kn_ax25_control_info info;

	if (build_and_decode(KN_AX25_FRAME_PLAN_UA, &frame, &info) != 0)
		return 1;

	return info.u_subtype == KN_AX25_U_SUBTYPE_UA ? 0 : 1;
}

static int
test_invalid_destination(void)
{
	struct kn_ax25_frame_builder_request request;
	uint8_t out[64];
	size_t out_len;

	base_request(&request, KN_AX25_FRAME_PLAN_UA);
	memset(&request.destination, 0, sizeof(request.destination));
	return kn_ax25_frame_builder_build(&request, out, sizeof(out),
	    &out_len) == KN_AX25_FRAME_BUILDER_ERR_INVALID_VALUE ? 0 : 1;
}

static int
test_invalid_digipeater(void)
{
	struct kn_ax25_frame_builder_request request;
	uint8_t out[64];
	size_t out_len;

	base_request(&request, KN_AX25_FRAME_PLAN_UA);
	request.digipeater_count = 1;
	memset(&request.digipeaters[0], 0, sizeof(request.digipeaters[0]));
	return kn_ax25_frame_builder_build(&request, out, sizeof(out),
	    &out_len) == KN_AX25_FRAME_BUILDER_ERR_INVALID_VALUE ? 0 : 1;
}

static int
test_invalid_nr(void)
{
	struct kn_ax25_frame_builder_request request;
	uint8_t out[64];
	size_t out_len;

	base_request(&request, KN_AX25_FRAME_PLAN_RR);
	request.nr = 8;
	return kn_ax25_frame_builder_build(&request, out, sizeof(out),
	    &out_len) == KN_AX25_FRAME_BUILDER_ERR_INVALID_VALUE ? 0 : 1;
}

static int
test_invalid_source(void)
{
	struct kn_ax25_frame_builder_request request;
	uint8_t out[64];
	size_t out_len;

	base_request(&request, KN_AX25_FRAME_PLAN_UA);
	memset(&request.source, 0, sizeof(request.source));
	return kn_ax25_frame_builder_build(&request, out, sizeof(out),
	    &out_len) == KN_AX25_FRAME_BUILDER_ERR_INVALID_VALUE ? 0 : 1;
}

static int
test_no_hdlc_or_fcs(void)
{
	struct kn_ax25_frame_builder_request request;
	uint8_t out[64];
	size_t out_len;

	base_request(&request, KN_AX25_FRAME_PLAN_UA);
	if (kn_ax25_frame_builder_build(&request, out, sizeof(out),
	    &out_len) != KN_AX25_FRAME_BUILDER_OK)
		return 1;
	if (out_len != (KN_AX25_ADDR_LEN * 2) + 1)
		return 1;

	return out[0] != 0x7e && out[out_len - 1] != 0x7e ? 0 : 1;
}

static int
test_pf_bit_changes_control(void)
{
	struct kn_ax25_frame_builder_request request;
	uint8_t out[64];
	size_t out_len;
	uint8_t no_pf;
	uint8_t with_pf;

	base_request(&request, KN_AX25_FRAME_PLAN_UA);
	if (kn_ax25_frame_builder_build(&request, out, sizeof(out),
	    &out_len) != KN_AX25_FRAME_BUILDER_OK)
		return 1;
	no_pf = out[out_len - 1];
	request.poll_final = 1;
	if (kn_ax25_frame_builder_build(&request, out, sizeof(out),
	    &out_len) != KN_AX25_FRAME_BUILDER_OK)
		return 1;
	with_pf = out[out_len - 1];

	return no_pf != with_pf && (with_pf & 0x10U) != 0 ? 0 : 1;
}

static int
test_too_many_digipeaters(void)
{
	struct kn_ax25_frame_builder_request request;
	uint8_t out[64];
	size_t out_len;

	base_request(&request, KN_AX25_FRAME_PLAN_UA);
	request.digipeater_count = KN_AX25_MAX_DIGIS + 1;
	return kn_ax25_frame_builder_build(&request, out, sizeof(out),
	    &out_len) == KN_AX25_FRAME_BUILDER_ERR_TOO_MANY_DIGIS ? 0 : 1;
}

static int
test_too_small_output(void)
{
	struct kn_ax25_frame_builder_request request;
	uint8_t out[4];
	size_t out_len;

	base_request(&request, KN_AX25_FRAME_PLAN_UA);
	return kn_ax25_frame_builder_build(&request, out, sizeof(out),
	    &out_len) == KN_AX25_FRAME_BUILDER_ERR_TOO_LARGE ? 0 : 1;
}
