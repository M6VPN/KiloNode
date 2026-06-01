/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_ax25_control.c */

#include <sys/types.h>

#include <string.h>

#include "kilonode/ax25.h"
#include "kilonode/ax25_control.h"

static int test_i_classification(void);
static int test_i_encode_decode(void);
static int test_invalid_output_safe(void);
static int test_s_encode_decode(void);
static int test_s_classification(void);
static int test_string_formatting(void);
static int test_u_encode_decode(void);
static int test_u_classification(void);
static int test_ui_classification(void);
static int test_unknown_u_subtype(void);

int
main(void)
{
	if (test_ui_classification() != 0)
		return 1;
	if (test_i_classification() != 0)
		return 1;
	if (test_i_encode_decode() != 0)
		return 1;
	if (test_s_classification() != 0)
		return 1;
	if (test_s_encode_decode() != 0)
		return 1;
	if (test_u_classification() != 0)
		return 1;
	if (test_u_encode_decode() != 0)
		return 1;
	if (test_unknown_u_subtype() != 0)
		return 1;
	if (test_string_formatting() != 0)
		return 1;
	if (test_invalid_output_safe() != 0)
		return 1;

	return 0;
}

static int
test_i_classification(void)
{
	struct kn_ax25_control_info info;

	kn_ax25_control_decode(0x00, &info);
	if (info.class != KN_AX25_CONTROL_CLASS_I)
		return 1;
	if (info.ns != 0 || info.nr != 0 || info.poll_final != 0)
		return 1;

	kn_ax25_control_decode(0xb6, &info);
	if (info.class != KN_AX25_CONTROL_CLASS_I)
		return 1;
	if (info.ns != 3 || info.nr != 5 || info.poll_final != 1)
		return 1;

	return 0;
}

static int
test_i_encode_decode(void)
{
	struct kn_ax25_control_info info;
	uint8_t control;

	if (kn_ax25_control_encode_i(7, 6, 1, &control) !=
	    KN_AX25_CONTROL_OK)
		return 1;
	kn_ax25_control_decode(control, &info);
	if (info.class != KN_AX25_CONTROL_CLASS_I ||
	    info.ns != 7 || info.nr != 6 || info.poll_final != 1)
		return 1;
	return kn_ax25_control_encode_i(8, 0, 0, &control) ==
	    KN_AX25_CONTROL_ERR_INVALID_VALUE ? 0 : 1;
}

static int
test_invalid_output_safe(void)
{
	kn_ax25_control_decode(KN_AX25_CONTROL_UI, NULL);
	return 0;
}

static int
test_s_encode_decode(void)
{
	struct kn_ax25_control_info info;
	uint8_t control;

	if (kn_ax25_control_encode_s(KN_AX25_S_SUBTYPE_RR, 4, 1,
	    &control) != KN_AX25_CONTROL_OK)
		return 1;
	kn_ax25_control_decode(control, &info);
	if (info.class != KN_AX25_CONTROL_CLASS_S ||
	    info.s_subtype != KN_AX25_S_SUBTYPE_RR ||
	    info.nr != 4 || info.poll_final != 1)
		return 1;
	if (kn_ax25_control_encode_s(KN_AX25_S_SUBTYPE_REJ, 8, 0,
	    &control) != KN_AX25_CONTROL_ERR_INVALID_VALUE)
		return 1;

	return 0;
}

static int
test_s_classification(void)
{
	struct kn_ax25_control_info info;

	kn_ax25_control_decode(0x01, &info);
	if (info.class != KN_AX25_CONTROL_CLASS_S ||
	    info.s_subtype != KN_AX25_S_SUBTYPE_RR)
		return 1;

	kn_ax25_control_decode(0x05, &info);
	if (info.s_subtype != KN_AX25_S_SUBTYPE_RNR)
		return 1;

	kn_ax25_control_decode(0x09, &info);
	if (info.s_subtype != KN_AX25_S_SUBTYPE_REJ)
		return 1;

	kn_ax25_control_decode(0x0d, &info);
	if (info.s_subtype != KN_AX25_S_SUBTYPE_SREJ)
		return 1;

	return 0;
}

static int
test_string_formatting(void)
{
	if (strcmp(kn_ax25_control_class_name(KN_AX25_CONTROL_CLASS_UI),
	    "UI") != 0)
		return 1;
	if (strcmp(kn_ax25_s_subtype_name(KN_AX25_S_SUBTYPE_REJ),
	    "REJ") != 0)
		return 1;
	if (strcmp(kn_ax25_u_subtype_name(KN_AX25_U_SUBTYPE_TEST),
	    "TEST") != 0)
		return 1;
	if (strcmp(kn_ax25_control_class_name(99), "unknown") != 0)
		return 1;

	return 0;
}

static int
test_u_encode_decode(void)
{
	struct kn_ax25_control_info info;
	uint8_t control;

	if (kn_ax25_control_encode_u(KN_AX25_U_SUBTYPE_UA, 1,
	    &control) != KN_AX25_CONTROL_OK)
		return 1;
	kn_ax25_control_decode(control, &info);
	if (info.class != KN_AX25_CONTROL_CLASS_U ||
	    info.u_subtype != KN_AX25_U_SUBTYPE_UA ||
	    info.poll_final != 1)
		return 1;
	if (kn_ax25_control_encode_u(KN_AX25_U_SUBTYPE_UNKNOWN, 0,
	    &control) != KN_AX25_CONTROL_ERR_INVALID_VALUE)
		return 1;

	return 0;
}

static int
test_u_classification(void)
{
	struct kn_ax25_control_info info;

	kn_ax25_control_decode(0x2f, &info);
	if (info.class != KN_AX25_CONTROL_CLASS_U ||
	    info.u_subtype != KN_AX25_U_SUBTYPE_SABM)
		return 1;

	kn_ax25_control_decode(0x6f, &info);
	if (info.u_subtype != KN_AX25_U_SUBTYPE_SABME)
		return 1;

	kn_ax25_control_decode(0x43, &info);
	if (info.u_subtype != KN_AX25_U_SUBTYPE_DISC)
		return 1;

	kn_ax25_control_decode(0x0f, &info);
	if (info.u_subtype != KN_AX25_U_SUBTYPE_DM)
		return 1;

	kn_ax25_control_decode(0x63, &info);
	if (info.u_subtype != KN_AX25_U_SUBTYPE_UA)
		return 1;

	kn_ax25_control_decode(0x87, &info);
	if (info.u_subtype != KN_AX25_U_SUBTYPE_FRMR)
		return 1;

	kn_ax25_control_decode(0xaf, &info);
	if (info.u_subtype != KN_AX25_U_SUBTYPE_XID)
		return 1;

	kn_ax25_control_decode(0xe3, &info);
	if (info.u_subtype != KN_AX25_U_SUBTYPE_TEST)
		return 1;

	return 0;
}

static int
test_ui_classification(void)
{
	struct kn_ax25_control_info info;

	kn_ax25_control_decode(KN_AX25_CONTROL_UI, &info);
	if (info.class != KN_AX25_CONTROL_CLASS_UI)
		return 1;
	if (info.u_subtype != KN_AX25_U_SUBTYPE_UI)
		return 1;

	kn_ax25_control_decode(0x13, &info);
	if (info.class != KN_AX25_CONTROL_CLASS_UI ||
	    info.poll_final != 1)
		return 1;

	return 0;
}

static int
test_unknown_u_subtype(void)
{
	struct kn_ax25_control_info info;

	kn_ax25_control_decode(0x23, &info);
	return info.class == KN_AX25_CONTROL_CLASS_U &&
	    info.u_subtype == KN_AX25_U_SUBTYPE_UNKNOWN ? 0 : 1;
}
