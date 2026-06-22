/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_external_modem_control.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "kilonode/external_modem_control.h"

static void make_table(struct kn_external_modem_status_table *);
static int test_modem_existing(void);
static int test_modem_missing(void);
static int test_modem_profiles(void);
static int test_modems_empty(void);
static int test_modems_populated(void);
static int test_no_write_commands(void);

int
main(void)
{
	if (test_modems_empty() != 0)
		return 1;
	if (test_modems_populated() != 0)
		return 1;
	if (test_modem_existing() != 0)
		return 1;
	if (test_modem_missing() != 0)
		return 1;
	if (test_modem_profiles() != 0)
		return 1;
	if (test_no_write_commands() != 0)
		return 1;
	return 0;
}

static void
make_table(struct kn_external_modem_status_table *table)
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
	(void)kn_external_modem_status_table_init(table, &config, 1);
}

static int
test_modem_existing(void)
{
	struct kn_external_modem_status_table table;
	char buf[512];

	make_table(&table);
	if (kn_external_modem_control_format("MODEM mercury0", &table, buf,
	    sizeof(buf)) != KN_EXTERNAL_MODEM_CONTROL_OK)
		return 1;
	if (strstr(buf, "OK MODEM name=mercury0") == NULL)
		return 1;
	return strstr(buf, "END\n") != NULL ? 0 : 1;
}

static int
test_modem_missing(void)
{
	struct kn_external_modem_status_table table;
	char buf[512];

	make_table(&table);
	return kn_external_modem_control_format("MODEM missing", &table, buf,
	    sizeof(buf)) == KN_EXTERNAL_MODEM_CONTROL_ERR_UNKNOWN_COMMAND ?
	    0 : 1;
}

static int
test_modem_profiles(void)
{
	char buf[1024];

	if (kn_external_modem_control_format("MODEM PROFILE mercury-ofdm",
	    NULL, buf, sizeof(buf)) != KN_EXTERNAL_MODEM_CONTROL_OK)
		return 1;
	if (strstr(buf, "type=mercury-ofdm") == NULL)
		return 1;
	if (kn_external_modem_control_format("MODEM PROFILES", NULL, buf,
	    sizeof(buf)) != KN_EXTERNAL_MODEM_CONTROL_OK)
		return 1;
	return strstr(buf, "type=kiss-tcp") != NULL ? 0 : 1;
}

static int
test_modems_empty(void)
{
	char buf[256];

	if (kn_external_modem_control_format("MODEMS", NULL, buf,
	    sizeof(buf)) != KN_EXTERNAL_MODEM_CONTROL_OK)
		return 1;
	return strstr(buf, "OK MODEMS count=0") != NULL ? 0 : 1;
}

static int
test_modems_populated(void)
{
	struct kn_external_modem_status_table table;
	char buf[512];

	make_table(&table);
	if (kn_external_modem_control_format("MODEMS", &table, buf,
	    sizeof(buf)) != KN_EXTERNAL_MODEM_CONTROL_OK)
		return 1;
	return strstr(buf, "MODEM name=mercury0") != NULL ? 0 : 1;
}

static int
test_no_write_commands(void)
{
	char buf[256];

	if (kn_external_modem_control_format("MODEM START mercury0", NULL,
	    buf, sizeof(buf)) != KN_EXTERNAL_MODEM_CONTROL_ERR_UNKNOWN_COMMAND)
		return 1;
	return strstr(buf, "ERR") != NULL ? 0 : 1;
}
