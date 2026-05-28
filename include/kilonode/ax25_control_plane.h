/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/ax25_control_plane.h */

#ifndef KILONODE_AX25_CONTROL_PLANE_H
#define KILONODE_AX25_CONTROL_PLANE_H

#include <sys/types.h>

#include <stdint.h>

#include "kilonode/ax25_runtime.h"

enum kn_ax25_control_plane_error {
	KN_AX25_CONTROL_PLANE_OK = 0,
	KN_AX25_CONTROL_PLANE_ERR_INVALID_ARGUMENT,
	KN_AX25_CONTROL_PLANE_ERR_INVALID_COMMAND,
	KN_AX25_CONTROL_PLANE_ERR_NOT_FOUND,
	KN_AX25_CONTROL_PLANE_ERR_BUFFER
};

enum kn_ax25_control_plane_error kn_ax25_control_plane_format(
	const char *, const struct kn_ax25_runtime *, char *, size_t);
enum kn_ax25_control_plane_error kn_ax25_control_plane_format_connection(
	const struct kn_ax25_runtime *, size_t, char *, size_t);
enum kn_ax25_control_plane_error kn_ax25_control_plane_format_connections(
	const struct kn_ax25_runtime *, const char *, char *, size_t);
enum kn_ax25_control_plane_error kn_ax25_control_plane_format_counters(
	const struct kn_ax25_runtime *, char *, size_t);
enum kn_ax25_control_plane_error kn_ax25_control_plane_format_live(
	const struct kn_ax25_runtime *, char *, size_t);
enum kn_ax25_control_plane_error kn_ax25_control_plane_format_params(
	const struct kn_ax25_runtime *, char *, size_t);
enum kn_ax25_control_plane_error kn_ax25_control_plane_format_prepared(
	const struct kn_ax25_runtime *, const char *, uint32_t, char *,
	size_t);
enum kn_ax25_control_plane_error kn_ax25_control_plane_format_prepared_counters(
	const struct kn_ax25_runtime *, char *, size_t);
enum kn_ax25_control_plane_error kn_ax25_control_plane_format_prepared_frame(
	const struct kn_ax25_runtime *, uint64_t, char *, size_t);
enum kn_ax25_control_plane_error
kn_ax25_control_plane_format_prepared_bridge(
	const struct kn_ax25_runtime *, char *, size_t);
enum kn_ax25_control_plane_error
kn_ax25_control_plane_format_prepared_bridge_counters(
	const struct kn_ax25_runtime *, char *, size_t);
enum kn_ax25_control_plane_error
kn_ax25_control_plane_format_prepared_bridge_frame(
	const struct kn_ax25_runtime *, uint64_t, char *, size_t);
enum kn_ax25_control_plane_error kn_ax25_control_plane_format_scheduler(
	const struct kn_ax25_runtime *, char *, size_t);
enum kn_ax25_control_plane_error kn_ax25_control_plane_format_scheduler_counters(
	const struct kn_ax25_runtime *, char *, size_t);
enum kn_ax25_control_plane_error kn_ax25_control_plane_format_scheduler_smoke(
	const struct kn_ax25_runtime *, char *, size_t);
enum kn_ax25_control_plane_error kn_ax25_control_plane_format_scheduler_timers(
	const struct kn_ax25_runtime *, char *, size_t);
enum kn_ax25_control_plane_error kn_ax25_control_plane_format_status(
	const struct kn_ax25_runtime *, char *, size_t);

#endif
