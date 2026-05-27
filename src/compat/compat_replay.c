/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/compat/compat_replay.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "kilonode/ax25.h"
#include "kilonode/compat_replay.h"
#include "kilonode/rf_reply.h"
#include "kilonode/tx_queue.h"

static void add_mismatch(struct kn_compat_replay_result *, const char *);
static void config_init_for_replay(struct kn_config *,
	const struct kn_compat_transcript *);
static void port_init_for_replay(struct kn_port_stats *,
	const struct kn_compat_transcript *);
static void rx_init_for_replay(struct kn_rx_event *,
	const struct kn_compat_transcript *);

void
kn_compat_replay_result_clear(struct kn_compat_replay_result *result)
{
	if (result == NULL)
		return;

	memset(result, 0, sizeof(*result));
	result->observed_command = KN_RF_COMMAND_UNKNOWN;
	result->observed_status = KN_RF_COMMAND_STATUS_IGNORED;
}

const char *
kn_compat_replay_error_name(enum kn_compat_replay_error error)
{
	switch (error) {
	case KN_COMPAT_REPLAY_OK:
		return "ok";
	case KN_COMPAT_REPLAY_ERR_INVALID_ARGUMENT:
		return "invalid-argument";
	case KN_COMPAT_REPLAY_ERR_UNSUPPORTED_MODE:
		return "unsupported-mode";
	case KN_COMPAT_REPLAY_ERR_INTERNAL:
		return "internal";
	case KN_COMPAT_REPLAY_ERR_MISMATCH:
		return "mismatch";
	}

	return "unknown";
}

enum kn_compat_replay_error
kn_compat_replay_transcript(const struct kn_compat_transcript *transcript,
	struct kn_compat_replay_result *result)
{
	struct kn_config config;
	struct kn_port_stats port;
	struct kn_daemon_stats stats;
	struct kn_rx_event rx;
	struct kn_rf_command_event event;
	struct kn_tx_policy tx_policy;
	struct kn_tx_queue tx_queue;
	const struct kn_tx_frame *frame;
	char reply[KN_COMPAT_REPLY_PREVIEW_MAX];
	char reason[KN_RF_REPLY_REASON_MAX];
	uint64_t tx_id;

	if (transcript == NULL || result == NULL)
		return KN_COMPAT_REPLAY_ERR_INVALID_ARGUMENT;
	if (transcript->mode != KN_COMPAT_MODE_RF_UI)
		return KN_COMPAT_REPLAY_ERR_UNSUPPORTED_MODE;

	kn_compat_replay_result_clear(result);
	(void)snprintf(result->transcript_name,
	    sizeof(result->transcript_name), "%s", transcript->name);
	result->mode = transcript->mode;

	config_init_for_replay(&config, transcript);
	port_init_for_replay(&port, transcript);
	rx_init_for_replay(&rx, transcript);
	kn_daemon_stats_init(&stats, 1, 1);
	kn_tx_policy_defaults(&tx_policy);
	tx_policy.enabled = 1;
	tx_policy.dry_run = 1;
	tx_policy.allow_ui = 1;
	if (kn_tx_queue_init(&tx_queue, &tx_policy) != KN_TX_QUEUE_OK)
		return KN_COMPAT_REPLAY_ERR_INTERNAL;

	if (kn_rf_command_from_rx(&event, 1, 10, &config, &port, 1, &rx,
	    transcript->input, transcript->input_len) != KN_RF_COMMAND_OK)
		return KN_COMPAT_REPLAY_ERR_INTERNAL;
	result->observed_command = event.command;
	result->observed_status = event.status;

	reply[0] = '\0';
	tx_id = 0;
	reason[0] = '\0';
	if (kn_rf_reply_format(&event, &config, &stats, &port, 1, NULL, 0,
	    reply, sizeof(reply)) == KN_RF_REPLY_OK)
		(void)snprintf(result->observed_reply_preview,
		    sizeof(result->observed_reply_preview), "%s", reply);
	if (kn_rf_reply_try_queue(&tx_queue, &config, &stats, &port, 1, NULL,
	    0, &event, 10, &tx_id, reason, sizeof(reason)) ==
	    KN_RF_REPLY_OK) {
		event.reply_queued = 1;
		event.tx_frame_id = tx_id;
	}
	result->observed_reply_queued = event.reply_queued;
	result->observed_tx_frame_id = event.tx_frame_id;

	if (result->observed_command != transcript->expect_command)
		add_mismatch(result, "command");
	if (result->observed_status != transcript->expect_status)
		add_mismatch(result, "status");
	if (transcript->has_expect_reply_queued != 0 &&
	    result->observed_reply_queued != transcript->expect_reply_queued)
		add_mismatch(result, "reply-queued");
	if (transcript->expect_reply == KN_COMPAT_REPLY_NONE &&
	    result->observed_reply_preview[0] != '\0')
		add_mismatch(result, "reply-none");
	if (transcript->expect_reply == KN_COMPAT_REPLY_CONTAINS &&
	    strstr(result->observed_reply_preview,
	    transcript->expect_reply_text) == NULL)
		add_mismatch(result, "reply-contains");
	if (transcript->expect_reply == KN_COMPAT_REPLY_EXACT &&
	    strcmp(result->observed_reply_preview,
	    transcript->expect_reply_text) != 0)
		add_mismatch(result, "reply-exact");
	if (transcript->has_expect_error != 0 &&
	    strcmp(event.error, transcript->expect_error) != 0)
		add_mismatch(result, "error");
	if (transcript->expect_no_dispatch != 0 && event.reply_queued != 0) {
		frame = kn_tx_queue_get(&tx_queue, event.tx_frame_id);
		if (frame == NULL || frame->status != KN_TX_FRAME_DRY_RUN)
			add_mismatch(result, "dispatch");
	}

	result->passed = result->mismatch_count == 0 ? 1 : 0;
	return result->passed != 0 ? KN_COMPAT_REPLAY_OK :
	    KN_COMPAT_REPLAY_ERR_MISMATCH;
}

static void
add_mismatch(struct kn_compat_replay_result *result, const char *text)
{
	int needed;

	if (result->mismatch_count >= KN_COMPAT_MISMATCH_MAX)
		return;
	needed = snprintf(result->mismatches[result->mismatch_count].text,
	    sizeof(result->mismatches[result->mismatch_count].text), "%s",
	    text == NULL ? "mismatch" : text);
	if (needed < 0 || (size_t)needed >=
	    sizeof(result->mismatches[result->mismatch_count].text))
		result->mismatches[result->mismatch_count].text[0] = '\0';
	result->mismatch_count++;
}

static void
config_init_for_replay(struct kn_config *config,
	const struct kn_compat_transcript *transcript)
{
	kn_config_init(config);
	config->node.callsign = transcript->node;
	config->node.has_callsign = 1;
	config->rf_command.enabled = 1;
	config->rf_command.reply_enabled = 1;
	config->rf_command.require_node_destination = 1;
	config->rf_command.rate_limit_enabled = 0;
	config->transmit.policy.enabled = 1;
	config->transmit.policy.dry_run = 1;
	config->transmit.policy.allow_ui = 1;
	config->port_count = 1;
	(void)snprintf(config->ports[0].name, sizeof(config->ports[0].name),
	    "%s", transcript->port_name);
	config->ports[0].type = KN_CONFIG_PORT_MEMORY_TEST;
	config->ports[0].enabled = 1;
	config->ports[0].tx_enabled = 0;
}

static void
port_init_for_replay(struct kn_port_stats *port,
	const struct kn_compat_transcript *transcript)
{
	memset(port, 0, sizeof(*port));
	(void)snprintf(port->name, sizeof(port->name), "%s",
	    transcript->port_name);
	port->type = KN_CONFIG_PORT_MEMORY_TEST;
	port->enabled = 1;
	port->open = 1;
	port->tx_enabled = 0;
}

static void
rx_init_for_replay(struct kn_rx_event *rx,
	const struct kn_compat_transcript *transcript)
{
	kn_rx_event_clear(rx);
	rx->id = 1;
	rx->timestamp = 10;
	(void)snprintf(rx->port_name, sizeof(rx->port_name), "%s",
	    transcript->port_name);
	rx->kiss_port = 0;
	rx->kiss_command = 0;
	rx->source = transcript->source;
	rx->destination = transcript->destination;
	rx->control = KN_AX25_CONTROL_UI;
	rx->pid = transcript->pid;
	rx->has_pid = 1;
	rx->kind = KN_RX_FRAME_UI;
	rx->payload_len = transcript->input_len;
}
