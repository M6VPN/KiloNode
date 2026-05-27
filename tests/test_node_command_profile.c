/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_node_command_profile.c */

#include <sys/types.h>

#include <string.h>

#include "kilonode/node_command_profile.h"

static int test_forbidden_rf_profiles(void);
static int test_profile_list(void);
static int test_profiles_exist(void);

int
main(void)
{
	if (test_profiles_exist() != 0)
		return 1;
	if (test_forbidden_rf_profiles() != 0)
		return 1;
	if (test_profile_list() != 0)
		return 1;
	return 0;
}

static int
test_forbidden_rf_profiles(void)
{
	const struct kn_node_command_profile *bbs;
	const struct kn_node_command_profile *bye;
	const struct kn_node_command_profile *ping;

	bbs = kn_node_command_profile_find("BBS");
	bye = kn_node_command_profile_find("BYE");
	ping = kn_node_command_profile_find("PING");
	if (bbs == NULL || bye == NULL || ping == NULL)
		return 1;
	if ((bbs->contexts & KN_NODE_COMMAND_CONTEXT_RF_UI) != 0)
		return 1;
	if (bbs->safety_class != KN_NODE_COMMAND_SAFETY_LOCAL_ONLY)
		return 1;
	if (bye->output_class != KN_NODE_COMMAND_OUTPUT_CLOSE_SESSION)
		return 1;
	return (ping->contexts & KN_NODE_COMMAND_CONTEXT_RF_UI) != 0 ? 0 : 1;
}

static int
test_profile_list(void)
{
	const struct kn_node_command_profile *profiles;
	size_t count;
	size_t i;

	profiles = kn_node_command_profiles(&count);
	if (profiles == NULL || count != 10)
		return 1;
	for (i = 0; i < count; i++) {
		if (profiles[i].aliases[0] != NULL ||
		    profiles[i].min_abbrev != 0)
			return 1;
	}
	return 0;
}

static int
test_profiles_exist(void)
{
	if (kn_node_command_profile_find("HELP") == NULL)
		return 1;
	if (kn_node_command_profile_find("INFO") == NULL)
		return 1;
	if (kn_node_command_profile_find("PORTS") == NULL)
		return 1;
	if (kn_node_command_profile_find("HEARD") == NULL)
		return 1;
	if (kn_node_command_profile_find("STATS") == NULL)
		return 1;
	if (kn_node_command_profile_find("PING") == NULL)
		return 1;
	if (kn_node_command_profile_find("C") != NULL)
		return 1;
	return strcmp(kn_node_command_id_name(KN_NODE_COMMAND_ID_QUIT),
	    "QUIT") == 0 ? 0 : 1;
}
