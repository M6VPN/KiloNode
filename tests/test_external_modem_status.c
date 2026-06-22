/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_external_modem_status.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "kilonode/external_modem_status.h"

static void make_mercury(struct kn_external_modem_config *);
static int test_lookup(void);
static int test_planned_status(void);
static int test_status_format(void);

int
main(void)
{
	if (test_planned_status() != 0)
		return 1;
	if (test_lookup() != 0)
		return 1;
	if (test_status_format() != 0)
		return 1;
	return 0;
}

static void
make_mercury(struct kn_external_modem_config *config)
{
	kn_external_modem_config_defaults(config);
	(void)snprintf(config->name, sizeof(config->name), "mercury0");
	config->type = KN_EXTERNAL_MODEM_TYPE_MERCURY_OFDM;
	config->mode = KN_EXTERNAL_MODEM_MODE_STATUS_ONLY;
	(void)snprintf(config->host, sizeof(config->host), "127.0.0.1");
	config->has_type = 1;
	config->has_mode = 1;
	config->has_host = 1;
	config->has_port = 1;
}

static int
test_lookup(void)
{
	struct kn_external_modem_config config;
	struct kn_external_modem_status_table table;

	make_mercury(&config);
	if (kn_external_modem_status_table_init(&table, &config, 1) !=
	    KN_EXTERNAL_MODEM_OK)
		return 1;
	if (kn_external_modem_status_lookup(&table, "mercury0") == NULL)
		return 1;
	return kn_external_modem_status_lookup(&table, "missing") == NULL ?
	    0 : 1;
}

static int
test_planned_status(void)
{
	struct kn_external_modem_config config;
	struct kn_external_modem_status_table table;

	make_mercury(&config);
	config.enabled = 1;
	if (kn_external_modem_status_table_init(&table, &config, 1) !=
	    KN_EXTERNAL_MODEM_OK)
		return 1;
	if (table.count != 1)
		return 1;
	if (table.entries[0].state != KN_EXTERNAL_MODEM_STATE_PLANNED)
		return 1;
	return table.entries[0].tx_enabled == 0 &&
	    table.entries[0].connect_enabled == 0 ? 0 : 1;
}

static int
test_status_format(void)
{
	struct kn_external_modem_config config;
	struct kn_external_modem_status_table table;
	char buf[256];

	make_mercury(&config);
	if (kn_external_modem_status_table_init(&table, &config, 1) !=
	    KN_EXTERNAL_MODEM_OK)
		return 1;
	if (kn_external_modem_status_format(&table.entries[0], buf,
	    sizeof(buf)) != KN_EXTERNAL_MODEM_OK)
		return 1;
	if (strstr(buf, "name=mercury0") == NULL)
		return 1;
	if (strstr(buf, "tx=false") == NULL)
		return 1;
	return strstr(buf, "connect=false") != NULL ? 0 : 1;
}
