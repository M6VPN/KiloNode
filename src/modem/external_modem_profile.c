/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/modem/external_modem_profile.c */

#include <sys/types.h>

#include <stdio.h>

#include "kilonode/external_modem_profile.h"

static const struct kn_external_modem_profile profiles[] = {
	{
		KN_EXTERNAL_MODEM_TYPE_KISS_TCP,
		"Dire Wolf TCP KISS",
		KN_EXTERNAL_MODEM_BOUNDARY_KISS,
		KN_EXTERNAL_MODEM_PROFILE_RECEIVE_ONLY,
		KN_EXTERNAL_MODEM_SUPPORT_BLOCKED,
		KN_EXTERNAL_MODEM_SUPPORT_BLOCKED,
		KN_EXTERNAL_MODEM_PROCESS_UNSUPPORTED,
		"receive-only path through existing TCP KISS"
	},
	{
		KN_EXTERNAL_MODEM_TYPE_MERCURY_OFDM,
		"Mercury OFDM",
		KN_EXTERNAL_MODEM_BOUNDARY_UNKNOWN,
		KN_EXTERNAL_MODEM_PROFILE_PLANNED,
		KN_EXTERNAL_MODEM_SUPPORT_BLOCKED,
		KN_EXTERNAL_MODEM_SUPPORT_BLOCKED,
		KN_EXTERNAL_MODEM_PROCESS_PLANNED,
		"Rhizomatica external HF OFDM modem; interface discovery required"
	},
	{
		KN_EXTERNAL_MODEM_TYPE_VARA_HF,
		"VARA HF",
		KN_EXTERNAL_MODEM_BOUNDARY_TCP_CONTROL,
		KN_EXTERNAL_MODEM_PROFILE_PLANNED,
		KN_EXTERNAL_MODEM_SUPPORT_BLOCKED,
		KN_EXTERNAL_MODEM_SUPPORT_BLOCKED,
		KN_EXTERNAL_MODEM_PROCESS_PLANNED,
		"planned external adapter"
	},
	{
		KN_EXTERNAL_MODEM_TYPE_VARA_FM,
		"VARA FM",
		KN_EXTERNAL_MODEM_BOUNDARY_TCP_CONTROL,
		KN_EXTERNAL_MODEM_PROFILE_PLANNED,
		KN_EXTERNAL_MODEM_SUPPORT_BLOCKED,
		KN_EXTERNAL_MODEM_SUPPORT_BLOCKED,
		KN_EXTERNAL_MODEM_PROCESS_PLANNED,
		"planned external adapter"
	},
	{
		KN_EXTERNAL_MODEM_TYPE_ARDOP,
		"ARDOP",
		KN_EXTERNAL_MODEM_BOUNDARY_TCP_HOST,
		KN_EXTERNAL_MODEM_PROFILE_PLANNED,
		KN_EXTERNAL_MODEM_SUPPORT_BLOCKED,
		KN_EXTERNAL_MODEM_SUPPORT_BLOCKED,
		KN_EXTERNAL_MODEM_PROCESS_PLANNED,
		"planned external adapter"
	},
	{
		KN_EXTERNAL_MODEM_TYPE_GENERIC_TCP,
		"Generic TCP Modem",
		KN_EXTERNAL_MODEM_BOUNDARY_TCP_HOST,
		KN_EXTERNAL_MODEM_PROFILE_PLANNED,
		KN_EXTERNAL_MODEM_SUPPORT_BLOCKED,
		KN_EXTERNAL_MODEM_SUPPORT_BLOCKED,
		KN_EXTERNAL_MODEM_PROCESS_UNSUPPORTED,
		"planned documented TCP adapter"
	}
};

const char *
kn_external_modem_profile_boundary_name(
	enum kn_external_modem_profile_boundary boundary)
{
	switch (boundary) {
	case KN_EXTERNAL_MODEM_BOUNDARY_KISS:
		return "kiss";
	case KN_EXTERNAL_MODEM_BOUNDARY_TCP_CONTROL:
		return "tcp-control";
	case KN_EXTERNAL_MODEM_BOUNDARY_TCP_HOST:
		return "tcp-host";
	case KN_EXTERNAL_MODEM_BOUNDARY_UNKNOWN:
		return "unknown";
	default:
		break;
	}

	return "unknown";
}

enum kn_external_modem_error
kn_external_modem_profile_format(
	const struct kn_external_modem_profile *profile, char *buf,
	size_t bufsiz)
{
	int needed;

	if (profile == NULL || buf == NULL || bufsiz == 0)
		return KN_EXTERNAL_MODEM_ERR_INVALID_ARGUMENT;

	needed = snprintf(buf, bufsiz,
	    "MODEM PROFILE type=%s status=%s boundary=%s tx=%s connect=%s "
	    "process=%s notes=\"%s\"",
	    kn_external_modem_type_name(profile->type),
	    kn_external_modem_profile_status_name(profile->current_status),
	    kn_external_modem_profile_boundary_name(profile->boundary),
	    kn_external_modem_profile_support_name(profile->tx_support),
	    kn_external_modem_profile_support_name(profile->connect_support),
	    kn_external_modem_profile_process_name(profile->process_launch),
	    profile->notes);
	if (needed < 0 || (size_t)needed >= bufsiz)
		return KN_EXTERNAL_MODEM_ERR_BUFFER;

	return KN_EXTERNAL_MODEM_OK;
}

const struct kn_external_modem_profile *
kn_external_modem_profile_for_type(enum kn_external_modem_type type)
{
	size_t i;

	for (i = 0; i < sizeof(profiles) / sizeof(profiles[0]); i++) {
		if (profiles[i].type == type)
			return &profiles[i];
	}

	return NULL;
}

const struct kn_external_modem_profile *
kn_external_modem_profiles(size_t *count)
{
	if (count != NULL)
		*count = sizeof(profiles) / sizeof(profiles[0]);

	return profiles;
}

const char *
kn_external_modem_profile_process_name(
	enum kn_external_modem_profile_process process)
{
	switch (process) {
	case KN_EXTERNAL_MODEM_PROCESS_UNSUPPORTED:
		return "unsupported";
	case KN_EXTERNAL_MODEM_PROCESS_PLANNED:
		return "planned";
	default:
		break;
	}

	return "unknown";
}

const char *
kn_external_modem_profile_status_name(
	enum kn_external_modem_profile_status status)
{
	switch (status) {
	case KN_EXTERNAL_MODEM_PROFILE_WORKING:
		return "working";
	case KN_EXTERNAL_MODEM_PROFILE_RECEIVE_ONLY:
		return "receive-only";
	case KN_EXTERNAL_MODEM_PROFILE_PLANNED:
		return "planned";
	case KN_EXTERNAL_MODEM_PROFILE_BLOCKED:
		return "blocked";
	default:
		break;
	}

	return "unknown";
}

const char *
kn_external_modem_profile_support_name(
	enum kn_external_modem_profile_support support)
{
	switch (support) {
	case KN_EXTERNAL_MODEM_SUPPORT_BLOCKED:
		return "blocked";
	case KN_EXTERNAL_MODEM_SUPPORT_PLANNED:
		return "planned";
	default:
		break;
	}

	return "unknown";
}
