/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/rf/rf_command.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "kilonode/ax25.h"
#include "kilonode/callsign.h"
#include "kilonode/node_command.h"
#include "kilonode/node_command_profile.h"
#include "kilonode/rf_command.h"

static uint8_t destination_allowed(const struct kn_config *,
	const struct kn_rx_event *);
static uint8_t destination_text_allowed(const struct kn_config *,
	const char *);
static const struct kn_port_stats *port_find(const struct kn_port_stats *,
	size_t, const char *);
static enum kn_rf_command_error raw_copy(const uint8_t *, size_t, char *,
	size_t);
static void set_error_text(struct kn_rf_command_event *, const char *);

void
kn_rf_command_event_clear(struct kn_rf_command_event *event)
{
	if (event == NULL)
		return;

	memset(event, 0, sizeof(*event));
	event->command = KN_RF_COMMAND_UNKNOWN;
	event->status = KN_RF_COMMAND_STATUS_IGNORED;
}

enum kn_rf_command_error
kn_rf_command_from_rx(struct kn_rf_command_event *event, uint64_t id,
	uint64_t now, const struct kn_config *config,
	const struct kn_port_stats *ports, size_t port_count,
	const struct kn_rx_event *rx, const uint8_t *payload, size_t payload_len)
{
	const struct kn_port_stats *port;

	if (event == NULL || config == NULL || rx == NULL)
		return KN_RF_COMMAND_ERR_INVALID_ARGUMENT;
	if (payload == NULL && payload_len > 0)
		return KN_RF_COMMAND_ERR_INVALID_ARGUMENT;

	kn_rf_command_event_clear(event);
	event->id = id;
	event->timestamp = now;
	event->rx_event_id = rx->id;
	event->source = rx->source;
	event->destination = rx->destination;
	(void)snprintf(event->port_name, sizeof(event->port_name), "%s",
	    rx->port_name);

	if (config->rf_command.enabled == 0) {
		event->status = KN_RF_COMMAND_STATUS_DISABLED;
		return KN_RF_COMMAND_ERR_IGNORED;
	}

	port = port_find(ports, port_count, rx->port_name);
	if (port == NULL || port->enabled == 0 || port->open == 0) {
		event->status = KN_RF_COMMAND_STATUS_INVALID_PORT;
		return KN_RF_COMMAND_ERR_IGNORED;
	}

	if (rx->malformed != 0 || rx->kind != KN_RX_FRAME_UI) {
		event->status = KN_RF_COMMAND_STATUS_NOT_UI;
		return KN_RF_COMMAND_ERR_IGNORED;
	}

	if (rx->has_pid == 0 || rx->pid != KN_AX25_PID_NO_LAYER_3) {
		event->status = KN_RF_COMMAND_STATUS_BAD_PID;
		return KN_RF_COMMAND_ERR_IGNORED;
	}

	if (payload_len > config->rf_command.max_command_bytes) {
		event->status = KN_RF_COMMAND_STATUS_OVERLONG;
		set_error_text(event, "command-too-large");
		return KN_RF_COMMAND_OK;
	}

	if (destination_allowed(config, rx) == 0) {
		event->status = KN_RF_COMMAND_STATUS_DESTINATION;
		return KN_RF_COMMAND_ERR_IGNORED;
	}

	if (kn_callsign_format(&rx->source, event->error,
	    sizeof(event->error)) != 0) {
		event->status = KN_RF_COMMAND_STATUS_INVALID_SOURCE;
		set_error_text(event, "invalid-source");
		return KN_RF_COMMAND_OK;
	}
	event->error[0] = '\0';

	return kn_rf_command_parse(payload, payload_len,
	    config->rf_command.max_command_bytes, &event->command, event->raw,
	    sizeof(event->raw), &event->status);
}

enum kn_rf_command_error
kn_rf_command_format_brief(const struct kn_rf_command_event *event, char *buf,
	size_t bufsiz)
{
	char source[KN_CALLSIGN_MAX + 4];
	char destination[KN_CALLSIGN_MAX + 4];
	int needed;

	if (event == NULL || buf == NULL || bufsiz == 0)
		return KN_RF_COMMAND_ERR_INVALID_ARGUMENT;
	if (kn_callsign_format(&event->source, source, sizeof(source)) != 0 ||
	    kn_callsign_format(&event->destination, destination,
	    sizeof(destination)) != 0)
		return KN_RF_COMMAND_ERR_INVALID_ARGUMENT;

	if (event->reply_queued != 0)
		needed = snprintf(buf, bufsiz,
		    "RF COMMAND id=%llu rx=%llu port=%s from=%s to=%s "
		    "command=%s status=%s reply_queued=true tx=%llu",
		    (unsigned long long)event->id,
		    (unsigned long long)event->rx_event_id,
		    event->port_name, source, destination,
		    kn_rf_command_name_string(event->command),
		    kn_rf_command_status_string(event->status),
		    (unsigned long long)event->tx_frame_id);
	else
		needed = snprintf(buf, bufsiz,
		    "RF COMMAND id=%llu rx=%llu port=%s from=%s to=%s "
		    "command=%s status=%s reply_queued=false",
		    (unsigned long long)event->id,
		    (unsigned long long)event->rx_event_id,
		    event->port_name, source, destination,
		    kn_rf_command_name_string(event->command),
		    kn_rf_command_status_string(event->status));

	return needed >= 0 && (size_t)needed < bufsiz ?
	    KN_RF_COMMAND_OK : KN_RF_COMMAND_ERR_BUFFER;
}

enum kn_rf_command_error
kn_rf_command_format_full(const struct kn_rf_command_event *event, char *buf,
	size_t bufsiz)
{
	char source[KN_CALLSIGN_MAX + 4];
	char destination[KN_CALLSIGN_MAX + 4];
	int needed;

	if (event == NULL || buf == NULL || bufsiz == 0)
		return KN_RF_COMMAND_ERR_INVALID_ARGUMENT;
	if (kn_callsign_format(&event->source, source, sizeof(source)) != 0 ||
	    kn_callsign_format(&event->destination, destination,
	    sizeof(destination)) != 0)
		return KN_RF_COMMAND_ERR_INVALID_ARGUMENT;

	if (event->reply_queued != 0)
		needed = snprintf(buf, bufsiz,
		    "RF COMMAND id=%llu time=%llu rx=%llu port=%s from=%s "
		    "to=%s command=%s status=%s raw=\"%s\" "
		    "reply_queued=true tx=%llu",
		    (unsigned long long)event->id,
		    (unsigned long long)event->timestamp,
		    (unsigned long long)event->rx_event_id,
		    event->port_name, source, destination,
		    kn_rf_command_name_string(event->command),
		    kn_rf_command_status_string(event->status),
		    event->raw[0] == '\0' ? "-" : event->raw,
		    (unsigned long long)event->tx_frame_id);
	else
		needed = snprintf(buf, bufsiz,
		    "RF COMMAND id=%llu time=%llu rx=%llu port=%s from=%s "
		    "to=%s command=%s status=%s raw=\"%s\" "
		    "reply_queued=false error=%s",
		    (unsigned long long)event->id,
		    (unsigned long long)event->timestamp,
		    (unsigned long long)event->rx_event_id,
		    event->port_name, source, destination,
		    kn_rf_command_name_string(event->command),
		    kn_rf_command_status_string(event->status),
		    event->raw[0] == '\0' ? "-" : event->raw,
		    event->error[0] == '\0' ? "-" : event->error);

	return needed >= 0 && (size_t)needed < bufsiz ?
	    KN_RF_COMMAND_OK : KN_RF_COMMAND_ERR_BUFFER;
}

const char *
kn_rf_command_name_string(enum kn_rf_command_name command)
{
	switch (command) {
	case KN_RF_COMMAND_HELP:
		return "HELP";
	case KN_RF_COMMAND_INFO:
		return "INFO";
	case KN_RF_COMMAND_PORTS:
		return "PORTS";
	case KN_RF_COMMAND_HEARD:
		return "HEARD";
	case KN_RF_COMMAND_STATS:
		return "STATS";
	case KN_RF_COMMAND_PING:
		return "PING";
	case KN_RF_COMMAND_UNKNOWN:
		return "UNKNOWN";
	}

	return "UNKNOWN";
}

enum kn_rf_command_error
kn_rf_command_parse(const uint8_t *payload, size_t payload_len,
	size_t max_command_bytes, enum kn_rf_command_name *command, char *raw,
	size_t raw_len, enum kn_rf_command_status *status)
{
	struct kn_node_command_input parsed;
	enum kn_node_command_error parse_rc;
	enum kn_node_command_id command_id;
	int needed;

	if (payload == NULL && payload_len > 0)
		return KN_RF_COMMAND_ERR_INVALID_ARGUMENT;
	if (command == NULL || raw == NULL || raw_len == 0 || status == NULL)
		return KN_RF_COMMAND_ERR_INVALID_ARGUMENT;
	if (max_command_bytes == 0 ||
	    max_command_bytes > KN_CONFIG_RF_COMMAND_BYTES_MAX)
		return KN_RF_COMMAND_ERR_INVALID_ARGUMENT;

	*command = KN_RF_COMMAND_UNKNOWN;
	*status = KN_RF_COMMAND_STATUS_IGNORED;
	raw[0] = '\0';

	parse_rc = kn_node_command_parse(payload, payload_len,
	    max_command_bytes, &parsed);
	if (parse_rc == KN_NODE_COMMAND_ERR_OVERLONG) {
		*status = KN_RF_COMMAND_STATUS_OVERLONG;
		return KN_RF_COMMAND_OK;
	}
	if (parse_rc == KN_NODE_COMMAND_ERR_EMPTY) {
		*status = KN_RF_COMMAND_STATUS_EMPTY;
		return KN_RF_COMMAND_OK;
	}
	if (parse_rc == KN_NODE_COMMAND_ERR_CONTROL) {
		*status = KN_RF_COMMAND_STATUS_BINARY;
		return raw_copy(payload, payload_len, raw, raw_len);
	}
	if (parse_rc != KN_NODE_COMMAND_OK)
		return KN_RF_COMMAND_ERR_BUFFER;
	needed = snprintf(raw, raw_len, "%s", parsed.preview);
	if (needed < 0 || (size_t)needed >= raw_len)
		return KN_RF_COMMAND_ERR_BUFFER;
	if (parsed.args_len != 0) {
		*status = KN_RF_COMMAND_STATUS_ARGUMENTS;
		return KN_RF_COMMAND_OK;
	}

	command_id = kn_node_command_id_from_name(parsed.command);
	if (command_id == KN_NODE_COMMAND_ID_HELP)
		*command = KN_RF_COMMAND_HELP;
	else if (command_id == KN_NODE_COMMAND_ID_INFO)
		*command = KN_RF_COMMAND_INFO;
	else if (command_id == KN_NODE_COMMAND_ID_PORTS)
		*command = KN_RF_COMMAND_PORTS;
	else if (command_id == KN_NODE_COMMAND_ID_HEARD)
		*command = KN_RF_COMMAND_HEARD;
	else if (command_id == KN_NODE_COMMAND_ID_STATS)
		*command = KN_RF_COMMAND_STATS;
	else if (command_id == KN_NODE_COMMAND_ID_PING)
		*command = KN_RF_COMMAND_PING;
	else {
		*status = KN_RF_COMMAND_STATUS_UNKNOWN;
		return KN_RF_COMMAND_OK;
	}

	*status = KN_RF_COMMAND_STATUS_OK;
	return KN_RF_COMMAND_OK;
}

const char *
kn_rf_command_status_string(enum kn_rf_command_status status)
{
	switch (status) {
	case KN_RF_COMMAND_STATUS_OK:
		return "ok";
	case KN_RF_COMMAND_STATUS_DISABLED:
		return "disabled";
	case KN_RF_COMMAND_STATUS_IGNORED:
		return "ignored";
	case KN_RF_COMMAND_STATUS_NOT_UI:
		return "not-ui";
	case KN_RF_COMMAND_STATUS_BAD_PID:
		return "bad-pid";
	case KN_RF_COMMAND_STATUS_OVERLONG:
		return "overlong";
	case KN_RF_COMMAND_STATUS_EMPTY:
		return "empty";
	case KN_RF_COMMAND_STATUS_UNKNOWN:
		return "unknown-command";
	case KN_RF_COMMAND_STATUS_ARGUMENTS:
		return "arguments";
	case KN_RF_COMMAND_STATUS_CONTROL:
		return "control";
	case KN_RF_COMMAND_STATUS_BINARY:
		return "binary";
	case KN_RF_COMMAND_STATUS_DESTINATION:
		return "destination";
	case KN_RF_COMMAND_STATUS_INVALID_SOURCE:
		return "invalid-source";
	case KN_RF_COMMAND_STATUS_INVALID_PORT:
		return "invalid-port";
	case KN_RF_COMMAND_STATUS_RATE_LIMITED:
		return "rate-limited";
	case KN_RF_COMMAND_STATUS_REPLY_SUPPRESSED:
		return "reply-suppressed";
	case KN_RF_COMMAND_STATUS_TX_GATE_BLOCKED:
		return "tx-gate-blocked";
	case KN_RF_COMMAND_STATUS_MALFORMED:
		return "malformed";
	}

	return "unknown";
}

static uint8_t
destination_allowed(const struct kn_config *config, const struct kn_rx_event *rx)
{
	char destination[KN_CALLSIGN_MAX + 4];
	char node[KN_CALLSIGN_MAX + 4];

	if (kn_callsign_format(&rx->destination, destination,
	    sizeof(destination)) != 0)
		return 0;
	if (config->rf_command.require_node_destination == 0)
		return destination_text_allowed(config, destination) != 0 ? 1 : 1;
	if (kn_callsign_format(&config->node.callsign, node,
	    sizeof(node)) == 0 && strcmp(destination, node) == 0)
		return 1;
	if (config->node.alias[0] != '\0' &&
	    strcmp(destination, config->node.alias) == 0)
		return 1;
	return destination_text_allowed(config, destination);
}

static uint8_t
destination_text_allowed(const struct kn_config *config, const char *destination)
{
	size_t i;

	for (i = 0; i < config->rf_command.accept_destination_count; i++) {
		if (strcmp(destination,
		    config->rf_command.accept_destinations[i]) == 0)
			return 1;
	}

	return 0;
}

static const struct kn_port_stats *
port_find(const struct kn_port_stats *ports, size_t port_count,
	const char *name)
{
	size_t i;

	if (ports == NULL || name == NULL)
		return NULL;

	for (i = 0; i < port_count; i++) {
		if (strcmp(ports[i].name, name) == 0)
			return &ports[i];
	}

	return NULL;
}

static enum kn_rf_command_error
raw_copy(const uint8_t *payload, size_t payload_len, char *raw, size_t raw_len)
{
	size_t i;

	if (payload_len >= raw_len)
		return KN_RF_COMMAND_ERR_BUFFER;

	for (i = 0; i < payload_len; i++)
		raw[i] = payload[i] >= 0x20 && payload[i] <= 0x7e ?
		    (char)payload[i] : '?';
	raw[payload_len] = '\0';
	return KN_RF_COMMAND_OK;
}

static void
set_error_text(struct kn_rf_command_event *event, const char *text)
{
	int needed;

	needed = snprintf(event->error, sizeof(event->error), "%s",
	    text == NULL ? "" : text);
	if (needed < 0 || (size_t)needed >= sizeof(event->error))
		event->error[0] = '\0';
}
