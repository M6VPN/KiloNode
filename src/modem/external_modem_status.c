/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/modem/external_modem_status.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "kilonode/external_modem_status.h"

static enum kn_external_modem_state config_state(
	const struct kn_external_modem_config *);
static const char *config_reason(const struct kn_external_modem_config *);

static enum kn_external_modem_state
config_state(const struct kn_external_modem_config *config)
{
	if (config->enabled == 0)
		return KN_EXTERNAL_MODEM_STATE_DISABLED;
	if (config->mode == KN_EXTERNAL_MODEM_MODE_PLANNED ||
	    config->type == KN_EXTERNAL_MODEM_TYPE_MERCURY_OFDM ||
	    config->type == KN_EXTERNAL_MODEM_TYPE_VARA_HF ||
	    config->type == KN_EXTERNAL_MODEM_TYPE_VARA_FM ||
	    config->type == KN_EXTERNAL_MODEM_TYPE_ARDOP)
		return KN_EXTERNAL_MODEM_STATE_PLANNED;
	if (config->mode == KN_EXTERNAL_MODEM_MODE_RECEIVE_ONLY &&
	    config->type == KN_EXTERNAL_MODEM_TYPE_KISS_TCP)
		return KN_EXTERNAL_MODEM_STATE_CONFIGURED;
	if (config->mode == KN_EXTERNAL_MODEM_MODE_STATUS_ONLY)
		return KN_EXTERNAL_MODEM_STATE_PLANNED;

	return KN_EXTERNAL_MODEM_STATE_UNKNOWN;
}

static const char *
config_reason(const struct kn_external_modem_config *config)
{
	if (config->enabled == 0)
		return "disabled";
	if (config->type == KN_EXTERNAL_MODEM_TYPE_MERCURY_OFDM)
		return "interface discovery required";
	if (config->type == KN_EXTERNAL_MODEM_TYPE_KISS_TCP &&
	    config->mode == KN_EXTERNAL_MODEM_MODE_RECEIVE_ONLY)
		return "receive-only external endpoint";
	if (config->mode == KN_EXTERNAL_MODEM_MODE_PLANNED)
		return "planned external adapter";
	if (config->mode == KN_EXTERNAL_MODEM_MODE_STATUS_ONLY)
		return "status-only external adapter";

	return "unknown";
}

enum kn_external_modem_error
kn_external_modem_status_format(
	const struct kn_external_modem_status_entry *entry, char *buf,
	size_t bufsiz)
{
	int needed;

	if (entry == NULL || buf == NULL || bufsiz == 0)
		return KN_EXTERNAL_MODEM_ERR_INVALID_ARGUMENT;

	needed = snprintf(buf, bufsiz,
	    "MODEM name=%s type=%s enabled=%s mode=%s host=%s port=%u "
	    "status=%s tx=%s connect=%s autostart=%s reason=\"%s\"",
	    entry->name, kn_external_modem_type_name(entry->type),
	    entry->enabled != 0 ? "true" : "false",
	    kn_external_modem_mode_name(entry->mode),
	    entry->host[0] != '\0' ? entry->host : "-",
	    (unsigned int)entry->port,
	    kn_external_modem_state_name(entry->state),
	    entry->tx_enabled != 0 ? "true" : "false",
	    entry->connect_enabled != 0 ? "true" : "false",
	    entry->auto_start != 0 ? "true" : "false",
	    entry->reason);
	if (needed < 0 || (size_t)needed >= bufsiz)
		return KN_EXTERNAL_MODEM_ERR_BUFFER;

	return KN_EXTERNAL_MODEM_OK;
}

const struct kn_external_modem_status_entry *
kn_external_modem_status_lookup(
	const struct kn_external_modem_status_table *table, const char *name)
{
	size_t i;

	if (table == NULL || name == NULL)
		return NULL;

	for (i = 0; i < table->count; i++) {
		if (strcmp(table->entries[i].name, name) == 0)
			return &table->entries[i];
	}

	return NULL;
}

enum kn_external_modem_error
kn_external_modem_status_table_init(
	struct kn_external_modem_status_table *table,
	const struct kn_external_modem_config *configs, size_t count)
{
	struct kn_external_modem_status_entry *entry;
	size_t i;
	int needed;

	if (table == NULL || (configs == NULL && count != 0))
		return KN_EXTERNAL_MODEM_ERR_INVALID_ARGUMENT;
	if (count > KN_EXTERNAL_MODEM_MAX)
		return KN_EXTERNAL_MODEM_ERR_FULL;

	memset(table, 0, sizeof(*table));
	for (i = 0; i < count; i++) {
		entry = &table->entries[i];
		needed = snprintf(entry->name, sizeof(entry->name), "%s",
		    configs[i].name);
		if (needed < 0 || (size_t)needed >= sizeof(entry->name))
			return KN_EXTERNAL_MODEM_ERR_BUFFER;
		needed = snprintf(entry->host, sizeof(entry->host), "%s",
		    configs[i].host);
		if (needed < 0 || (size_t)needed >= sizeof(entry->host))
			return KN_EXTERNAL_MODEM_ERR_BUFFER;
		needed = snprintf(entry->reason, sizeof(entry->reason), "%s",
		    config_reason(&configs[i]));
		if (needed < 0 || (size_t)needed >= sizeof(entry->reason))
			return KN_EXTERNAL_MODEM_ERR_BUFFER;
		entry->type = configs[i].type;
		entry->mode = configs[i].mode;
		entry->port = configs[i].port;
		entry->configured = 1;
		entry->enabled = configs[i].enabled;
		entry->auto_start = configs[i].auto_start;
		entry->tx_enabled = configs[i].tx_enabled;
		entry->connect_enabled = configs[i].connect_enabled;
		entry->state = config_state(&configs[i]);
		entry->profile = kn_external_modem_profile_for_type(
		    configs[i].type);
	}
	table->count = count;

	return KN_EXTERNAL_MODEM_OK;
}
