/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_external_modem_profile.c */

#include <sys/types.h>

#include <string.h>

#include "kilonode/external_modem_profile.h"

static int test_all_requested_profiles_exist(void);
static int test_format_mercury(void);
static int test_unknown_profile_rejected(void);

int
main(void)
{
	if (test_all_requested_profiles_exist() != 0)
		return 1;
	if (test_format_mercury() != 0)
		return 1;
	if (test_unknown_profile_rejected() != 0)
		return 1;
	return 0;
}

static int
test_all_requested_profiles_exist(void)
{
	if (kn_external_modem_profile_for_type(
	    KN_EXTERNAL_MODEM_TYPE_KISS_TCP) == NULL)
		return 1;
	if (kn_external_modem_profile_for_type(
	    KN_EXTERNAL_MODEM_TYPE_MERCURY_OFDM) == NULL)
		return 1;
	if (kn_external_modem_profile_for_type(
	    KN_EXTERNAL_MODEM_TYPE_VARA_HF) == NULL)
		return 1;
	if (kn_external_modem_profile_for_type(
	    KN_EXTERNAL_MODEM_TYPE_VARA_FM) == NULL)
		return 1;
	if (kn_external_modem_profile_for_type(
	    KN_EXTERNAL_MODEM_TYPE_ARDOP) == NULL)
		return 1;
	return 0;
}

static int
test_format_mercury(void)
{
	const struct kn_external_modem_profile *profile;
	char buf[256];

	profile = kn_external_modem_profile_for_type(
	    KN_EXTERNAL_MODEM_TYPE_MERCURY_OFDM);
	if (profile == NULL)
		return 1;
	if (profile->current_status != KN_EXTERNAL_MODEM_PROFILE_PLANNED)
		return 1;
	if (kn_external_modem_profile_format(profile, buf, sizeof(buf)) !=
	    KN_EXTERNAL_MODEM_OK)
		return 1;
	if (strstr(buf, "type=mercury-ofdm") == NULL)
		return 1;
	if (strstr(buf, "status=planned") == NULL)
		return 1;
	return strstr(buf, "interface discovery required") != NULL ? 0 : 1;
}

static int
test_unknown_profile_rejected(void)
{
	return kn_external_modem_profile_for_type(
	    KN_EXTERNAL_MODEM_TYPE_NONE) == NULL ? 0 : 1;
}
