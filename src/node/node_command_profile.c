/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/node/node_command_profile.c */

#include <sys/types.h>

#include <string.h>

#include "kilonode/node_command_profile.h"

static const struct kn_node_command_profile profiles[] = {
	{
		KN_NODE_COMMAND_ID_HELP, "HELP", 0, { NULL, NULL },
		KN_NODE_COMMAND_CONTEXT_LOCAL | KN_NODE_COMMAND_CONTEXT_RF_UI,
		KN_NODE_COMMAND_ARGS_NONE, KN_NODE_COMMAND_OUTPUT_MULTI_LINE,
		KN_NODE_COMMAND_SAFETY_READ_ONLY
	},
	{
		KN_NODE_COMMAND_ID_INFO, "INFO", 0, { NULL, NULL },
		KN_NODE_COMMAND_CONTEXT_LOCAL | KN_NODE_COMMAND_CONTEXT_RF_UI,
		KN_NODE_COMMAND_ARGS_NONE, KN_NODE_COMMAND_OUTPUT_ONE_LINE,
		KN_NODE_COMMAND_SAFETY_READ_ONLY
	},
	{
		KN_NODE_COMMAND_ID_PORTS, "PORTS", 0, { NULL, NULL },
		KN_NODE_COMMAND_CONTEXT_LOCAL | KN_NODE_COMMAND_CONTEXT_RF_UI,
		KN_NODE_COMMAND_ARGS_NONE, KN_NODE_COMMAND_OUTPUT_MULTI_LINE,
		KN_NODE_COMMAND_SAFETY_READ_ONLY
	},
	{
		KN_NODE_COMMAND_ID_HEARD, "HEARD", 0, { NULL, NULL },
		KN_NODE_COMMAND_CONTEXT_LOCAL | KN_NODE_COMMAND_CONTEXT_RF_UI,
		KN_NODE_COMMAND_ARGS_OPTIONAL, KN_NODE_COMMAND_OUTPUT_MULTI_LINE,
		KN_NODE_COMMAND_SAFETY_READ_ONLY
	},
	{
		KN_NODE_COMMAND_ID_STATS, "STATS", 0, { NULL, NULL },
		KN_NODE_COMMAND_CONTEXT_LOCAL | KN_NODE_COMMAND_CONTEXT_RF_UI,
		KN_NODE_COMMAND_ARGS_NONE, KN_NODE_COMMAND_OUTPUT_ONE_LINE,
		KN_NODE_COMMAND_SAFETY_READ_ONLY
	},
	{
		KN_NODE_COMMAND_ID_USERS, "USERS", 0, { NULL, NULL },
		KN_NODE_COMMAND_CONTEXT_LOCAL,
		KN_NODE_COMMAND_ARGS_NONE, KN_NODE_COMMAND_OUTPUT_MULTI_LINE,
		KN_NODE_COMMAND_SAFETY_DISABLED_OVER_RF
	},
	{
		KN_NODE_COMMAND_ID_BBS, "BBS", 0, { NULL, NULL },
		KN_NODE_COMMAND_CONTEXT_LOCAL,
		KN_NODE_COMMAND_ARGS_REQUIRED,
		KN_NODE_COMMAND_OUTPUT_MODE_TRANSITION,
		KN_NODE_COMMAND_SAFETY_LOCAL_ONLY
	},
	{
		KN_NODE_COMMAND_ID_BYE, "BYE", 0, { NULL, NULL },
		KN_NODE_COMMAND_CONTEXT_LOCAL,
		KN_NODE_COMMAND_ARGS_NONE, KN_NODE_COMMAND_OUTPUT_CLOSE_SESSION,
		KN_NODE_COMMAND_SAFETY_LOCAL_ONLY
	},
	{
		KN_NODE_COMMAND_ID_QUIT, "QUIT", 0, { NULL, NULL },
		KN_NODE_COMMAND_CONTEXT_LOCAL,
		KN_NODE_COMMAND_ARGS_NONE, KN_NODE_COMMAND_OUTPUT_CLOSE_SESSION,
		KN_NODE_COMMAND_SAFETY_LOCAL_ONLY
	},
	{
		KN_NODE_COMMAND_ID_PING, "PING", 0, { NULL, NULL },
		KN_NODE_COMMAND_CONTEXT_RF_UI,
		KN_NODE_COMMAND_ARGS_NONE, KN_NODE_COMMAND_OUTPUT_ONE_LINE,
		KN_NODE_COMMAND_SAFETY_READ_ONLY
	}
};

const char *
kn_node_command_argument_policy_name(enum kn_node_command_argument_policy policy)
{
	switch (policy) {
	case KN_NODE_COMMAND_ARGS_NONE:
		return "none";
	case KN_NODE_COMMAND_ARGS_OPTIONAL:
		return "optional";
	case KN_NODE_COMMAND_ARGS_REQUIRED:
		return "required";
	case KN_NODE_COMMAND_ARGS_FREE_TEXT:
		return "free-text";
	}

	return "unknown";
}

enum kn_node_command_id
kn_node_command_id_from_name(const char *name)
{
	const struct kn_node_command_profile *profile;

	profile = kn_node_command_profile_find(name);
	return profile == NULL ? KN_NODE_COMMAND_ID_UNKNOWN : profile->id;
}

const char *
kn_node_command_id_name(enum kn_node_command_id id)
{
	const struct kn_node_command_profile *profile;

	profile = kn_node_command_profile_find_id(id);
	return profile == NULL ? "UNKNOWN" : profile->name;
}

const char *
kn_node_command_output_class_name(enum kn_node_command_output_class output)
{
	switch (output) {
	case KN_NODE_COMMAND_OUTPUT_ONE_LINE:
		return "one-line";
	case KN_NODE_COMMAND_OUTPUT_MULTI_LINE:
		return "multi-line";
	case KN_NODE_COMMAND_OUTPUT_MODE_TRANSITION:
		return "mode-transition";
	case KN_NODE_COMMAND_OUTPUT_CLOSE_SESSION:
		return "close-session";
	case KN_NODE_COMMAND_OUTPUT_NO_REPLY:
		return "no-reply";
	}

	return "unknown";
}

const struct kn_node_command_profile *
kn_node_command_profile_find(const char *name)
{
	size_t i;
	size_t j;

	if (name == NULL)
		return NULL;
	for (i = 0; i < sizeof(profiles) / sizeof(profiles[0]); i++) {
		if (strcmp(name, profiles[i].name) == 0)
			return &profiles[i];
		for (j = 0; j < sizeof(profiles[i].aliases) /
		    sizeof(profiles[i].aliases[0]); j++) {
			if (profiles[i].aliases[j] != NULL &&
			    strcmp(name, profiles[i].aliases[j]) == 0)
				return &profiles[i];
		}
	}

	return NULL;
}

const struct kn_node_command_profile *
kn_node_command_profile_find_id(enum kn_node_command_id id)
{
	size_t i;

	for (i = 0; i < sizeof(profiles) / sizeof(profiles[0]); i++) {
		if (profiles[i].id == id)
			return &profiles[i];
	}

	return NULL;
}

const struct kn_node_command_profile *
kn_node_command_profiles(size_t *count)
{
	if (count != NULL)
		*count = sizeof(profiles) / sizeof(profiles[0]);
	return profiles;
}

const char *
kn_node_command_safety_class_name(enum kn_node_command_safety_class safety)
{
	switch (safety) {
	case KN_NODE_COMMAND_SAFETY_READ_ONLY:
		return "read-only";
	case KN_NODE_COMMAND_SAFETY_LOCAL_ONLY:
		return "local-only";
	case KN_NODE_COMMAND_SAFETY_DISABLED_OVER_RF:
		return "disabled-over-rf";
	case KN_NODE_COMMAND_SAFETY_FUTURE:
		return "future";
	}

	return "unknown";
}
