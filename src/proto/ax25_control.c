/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/proto/ax25_control.c */

#include <sys/types.h>

#include <string.h>

#include "kilonode/ax25.h"
#include "kilonode/ax25_control.h"

static enum kn_ax25_s_subtype decode_s_subtype(uint8_t);
static enum kn_ax25_u_subtype decode_u_subtype(uint8_t);
static enum kn_ax25_control_error encode_s_base(
	enum kn_ax25_s_subtype, uint8_t *);
static enum kn_ax25_control_error encode_u_base(
	enum kn_ax25_u_subtype, uint8_t *);

static enum kn_ax25_s_subtype
decode_s_subtype(uint8_t control)
{
	switch (control & 0x0fU) {
	case 0x01U:
		return KN_AX25_S_SUBTYPE_RR;
	case 0x05U:
		return KN_AX25_S_SUBTYPE_RNR;
	case 0x09U:
		return KN_AX25_S_SUBTYPE_REJ;
	case 0x0dU:
		return KN_AX25_S_SUBTYPE_SREJ;
	}

	return KN_AX25_S_SUBTYPE_UNKNOWN;
}

static enum kn_ax25_u_subtype
decode_u_subtype(uint8_t control)
{
	switch (control & 0xefU) {
	case 0x2fU:
		return KN_AX25_U_SUBTYPE_SABM;
	case 0x6fU:
		return KN_AX25_U_SUBTYPE_SABME;
	case 0x43U:
		return KN_AX25_U_SUBTYPE_DISC;
	case 0x0fU:
		return KN_AX25_U_SUBTYPE_DM;
	case 0x63U:
		return KN_AX25_U_SUBTYPE_UA;
	case 0x87U:
		return KN_AX25_U_SUBTYPE_FRMR;
	case KN_AX25_CONTROL_UI:
		return KN_AX25_U_SUBTYPE_UI;
	case 0xafU:
		return KN_AX25_U_SUBTYPE_XID;
	case 0xe3U:
		return KN_AX25_U_SUBTYPE_TEST;
	}

	return KN_AX25_U_SUBTYPE_UNKNOWN;
}

static enum kn_ax25_control_error
encode_s_base(enum kn_ax25_s_subtype subtype, uint8_t *control)
{
	if (control == NULL)
		return KN_AX25_CONTROL_ERR_INVALID_ARGUMENT;

	switch (subtype) {
	case KN_AX25_S_SUBTYPE_RR:
		*control = 0x01U;
		return KN_AX25_CONTROL_OK;
	case KN_AX25_S_SUBTYPE_RNR:
		*control = 0x05U;
		return KN_AX25_CONTROL_OK;
	case KN_AX25_S_SUBTYPE_REJ:
		*control = 0x09U;
		return KN_AX25_CONTROL_OK;
	case KN_AX25_S_SUBTYPE_SREJ:
		*control = 0x0dU;
		return KN_AX25_CONTROL_OK;
	case KN_AX25_S_SUBTYPE_UNKNOWN:
		break;
	}

	return KN_AX25_CONTROL_ERR_INVALID_VALUE;
}

static enum kn_ax25_control_error
encode_u_base(enum kn_ax25_u_subtype subtype, uint8_t *control)
{
	if (control == NULL)
		return KN_AX25_CONTROL_ERR_INVALID_ARGUMENT;

	switch (subtype) {
	case KN_AX25_U_SUBTYPE_SABM:
		*control = 0x2fU;
		return KN_AX25_CONTROL_OK;
	case KN_AX25_U_SUBTYPE_SABME:
		*control = 0x6fU;
		return KN_AX25_CONTROL_OK;
	case KN_AX25_U_SUBTYPE_DISC:
		*control = 0x43U;
		return KN_AX25_CONTROL_OK;
	case KN_AX25_U_SUBTYPE_DM:
		*control = 0x0fU;
		return KN_AX25_CONTROL_OK;
	case KN_AX25_U_SUBTYPE_UA:
		*control = 0x63U;
		return KN_AX25_CONTROL_OK;
	case KN_AX25_U_SUBTYPE_FRMR:
		*control = 0x87U;
		return KN_AX25_CONTROL_OK;
	case KN_AX25_U_SUBTYPE_UI:
		*control = KN_AX25_CONTROL_UI;
		return KN_AX25_CONTROL_OK;
	case KN_AX25_U_SUBTYPE_XID:
		*control = 0xafU;
		return KN_AX25_CONTROL_OK;
	case KN_AX25_U_SUBTYPE_TEST:
		*control = 0xe3U;
		return KN_AX25_CONTROL_OK;
	case KN_AX25_U_SUBTYPE_UNKNOWN:
		break;
	}

	return KN_AX25_CONTROL_ERR_INVALID_VALUE;
}

void
kn_ax25_control_decode(uint8_t control, struct kn_ax25_control_info *info)
{
	if (info == NULL)
		return;

	memset(info, 0, sizeof(*info));
	info->class = KN_AX25_CONTROL_CLASS_UNKNOWN;
	info->s_subtype = KN_AX25_S_SUBTYPE_UNKNOWN;
	info->u_subtype = KN_AX25_U_SUBTYPE_UNKNOWN;

	if ((control & 0xefU) == KN_AX25_CONTROL_UI) {
		info->class = KN_AX25_CONTROL_CLASS_UI;
		info->u_subtype = KN_AX25_U_SUBTYPE_UI;
		info->poll_final = (uint8_t)((control & 0x10U) != 0);
		return;
	}

	if ((control & 0x01U) == 0) {
		info->class = KN_AX25_CONTROL_CLASS_I;
		info->ns = (uint8_t)((control >> 1) & 0x07U);
		info->poll_final = (uint8_t)((control & 0x10U) != 0);
		info->nr = (uint8_t)((control >> 5) & 0x07U);
		return;
	}

	if ((control & 0x03U) == 0x01U) {
		info->class = KN_AX25_CONTROL_CLASS_S;
		info->s_subtype = decode_s_subtype(control);
		info->poll_final = (uint8_t)((control & 0x10U) != 0);
		info->nr = (uint8_t)((control >> 5) & 0x07U);
		return;
	}

	if ((control & 0x03U) == 0x03U) {
		info->class = KN_AX25_CONTROL_CLASS_U;
		info->u_subtype = decode_u_subtype(control);
		info->poll_final = (uint8_t)((control & 0x10U) != 0);
	}
}

enum kn_ax25_control_error
kn_ax25_control_encode_i(uint8_t ns, uint8_t nr, uint8_t poll_final,
	uint8_t *control)
{
	uint8_t value;

	if (control == NULL)
		return KN_AX25_CONTROL_ERR_INVALID_ARGUMENT;
	if (ns > 7 || nr > 7 || poll_final > 1)
		return KN_AX25_CONTROL_ERR_INVALID_VALUE;

	value = (uint8_t)((ns << 1) | (nr << 5));
	if (poll_final != 0)
		value = (uint8_t)(value | 0x10U);

	*control = value;
	return KN_AX25_CONTROL_OK;
}

enum kn_ax25_control_error
kn_ax25_control_encode_s(enum kn_ax25_s_subtype subtype, uint8_t nr,
	uint8_t poll_final, uint8_t *control)
{
	enum kn_ax25_control_error rc;
	uint8_t value;

	if (control == NULL)
		return KN_AX25_CONTROL_ERR_INVALID_ARGUMENT;
	if (nr > 7 || poll_final > 1)
		return KN_AX25_CONTROL_ERR_INVALID_VALUE;

	rc = encode_s_base(subtype, &value);
	if (rc != KN_AX25_CONTROL_OK)
		return rc;

	value = (uint8_t)(value | (uint8_t)(nr << 5));
	if (poll_final != 0)
		value = (uint8_t)(value | 0x10U);

	*control = value;
	return KN_AX25_CONTROL_OK;
}

enum kn_ax25_control_error
kn_ax25_control_encode_u(enum kn_ax25_u_subtype subtype, uint8_t poll_final,
	uint8_t *control)
{
	enum kn_ax25_control_error rc;
	uint8_t value;

	if (control == NULL)
		return KN_AX25_CONTROL_ERR_INVALID_ARGUMENT;
	if (poll_final > 1)
		return KN_AX25_CONTROL_ERR_INVALID_VALUE;

	rc = encode_u_base(subtype, &value);
	if (rc != KN_AX25_CONTROL_OK)
		return rc;

	if (poll_final != 0)
		value = (uint8_t)(value | 0x10U);

	*control = value;
	return KN_AX25_CONTROL_OK;
}

const char *
kn_ax25_control_class_name(enum kn_ax25_control_class class)
{
	switch (class) {
	case KN_AX25_CONTROL_CLASS_UI:
		return "UI";
	case KN_AX25_CONTROL_CLASS_I:
		return "I";
	case KN_AX25_CONTROL_CLASS_S:
		return "S";
	case KN_AX25_CONTROL_CLASS_U:
		return "U";
	case KN_AX25_CONTROL_CLASS_UNKNOWN:
		return "unknown";
	}

	return "unknown";
}

const char *
kn_ax25_s_subtype_name(enum kn_ax25_s_subtype subtype)
{
	switch (subtype) {
	case KN_AX25_S_SUBTYPE_RR:
		return "RR";
	case KN_AX25_S_SUBTYPE_RNR:
		return "RNR";
	case KN_AX25_S_SUBTYPE_REJ:
		return "REJ";
	case KN_AX25_S_SUBTYPE_SREJ:
		return "SREJ";
	case KN_AX25_S_SUBTYPE_UNKNOWN:
		return "unknown";
	}

	return "unknown";
}

const char *
kn_ax25_u_subtype_name(enum kn_ax25_u_subtype subtype)
{
	switch (subtype) {
	case KN_AX25_U_SUBTYPE_SABM:
		return "SABM";
	case KN_AX25_U_SUBTYPE_SABME:
		return "SABME";
	case KN_AX25_U_SUBTYPE_DISC:
		return "DISC";
	case KN_AX25_U_SUBTYPE_DM:
		return "DM";
	case KN_AX25_U_SUBTYPE_UA:
		return "UA";
	case KN_AX25_U_SUBTYPE_FRMR:
		return "FRMR";
	case KN_AX25_U_SUBTYPE_UI:
		return "UI";
	case KN_AX25_U_SUBTYPE_XID:
		return "XID";
	case KN_AX25_U_SUBTYPE_TEST:
		return "TEST";
	case KN_AX25_U_SUBTYPE_UNKNOWN:
		return "unknown";
	}

	return "unknown";
}
