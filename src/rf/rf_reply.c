/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/rf/rf_reply.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "kilonode/ax25.h"
#include "kilonode/callsign.h"
#include "kilonode/node_command_context.h"
#include "kilonode/node_command_dispatch.h"
#include "kilonode/rf_reply.h"
#include "kilonode/tx_frame.h"
#include "kilonode/tx_policy.h"
#include "kilonode/tx_transport_gate.h"

static const struct kn_config_port *config_port_find(const struct kn_config *,
	const char *);
static enum kn_rf_reply_error gate_reply(const struct kn_tx_queue *,
	const struct kn_config *, const struct kn_port_stats *,
	const struct kn_rf_command_event *, size_t, char *, size_t);
static const struct kn_port_stats *stats_port_find(const struct kn_port_stats *,
	size_t, const char *);
static void reason_set(char *, size_t, const char *);

enum kn_rf_reply_error
kn_rf_reply_format(const struct kn_rf_command_event *event,
	const struct kn_config *config, const struct kn_daemon_stats *stats,
	const struct kn_port_stats *ports, size_t port_count,
	const struct kn_heard_entry *heard, size_t heard_count, char *buf,
	size_t bufsiz)
{
	struct kn_node_command_context context;
	struct kn_node_command_dispatch_result result;
	const char *input;
	size_t input_len;
	enum kn_node_command_dispatch_status rc;

	if (event == NULL || config == NULL || buf == NULL || bufsiz == 0)
		return KN_RF_REPLY_ERR_INVALID_ARGUMENT;

	kn_node_command_context_clear(&context);
	context.node = &config->node;
	context.daemon = stats;
	context.ports = ports;
	context.port_count = port_count;
	context.heard = heard;
	context.heard_count = heard_count;
	context.output_limit = bufsiz;
	input = event->raw[0] != '\0' ? event->raw :
	    kn_rf_command_name_string(event->command);
	input_len = strlen(input);
	rc = kn_node_command_dispatch(KN_NODE_COMMAND_CONTEXT_RF_UI, &context,
	    (const uint8_t *)input, input_len,
	    config->rf_command.max_command_bytes, &result);
	if (rc == KN_NODE_COMMAND_DISPATCH_INTERNAL_ERROR)
		return KN_RF_REPLY_ERR_BUFFER;
	if (result.output_len >= bufsiz)
		return KN_RF_REPLY_ERR_BUFFER;
	memcpy(buf, result.output, result.output_len + 1);
	return KN_RF_REPLY_OK;
}

enum kn_rf_reply_error
kn_rf_reply_try_queue(struct kn_tx_queue *tx_queue,
	const struct kn_config *config, const struct kn_daemon_stats *stats,
	const struct kn_port_stats *ports, size_t port_count,
	const struct kn_heard_entry *heard, size_t heard_count,
	const struct kn_rf_command_event *event, uint64_t now,
	uint64_t *tx_frame_id, char *reason, size_t reason_len)
{
	struct kn_tx_frame frame;
	const struct kn_port_stats *port;
	char reply[KN_CONFIG_RF_REPLY_BYTES_MAX + 1];
	char source[KN_CALLSIGN_MAX + 4];
	char destination[KN_CALLSIGN_MAX + 4];
	uint64_t id;
	enum kn_rf_reply_error rc;

	if (tx_frame_id != NULL)
		*tx_frame_id = 0;
	if (reason != NULL && reason_len > 0)
		reason[0] = '\0';
	if (tx_queue == NULL || config == NULL || event == NULL)
		return KN_RF_REPLY_ERR_INVALID_ARGUMENT;
	if (config->rf_command.reply_enabled == 0) {
		reason_set(reason, reason_len, "reply-disabled");
		return KN_RF_REPLY_ERR_DISABLED;
	}
	if (event->status != KN_RF_COMMAND_STATUS_OK &&
	    event->status != KN_RF_COMMAND_STATUS_UNKNOWN) {
		reason_set(reason, reason_len, "command-not-replyable");
		return KN_RF_REPLY_ERR_DISABLED;
	}

	rc = kn_rf_reply_format(event, config, stats, ports, port_count, heard,
	    heard_count, reply, sizeof(reply));
	if (rc != KN_RF_REPLY_OK) {
		reason_set(reason, reason_len, "reply-format-failed");
		return rc;
	}
	if (strlen(reply) > config->rf_command.max_reply_bytes) {
		reply[config->rf_command.max_reply_bytes] = '\0';
	}

	port = stats_port_find(ports, port_count, event->port_name);
	rc = gate_reply(tx_queue, config, port, event, strlen(reply), reason,
	    reason_len);
	if (rc != KN_RF_REPLY_OK)
		return rc;

	if (kn_callsign_format(&config->node.callsign, source,
	    sizeof(source)) != 0 ||
	    kn_callsign_format(&event->source, destination,
	    sizeof(destination)) != 0) {
		reason_set(reason, reason_len, "invalid-callsign");
		return KN_RF_REPLY_ERR_TX_BLOCKED;
	}

	id = kn_tx_queue_reserve_id(tx_queue);
	if (kn_tx_frame_build_ui(&frame, id, now, event->port_name, 0, source,
	    destination, NULL, 0, KN_AX25_PID_NO_LAYER_3,
	    (const uint8_t *)reply, strlen(reply), &tx_queue->policy) !=
	    KN_TX_FRAME_OK) {
		reason_set(reason, reason_len, "tx-frame-failed");
		return KN_RF_REPLY_ERR_TX_BLOCKED;
	}
	if (kn_tx_queue_enqueue(tx_queue, &frame) != KN_TX_QUEUE_OK) {
		reason_set(reason, reason_len, "tx-queue-blocked");
		return KN_RF_REPLY_ERR_TX_BLOCKED;
	}

	if (tx_frame_id != NULL)
		*tx_frame_id = id;
	return KN_RF_REPLY_OK;
}

static const struct kn_config_port *
config_port_find(const struct kn_config *config, const char *name)
{
	size_t i;

	if (config == NULL || name == NULL)
		return NULL;

	for (i = 0; i < config->port_count; i++) {
		if (strcmp(config->ports[i].name, name) == 0)
			return &config->ports[i];
	}

	return NULL;
}

static enum kn_rf_reply_error
gate_reply(const struct kn_tx_queue *tx_queue, const struct kn_config *config,
	const struct kn_port_stats *port,
	const struct kn_rf_command_event *event, size_t payload_len,
	char *reason, size_t reason_len)
{
	struct kn_tx_transport_gate_port gate_port;
	const struct kn_config_port *config_port;
	enum kn_tx_transport_gate_error gate_rc;

	if (kn_tx_policy_allow_ui(&tx_queue->policy, payload_len) !=
	    KN_TX_POLICY_OK) {
		reason_set(reason, reason_len, "tx-ui-blocked");
		return KN_RF_REPLY_ERR_TX_BLOCKED;
	}
	if (port == NULL || port->enabled == 0 || port->open == 0) {
		reason_set(reason, reason_len, "port-not-open");
		return KN_RF_REPLY_ERR_TX_BLOCKED;
	}
	if (tx_queue->policy.dry_run != 0)
		return KN_RF_REPLY_OK;

	config_port = config_port_find(config, event->port_name);
	if (config_port == NULL) {
		reason_set(reason, reason_len, "port-missing");
		return KN_RF_REPLY_ERR_TX_BLOCKED;
	}

	memset(&gate_port, 0, sizeof(gate_port));
	(void)snprintf(gate_port.name, sizeof(gate_port.name), "%s",
	    event->port_name);
	gate_port.config_type = config_port->type;
	gate_port.transport_kind = kn_config_port_transport_kind(config_port);
	gate_port.enabled = port->enabled;
	gate_port.open = port->open;
	gate_port.tx_enabled = port->tx_enabled;
	gate_port.writable = 1;

	gate_rc = kn_tx_transport_gate_real(&tx_queue->policy, &gate_port);
	if (gate_rc != KN_TX_TRANSPORT_GATE_OK) {
		reason_set(reason, reason_len,
		    kn_tx_transport_gate_error_name(gate_rc));
		return KN_RF_REPLY_ERR_TX_BLOCKED;
	}

	return KN_RF_REPLY_OK;
}

static const struct kn_port_stats *
stats_port_find(const struct kn_port_stats *ports, size_t port_count,
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

static void
reason_set(char *reason, size_t reason_len, const char *text)
{
	int needed;

	if (reason == NULL || reason_len == 0)
		return;

	needed = snprintf(reason, reason_len, "%s", text == NULL ? "" : text);
	if (needed < 0 || (size_t)needed >= reason_len)
		reason[0] = '\0';
}
