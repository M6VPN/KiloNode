/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/rf/tx_dispatch.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "kilonode/tx_dispatch.h"

static size_t count_remaining(const struct kn_tx_queue *, const char *);
static uint8_t frame_dispatchable(const struct kn_tx_frame *, const char *);
static enum kn_tx_dispatch_error dispatch_memory(struct kn_tx_frame *,
	struct kn_tx_dispatcher *, struct kn_tx_dispatch_target *,
	struct kn_tx_dispatch_result *);
static enum kn_tx_dispatch_error dispatch_transport(struct kn_tx_frame *,
	struct kn_tx_dispatcher *, struct kn_tx_dispatch_target *,
	struct kn_tx_dispatch_result *);
static struct kn_tx_dispatch_target *target_find(struct kn_tx_dispatcher *,
	const char *);

enum kn_tx_dispatch_error
kn_tx_dispatch_add_memory_target(struct kn_tx_dispatcher *dispatcher,
	const char *port_name, struct kn_transport_memory *memory,
	uint8_t enabled, uint8_t open)
{
	struct kn_tx_dispatch_target *target;
	int needed;

	if (dispatcher == NULL || port_name == NULL || port_name[0] == '\0' ||
	    memory == NULL)
		return KN_TX_DISPATCH_ERR_INVALID_ARGUMENT;
	if (dispatcher->target_count >= KN_TX_DISPATCH_TARGET_MAX)
		return KN_TX_DISPATCH_ERR_INVALID_ARGUMENT;

	target = &dispatcher->targets[dispatcher->target_count];
	memset(target, 0, sizeof(*target));
	needed = snprintf(target->port_name, sizeof(target->port_name), "%s",
	    port_name);
	if (needed < 0 || (size_t)needed >= sizeof(target->port_name))
		return KN_TX_DISPATCH_ERR_INVALID_ARGUMENT;
	target->memory = memory;
	target->config_type = KN_CONFIG_PORT_MEMORY_TEST;
	target->transport_kind = KN_TRANSPORT_KIND_MEMORY_TEST;
	target->enabled = enabled;
	target->open = open;
	target->tx_enabled = 0;
	target->test_safe = 1;
	dispatcher->target_count++;
	return KN_TX_DISPATCH_OK;
}

enum kn_tx_dispatch_error
kn_tx_dispatch_add_transport_target(struct kn_tx_dispatcher *dispatcher,
	const char *port_name, struct kn_transport *transport,
	enum kn_config_port_type config_type, enum kn_transport_kind kind,
	uint8_t enabled, uint8_t open, uint8_t tx_enabled)
{
	struct kn_tx_dispatch_target *target;
	int needed;

	if (dispatcher == NULL || port_name == NULL || port_name[0] == '\0' ||
	    transport == NULL)
		return KN_TX_DISPATCH_ERR_INVALID_ARGUMENT;
	if (dispatcher->target_count >= KN_TX_DISPATCH_TARGET_MAX)
		return KN_TX_DISPATCH_ERR_INVALID_ARGUMENT;

	target = &dispatcher->targets[dispatcher->target_count];
	memset(target, 0, sizeof(*target));
	needed = snprintf(target->port_name, sizeof(target->port_name), "%s",
	    port_name);
	if (needed < 0 || (size_t)needed >= sizeof(target->port_name))
		return KN_TX_DISPATCH_ERR_INVALID_ARGUMENT;
	target->transport = transport;
	target->config_type = config_type;
	target->transport_kind = kind;
	target->enabled = enabled;
	target->open = open;
	target->tx_enabled = tx_enabled;
	target->test_safe = 0;
	dispatcher->target_count++;
	return KN_TX_DISPATCH_OK;
}

void
kn_tx_dispatch_clear(struct kn_tx_dispatcher *dispatcher)
{
	if (dispatcher == NULL)
		return;

	memset(dispatcher, 0, sizeof(*dispatcher));
}

const char *
kn_tx_dispatch_error_name(enum kn_tx_dispatch_error error)
{
	switch (error) {
	case KN_TX_DISPATCH_OK:
		return "ok";
	case KN_TX_DISPATCH_ERR_INVALID_ARGUMENT:
		return "invalid-argument";
	case KN_TX_DISPATCH_ERR_DISABLED:
		return "tx-dispatch-disabled";
	case KN_TX_DISPATCH_ERR_TEST_ONLY_REQUIRED:
		return "tx-dispatch-test-only-required";
	case KN_TX_DISPATCH_ERR_NO_SAFE_TARGET:
		return "tx-dispatch-no-safe-target";
	case KN_TX_DISPATCH_ERR_INVALID_PORT:
		return "invalid-port";
	case KN_TX_DISPATCH_ERR_GATE_BLOCKED:
		return "tx-dispatch-gate-blocked";
	case KN_TX_DISPATCH_ERR_WRITE:
		return "tx-dispatch-write-error";
	}

	return "tx-dispatch-error";
}

enum kn_tx_dispatch_error
kn_tx_dispatch_run(struct kn_tx_queue *queue,
	struct kn_tx_dispatcher *dispatcher, const char *port_name,
	struct kn_tx_dispatch_result *result)
{
	struct kn_tx_frame *frame;
	struct kn_tx_dispatch_target *target;
	size_t i;
	size_t limit;
	enum kn_tx_policy_error policy_rc;
	enum kn_tx_transport_gate_error gate_rc;

	if (result != NULL)
		memset(result, 0, sizeof(*result));
	if (queue == NULL || dispatcher == NULL || result == NULL)
		return KN_TX_DISPATCH_ERR_INVALID_ARGUMENT;
	if (port_name != NULL && (port_name[0] == '\0' ||
	    strlen(port_name) >= KN_CONFIG_PORT_NAME_MAX ||
	    strchr(port_name, ' ') != NULL))
		return KN_TX_DISPATCH_ERR_INVALID_PORT;

	policy_rc = kn_tx_policy_allow_dispatch(&queue->policy);
	if (policy_rc == KN_TX_POLICY_ERR_DISABLED ||
	    policy_rc == KN_TX_POLICY_ERR_DISPATCH_DISABLED) {
		dispatcher->stats.blocked++;
		return KN_TX_DISPATCH_ERR_DISABLED;
	}
	if (policy_rc == KN_TX_POLICY_ERR_DISPATCH_TEST_ONLY_REQUIRED) {
		dispatcher->stats.blocked++;
		return KN_TX_DISPATCH_ERR_TEST_ONLY_REQUIRED;
	}
	if (policy_rc == KN_TX_POLICY_ERR_DRY_RUN_REQUIRED ||
	    policy_rc == KN_TX_POLICY_ERR_REAL_KISS_DISABLED) {
		if (policy_rc == KN_TX_POLICY_ERR_DRY_RUN_REQUIRED)
			result->gate_error =
			    KN_TX_TRANSPORT_GATE_ERR_DRY_RUN_ENABLED;
		else
			result->gate_error =
			    KN_TX_TRANSPORT_GATE_ERR_REAL_KISS_DISABLED;
		dispatcher->stats.blocked++;
		return KN_TX_DISPATCH_ERR_GATE_BLOCKED;
	}
	if (policy_rc != KN_TX_POLICY_OK)
		return KN_TX_DISPATCH_ERR_INVALID_ARGUMENT;

	if (port_name != NULL && target_find(dispatcher, port_name) == NULL)
		return KN_TX_DISPATCH_ERR_INVALID_PORT;
	if (dispatcher->target_count == 0)
		return KN_TX_DISPATCH_ERR_NO_SAFE_TARGET;

	limit = queue->policy.dispatch_max_per_cycle;
	for (i = 0; i < queue->count && result->attempted < limit; i++) {
		frame = &queue->frames[i];
		if (frame_dispatchable(frame, port_name) == 0)
			continue;
		target = target_find(dispatcher, frame->port_name);
		if (queue->policy.dispatch_test_only != 0) {
			struct kn_tx_transport_gate_port gate_port;

			memset(&gate_port, 0, sizeof(gate_port));
			if (target != NULL) {
				(void)snprintf(gate_port.name,
				    sizeof(gate_port.name), "%s",
				    target->port_name);
				gate_port.config_type = target->config_type;
				gate_port.transport_kind =
				    target->transport_kind;
				gate_port.enabled = target->enabled;
				gate_port.open = target->open;
				gate_port.tx_enabled = target->tx_enabled;
				gate_port.writable =
				    target->memory != NULL &&
				    target->memory->open != 0;
				gate_port.memory_test = target->test_safe;
			}
			gate_rc = kn_tx_transport_gate_test(&queue->policy,
			    target == NULL ? NULL : &gate_port);
			if (gate_rc != KN_TX_TRANSPORT_GATE_OK) {
				result->gate_error = gate_rc;
				dispatcher->stats.blocked++;
				return KN_TX_DISPATCH_ERR_GATE_BLOCKED;
			}
			(void)dispatch_memory(frame, dispatcher, target,
			    result);
		} else {
			struct kn_tx_transport_gate_port gate_port;

			memset(&gate_port, 0, sizeof(gate_port));
			if (target != NULL) {
				(void)snprintf(gate_port.name,
				    sizeof(gate_port.name), "%s",
				    target->port_name);
				gate_port.config_type = target->config_type;
				gate_port.transport_kind =
				    target->transport_kind;
				gate_port.enabled = target->enabled;
				gate_port.open = target->open;
				gate_port.tx_enabled = target->tx_enabled;
				gate_port.writable =
				    target->transport != NULL &&
				    target->transport->open != 0 &&
				    target->transport->write_fd >= 0;
				gate_port.memory_test = target->test_safe;
			}
			gate_rc = kn_tx_transport_gate_real(&queue->policy,
			    target == NULL ? NULL : &gate_port);
			if (gate_rc != KN_TX_TRANSPORT_GATE_OK) {
				result->gate_error = gate_rc;
				dispatcher->stats.blocked++;
				return KN_TX_DISPATCH_ERR_GATE_BLOCKED;
			}
			(void)dispatch_transport(frame, dispatcher, target,
			    result);
		}
	}

	result->remaining = count_remaining(queue, port_name);
	return KN_TX_DISPATCH_OK;
}

const struct kn_tx_dispatch_target *
kn_tx_dispatch_target_find(const struct kn_tx_dispatcher *dispatcher,
	const char *port_name)
{
	size_t i;

	if (dispatcher == NULL || port_name == NULL)
		return NULL;

	for (i = 0; i < dispatcher->target_count; i++) {
		if (strcmp(dispatcher->targets[i].port_name, port_name) == 0)
			return &dispatcher->targets[i];
	}

	return NULL;
}

static enum kn_tx_dispatch_error
dispatch_memory(struct kn_tx_frame *frame, struct kn_tx_dispatcher *dispatcher,
	struct kn_tx_dispatch_target *target, struct kn_tx_dispatch_result *result)
{
	enum kn_transport_memory_error write_rc;

	dispatcher->stats.attempts++;
	result->attempted++;
	write_rc = kn_transport_memory_write(target->memory, frame->kiss,
	    frame->kiss_len);
	if (write_rc == KN_TRANSPORT_MEMORY_OK) {
		frame->status = KN_TX_FRAME_SENT;
		dispatcher->stats.sent++;
		dispatcher->stats.bytes_written += (uint64_t)frame->kiss_len;
		result->sent++;
		result->bytes_written += (uint64_t)frame->kiss_len;
		return KN_TX_DISPATCH_OK;
	}

	frame->status = KN_TX_FRAME_FAILED;
	frame->last_error = (int)write_rc;
	dispatcher->stats.failed++;
	result->failed++;
	return KN_TX_DISPATCH_ERR_WRITE;
}

static enum kn_tx_dispatch_error
dispatch_transport(struct kn_tx_frame *frame,
	struct kn_tx_dispatcher *dispatcher,
	struct kn_tx_dispatch_target *target, struct kn_tx_dispatch_result *result)
{
	enum kn_transport_error write_rc;

	dispatcher->stats.attempts++;
	result->attempted++;
	write_rc = kn_transport_write(target->transport, frame->kiss,
	    frame->kiss_len);
	if (write_rc == KN_TRANSPORT_OK) {
		frame->status = KN_TX_FRAME_SENT;
		dispatcher->stats.sent++;
		dispatcher->stats.bytes_written += (uint64_t)frame->kiss_len;
		result->sent++;
		result->bytes_written += (uint64_t)frame->kiss_len;
		return KN_TX_DISPATCH_OK;
	}

	frame->status = KN_TX_FRAME_FAILED;
	frame->last_error = (int)write_rc;
	dispatcher->stats.failed++;
	result->failed++;
	return KN_TX_DISPATCH_ERR_WRITE;
}

static size_t
count_remaining(const struct kn_tx_queue *queue, const char *port_name)
{
	size_t i;
	size_t count;

	count = 0;
	for (i = 0; i < queue->count; i++) {
		if (frame_dispatchable(&queue->frames[i], port_name) != 0)
			count++;
	}

	return count;
}

static uint8_t
frame_dispatchable(const struct kn_tx_frame *frame, const char *port_name)
{
	if (frame == NULL || frame->kiss_len == 0)
		return 0;
	if (frame->status != KN_TX_FRAME_QUEUED &&
	    frame->status != KN_TX_FRAME_DRY_RUN)
		return 0;
	if (port_name != NULL && strcmp(frame->port_name, port_name) != 0)
		return 0;
	return 1;
}

static struct kn_tx_dispatch_target *
target_find(struct kn_tx_dispatcher *dispatcher, const char *port_name)
{
	size_t i;

	if (dispatcher == NULL || port_name == NULL)
		return NULL;

	for (i = 0; i < dispatcher->target_count; i++) {
		if (strcmp(dispatcher->targets[i].port_name, port_name) == 0)
			return &dispatcher->targets[i];
	}

	return NULL;
}
