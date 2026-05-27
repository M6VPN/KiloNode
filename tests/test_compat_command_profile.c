/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_compat_command_profile.c */

#include <sys/types.h>

#include <string.h>

#include "kilonode/compat_command_profile.h"
#include "kilonode/compat_observation_pack.h"

static int test_bad_values(void);
static int test_find(void);
static int test_generate(void);
static int test_report(void);
static int test_valid(void);

int
main(void)
{
	if (test_valid() != 0)
		return 1;
	if (test_bad_values() != 0)
		return 1;
	if (test_find() != 0)
		return 1;
	if (test_report() != 0)
		return 1;
	if (test_generate() != 0)
		return 1;
	return 0;
}

static int
test_bad_values(void)
{
	const char bad_category[] =
	    "name x\nsubject y\nclean-room true\nsource-code-used false\n"
	    "command HELP {\ncategory nope\ntransport telnet\nargs none\n"
	    "reply one-line\nstateful false\nrequires-connected-mode false\n"
	    "compat-status planned\n}\n";
	const char source_used[] =
	    "name x\nsubject y\nclean-room true\nsource-code-used true\n"
	    "command HELP {\ncategory informational\ntransport telnet\n"
	    "args none\nreply one-line\nstateful false\n"
	    "requires-connected-mode false\ncompat-status planned\n}\n";
	struct kn_compat_command_profiles profiles;

	if (kn_compat_command_profiles_parse_text(bad_category, &profiles,
	    NULL) != KN_COMPAT_PROFILE_ERR_INVALID_VALUE)
		return 1;
	return kn_compat_command_profiles_parse_text(source_used, &profiles,
	    NULL) == KN_COMPAT_PROFILE_ERR_INVALID_VALUE ? 0 : 1;
}

static int
test_find(void)
{
	struct kn_compat_command_profiles profiles;

	if (kn_compat_command_profiles_parse_file(
	    "../tests/fixtures/compat/linbpq-node/command-profiles.plan",
	    &profiles, NULL) != KN_COMPAT_PROFILE_OK)
		return 1;
	if (kn_compat_command_profile_find(&profiles, "HELP") == NULL)
		return 1;
	return kn_compat_command_profile_find(&profiles, "NOPE") == NULL ?
	    0 : 1;
}

static int
test_generate(void)
{
	struct kn_compat_observation_pack pack;
	struct kn_compat_command_profiles profiles;
	char text[KN_COMPAT_PROFILE_TEXT_MAX];

	if (kn_compat_observation_pack_parse_file(
	    "../tests/fixtures/compat/linbpq-node/manifest.pack", &pack,
	    NULL) != KN_COMPAT_PACK_OK)
		return 1;
	if (kn_compat_command_profiles_generate_from_pack(&pack, text,
	    sizeof(text)) != KN_COMPAT_PROFILE_OK)
		return 1;
	if (kn_compat_command_profiles_parse_text(text, &profiles, NULL) !=
	    KN_COMPAT_PROFILE_OK)
		return 1;
	return kn_compat_command_profile_find(&profiles, "NODES") != NULL ?
	    0 : 1;
}

static int
test_report(void)
{
	struct kn_compat_command_profiles profiles;
	char report[KN_COMPAT_PROFILE_REPORT_MAX];

	if (kn_compat_command_profiles_parse_file(
	    "../tests/fixtures/compat/linbpq-node/command-profiles.plan",
	    &profiles, NULL) != KN_COMPAT_PROFILE_OK)
		return 1;
	if (kn_compat_command_profiles_report(&profiles, report,
	    sizeof(report)) != KN_COMPAT_PROFILE_OK)
		return 1;
	return strstr(report, "COMMAND-PROFILE command=CONNECT") != NULL ?
	    0 : 1;
}

static int
test_valid(void)
{
	struct kn_compat_command_profiles profiles;

	if (kn_compat_command_profiles_parse_file(
	    "../tests/fixtures/compat/linbpq-node/command-profiles.plan",
	    &profiles, NULL) != KN_COMPAT_PROFILE_OK)
		return 1;
	if (strcmp(profiles.name, "linbpq-node-command-profiles") != 0)
		return 1;
	return profiles.profile_count == 9 ? 0 : 1;
}
