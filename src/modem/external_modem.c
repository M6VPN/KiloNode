/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/modem/external_modem.c */

#include <sys/types.h>

#include <ctype.h>
#include <stdint.h>
#include <string.h>

#include "kilonode/external_modem.h"

void
kn_external_modem_config_defaults(struct kn_external_modem_config *config)
{
	if (config == NULL)
		return;

	memset(config, 0, sizeof(*config));
	config->mode = KN_EXTERNAL_MODEM_MODE_DISABLED;
}

enum kn_external_modem_error
kn_external_modem_config_validate(
	const struct kn_external_modem_config *config)
{
	if (config == NULL)
		return KN_EXTERNAL_MODEM_ERR_INVALID_ARGUMENT;
	if (kn_external_modem_name_valid(config->name) == 0)
		return KN_EXTERNAL_MODEM_ERR_INVALID_VALUE;
	if (config->type == KN_EXTERNAL_MODEM_TYPE_NONE)
		return KN_EXTERNAL_MODEM_ERR_INVALID_VALUE;
	if (config->mode == KN_EXTERNAL_MODEM_MODE_NONE)
		return KN_EXTERNAL_MODEM_ERR_INVALID_VALUE;
	if (config->tx_enabled != 0 || config->connect_enabled != 0 ||
	    config->auto_start != 0)
		return KN_EXTERNAL_MODEM_ERR_INVALID_VALUE;
	if (config->has_host != 0 &&
	    kn_external_modem_host_valid(config->host) == 0)
		return KN_EXTERNAL_MODEM_ERR_INVALID_VALUE;
	if (config->mode == KN_EXTERNAL_MODEM_MODE_RECEIVE_ONLY &&
	    config->type == KN_EXTERNAL_MODEM_TYPE_KISS_TCP &&
	    (config->has_host == 0 || config->port == 0))
		return KN_EXTERNAL_MODEM_ERR_INVALID_VALUE;

	return KN_EXTERNAL_MODEM_OK;
}

const char *
kn_external_modem_error_name(enum kn_external_modem_error error)
{
	switch (error) {
	case KN_EXTERNAL_MODEM_OK:
		return "ok";
	case KN_EXTERNAL_MODEM_ERR_INVALID_ARGUMENT:
		return "invalid-argument";
	case KN_EXTERNAL_MODEM_ERR_INVALID_VALUE:
		return "invalid-value";
	case KN_EXTERNAL_MODEM_ERR_NOT_FOUND:
		return "not-found";
	case KN_EXTERNAL_MODEM_ERR_DUPLICATE:
		return "duplicate";
	case KN_EXTERNAL_MODEM_ERR_FULL:
		return "full";
	case KN_EXTERNAL_MODEM_ERR_BUFFER:
		return "buffer";
	default:
		break;
	}

	return "unknown";
}

uint8_t
kn_external_modem_host_valid(const char *host)
{
	size_t i;
	unsigned char ch;

	if (host == NULL || host[0] == '\0' ||
	    strlen(host) >= KN_EXTERNAL_MODEM_HOST_MAX)
		return 0;

	for (i = 0; host[i] != '\0'; i++) {
		ch = (unsigned char)host[i];
		if (isalnum(ch) != 0 || ch == '.' || ch == '-' || ch == '_' ||
		    ch == ':')
			continue;
		return 0;
	}

	return 1;
}

uint8_t
kn_external_modem_name_valid(const char *name)
{
	size_t i;
	unsigned char ch;

	if (name == NULL || name[0] == '\0' ||
	    strlen(name) >= KN_EXTERNAL_MODEM_NAME_MAX)
		return 0;

	for (i = 0; name[i] != '\0'; i++) {
		ch = (unsigned char)name[i];
		if (isalnum(ch) != 0 || ch == '-' || ch == '_')
			continue;
		return 0;
	}

	return 1;
}

const char *
kn_external_modem_mode_name(enum kn_external_modem_mode mode)
{
	switch (mode) {
	case KN_EXTERNAL_MODEM_MODE_DISABLED:
		return "disabled";
	case KN_EXTERNAL_MODEM_MODE_STATUS_ONLY:
		return "status-only";
	case KN_EXTERNAL_MODEM_MODE_RECEIVE_ONLY:
		return "receive-only";
	case KN_EXTERNAL_MODEM_MODE_PLANNED:
		return "planned";
	case KN_EXTERNAL_MODEM_MODE_NONE:
	default:
		break;
	}

	return "unknown";
}

enum kn_external_modem_mode
kn_external_modem_mode_parse(const char *input)
{
	if (input == NULL)
		return KN_EXTERNAL_MODEM_MODE_NONE;
	if (strcmp(input, "disabled") == 0)
		return KN_EXTERNAL_MODEM_MODE_DISABLED;
	if (strcmp(input, "status-only") == 0)
		return KN_EXTERNAL_MODEM_MODE_STATUS_ONLY;
	if (strcmp(input, "receive-only") == 0)
		return KN_EXTERNAL_MODEM_MODE_RECEIVE_ONLY;
	if (strcmp(input, "planned") == 0)
		return KN_EXTERNAL_MODEM_MODE_PLANNED;

	return KN_EXTERNAL_MODEM_MODE_NONE;
}

const char *
kn_external_modem_state_name(enum kn_external_modem_state state)
{
	switch (state) {
	case KN_EXTERNAL_MODEM_STATE_DISABLED:
		return "disabled";
	case KN_EXTERNAL_MODEM_STATE_CONFIGURED:
		return "configured";
	case KN_EXTERNAL_MODEM_STATE_PLANNED:
		return "planned";
	case KN_EXTERNAL_MODEM_STATE_UNAVAILABLE:
		return "unavailable";
	case KN_EXTERNAL_MODEM_STATE_UNKNOWN:
		return "unknown";
	default:
		break;
	}

	return "unknown";
}

const char *
kn_external_modem_type_name(enum kn_external_modem_type type)
{
	switch (type) {
	case KN_EXTERNAL_MODEM_TYPE_KISS_TCP:
		return "kiss-tcp";
	case KN_EXTERNAL_MODEM_TYPE_MERCURY_OFDM:
		return "mercury-ofdm";
	case KN_EXTERNAL_MODEM_TYPE_VARA_HF:
		return "vara-hf";
	case KN_EXTERNAL_MODEM_TYPE_VARA_FM:
		return "vara-fm";
	case KN_EXTERNAL_MODEM_TYPE_ARDOP:
		return "ardop";
	case KN_EXTERNAL_MODEM_TYPE_GENERIC_TCP:
		return "generic-tcp";
	case KN_EXTERNAL_MODEM_TYPE_NONE:
	default:
		break;
	}

	return "unknown";
}

enum kn_external_modem_type
kn_external_modem_type_parse(const char *input)
{
	if (input == NULL)
		return KN_EXTERNAL_MODEM_TYPE_NONE;
	if (strcmp(input, "kiss-tcp") == 0)
		return KN_EXTERNAL_MODEM_TYPE_KISS_TCP;
	if (strcmp(input, "mercury-ofdm") == 0)
		return KN_EXTERNAL_MODEM_TYPE_MERCURY_OFDM;
	if (strcmp(input, "vara-hf") == 0)
		return KN_EXTERNAL_MODEM_TYPE_VARA_HF;
	if (strcmp(input, "vara-fm") == 0)
		return KN_EXTERNAL_MODEM_TYPE_VARA_FM;
	if (strcmp(input, "ardop") == 0)
		return KN_EXTERNAL_MODEM_TYPE_ARDOP;
	if (strcmp(input, "generic-tcp") == 0)
		return KN_EXTERNAL_MODEM_TYPE_GENERIC_TCP;

	return KN_EXTERNAL_MODEM_TYPE_NONE;
}
