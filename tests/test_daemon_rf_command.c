/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_daemon_rf_command.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "kilonode/rf_abuse.h"
#include "kilonode/rf_command.h"
#include "kilonode/rf_command_queue.h"
#include "kilonode/rf_ignore.h"
#include "kilonode/rf_reply.h"

static void setup_config(struct kn_config *);
static void setup_rx(struct kn_rx_event *);
static int process_command(struct kn_config *, struct kn_tx_queue *,
	struct kn_rf_abuse_state *, const struct kn_rf_ignore_list *,
	struct kn_rf_command_event *, const uint8_t *, size_t, uint64_t);
static int test_ping_event_and_blocked_reply(void);
static int test_ping_event_and_queued_reply(void);
static int test_rate_limited_no_reply(void);
static int test_reply_limited_no_tx(void);

int
main(void)
{
	if (test_ping_event_and_blocked_reply() != 0)
		return 1;
	if (test_ping_event_and_queued_reply() != 0)
		return 1;
	if (test_rate_limited_no_reply() != 0)
		return 1;
	if (test_reply_limited_no_tx() != 0)
		return 1;

	return 0;
}

static void
setup_config(struct kn_config *config)
{
	kn_config_init(config);
	(void)kn_callsign_parse("M6VPN-1", &config->node.callsign);
	config->node.has_callsign = 1;
	config->rf_command.enabled = 1;
	config->rf_command.reply_enabled = 1;
	config->port_count = 1;
	(void)snprintf(config->ports[0].name, sizeof(config->ports[0].name),
	    "kiss0");
	config->ports[0].type = KN_CONFIG_PORT_MEMORY_TEST;
	config->ports[0].enabled = 1;
	config->ports[0].tx_enabled = 1;
}

static int
process_command(struct kn_config *config, struct kn_tx_queue *queue,
	struct kn_rf_abuse_state *abuse, const struct kn_rf_ignore_list *ignore,
	struct kn_rf_command_event *event, const uint8_t *payload,
	size_t payload_len, uint64_t now)
{
	struct kn_rx_event rx;
	struct kn_port_stats port;
	struct kn_daemon_stats stats;
	char reason[KN_RF_REPLY_REASON_MAX];
	uint64_t id;
	enum kn_rf_abuse_error abuse_rc;

	setup_rx(&rx);
	memset(&port, 0, sizeof(port));
	(void)snprintf(port.name, sizeof(port.name), "kiss0");
	port.enabled = 1;
	port.open = 1;
	port.tx_enabled = 1;
	kn_daemon_stats_init(&stats, 1, 1);

	if (kn_rf_command_from_rx(event, 1, now, config, &port, 1, &rx,
	    payload, payload_len) != KN_RF_COMMAND_OK)
		return 1;

	reason[0] = '\0';
	abuse_rc = kn_rf_abuse_check_command(abuse, &config->rf_command,
	    ignore, &event->source, now, reason, sizeof(reason));
	if (abuse_rc == KN_RF_ABUSE_ERR_IGNORED ||
	    abuse_rc == KN_RF_ABUSE_ERR_RATE_LIMITED) {
		event->status = abuse_rc == KN_RF_ABUSE_ERR_IGNORED ?
		    KN_RF_COMMAND_STATUS_IGNORED :
		    KN_RF_COMMAND_STATUS_RATE_LIMITED;
		(void)snprintf(event->error, sizeof(event->error), "%s",
		    reason);
		(void)kn_rf_abuse_record_rejected(abuse, &config->rf_command,
		    &event->source, now, event->error);
		return 0;
	}

	if (event->status == KN_RF_COMMAND_STATUS_OK)
		(void)kn_rf_abuse_record_accepted(abuse, &event->source, now);
	else
		(void)kn_rf_abuse_record_rejected(abuse, &config->rf_command,
		    &event->source, now,
		    kn_rf_command_status_string(event->status));

	if (config->rf_command.reply_enabled == 0)
		return 0;
	reason[0] = '\0';
	if (kn_rf_abuse_check_reply(abuse, &config->rf_command,
	    &event->source, now, reason, sizeof(reason)) ==
	    KN_RF_ABUSE_ERR_REPLY_RATE_LIMITED) {
		event->status = KN_RF_COMMAND_STATUS_REPLY_SUPPRESSED;
		(void)snprintf(event->error, sizeof(event->error), "%s",
		    reason);
		return 0;
	}
	id = 0;
	if (kn_rf_reply_try_queue(queue, config, &stats, &port, 1, NULL, 0,
	    event, now, &id, reason, sizeof(reason)) == KN_RF_REPLY_OK) {
		event->reply_queued = 1;
		event->tx_frame_id = id;
		(void)kn_rf_abuse_record_reply(abuse, &event->source, now);
	} else if (reason[0] != '\0') {
		event->status = KN_RF_COMMAND_STATUS_TX_GATE_BLOCKED;
		(void)snprintf(event->error, sizeof(event->error), "%s",
		    reason);
	}

	return 0;
}

static void
setup_rx(struct kn_rx_event *rx)
{
	kn_rx_event_clear(rx);
	rx->id = 1;
	(void)snprintf(rx->port_name, sizeof(rx->port_name), "kiss0");
	(void)kn_callsign_parse("N0CALL", &rx->source);
	(void)kn_callsign_parse("M6VPN-1", &rx->destination);
	rx->kind = KN_RX_FRAME_UI;
	rx->has_pid = 1;
	rx->pid = KN_AX25_PID_NO_LAYER_3;
}

static int
test_ping_event_and_blocked_reply(void)
{
	const uint8_t payload[] = "PING";
	struct kn_config config;
	struct kn_rx_event rx;
	struct kn_port_stats port;
	struct kn_daemon_stats stats;
	struct kn_rf_command_event event;
	struct kn_tx_policy policy;
	struct kn_tx_queue queue;
	char reason[KN_RF_REPLY_REASON_MAX];
	uint64_t id;

	setup_config(&config);
	memset(&port, 0, sizeof(port));
	(void)snprintf(port.name, sizeof(port.name), "kiss0");
	port.enabled = 1;
	port.open = 1;
	port.tx_enabled = 1;
	kn_daemon_stats_init(&stats, 1, 1);
	kn_tx_policy_defaults(&policy);
	(void)kn_tx_queue_init(&queue, &policy);

	setup_rx(&rx);

	if (kn_rf_command_from_rx(&event, 1, 10, &config, &port, 1, &rx,
	    payload, 4) != KN_RF_COMMAND_OK)
		return 1;
	if (kn_rf_reply_try_queue(&queue, &config, &stats, &port, 1, NULL, 0,
	    &event, 10, &id, reason, sizeof(reason)) !=
	    KN_RF_REPLY_ERR_TX_BLOCKED)
		return 1;

	return kn_tx_queue_count(&queue) == 0 &&
	    strcmp(reason, "tx-ui-blocked") == 0 ? 0 : 1;
}

static int
test_ping_event_and_queued_reply(void)
{
	const uint8_t payload[] = "PING";
	struct kn_config config;
	struct kn_rx_event rx;
	struct kn_port_stats port;
	struct kn_daemon_stats stats;
	struct kn_rf_command_event event;
	struct kn_tx_policy policy;
	struct kn_tx_queue queue;
	uint64_t id;

	setup_config(&config);
	memset(&port, 0, sizeof(port));
	(void)snprintf(port.name, sizeof(port.name), "kiss0");
	port.enabled = 1;
	port.open = 1;
	port.tx_enabled = 1;
	kn_daemon_stats_init(&stats, 1, 1);
	kn_tx_policy_defaults(&policy);
	policy.enabled = 1;
	policy.dry_run = 1;
	policy.allow_ui = 1;
	(void)kn_tx_queue_init(&queue, &policy);

	setup_rx(&rx);

	if (kn_rf_command_from_rx(&event, 1, 10, &config, &port, 1, &rx,
	    payload, 4) != KN_RF_COMMAND_OK)
		return 1;
	if (kn_rf_reply_try_queue(&queue, &config, &stats, &port, 1, NULL, 0,
	    &event, 10, &id, NULL, 0) != KN_RF_REPLY_OK)
		return 1;

	return kn_tx_queue_count(&queue) == 1 &&
	    kn_tx_queue_get(&queue, id)->status == KN_TX_FRAME_DRY_RUN ? 0 : 1;
}

static int
test_rate_limited_no_reply(void)
{
	const uint8_t payload[] = "PING";
	struct kn_config config;
	struct kn_rf_abuse_state abuse;
	struct kn_rf_ignore_list ignore;
	struct kn_rf_command_event event;
	struct kn_tx_policy policy;
	struct kn_tx_queue queue;

	setup_config(&config);
	config.rf_command.rate_limit_commands = 1;
	kn_rf_abuse_init(&abuse, 4);
	kn_rf_ignore_init(&ignore);
	kn_tx_policy_defaults(&policy);
	policy.enabled = 1;
	policy.dry_run = 1;
	policy.allow_ui = 1;
	(void)kn_tx_queue_init(&queue, &policy);

	if (process_command(&config, &queue, &abuse, &ignore, &event,
	    payload, 4, 10) != 0)
		return 1;
	if (process_command(&config, &queue, &abuse, &ignore, &event,
	    payload, 4, 11) != 0)
		return 1;

	return event.status == KN_RF_COMMAND_STATUS_RATE_LIMITED &&
	    event.reply_queued == 0 && kn_tx_queue_count(&queue) == 1 ? 0 : 1;
}

static int
test_reply_limited_no_tx(void)
{
	const uint8_t payload[] = "PING";
	struct kn_config config;
	struct kn_rf_abuse_state abuse;
	struct kn_rf_ignore_list ignore;
	struct kn_rf_command_event event;
	struct kn_tx_policy policy;
	struct kn_tx_queue queue;

	setup_config(&config);
	config.rf_command.reply_rate_limit_commands = 1;
	kn_rf_abuse_init(&abuse, 4);
	kn_rf_ignore_init(&ignore);
	kn_tx_policy_defaults(&policy);
	policy.enabled = 1;
	policy.dry_run = 1;
	policy.allow_ui = 1;
	(void)kn_tx_queue_init(&queue, &policy);

	if (process_command(&config, &queue, &abuse, &ignore, &event,
	    payload, 4, 10) != 0)
		return 1;
	if (process_command(&config, &queue, &abuse, &ignore, &event,
	    payload, 4, 11) != 0)
		return 1;

	return event.status == KN_RF_COMMAND_STATUS_REPLY_SUPPRESSED &&
	    event.reply_queued == 0 && kn_tx_queue_count(&queue) == 1 ? 0 : 1;
}
