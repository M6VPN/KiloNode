/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_rf_reply.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "kilonode/rf_reply.h"

static void config_base(struct kn_config *);
static void event_base(struct kn_rf_command_event *);
static void port_base(struct kn_port_stats *);
static int test_format(void);
static int test_reply_blocked(void);
static int test_reply_disabled(void);
static int test_reply_queues(void);

int
main(void)
{
	if (test_format() != 0)
		return 1;
	if (test_reply_disabled() != 0)
		return 1;
	if (test_reply_blocked() != 0)
		return 1;
	if (test_reply_queues() != 0)
		return 1;

	return 0;
}

static void
config_base(struct kn_config *config)
{
	kn_config_init(config);
	(void)kn_callsign_parse("M6VPN-1", &config->node.callsign);
	config->node.has_callsign = 1;
	config->rf_command.reply_enabled = 1;
	config->port_count = 1;
	(void)snprintf(config->ports[0].name, sizeof(config->ports[0].name),
	    "kiss0");
	config->ports[0].type = KN_CONFIG_PORT_MEMORY_TEST;
	config->ports[0].enabled = 1;
	config->ports[0].tx_enabled = 1;
}

static void
event_base(struct kn_rf_command_event *event)
{
	kn_rf_command_event_clear(event);
	event->id = 1;
	event->timestamp = 10;
	event->rx_event_id = 2;
	(void)snprintf(event->port_name, sizeof(event->port_name), "kiss0");
	(void)kn_callsign_parse("N0CALL", &event->source);
	(void)kn_callsign_parse("M6VPN-1", &event->destination);
	event->command = KN_RF_COMMAND_PING;
	event->status = KN_RF_COMMAND_STATUS_OK;
	(void)snprintf(event->raw, sizeof(event->raw), "PING");
}

static void
port_base(struct kn_port_stats *port)
{
	memset(port, 0, sizeof(*port));
	(void)snprintf(port->name, sizeof(port->name), "kiss0");
	port->type = KN_CONFIG_PORT_MEMORY_TEST;
	port->enabled = 1;
	port->open = 1;
	port->tx_enabled = 1;
}

static int
test_format(void)
{
	struct kn_config config;
	struct kn_daemon_stats stats;
	struct kn_rf_command_event event;
	struct kn_port_stats port;
	char reply[64];

	config_base(&config);
	kn_daemon_stats_init(&stats, 1, 1);
	event_base(&event);
	port_base(&port);
	if (kn_rf_reply_format(&event, &config, &stats, &port, 1, NULL, 0,
	    reply, sizeof(reply)) != KN_RF_REPLY_OK)
		return 1;

	return strcmp(reply, "PONG") == 0 ? 0 : 1;
}

static int
test_reply_blocked(void)
{
	struct kn_config config;
	struct kn_daemon_stats stats;
	struct kn_rf_command_event event;
	struct kn_port_stats port;
	struct kn_tx_policy policy;
	struct kn_tx_queue queue;
	char reason[KN_RF_REPLY_REASON_MAX];
	uint64_t id;

	config_base(&config);
	kn_daemon_stats_init(&stats, 1, 1);
	event_base(&event);
	port_base(&port);
	kn_tx_policy_defaults(&policy);
	(void)kn_tx_queue_init(&queue, &policy);

	if (kn_rf_reply_try_queue(&queue, &config, &stats, &port, 1, NULL, 0,
	    &event, 10, &id, reason, sizeof(reason)) !=
	    KN_RF_REPLY_ERR_TX_BLOCKED)
		return 1;

	return strcmp(reason, "tx-ui-blocked") == 0 ? 0 : 1;
}

static int
test_reply_disabled(void)
{
	struct kn_config config;
	struct kn_daemon_stats stats;
	struct kn_rf_command_event event;
	struct kn_port_stats port;
	struct kn_tx_policy policy;
	struct kn_tx_queue queue;
	char reason[KN_RF_REPLY_REASON_MAX];
	uint64_t id;

	config_base(&config);
	config.rf_command.reply_enabled = 0;
	kn_daemon_stats_init(&stats, 1, 1);
	event_base(&event);
	port_base(&port);
	kn_tx_policy_defaults(&policy);
	policy.enabled = 1;
	policy.allow_ui = 1;
	(void)kn_tx_queue_init(&queue, &policy);

	if (kn_rf_reply_try_queue(&queue, &config, &stats, &port, 1, NULL, 0,
	    &event, 10, &id, reason, sizeof(reason)) !=
	    KN_RF_REPLY_ERR_DISABLED)
		return 1;

	return strcmp(reason, "reply-disabled") == 0 ? 0 : 1;
}

static int
test_reply_queues(void)
{
	struct kn_config config;
	struct kn_daemon_stats stats;
	struct kn_rf_command_event event;
	struct kn_port_stats port;
	struct kn_tx_policy policy;
	struct kn_tx_queue queue;
	uint64_t id;

	config_base(&config);
	kn_daemon_stats_init(&stats, 1, 1);
	event_base(&event);
	port_base(&port);
	kn_tx_policy_defaults(&policy);
	policy.enabled = 1;
	policy.dry_run = 1;
	policy.allow_ui = 1;
	(void)kn_tx_queue_init(&queue, &policy);

	if (kn_rf_reply_try_queue(&queue, &config, &stats, &port, 1, NULL, 0,
	    &event, 10, &id, NULL, 0) != KN_RF_REPLY_OK)
		return 1;
	if (id == 0 || kn_tx_queue_count(&queue) != 1)
		return 1;

	return kn_tx_queue_get(&queue, id)->status == KN_TX_FRAME_DRY_RUN ?
	    0 : 1;
}
