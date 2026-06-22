/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/external_modem_profile.h */

#ifndef KILONODE_EXTERNAL_MODEM_PROFILE_H
#define KILONODE_EXTERNAL_MODEM_PROFILE_H

#include <sys/types.h>

#include "kilonode/external_modem.h"

enum kn_external_modem_profile_boundary {
	KN_EXTERNAL_MODEM_BOUNDARY_KISS = 0,
	KN_EXTERNAL_MODEM_BOUNDARY_TCP_CONTROL,
	KN_EXTERNAL_MODEM_BOUNDARY_TCP_HOST,
	KN_EXTERNAL_MODEM_BOUNDARY_UNKNOWN
};

enum kn_external_modem_profile_status {
	KN_EXTERNAL_MODEM_PROFILE_WORKING = 0,
	KN_EXTERNAL_MODEM_PROFILE_RECEIVE_ONLY,
	KN_EXTERNAL_MODEM_PROFILE_PLANNED,
	KN_EXTERNAL_MODEM_PROFILE_BLOCKED
};

enum kn_external_modem_profile_support {
	KN_EXTERNAL_MODEM_SUPPORT_BLOCKED = 0,
	KN_EXTERNAL_MODEM_SUPPORT_PLANNED
};

enum kn_external_modem_profile_process {
	KN_EXTERNAL_MODEM_PROCESS_UNSUPPORTED = 0,
	KN_EXTERNAL_MODEM_PROCESS_PLANNED
};

struct kn_external_modem_profile {
	enum kn_external_modem_type type;
	const char *display_name;
	enum kn_external_modem_profile_boundary boundary;
	enum kn_external_modem_profile_status current_status;
	enum kn_external_modem_profile_support tx_support;
	enum kn_external_modem_profile_support connect_support;
	enum kn_external_modem_profile_process process_launch;
	const char *notes;
};

const char *kn_external_modem_profile_boundary_name(
	enum kn_external_modem_profile_boundary);
enum kn_external_modem_error kn_external_modem_profile_format(
	const struct kn_external_modem_profile *, char *, size_t);
const struct kn_external_modem_profile *kn_external_modem_profile_for_type(
	enum kn_external_modem_type);
const struct kn_external_modem_profile *kn_external_modem_profiles(size_t *);
const char *kn_external_modem_profile_process_name(
	enum kn_external_modem_profile_process);
const char *kn_external_modem_profile_status_name(
	enum kn_external_modem_profile_status);
const char *kn_external_modem_profile_support_name(
	enum kn_external_modem_profile_support);

#endif
