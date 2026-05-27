/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/ax25_connection_diag.h */

#ifndef KILONODE_AX25_CONNECTION_DIAG_H
#define KILONODE_AX25_CONNECTION_DIAG_H

#include <sys/types.h>

#include "kilonode/ax25_connection_table.h"

enum kn_ax25_connection_diag_error {
	KN_AX25_CONNECTION_DIAG_OK = 0,
	KN_AX25_CONNECTION_DIAG_ERR_INVALID_ARGUMENT,
	KN_AX25_CONNECTION_DIAG_ERR_BUFFER
};

enum kn_ax25_connection_diag_error kn_ax25_connection_diag_format_actions(
	const struct kn_ax25_action_list *, char *, size_t);
enum kn_ax25_connection_diag_error kn_ax25_connection_diag_format_frame_plans(
	const struct kn_ax25_frame_plan_list *, char *, size_t);
enum kn_ax25_connection_diag_error kn_ax25_connection_diag_format_key(
	const struct kn_ax25_connection_key *, char *, size_t);
enum kn_ax25_connection_diag_error kn_ax25_connection_diag_format_record(
	const struct kn_ax25_connection_record *, char *, size_t);
enum kn_ax25_connection_diag_error kn_ax25_connection_diag_format_state(
	const struct kn_ax25_connection *, char *, size_t);
enum kn_ax25_connection_diag_error kn_ax25_connection_diag_format_table(
	const struct kn_ax25_connection_table *, char *, size_t);

#endif
