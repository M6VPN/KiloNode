/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/rf/rf_abuse.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "kilonode/rf_abuse.h"

static int callsign_equal(const struct kn_callsign *,
	const struct kn_callsign *);
static void clear_expired_ignore(struct kn_rf_abuse_source *, uint64_t);
static struct kn_rf_abuse_source *find_source(struct kn_rf_abuse_state *,
	const struct kn_callsign *);
static struct kn_rf_abuse_source *get_source(struct kn_rf_abuse_state *,
	const struct kn_callsign *, uint64_t);
static void reason_set(char *, size_t, const char *);

enum kn_rf_abuse_error
kn_rf_abuse_check_command(struct kn_rf_abuse_state *state,
	const struct kn_config_rf_command *config,
	const struct kn_rf_ignore_list *ignore_list,
	const struct kn_callsign *callsign, uint64_t now,
	char *reason, size_t reason_len)
{
	struct kn_rf_abuse_source *source;
	const struct kn_rf_ignore_entry *ignore;

	if (state == NULL || config == NULL || callsign == NULL)
		return KN_RF_ABUSE_ERR_INVALID_ARGUMENT;

	source = get_source(state, callsign, now);
	if (source == NULL)
		return KN_RF_ABUSE_ERR_FULL;
	clear_expired_ignore(source, now);

	if (kn_rf_ignore_source_ignored(ignore_list, callsign, &ignore) != 0) {
		source->ignored = 1;
		source->ignore_until = 0;
		reason_set(source->last_reason,
		    sizeof(source->last_reason),
		    ignore != NULL ? ignore->reason : "manual-ignore");
		reason_set(reason, reason_len, source->last_reason);
		return KN_RF_ABUSE_ERR_IGNORED;
	}

	if (source->ignored != 0) {
		reason_set(reason, reason_len, source->last_reason[0] == '\0' ?
		    "auto-ignore" : source->last_reason);
		return KN_RF_ABUSE_ERR_IGNORED;
	}

	if (config->rate_limit_enabled == 0)
		return KN_RF_ABUSE_OK;

	if (source->command_window_start == 0 ||
	    now - source->command_window_start >=
	    config->rate_limit_window_seconds) {
		source->command_window_start = now;
		source->command_count = 0;
	}
	if (source->command_count >= config->rate_limit_commands) {
		reason_set(source->last_reason, sizeof(source->last_reason),
		    "rate-limited");
		reason_set(reason, reason_len, "rate-limited");
		return KN_RF_ABUSE_ERR_RATE_LIMITED;
	}

	source->command_count++;
	source->last_seen = now;
	return KN_RF_ABUSE_OK;
}

enum kn_rf_abuse_error
kn_rf_abuse_check_reply(struct kn_rf_abuse_state *state,
	const struct kn_config_rf_command *config,
	const struct kn_callsign *callsign, uint64_t now,
	char *reason, size_t reason_len)
{
	struct kn_rf_abuse_source *source;

	if (state == NULL || config == NULL || callsign == NULL)
		return KN_RF_ABUSE_ERR_INVALID_ARGUMENT;

	source = get_source(state, callsign, now);
	if (source == NULL)
		return KN_RF_ABUSE_ERR_FULL;

	if (source->reply_window_start == 0 ||
	    now - source->reply_window_start >=
	    config->reply_rate_limit_window_seconds) {
		source->reply_window_start = now;
		source->reply_count = 0;
	}
	if (source->reply_count >= config->reply_rate_limit_commands) {
		reason_set(source->last_reason, sizeof(source->last_reason),
		    "reply-rate-limited");
		reason_set(reason, reason_len, "reply-rate-limited");
		return KN_RF_ABUSE_ERR_REPLY_RATE_LIMITED;
	}

	return KN_RF_ABUSE_OK;
}

void
kn_rf_abuse_clear(struct kn_rf_abuse_state *state)
{
	if (state == NULL)
		return;

	memset(state, 0, sizeof(*state));
	state->max_sources = KN_RF_ABUSE_MAX_SOURCES;
}

size_t
kn_rf_abuse_count(const struct kn_rf_abuse_state *state)
{
	return state == NULL ? 0 : state->count;
}

const char *
kn_rf_abuse_error_name(enum kn_rf_abuse_error error)
{
	switch (error) {
	case KN_RF_ABUSE_OK:
		return "ok";
	case KN_RF_ABUSE_ERR_INVALID_ARGUMENT:
		return "invalid-argument";
	case KN_RF_ABUSE_ERR_IGNORED:
		return "ignored";
	case KN_RF_ABUSE_ERR_RATE_LIMITED:
		return "rate-limited";
	case KN_RF_ABUSE_ERR_REPLY_RATE_LIMITED:
		return "reply-rate-limited";
	case KN_RF_ABUSE_ERR_FULL:
		return "full";
	}

	return "unknown";
}

const struct kn_rf_abuse_source *
kn_rf_abuse_find(const struct kn_rf_abuse_state *state,
	const struct kn_callsign *callsign)
{
	size_t i;

	if (state == NULL || callsign == NULL)
		return NULL;

	for (i = 0; i < state->count; i++) {
		if (callsign_equal(&state->sources[i].callsign, callsign) != 0)
			return &state->sources[i];
	}

	return NULL;
}

void
kn_rf_abuse_init(struct kn_rf_abuse_state *state, size_t max_sources)
{
	if (state == NULL)
		return;

	memset(state, 0, sizeof(*state));
	if (max_sources == 0 || max_sources > KN_RF_ABUSE_MAX_SOURCES)
		state->max_sources = KN_RF_ABUSE_MAX_SOURCES;
	else
		state->max_sources = max_sources;
}

enum kn_rf_abuse_error
kn_rf_abuse_record_accepted(struct kn_rf_abuse_state *state,
	const struct kn_callsign *callsign, uint64_t now)
{
	struct kn_rf_abuse_source *source;

	if (state == NULL || callsign == NULL)
		return KN_RF_ABUSE_ERR_INVALID_ARGUMENT;

	source = get_source(state, callsign, now);
	if (source == NULL)
		return KN_RF_ABUSE_ERR_FULL;
	source->accepted_count++;
	source->last_seen = now;
	reason_set(source->last_reason, sizeof(source->last_reason), "ok");
	return KN_RF_ABUSE_OK;
}

enum kn_rf_abuse_error
kn_rf_abuse_record_rejected(struct kn_rf_abuse_state *state,
	const struct kn_config_rf_command *config,
	const struct kn_callsign *callsign, uint64_t now, const char *reason)
{
	struct kn_rf_abuse_source *source;

	if (state == NULL || config == NULL || callsign == NULL)
		return KN_RF_ABUSE_ERR_INVALID_ARGUMENT;

	source = get_source(state, callsign, now);
	if (source == NULL)
		return KN_RF_ABUSE_ERR_FULL;
	source->rejected_count++;
	source->last_seen = now;
	reason_set(source->last_reason, sizeof(source->last_reason), reason);

	if (source->reject_window_start == 0 ||
	    now - source->reject_window_start >=
	    config->rate_limit_window_seconds) {
		source->reject_window_start = now;
		source->reject_count = 0;
	}
	source->reject_count++;
	if (config->auto_ignore_enabled != 0 &&
	    source->reject_count >= config->auto_ignore_after_rejects) {
		source->ignored = 1;
		source->ignore_until = now + config->auto_ignore_seconds;
		reason_set(source->last_reason, sizeof(source->last_reason),
		    "auto-ignore");
		return KN_RF_ABUSE_ERR_IGNORED;
	}

	return KN_RF_ABUSE_OK;
}

enum kn_rf_abuse_error
kn_rf_abuse_record_reply(struct kn_rf_abuse_state *state,
	const struct kn_callsign *callsign, uint64_t now)
{
	struct kn_rf_abuse_source *source;

	if (state == NULL || callsign == NULL)
		return KN_RF_ABUSE_ERR_INVALID_ARGUMENT;

	source = get_source(state, callsign, now);
	if (source == NULL)
		return KN_RF_ABUSE_ERR_FULL;
	source->reply_count++;
	source->replies++;
	source->last_seen = now;
	return KN_RF_ABUSE_OK;
}

const struct kn_rf_abuse_source *
kn_rf_abuse_sources(const struct kn_rf_abuse_state *state)
{
	return state == NULL ? NULL : state->sources;
}

uint8_t
kn_rf_abuse_source_ignored(struct kn_rf_abuse_state *state,
	const struct kn_callsign *callsign, uint64_t now,
	char *reason, size_t reason_len)
{
	struct kn_rf_abuse_source *source;

	if (state == NULL || callsign == NULL)
		return 0;
	source = find_source(state, callsign);
	if (source == NULL)
		return 0;
	clear_expired_ignore(source, now);
	if (source->ignored == 0)
		return 0;
	reason_set(reason, reason_len, source->last_reason[0] == '\0' ?
	    "ignored" : source->last_reason);
	return 1;
}

static int
callsign_equal(const struct kn_callsign *a, const struct kn_callsign *b)
{
	return strcmp(a->call, b->call) == 0 && a->ssid == b->ssid;
}

static void
clear_expired_ignore(struct kn_rf_abuse_source *source, uint64_t now)
{
	if (source->ignored != 0 && source->ignore_until != 0 &&
	    now >= source->ignore_until) {
		source->ignored = 0;
		source->ignore_until = 0;
		source->reject_count = 0;
		source->reject_window_start = now;
		reason_set(source->last_reason, sizeof(source->last_reason),
		    "ignore-expired");
	}
}

static struct kn_rf_abuse_source *
find_source(struct kn_rf_abuse_state *state, const struct kn_callsign *callsign)
{
	size_t i;

	for (i = 0; i < state->count; i++) {
		if (callsign_equal(&state->sources[i].callsign, callsign) != 0)
			return &state->sources[i];
	}

	return NULL;
}

static struct kn_rf_abuse_source *
get_source(struct kn_rf_abuse_state *state, const struct kn_callsign *callsign,
	uint64_t now)
{
	struct kn_rf_abuse_source *source;
	size_t oldest;
	size_t i;

	source = find_source(state, callsign);
	if (source != NULL) {
		if (source->first_seen == 0)
			source->first_seen = now;
		source->last_seen = now;
		return source;
	}

	if (state->max_sources == 0)
		state->max_sources = KN_RF_ABUSE_MAX_SOURCES;
	if (state->count < state->max_sources) {
		source = &state->sources[state->count++];
		memset(source, 0, sizeof(*source));
		source->callsign = *callsign;
		source->first_seen = now;
		source->last_seen = now;
		return source;
	}

	oldest = 0;
	for (i = 1; i < state->count; i++) {
		if (state->sources[i].last_seen < state->sources[oldest].last_seen)
			oldest = i;
	}
	source = &state->sources[oldest];
	memset(source, 0, sizeof(*source));
	source->callsign = *callsign;
	source->first_seen = now;
	source->last_seen = now;
	return source;
}

static void
reason_set(char *dst, size_t dst_len, const char *reason)
{
	int needed;

	if (dst == NULL || dst_len == 0)
		return;

	needed = snprintf(dst, dst_len, "%s",
	    reason == NULL || reason[0] == '\0' ? "-" : reason);
	if (needed < 0 || (size_t)needed >= dst_len)
		dst[0] = '\0';
}
