/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/rf/tx_dry_run.c */

#include <sys/types.h>

#include <stdint.h>
#include <string.h>

#include "kilonode/ax25.h"
#include "kilonode/tx_dry_run.h"

static enum kn_tx_dry_run_error map_policy(enum kn_tx_policy_error);
static enum kn_tx_dry_run_error parse_via(char *, const char **, size_t *);
static const struct kn_port_stats *port_find(const struct kn_port_stats *,
	size_t, const char *);

enum kn_tx_dry_run_error
kn_tx_dry_run_enqueue_ui(struct kn_tx_queue *queue,
	const struct kn_port_stats *ports, size_t port_count,
	enum kn_tx_dry_run_origin origin, uint64_t now, const char *port_name,
	const char *source, const char *destination, const char *via,
	uint8_t pid, const uint8_t *payload, size_t payload_len,
	uint64_t *out_id)
{
	const struct kn_port_stats *port;
	const char *digipeaters[KN_AX25_MAX_DIGIS];
	char via_copy[KN_TX_FRAME_PATH_MAX];
	struct kn_tx_frame frame;
	size_t digipeater_count;
	uint64_t id;
	enum kn_tx_policy_error policy_rc;
	enum kn_tx_frame_error frame_rc;
	enum kn_tx_queue_error queue_rc;

	if (queue == NULL || port_name == NULL || source == NULL ||
	    destination == NULL || out_id == NULL ||
	    (payload == NULL && payload_len > 0))
		return KN_TX_DRY_RUN_ERR_INVALID_ARGUMENT;

	if (origin == KN_TX_DRY_RUN_ORIGIN_CONTROL)
		policy_rc = kn_tx_policy_allow_control_enqueue(&queue->policy,
		    payload_len);
	else
		policy_rc = kn_tx_policy_allow_shell_enqueue(&queue->policy,
		    payload_len);
	if (policy_rc != KN_TX_POLICY_OK)
		return map_policy(policy_rc);
	if (kn_tx_policy_allow_ui(&queue->policy, payload_len) !=
	    KN_TX_POLICY_OK)
		return KN_TX_DRY_RUN_ERR_UI_DISABLED;

	port = port_find(ports, port_count, port_name);
	if (port == NULL || port->enabled == 0 || port->open == 0)
		return KN_TX_DRY_RUN_ERR_INVALID_PORT;

	digipeater_count = 0;
	if (via != NULL && via[0] != '\0') {
		if (strlen(via) >= sizeof(via_copy))
			return KN_TX_DRY_RUN_ERR_INVALID_VIA;
		memcpy(via_copy, via, strlen(via) + 1);
		if (parse_via(via_copy, digipeaters, &digipeater_count) !=
		    KN_TX_DRY_RUN_OK)
			return KN_TX_DRY_RUN_ERR_INVALID_VIA;
	}

	id = kn_tx_queue_reserve_id(queue);
	frame_rc = kn_tx_frame_build_ui(&frame, id, now, port_name, 0,
	    source, destination, digipeaters, digipeater_count, pid, payload,
	    payload_len, &queue->policy);
	if (frame_rc == KN_TX_FRAME_ERR_INVALID_VALUE)
		return KN_TX_DRY_RUN_ERR_INVALID_CALLSIGN;
	if (frame_rc == KN_TX_FRAME_ERR_TOO_LARGE)
		return KN_TX_DRY_RUN_ERR_PAYLOAD_TOO_LARGE;
	if (frame_rc != KN_TX_FRAME_OK)
		return KN_TX_DRY_RUN_ERR_BUILD;
	frame.status = KN_TX_FRAME_DRY_RUN;

	queue_rc = kn_tx_queue_enqueue(queue, &frame);
	if (queue_rc == KN_TX_QUEUE_ERR_FULL)
		return KN_TX_DRY_RUN_ERR_QUEUE_FULL;
	if (queue_rc != KN_TX_QUEUE_OK)
		return KN_TX_DRY_RUN_ERR_BUILD;

	*out_id = id;
	return KN_TX_DRY_RUN_OK;
}

const char *
kn_tx_dry_run_error_name(enum kn_tx_dry_run_error error)
{
	switch (error) {
	case KN_TX_DRY_RUN_OK:
		return "ok";
	case KN_TX_DRY_RUN_ERR_INVALID_ARGUMENT:
		return "invalid-argument";
	case KN_TX_DRY_RUN_ERR_DISABLED:
		return "tx-disabled";
	case KN_TX_DRY_RUN_ERR_DRY_RUN_REQUIRED:
		return "tx-dry-run-required";
	case KN_TX_DRY_RUN_ERR_CONTROL_DISABLED:
		return "tx-control-enqueue-disabled";
	case KN_TX_DRY_RUN_ERR_SHELL_DISABLED:
		return "tx-shell-enqueue-disabled";
	case KN_TX_DRY_RUN_ERR_UI_DISABLED:
		return "tx-ui-disabled";
	case KN_TX_DRY_RUN_ERR_INVALID_PORT:
		return "invalid-port";
	case KN_TX_DRY_RUN_ERR_INVALID_CALLSIGN:
		return "invalid-callsign";
	case KN_TX_DRY_RUN_ERR_INVALID_VIA:
		return "invalid-via";
	case KN_TX_DRY_RUN_ERR_PAYLOAD_TOO_LARGE:
		return "payload-too-large";
	case KN_TX_DRY_RUN_ERR_QUEUE_FULL:
		return "tx-queue-full";
	case KN_TX_DRY_RUN_ERR_BUILD:
		return "tx-build-error";
	}

	return "tx-error";
}

static enum kn_tx_dry_run_error
map_policy(enum kn_tx_policy_error error)
{
	switch (error) {
	case KN_TX_POLICY_OK:
		return KN_TX_DRY_RUN_OK;
	case KN_TX_POLICY_ERR_DISABLED:
		return KN_TX_DRY_RUN_ERR_DISABLED;
	case KN_TX_POLICY_ERR_DISPATCH_DISABLED:
	case KN_TX_POLICY_ERR_DISPATCH_TEST_ONLY_REQUIRED:
		return KN_TX_DRY_RUN_ERR_DRY_RUN_REQUIRED;
	case KN_TX_POLICY_ERR_CONTROL_DISABLED:
		return KN_TX_DRY_RUN_ERR_CONTROL_DISABLED;
	case KN_TX_POLICY_ERR_SHELL_DISABLED:
		return KN_TX_DRY_RUN_ERR_SHELL_DISABLED;
	case KN_TX_POLICY_ERR_UI_NOT_ALLOWED:
		return KN_TX_DRY_RUN_ERR_UI_DISABLED;
	case KN_TX_POLICY_ERR_TOO_LARGE:
		return KN_TX_DRY_RUN_ERR_PAYLOAD_TOO_LARGE;
	case KN_TX_POLICY_ERR_INVALID_ARGUMENT:
		return KN_TX_DRY_RUN_ERR_INVALID_ARGUMENT;
	}

	return KN_TX_DRY_RUN_ERR_BUILD;
}

static enum kn_tx_dry_run_error
parse_via(char *via, const char **digipeaters, size_t *digipeater_count)
{
	char *p;
	char *start;
	struct kn_callsign callsign;
	size_t count;

	if (via == NULL || digipeaters == NULL || digipeater_count == NULL)
		return KN_TX_DRY_RUN_ERR_INVALID_ARGUMENT;

	count = 0;
	start = via;
	for (p = via;; p++) {
		char delim;

		if (*p != ',' && *p != '\0')
			continue;
		delim = *p;
		if (p == start || count >= KN_AX25_MAX_DIGIS)
			return KN_TX_DRY_RUN_ERR_INVALID_VIA;
		if (delim == ',')
			*p = '\0';
		if (kn_callsign_parse(start, &callsign) != 0)
			return KN_TX_DRY_RUN_ERR_INVALID_VIA;
		digipeaters[count++] = start;
		if (delim == '\0')
			break;
		start = p + 1;
	}

	*digipeater_count = count;
	return KN_TX_DRY_RUN_OK;
}

static const struct kn_port_stats *
port_find(const struct kn_port_stats *ports, size_t port_count,
	const char *port_name)
{
	size_t i;

	if (ports == NULL || port_name == NULL)
		return NULL;

	for (i = 0; i < port_count; i++) {
		if (strcmp(ports[i].name, port_name) == 0)
			return &ports[i];
	}

	return NULL;
}
