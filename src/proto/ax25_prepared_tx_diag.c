/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/proto/ax25_prepared_tx_diag.c */

#include <sys/types.h>

#include <stdio.h>

#include "kilonode/ax25_prepared_tx_diag.h"
#include "kilonode/ax25_prepared_tx_gate.h"

static void make_default_gate_input(const struct kn_ax25_runtime *,
	const struct kn_ax25_prepared_frame *,
	struct kn_ax25_prepared_tx_gate_input *,
	struct kn_tx_policy *);

static void
make_default_gate_input(const struct kn_ax25_runtime *runtime,
	const struct kn_ax25_prepared_frame *frame,
	struct kn_ax25_prepared_tx_gate_input *input,
	struct kn_tx_policy *tx_policy)
{
	kn_tx_policy_defaults(tx_policy);
	input->prepared = frame;
	input->policy = &runtime->prepared_tx_policy;
	input->tx_policy = tx_policy;
	input->port = NULL;
}

enum kn_ax25_prepared_tx_diag_error
kn_ax25_prepared_tx_diag_format_counters(
	const struct kn_ax25_runtime *runtime, char *buf, size_t bufsiz)
{
	struct kn_ax25_runtime fallback;
	const struct kn_ax25_runtime *rt;
	int needed;

	if (buf == NULL || bufsiz == 0)
		return KN_AX25_PREPARED_TX_DIAG_ERR_INVALID_ARGUMENT;

	rt = runtime;
	if (rt == NULL) {
		kn_ax25_runtime_init(&fallback);
		rt = &fallback;
	}
	needed = snprintf(buf, bufsiz,
	    "OK AX25 PREPARED BRIDGE COUNTERS checks=%llu allowed=%llu "
	    "blocked=%llu test_conversions=%llu tx_writes=%llu "
	    "fx25_blocked=%llu\nEND\n",
	    (unsigned long long)rt->prepared_tx_counters.checks,
	    (unsigned long long)rt->prepared_tx_counters.allowed,
	    (unsigned long long)rt->prepared_tx_counters.blocked,
	    (unsigned long long)rt->prepared_tx_counters.test_conversions,
	    (unsigned long long)rt->prepared_tx_counters.tx_queue_writes,
	    (unsigned long long)rt->prepared_tx_counters.fx25_blocked);
	if (needed < 0 || (size_t)needed >= bufsiz)
		return KN_AX25_PREPARED_TX_DIAG_ERR_BUFFER;

	return KN_AX25_PREPARED_TX_DIAG_OK;
}

enum kn_ax25_prepared_tx_diag_error
kn_ax25_prepared_tx_diag_format_frame(
	const struct kn_ax25_runtime *runtime, uint64_t id, char *buf,
	size_t bufsiz)
{
	struct kn_ax25_runtime fallback;
	const struct kn_ax25_runtime *rt;
	const struct kn_ax25_prepared_frame *frame;
	struct kn_ax25_prepared_tx_gate_input input;
	struct kn_ax25_prepared_tx_gate_decision decision;
	struct kn_tx_policy tx_policy;
	int needed;

	if (buf == NULL || bufsiz == 0 || id == 0)
		return KN_AX25_PREPARED_TX_DIAG_ERR_INVALID_ARGUMENT;

	rt = runtime;
	if (rt == NULL) {
		kn_ax25_runtime_init(&fallback);
		rt = &fallback;
	}
	frame = kn_ax25_prepared_queue_get(&rt->prepared_queue, id);
	if (frame == NULL) {
		(void)snprintf(buf, bufsiz, "ERR prepared-frame-not-found\n");
		return KN_AX25_PREPARED_TX_DIAG_ERR_NOT_FOUND;
	}
	make_default_gate_input(rt, frame, &input, &tx_policy);
	kn_ax25_prepared_tx_gate_evaluate(&input, &decision);

	needed = snprintf(buf, bufsiz,
	    "OK AX25 PREPARED BRIDGE FRAME id=%llu\n"
	    "AX25 PREPARED BRIDGE FRAME id=%llu allowed=%s reason=%s "
	    "would_tx=%s would_fx25=%s would_kiss=%s\nEND\n",
	    (unsigned long long)id, (unsigned long long)id,
	    decision.allowed != 0 ? "true" : "false",
	    kn_ax25_prepared_tx_gate_reason_name(decision.reason),
	    decision.would_create_tx_frame != 0 ? "true" : "false",
	    decision.would_require_fx25 != 0 ? "true" : "false",
	    decision.would_require_kiss != 0 ? "true" : "false");
	if (needed < 0 || (size_t)needed >= bufsiz)
		return KN_AX25_PREPARED_TX_DIAG_ERR_BUFFER;

	return KN_AX25_PREPARED_TX_DIAG_OK;
}

enum kn_ax25_prepared_tx_diag_error
kn_ax25_prepared_tx_diag_format_status(
	const struct kn_ax25_runtime *runtime, char *buf, size_t bufsiz)
{
	struct kn_ax25_runtime fallback;
	const struct kn_ax25_runtime *rt;
	int needed;

	if (buf == NULL || bufsiz == 0)
		return KN_AX25_PREPARED_TX_DIAG_ERR_INVALID_ARGUMENT;

	rt = runtime;
	if (rt == NULL) {
		kn_ax25_runtime_init(&fallback);
		rt = &fallback;
	}
	needed = snprintf(buf, bufsiz,
	    "OK AX25 PREPARED BRIDGE enabled=%s test_only=%s "
	    "control_frames=%s i_frames=%s max_per_call=%llu fx25=%s\n"
	    "END\n",
	    rt->prepared_tx_policy.bridge_enabled != 0 ? "true" : "false",
	    rt->prepared_tx_policy.test_only != 0 ? "true" : "false",
	    rt->prepared_tx_policy.allow_control_frames != 0 ?
	    "true" : "false",
	    rt->prepared_tx_policy.allow_i_frames != 0 ? "true" : "false",
	    (unsigned long long)
	    rt->prepared_tx_policy.max_bridge_per_call,
	    rt->prepared_tx_policy.allow_fx25_wrapping != 0 ?
	    "true" : "false");
	if (needed < 0 || (size_t)needed >= bufsiz)
		return KN_AX25_PREPARED_TX_DIAG_ERR_BUFFER;

	return KN_AX25_PREPARED_TX_DIAG_OK;
}
