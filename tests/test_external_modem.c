/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_external_modem.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "kilonode/external_modem.h"

static int test_defaults_disabled(void);
static int test_invalid_host_rejected(void);
static int test_invalid_name_rejected(void);
static int test_kiss_receive_requires_endpoint(void);
static int test_mercury_planned_valid(void);
static int test_unsafe_booleans_rejected(void);

int
main(void)
{
	if (test_defaults_disabled() != 0)
		return 1;
	if (test_mercury_planned_valid() != 0)
		return 1;
	if (test_kiss_receive_requires_endpoint() != 0)
		return 1;
	if (test_invalid_name_rejected() != 0)
		return 1;
	if (test_invalid_host_rejected() != 0)
		return 1;
	if (test_unsafe_booleans_rejected() != 0)
		return 1;
	return 0;
}

static int
test_defaults_disabled(void)
{
	struct kn_external_modem_config config;

	kn_external_modem_config_defaults(&config);
	if (config.enabled != 0)
		return 1;
	return config.mode == KN_EXTERNAL_MODEM_MODE_DISABLED ? 0 : 1;
}

static int
test_invalid_host_rejected(void)
{
	return kn_external_modem_host_valid("127.0.0.1;sh") == 0 ? 0 : 1;
}

static int
test_invalid_name_rejected(void)
{
	return kn_external_modem_name_valid("../mercury") == 0 ? 0 : 1;
}

static int
test_kiss_receive_requires_endpoint(void)
{
	struct kn_external_modem_config config;

	kn_external_modem_config_defaults(&config);
	(void)snprintf(config.name, sizeof(config.name), "direwolf0");
	config.type = KN_EXTERNAL_MODEM_TYPE_KISS_TCP;
	config.mode = KN_EXTERNAL_MODEM_MODE_RECEIVE_ONLY;
	config.has_type = 1;
	config.has_mode = 1;
	if (kn_external_modem_config_validate(&config) !=
	    KN_EXTERNAL_MODEM_ERR_INVALID_VALUE)
		return 1;
	(void)snprintf(config.host, sizeof(config.host), "127.0.0.1");
	config.has_host = 1;
	config.port = 8001;
	config.has_port = 1;
	return kn_external_modem_config_validate(&config) ==
	    KN_EXTERNAL_MODEM_OK ? 0 : 1;
}

static int
test_mercury_planned_valid(void)
{
	struct kn_external_modem_config config;

	kn_external_modem_config_defaults(&config);
	(void)snprintf(config.name, sizeof(config.name), "mercury0");
	config.type = KN_EXTERNAL_MODEM_TYPE_MERCURY_OFDM;
	config.mode = KN_EXTERNAL_MODEM_MODE_STATUS_ONLY;
	(void)snprintf(config.host, sizeof(config.host), "127.0.0.1");
	config.has_type = 1;
	config.has_mode = 1;
	config.has_host = 1;
	config.has_port = 1;
	return kn_external_modem_config_validate(&config) ==
	    KN_EXTERNAL_MODEM_OK ? 0 : 1;
}

static int
test_unsafe_booleans_rejected(void)
{
	struct kn_external_modem_config config;

	kn_external_modem_config_defaults(&config);
	(void)snprintf(config.name, sizeof(config.name), "mercury0");
	config.type = KN_EXTERNAL_MODEM_TYPE_MERCURY_OFDM;
	config.mode = KN_EXTERNAL_MODEM_MODE_STATUS_ONLY;
	config.tx_enabled = 1;
	if (kn_external_modem_config_validate(&config) !=
	    KN_EXTERNAL_MODEM_ERR_INVALID_VALUE)
		return 1;
	config.tx_enabled = 0;
	config.connect_enabled = 1;
	if (kn_external_modem_config_validate(&config) !=
	    KN_EXTERNAL_MODEM_ERR_INVALID_VALUE)
		return 1;
	config.connect_enabled = 0;
	config.auto_start = 1;
	return kn_external_modem_config_validate(&config) ==
	    KN_EXTERNAL_MODEM_ERR_INVALID_VALUE ? 0 : 1;
}
