/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/ax25_control.h */

#ifndef KILONODE_AX25_CONTROL_H
#define KILONODE_AX25_CONTROL_H

#include <stdint.h>

enum kn_ax25_control_class {
	KN_AX25_CONTROL_CLASS_UI = 0,
	KN_AX25_CONTROL_CLASS_I,
	KN_AX25_CONTROL_CLASS_S,
	KN_AX25_CONTROL_CLASS_U,
	KN_AX25_CONTROL_CLASS_UNKNOWN
};

enum kn_ax25_s_subtype {
	KN_AX25_S_SUBTYPE_RR = 0,
	KN_AX25_S_SUBTYPE_RNR,
	KN_AX25_S_SUBTYPE_REJ,
	KN_AX25_S_SUBTYPE_SREJ,
	KN_AX25_S_SUBTYPE_UNKNOWN
};

enum kn_ax25_u_subtype {
	KN_AX25_U_SUBTYPE_SABM = 0,
	KN_AX25_U_SUBTYPE_SABME,
	KN_AX25_U_SUBTYPE_DISC,
	KN_AX25_U_SUBTYPE_DM,
	KN_AX25_U_SUBTYPE_UA,
	KN_AX25_U_SUBTYPE_FRMR,
	KN_AX25_U_SUBTYPE_UI,
	KN_AX25_U_SUBTYPE_XID,
	KN_AX25_U_SUBTYPE_TEST,
	KN_AX25_U_SUBTYPE_UNKNOWN
};

enum kn_ax25_control_error {
	KN_AX25_CONTROL_OK = 0,
	KN_AX25_CONTROL_ERR_INVALID_ARGUMENT,
	KN_AX25_CONTROL_ERR_INVALID_VALUE
};

struct kn_ax25_control_info {
	enum kn_ax25_control_class class;
	enum kn_ax25_s_subtype s_subtype;
	enum kn_ax25_u_subtype u_subtype;
	uint8_t poll_final;
	uint8_t ns;
	uint8_t nr;
};

void kn_ax25_control_decode(uint8_t, struct kn_ax25_control_info *);
enum kn_ax25_control_error kn_ax25_control_encode_i(uint8_t, uint8_t,
	uint8_t, uint8_t *);
enum kn_ax25_control_error kn_ax25_control_encode_s(
	enum kn_ax25_s_subtype, uint8_t, uint8_t, uint8_t *);
enum kn_ax25_control_error kn_ax25_control_encode_u(
	enum kn_ax25_u_subtype, uint8_t, uint8_t *);
const char *kn_ax25_control_class_name(enum kn_ax25_control_class);
const char *kn_ax25_s_subtype_name(enum kn_ax25_s_subtype);
const char *kn_ax25_u_subtype_name(enum kn_ax25_u_subtype);

#endif
