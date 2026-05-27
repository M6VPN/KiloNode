/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_rf_abuse.c */

#include <sys/types.h>

#include <string.h>

#include "kilonode/rf_abuse.h"

static void config_base(struct kn_config_rf_command *);
static int test_auto_ignore(void);
static int test_command_rate_limit(void);
static int test_default_state(void);
static int test_eviction(void);
static int test_per_source_independent(void);
static int test_reply_rate_limit(void);

int
main(void)
{
	if (test_default_state() != 0)
		return 1;
	if (test_command_rate_limit() != 0)
		return 1;
	if (test_reply_rate_limit() != 0)
		return 1;
	if (test_per_source_independent() != 0)
		return 1;
	if (test_eviction() != 0)
		return 1;
	if (test_auto_ignore() != 0)
		return 1;

	return 0;
}

static void
config_base(struct kn_config_rf_command *config)
{
	memset(config, 0, sizeof(*config));
	config->rate_limit_enabled = 1;
	config->rate_limit_commands = 2;
	config->rate_limit_window_seconds = 60;
	config->reply_rate_limit_commands = 1;
	config->reply_rate_limit_window_seconds = 60;
	config->auto_ignore_after_rejects = 2;
	config->auto_ignore_seconds = 30;
}

static int
test_auto_ignore(void)
{
	struct kn_rf_abuse_state state;
	struct kn_config_rf_command config;
	struct kn_callsign call;
	char reason[KN_RF_ABUSE_REASON_MAX];

	config_base(&config);
	config.auto_ignore_enabled = 1;
	kn_rf_abuse_init(&state, 4);
	(void)kn_callsign_parse("N0CALL", &call);

	if (kn_rf_abuse_record_rejected(&state, &config, &call, 10,
	    "unknown-command") != KN_RF_ABUSE_OK)
		return 1;
	if (kn_rf_abuse_record_rejected(&state, &config, &call, 11,
	    "unknown-command") != KN_RF_ABUSE_ERR_IGNORED)
		return 1;
	if (kn_rf_abuse_check_command(&state, &config, NULL, &call, 12,
	    reason, sizeof(reason)) != KN_RF_ABUSE_ERR_IGNORED)
		return 1;
	if (strcmp(reason, "auto-ignore") != 0)
		return 1;

	return kn_rf_abuse_check_command(&state, &config, NULL, &call, 42,
	    reason, sizeof(reason)) == KN_RF_ABUSE_OK ? 0 : 1;
}

static int
test_command_rate_limit(void)
{
	struct kn_rf_abuse_state state;
	struct kn_config_rf_command config;
	struct kn_callsign call;
	char reason[KN_RF_ABUSE_REASON_MAX];

	config_base(&config);
	kn_rf_abuse_init(&state, 4);
	(void)kn_callsign_parse("N0CALL", &call);

	if (kn_rf_abuse_check_command(&state, &config, NULL, &call, 10,
	    reason, sizeof(reason)) != KN_RF_ABUSE_OK)
		return 1;
	if (kn_rf_abuse_check_command(&state, &config, NULL, &call, 11,
	    reason, sizeof(reason)) != KN_RF_ABUSE_OK)
		return 1;
	if (kn_rf_abuse_check_command(&state, &config, NULL, &call, 12,
	    reason, sizeof(reason)) != KN_RF_ABUSE_ERR_RATE_LIMITED)
		return 1;
	if (strcmp(reason, "rate-limited") != 0)
		return 1;

	return kn_rf_abuse_check_command(&state, &config, NULL, &call, 71,
	    reason, sizeof(reason)) == KN_RF_ABUSE_OK ? 0 : 1;
}

static int
test_default_state(void)
{
	struct kn_rf_abuse_state state;

	kn_rf_abuse_init(&state, 4);
	return kn_rf_abuse_count(&state) == 0 && state.max_sources == 4 ? 0 : 1;
}

static int
test_eviction(void)
{
	struct kn_rf_abuse_state state;
	struct kn_config_rf_command config;
	struct kn_callsign a;
	struct kn_callsign b;
	struct kn_callsign c;
	char reason[KN_RF_ABUSE_REASON_MAX];

	config_base(&config);
	kn_rf_abuse_init(&state, 2);
	(void)kn_callsign_parse("N0CALL", &a);
	(void)kn_callsign_parse("M6VPN-1", &b);
	(void)kn_callsign_parse("K1ABC", &c);

	(void)kn_rf_abuse_check_command(&state, &config, NULL, &a, 10,
	    reason, sizeof(reason));
	(void)kn_rf_abuse_check_command(&state, &config, NULL, &b, 11,
	    reason, sizeof(reason));
	(void)kn_rf_abuse_check_command(&state, &config, NULL, &c, 12,
	    reason, sizeof(reason));

	return kn_rf_abuse_find(&state, &a) == NULL &&
	    kn_rf_abuse_find(&state, &b) != NULL &&
	    kn_rf_abuse_find(&state, &c) != NULL ? 0 : 1;
}

static int
test_per_source_independent(void)
{
	struct kn_rf_abuse_state state;
	struct kn_config_rf_command config;
	struct kn_callsign a;
	struct kn_callsign b;
	char reason[KN_RF_ABUSE_REASON_MAX];

	config_base(&config);
	kn_rf_abuse_init(&state, 4);
	(void)kn_callsign_parse("N0CALL", &a);
	(void)kn_callsign_parse("M6VPN-1", &b);

	(void)kn_rf_abuse_check_command(&state, &config, NULL, &a, 10,
	    reason, sizeof(reason));
	(void)kn_rf_abuse_check_command(&state, &config, NULL, &a, 11,
	    reason, sizeof(reason));
	if (kn_rf_abuse_check_command(&state, &config, NULL, &a, 12,
	    reason, sizeof(reason)) != KN_RF_ABUSE_ERR_RATE_LIMITED)
		return 1;

	return kn_rf_abuse_check_command(&state, &config, NULL, &b, 12,
	    reason, sizeof(reason)) == KN_RF_ABUSE_OK ? 0 : 1;
}

static int
test_reply_rate_limit(void)
{
	struct kn_rf_abuse_state state;
	struct kn_config_rf_command config;
	struct kn_callsign call;
	char reason[KN_RF_ABUSE_REASON_MAX];

	config_base(&config);
	kn_rf_abuse_init(&state, 4);
	(void)kn_callsign_parse("N0CALL", &call);

	if (kn_rf_abuse_check_reply(&state, &config, &call, 10, reason,
	    sizeof(reason)) != KN_RF_ABUSE_OK)
		return 1;
	(void)kn_rf_abuse_record_reply(&state, &call, 10);
	if (kn_rf_abuse_check_reply(&state, &config, &call, 11, reason,
	    sizeof(reason)) != KN_RF_ABUSE_ERR_REPLY_RATE_LIMITED)
		return 1;
	if (strcmp(reason, "reply-rate-limited") != 0)
		return 1;

	return kn_rf_abuse_check_reply(&state, &config, &call, 71, reason,
	    sizeof(reason)) == KN_RF_ABUSE_OK ? 0 : 1;
}
