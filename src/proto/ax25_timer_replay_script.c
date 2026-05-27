/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/proto/ax25_timer_replay_script.c */

#include <sys/types.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kilonode/ax25_connection_key.h"
#include "kilonode/ax25_timer_replay_script.h"

#define TOKEN_MAX 16

static void append_error(struct kn_ax25_timer_replay_error_info *,
	enum kn_ax25_timer_replay_error, size_t, const char *);
static enum kn_ax25_timer_replay_error append_command(
	struct kn_ax25_timer_replay_script *,
	const struct kn_ax25_timer_replay_command *,
	struct kn_ax25_timer_replay_error_info *);
static int bool_from_text(const char *, uint8_t *);
static int counter_from_text(const char *,
	enum kn_ax25_timer_replay_counter *);
static int event_from_text(const char *, enum kn_ax25_connection_event *);
static int plan_from_text(const char *, enum kn_ax25_frame_plan_type *);
static enum kn_ax25_timer_replay_error parse_command(size_t, char **,
	int, struct kn_ax25_timer_replay_script *,
	struct kn_ax25_timer_replay_error_info *);
static enum kn_ax25_timer_replay_error parse_expect(
	struct kn_ax25_timer_replay_command *, int, char **,
	struct kn_ax25_timer_replay_error_info *);
static enum kn_ax25_timer_replay_error parse_prepared_token(
	struct kn_ax25_prepared_expect_frame *, const char *);
static enum kn_ax25_timer_replay_error parse_event(
	struct kn_ax25_timer_replay_command *, int, char **,
	struct kn_ax25_timer_replay_error_info *);
static enum kn_ax25_timer_replay_error parse_params(
	struct kn_ax25_timer_replay_command *, int, char **,
	struct kn_ax25_timer_replay_error_info *);
static int parse_u64(const char *, uint64_t *);
static int split_tokens(char *, char **, size_t, int *);
static int state_from_text(const char *, enum kn_ax25_connection_state *);
static int timer_from_text(const char *, enum kn_ax25_timer_kind *);
static enum kn_ax25_timer_replay_error validate_script(
	const struct kn_ax25_timer_replay_script *,
	struct kn_ax25_timer_replay_error_info *);

static void
append_error(struct kn_ax25_timer_replay_error_info *error,
	enum kn_ax25_timer_replay_error code, size_t line, const char *message)
{
	int needed;

	if (error == NULL)
		return;

	memset(error, 0, sizeof(*error));
	error->error = code;
	error->line = line;
	if (message == NULL)
		return;
	needed = snprintf(error->message, sizeof(error->message), "%s",
	    message);
	if (needed < 0 || (size_t)needed >= sizeof(error->message))
		error->message[sizeof(error->message) - 1] = '\0';
}

static enum kn_ax25_timer_replay_error
append_command(struct kn_ax25_timer_replay_script *script,
	const struct kn_ax25_timer_replay_command *command,
	struct kn_ax25_timer_replay_error_info *error)
{
	if (script->command_count >= KN_AX25_TIMER_REPLAY_COMMAND_MAX) {
		append_error(error, KN_AX25_TIMER_REPLAY_ERR_FULL,
		    command->line, "too-many-commands");
		return KN_AX25_TIMER_REPLAY_ERR_FULL;
	}

	script->commands[script->command_count] = *command;
	script->command_count++;
	return KN_AX25_TIMER_REPLAY_OK;
}

static int
bool_from_text(const char *text, uint8_t *value)
{
	if (text == NULL || value == NULL)
		return -1;
	if (strcmp(text, "true") == 0) {
		*value = 1;
		return 0;
	}
	if (strcmp(text, "false") == 0) {
		*value = 0;
		return 0;
	}

	return -1;
}

static int
counter_from_text(const char *text, enum kn_ax25_timer_replay_counter *counter)
{
	if (text == NULL || counter == NULL)
		return -1;
	if (strcmp(text, "timers-started") == 0)
		*counter = KN_AX25_TIMER_REPLAY_COUNTER_TIMERS_STARTED;
	else if (strcmp(text, "timers-stopped") == 0)
		*counter = KN_AX25_TIMER_REPLAY_COUNTER_TIMERS_STOPPED;
	else if (strcmp(text, "timers-expired") == 0)
		*counter = KN_AX25_TIMER_REPLAY_COUNTER_TIMERS_EXPIRED;
	else if (strcmp(text, "retries-incremented") == 0)
		*counter = KN_AX25_TIMER_REPLAY_COUNTER_RETRIES_INCREMENTED;
	else if (strcmp(text, "retries-exhausted") == 0)
		*counter = KN_AX25_TIMER_REPLAY_COUNTER_RETRIES_EXHAUSTED;
	else if (strcmp(text, "frame-plans") == 0)
		*counter = KN_AX25_TIMER_REPLAY_COUNTER_FRAME_PLANS;
	else if (strcmp(text, "events-accepted") == 0)
		*counter = KN_AX25_TIMER_REPLAY_COUNTER_EVENTS_ACCEPTED;
	else if (strcmp(text, "protocol-errors") == 0)
		*counter = KN_AX25_TIMER_REPLAY_COUNTER_PROTOCOL_ERRORS;
	else
		return -1;

	return 0;
}

static int
event_from_text(const char *text, enum kn_ax25_connection_event *event)
{
	if (text == NULL || event == NULL)
		return -1;
	if (strcmp(text, "local-connect") == 0)
		*event = KN_AX25_CONNECTION_EVENT_LOCAL_CONNECT_REQUEST;
	else if (strcmp(text, "local-disconnect") == 0)
		*event = KN_AX25_CONNECTION_EVENT_LOCAL_DISCONNECT_REQUEST;
	else if (strcmp(text, "rx-sabm") == 0)
		*event = KN_AX25_CONNECTION_EVENT_RX_SABM;
	else if (strcmp(text, "rx-sabme") == 0)
		*event = KN_AX25_CONNECTION_EVENT_RX_SABME;
	else if (strcmp(text, "rx-ua") == 0)
		*event = KN_AX25_CONNECTION_EVENT_RX_UA;
	else if (strcmp(text, "rx-disc") == 0)
		*event = KN_AX25_CONNECTION_EVENT_RX_DISC;
	else if (strcmp(text, "rx-dm") == 0)
		*event = KN_AX25_CONNECTION_EVENT_RX_DM;
	else if (strcmp(text, "rx-rr") == 0)
		*event = KN_AX25_CONNECTION_EVENT_RX_RR;
	else if (strcmp(text, "rx-rnr") == 0)
		*event = KN_AX25_CONNECTION_EVENT_RX_RNR;
	else if (strcmp(text, "rx-rej") == 0)
		*event = KN_AX25_CONNECTION_EVENT_RX_REJ;
	else if (strcmp(text, "rx-i") == 0)
		*event = KN_AX25_CONNECTION_EVENT_RX_I;
	else
		return -1;

	return 0;
}

static int
plan_from_text(const char *text, enum kn_ax25_frame_plan_type *plan)
{
	int i;

	if (text == NULL || plan == NULL)
		return -1;
	for (i = 0; i < (int)KN_AX25_FRAME_PLAN_UNKNOWN; i++) {
		if (strcmp(text, kn_ax25_frame_plan_type_name(
		    (enum kn_ax25_frame_plan_type)i)) == 0) {
			*plan = (enum kn_ax25_frame_plan_type)i;
			return 0;
		}
	}

	return -1;
}

static enum kn_ax25_timer_replay_error
parse_command(size_t line, char **tokens, int count,
	struct kn_ax25_timer_replay_script *script,
	struct kn_ax25_timer_replay_error_info *error)
{
	struct kn_ax25_timer_replay_command command;
	uint64_t value;
	int needed;

	memset(&command, 0, sizeof(command));
	command.line = line;
	if (strcmp(tokens[0], "name") == 0 && count == 2) {
		needed = snprintf(script->name, sizeof(script->name), "%s",
		    tokens[1]);
		if (needed < 0 || (size_t)needed >= sizeof(script->name))
			goto invalid;
		return KN_AX25_TIMER_REPLAY_OK;
	}
	if (strcmp(tokens[0], "node") == 0 && count == 2) {
		needed = snprintf(script->node, sizeof(script->node), "%s",
		    tokens[1]);
		if (needed < 0 || (size_t)needed >= sizeof(script->node))
			goto invalid;
		return KN_AX25_TIMER_REPLAY_OK;
	}
	if (strcmp(tokens[0], "remote") == 0 && count == 2) {
		needed = snprintf(script->remote, sizeof(script->remote), "%s",
		    tokens[1]);
		if (needed < 0 || (size_t)needed >= sizeof(script->remote))
			goto invalid;
		return KN_AX25_TIMER_REPLAY_OK;
	}
	if (strcmp(tokens[0], "port") == 0 && count == 2) {
		needed = snprintf(script->port, sizeof(script->port), "%s",
		    tokens[1]);
		if (needed < 0 || (size_t)needed >= sizeof(script->port))
			goto invalid;
		return KN_AX25_TIMER_REPLAY_OK;
	}
	if (strcmp(tokens[0], "now") == 0 && count == 2) {
		if (parse_u64(tokens[1], &value) != 0)
			goto invalid;
		command.type = KN_AX25_TIMER_REPLAY_CMD_NOW;
		command.value = value;
		return append_command(script, &command, error);
	}
	if (strcmp(tokens[0], "advance") == 0 && count == 2) {
		if (parse_u64(tokens[1], &value) != 0)
			goto invalid;
		command.type = KN_AX25_TIMER_REPLAY_CMD_ADVANCE;
		command.value = value;
		return append_command(script, &command, error);
	}
	if (strcmp(tokens[0], "params") == 0)
		return parse_params(&command, count, tokens, error) ==
		    KN_AX25_TIMER_REPLAY_OK ?
		    append_command(script, &command, error) :
		    KN_AX25_TIMER_REPLAY_ERR_PARSE;
	if (strcmp(tokens[0], "event") == 0)
		return parse_event(&command, count, tokens, error) ==
		    KN_AX25_TIMER_REPLAY_OK ?
		    append_command(script, &command, error) :
		    KN_AX25_TIMER_REPLAY_ERR_PARSE;
	if (strcmp(tokens[0], "process-expired") == 0 && count == 1) {
		command.type = KN_AX25_TIMER_REPLAY_CMD_PROCESS_EXPIRED;
		return append_command(script, &command, error);
	}
	if (strcmp(tokens[0], "expect") == 0)
		return parse_expect(&command, count, tokens, error) ==
		    KN_AX25_TIMER_REPLAY_OK ?
		    append_command(script, &command, error) :
		    KN_AX25_TIMER_REPLAY_ERR_PARSE;

invalid:
	append_error(error, KN_AX25_TIMER_REPLAY_ERR_PARSE, line,
	    "invalid-command");
	return KN_AX25_TIMER_REPLAY_ERR_PARSE;
}

static enum kn_ax25_timer_replay_error
parse_expect(struct kn_ax25_timer_replay_command *command, int count,
	char **tokens, struct kn_ax25_timer_replay_error_info *error)
{
	char *value;
	uint64_t number;
	int needed;

	if (command == NULL || count < 2)
		goto invalid;
	command->type = KN_AX25_TIMER_REPLAY_CMD_EXPECT;
	if (strncmp(tokens[1], "state=", 6) == 0) {
		command->expect.type = KN_AX25_TIMER_REPLAY_EXPECT_STATE;
		if (state_from_text(tokens[1] + 6,
		    &command->expect.state) != 0)
			goto invalid;
		return KN_AX25_TIMER_REPLAY_OK;
	}
	if (strncmp(tokens[1], "retry=", 6) == 0) {
		command->expect.type = KN_AX25_TIMER_REPLAY_EXPECT_RETRY;
		if (parse_u64(tokens[1] + 6, &command->expect.value) != 0)
			goto invalid;
		return KN_AX25_TIMER_REPLAY_OK;
	}
	if (strcmp(tokens[1], "timer-running") == 0 && count == 4) {
		command->expect.type =
		    KN_AX25_TIMER_REPLAY_EXPECT_TIMER_RUNNING;
		if (timer_from_text(tokens[2],
		    &command->expect.timer_kind) != 0 ||
		    bool_from_text(tokens[3], &command->expect.bool_value) !=
		    0)
			goto invalid;
		return KN_AX25_TIMER_REPLAY_OK;
	}
	if (strcmp(tokens[1], "timer-expired") == 0 && count == 4) {
		command->expect.type =
		    KN_AX25_TIMER_REPLAY_EXPECT_TIMER_EXPIRED;
		if (timer_from_text(tokens[2],
		    &command->expect.timer_kind) != 0 ||
		    bool_from_text(tokens[3], &command->expect.bool_value) !=
		    0)
			goto invalid;
		return KN_AX25_TIMER_REPLAY_OK;
	}
	if (strcmp(tokens[1], "action") == 0 && count == 3) {
		command->expect.type = KN_AX25_TIMER_REPLAY_EXPECT_ACTION;
		for (number = 0; number <=
		    (uint64_t)KN_AX25_ACTION_RETRANSMIT_NEEDED; number++) {
			if (strcmp(tokens[2], kn_ax25_action_intent_name(
			    (enum kn_ax25_action_intent)number)) == 0) {
				command->expect.action =
				    (enum kn_ax25_action_intent)number;
				return KN_AX25_TIMER_REPLAY_OK;
			}
		}
		goto invalid;
	}
	if (strcmp(tokens[1], "plan") == 0 && count == 3) {
		command->expect.type = KN_AX25_TIMER_REPLAY_EXPECT_PLAN;
		if (plan_from_text(tokens[2], &command->expect.plan) != 0)
			goto invalid;
		return KN_AX25_TIMER_REPLAY_OK;
	}
	if (strcmp(tokens[1], "counter") == 0 && count == 4) {
		command->expect.type = KN_AX25_TIMER_REPLAY_EXPECT_COUNTER;
		if (counter_from_text(tokens[2], &command->expect.counter) !=
		    0 || parse_u64(tokens[3], &command->expect.value) != 0)
			goto invalid;
		return KN_AX25_TIMER_REPLAY_OK;
	}
	if (strcmp(tokens[1], "tx-writes") == 0 && count == 3) {
		command->expect.type = KN_AX25_TIMER_REPLAY_EXPECT_TX_WRITES;
		if (parse_u64(tokens[2], &command->expect.value) != 0)
			goto invalid;
		return KN_AX25_TIMER_REPLAY_OK;
	}
	if (strcmp(tokens[1], "connection-count") == 0 && count == 3) {
		command->expect.type =
		    KN_AX25_TIMER_REPLAY_EXPECT_CONNECTION_COUNT;
		if (parse_u64(tokens[2], &command->expect.value) != 0)
			goto invalid;
		return KN_AX25_TIMER_REPLAY_OK;
	}
	if (strcmp(tokens[1], "last-error") == 0 && count == 3) {
		command->expect.type = KN_AX25_TIMER_REPLAY_EXPECT_LAST_ERROR;
		needed = snprintf(command->expect.last_error,
		    sizeof(command->expect.last_error), "%s", tokens[2]);
		if (needed < 0 ||
		    (size_t)needed >= sizeof(command->expect.last_error))
			goto invalid;
		return KN_AX25_TIMER_REPLAY_OK;
	}
	if (strcmp(tokens[1], "prepared-count") == 0 && count == 3) {
		command->expect.type =
		    KN_AX25_TIMER_REPLAY_EXPECT_PREPARED_COUNT;
		if (parse_u64(tokens[2], &command->expect.value) != 0)
			goto invalid;
		return KN_AX25_TIMER_REPLAY_OK;
	}
	if (strcmp(tokens[1], "prepared") == 0 && count >= 3) {
		command->expect.type = KN_AX25_TIMER_REPLAY_EXPECT_PREPARED;
		command->expect.prepared.line = command->line;
		for (needed = 2; needed < count; needed++) {
			if (parse_prepared_token(&command->expect.prepared,
			    tokens[needed]) != KN_AX25_TIMER_REPLAY_OK)
				goto invalid;
		}
		return KN_AX25_TIMER_REPLAY_OK;
	}
	value = strchr(tokens[1], '=');
	if (value != NULL)
		goto invalid;

invalid:
	append_error(error, KN_AX25_TIMER_REPLAY_ERR_PARSE, command->line,
	    "invalid-expectation");
	return KN_AX25_TIMER_REPLAY_ERR_PARSE;
}

static enum kn_ax25_timer_replay_error
parse_prepared_token(struct kn_ax25_prepared_expect_frame *frame,
	const char *token)
{
	struct kn_callsign callsign;
	uint64_t value;
	int needed;

	if (frame == NULL || token == NULL)
		return KN_AX25_TIMER_REPLAY_ERR_INVALID_ARGUMENT;
	if (strncmp(token, "kind=", 5) == 0) {
		if (kn_ax25_prepared_expect_kind_from_text(token + 5,
		    &frame->kind) != 0)
			return KN_AX25_TIMER_REPLAY_ERR_PARSE;
		frame->has_kind = 1;
	} else if (strncmp(token, "action=", 7) == 0) {
		if (kn_ax25_prepared_expect_action_from_text(token + 7,
		    &frame->action) != 0)
			return KN_AX25_TIMER_REPLAY_ERR_PARSE;
		frame->has_action = 1;
	} else if (strncmp(token, "status=", 7) == 0) {
		if (kn_ax25_prepared_expect_status_from_text(token + 7,
		    &frame->status) != 0)
			return KN_AX25_TIMER_REPLAY_ERR_PARSE;
		frame->has_status = 1;
	} else if (strncmp(token, "local=", 6) == 0) {
		if (kn_callsign_parse(token + 6, &callsign) != 0)
			return KN_AX25_TIMER_REPLAY_ERR_PARSE;
		(void)snprintf(frame->local, sizeof(frame->local), "%s",
		    token + 6);
		frame->has_local = 1;
	} else if (strncmp(token, "remote=", 7) == 0) {
		if (kn_callsign_parse(token + 7, &callsign) != 0)
			return KN_AX25_TIMER_REPLAY_ERR_PARSE;
		(void)snprintf(frame->remote, sizeof(frame->remote), "%s",
		    token + 7);
		frame->has_remote = 1;
	} else if (strncmp(token, "port=", 5) == 0) {
		if (token[5] == '\0' || token[5] == '/' ||
		    strstr(token + 5, "..") != NULL ||
		    strchr(token + 5, '\\') != NULL)
			return KN_AX25_TIMER_REPLAY_ERR_PARSE;
		needed = snprintf(frame->port, sizeof(frame->port), "%s",
		    token + 5);
		if (needed < 0 || (size_t)needed >= sizeof(frame->port))
			return KN_AX25_TIMER_REPLAY_ERR_PARSE;
		frame->has_port = 1;
	} else if (strncmp(token, "ax25-len=", 9) == 0) {
		if (parse_u64(token + 9, &value) != 0)
			return KN_AX25_TIMER_REPLAY_ERR_PARSE;
		frame->ax25_len = (size_t)value;
		frame->has_ax25_len = 1;
	} else if (strncmp(token, "tx-writes=", 10) == 0) {
		if (parse_u64(token + 10, &value) != 0 || value != 0)
			return KN_AX25_TIMER_REPLAY_ERR_PARSE;
		frame->tx_writes = value;
		frame->has_tx_writes = 1;
	} else {
		return KN_AX25_TIMER_REPLAY_ERR_PARSE;
	}

	return KN_AX25_TIMER_REPLAY_OK;
}

static enum kn_ax25_timer_replay_error
parse_event(struct kn_ax25_timer_replay_command *command, int count,
	char **tokens, struct kn_ax25_timer_replay_error_info *error)
{
	uint64_t value;
	int i;

	if (command == NULL || count < 2 ||
	    event_from_text(tokens[1], &command->event) != 0)
		goto invalid;
	command->type = KN_AX25_TIMER_REPLAY_CMD_EVENT;
	for (i = 2; i < count; i++) {
		if (strncmp(tokens[i], "ns=", 3) == 0) {
			if (parse_u64(tokens[i] + 3, &value) != 0 ||
			    value > 7)
				goto invalid;
			command->ns = (uint8_t)value;
		} else if (strncmp(tokens[i], "nr=", 3) == 0) {
			if (parse_u64(tokens[i] + 3, &value) != 0 ||
			    value > 7)
				goto invalid;
			command->nr = (uint8_t)value;
		} else if (strncmp(tokens[i], "len=", 4) == 0) {
			if (parse_u64(tokens[i] + 4, &value) != 0)
				goto invalid;
			command->payload_len = (size_t)value;
		} else if (strncmp(tokens[i], "pf=", 3) == 0) {
			if (parse_u64(tokens[i] + 3, &value) != 0 ||
			    value > 1)
				goto invalid;
			command->poll_final = (uint8_t)value;
		} else {
			goto invalid;
		}
	}
	return KN_AX25_TIMER_REPLAY_OK;

invalid:
	append_error(error, KN_AX25_TIMER_REPLAY_ERR_PARSE, command->line,
	    "invalid-event");
	return KN_AX25_TIMER_REPLAY_ERR_PARSE;
}

static enum kn_ax25_timer_replay_error
parse_params(struct kn_ax25_timer_replay_command *command, int count,
	char **tokens, struct kn_ax25_timer_replay_error_info *error)
{
	uint64_t value;
	int i;

	if (command == NULL || count < 2)
		goto invalid;
	command->type = KN_AX25_TIMER_REPLAY_CMD_PARAMS;
	kn_ax25_params_default(&command->params);
	command->params.allow_connected_mode = 1;
	for (i = 1; i < count; i++) {
		if (strcmp(tokens[i], "modulo=8") == 0) {
			command->params.modulo_mode = KN_AX25_MODULO_8;
		} else if (strncmp(tokens[i], "window=", 7) == 0) {
			if (parse_u64(tokens[i] + 7, &value) != 0 ||
			    value > UINT8_MAX)
				goto invalid;
			command->params.window_size = (uint8_t)value;
		} else if (strncmp(tokens[i], "t1=", 3) == 0) {
			if (parse_u64(tokens[i] + 3, &value) != 0 ||
			    value > UINT32_MAX)
				goto invalid;
			command->params.t1_ms = (uint32_t)value;
		} else if (strncmp(tokens[i], "t2=", 3) == 0) {
			if (parse_u64(tokens[i] + 3, &value) != 0 ||
			    value > UINT32_MAX)
				goto invalid;
			command->params.t2_ms = (uint32_t)value;
		} else if (strncmp(tokens[i], "t3=", 3) == 0) {
			if (parse_u64(tokens[i] + 3, &value) != 0 ||
			    value > UINT32_MAX)
				goto invalid;
			command->params.t3_ms = (uint32_t)value;
		} else if (strncmp(tokens[i], "n2=", 3) == 0) {
			if (parse_u64(tokens[i] + 3, &value) != 0 ||
			    value > UINT8_MAX)
				goto invalid;
			command->params.n2_retry_count = (uint8_t)value;
		} else {
			goto invalid;
		}
	}
	if (kn_ax25_params_validate(&command->params) != KN_AX25_PARAMS_OK)
		goto invalid;
	return KN_AX25_TIMER_REPLAY_OK;

invalid:
	append_error(error, KN_AX25_TIMER_REPLAY_ERR_PARSE, command->line,
	    "invalid-params");
	return KN_AX25_TIMER_REPLAY_ERR_PARSE;
}

static int
parse_u64(const char *text, uint64_t *value)
{
	char *end;
	unsigned long long parsed;

	if (text == NULL || value == NULL || text[0] == '\0' ||
	    text[0] == '-')
		return -1;
	errno = 0;
	parsed = strtoull(text, &end, 10);
	if (errno != 0 || end == text || *end != '\0')
		return -1;

	*value = (uint64_t)parsed;
	return 0;
}

static int
split_tokens(char *line, char **tokens, size_t tokens_len, int *count)
{
	char *token;
	size_t used;

	if (line == NULL || tokens == NULL || count == NULL)
		return -1;
	used = 0;
	token = strtok(line, " \t\r\n");
	while (token != NULL) {
		if (used >= tokens_len)
			return -1;
		tokens[used] = token;
		used++;
		token = strtok(NULL, " \t\r\n");
	}
	*count = (int)used;
	return 0;
}

static int
state_from_text(const char *text, enum kn_ax25_connection_state *state)
{
	int i;

	if (text == NULL || state == NULL)
		return -1;
	for (i = (int)KN_AX25_CONNECTION_DISABLED;
	    i <= (int)KN_AX25_CONNECTION_TIMER_RECOVERY; i++) {
		if (strcmp(text, kn_ax25_connection_state_name(
		    (enum kn_ax25_connection_state)i)) == 0) {
			*state = (enum kn_ax25_connection_state)i;
			return 0;
		}
	}

	return -1;
}

static int
timer_from_text(const char *text, enum kn_ax25_timer_kind *kind)
{
	if (text == NULL || kind == NULL)
		return -1;
	if (strcmp(text, "T1") == 0 || strcmp(text, "t1") == 0)
		*kind = KN_AX25_TIMER_T1;
	else if (strcmp(text, "T2") == 0 || strcmp(text, "t2") == 0)
		*kind = KN_AX25_TIMER_T2;
	else if (strcmp(text, "T3") == 0 || strcmp(text, "t3") == 0)
		*kind = KN_AX25_TIMER_T3;
	else
		return -1;

	return 0;
}

static enum kn_ax25_timer_replay_error
validate_script(const struct kn_ax25_timer_replay_script *script,
	struct kn_ax25_timer_replay_error_info *error)
{
	struct kn_ax25_connection_key key;

	if (script->name[0] == '\0') {
		append_error(error, KN_AX25_TIMER_REPLAY_ERR_PARSE, 0,
		    "missing-name");
		return KN_AX25_TIMER_REPLAY_ERR_PARSE;
	}
	if (script->node[0] == '\0') {
		append_error(error, KN_AX25_TIMER_REPLAY_ERR_PARSE, 0,
		    "missing-node");
		return KN_AX25_TIMER_REPLAY_ERR_PARSE;
	}
	if (script->remote[0] == '\0') {
		append_error(error, KN_AX25_TIMER_REPLAY_ERR_PARSE, 0,
		    "missing-remote");
		return KN_AX25_TIMER_REPLAY_ERR_PARSE;
	}
	if (script->port[0] == '\0') {
		append_error(error, KN_AX25_TIMER_REPLAY_ERR_PARSE, 0,
		    "missing-port");
		return KN_AX25_TIMER_REPLAY_ERR_PARSE;
	}
	if (kn_ax25_connection_key_from_callsigns(&key, script->port,
	    script->node, script->remote, NULL, 0) !=
	    KN_AX25_CONNECTION_KEY_OK) {
		append_error(error, KN_AX25_TIMER_REPLAY_ERR_PARSE, 0,
		    "invalid-key");
		return KN_AX25_TIMER_REPLAY_ERR_PARSE;
	}

	return KN_AX25_TIMER_REPLAY_OK;
}

void
kn_ax25_timer_replay_error_clear(
	struct kn_ax25_timer_replay_error_info *error)
{
	if (error == NULL)
		return;

	memset(error, 0, sizeof(*error));
}

const char *
kn_ax25_timer_replay_error_name(enum kn_ax25_timer_replay_error error)
{
	switch (error) {
	case KN_AX25_TIMER_REPLAY_OK:
		return "ok";
	case KN_AX25_TIMER_REPLAY_ERR_INVALID_ARGUMENT:
		return "invalid-argument";
	case KN_AX25_TIMER_REPLAY_ERR_INVALID_VALUE:
		return "invalid-value";
	case KN_AX25_TIMER_REPLAY_ERR_PARSE:
		return "parse";
	case KN_AX25_TIMER_REPLAY_ERR_IO:
		return "io";
	case KN_AX25_TIMER_REPLAY_ERR_FULL:
		return "full";
	case KN_AX25_TIMER_REPLAY_ERR_MISMATCH:
		return "mismatch";
	case KN_AX25_TIMER_REPLAY_ERR_BUFFER:
		return "buffer";
	}

	return "unknown";
}

void
kn_ax25_timer_replay_script_clear(struct kn_ax25_timer_replay_script *script)
{
	if (script == NULL)
		return;

	memset(script, 0, sizeof(*script));
}

enum kn_ax25_timer_replay_error
kn_ax25_timer_replay_script_parse_file(const char *path,
	struct kn_ax25_timer_replay_script *script,
	struct kn_ax25_timer_replay_error_info *error)
{
	FILE *fp;
	char line[KN_AX25_TIMER_REPLAY_LINE_MAX];
	char work[KN_AX25_TIMER_REPLAY_LINE_MAX];
	char *tokens[TOKEN_MAX];
	char *start;
	size_t line_no;
	size_t len;
	int count;

	if (path == NULL || script == NULL)
		return KN_AX25_TIMER_REPLAY_ERR_INVALID_ARGUMENT;

	kn_ax25_timer_replay_script_clear(script);
	kn_ax25_timer_replay_error_clear(error);
	fp = fopen(path, "r");
	if (fp == NULL) {
		append_error(error, KN_AX25_TIMER_REPLAY_ERR_IO, 0,
		    "open-failed");
		return KN_AX25_TIMER_REPLAY_ERR_IO;
	}

	line_no = 0;
	while (fgets(line, sizeof(line), fp) != NULL) {
		line_no++;
		len = strlen(line);
		if (len > 0 && line[len - 1] != '\n' && !feof(fp)) {
			(void)fclose(fp);
			append_error(error, KN_AX25_TIMER_REPLAY_ERR_PARSE,
			    line_no, "line-too-long");
			return KN_AX25_TIMER_REPLAY_ERR_PARSE;
		}
		start = line;
		while (*start == ' ' || *start == '\t')
			start++;
		if (*start == '#' || *start == '\n' || *start == '\0')
			continue;
		(void)snprintf(work, sizeof(work), "%s", start);
		if (split_tokens(work, tokens, TOKEN_MAX, &count) != 0 ||
		    count == 0) {
			(void)fclose(fp);
			append_error(error, KN_AX25_TIMER_REPLAY_ERR_PARSE,
			    line_no, "tokenize");
			return KN_AX25_TIMER_REPLAY_ERR_PARSE;
		}
		if (parse_command(line_no, tokens, count, script, error) !=
		    KN_AX25_TIMER_REPLAY_OK) {
			(void)fclose(fp);
			return error != NULL ? error->error :
			    KN_AX25_TIMER_REPLAY_ERR_PARSE;
		}
	}
	if (ferror(fp)) {
		(void)fclose(fp);
		append_error(error, KN_AX25_TIMER_REPLAY_ERR_IO, line_no,
		    "read-failed");
		return KN_AX25_TIMER_REPLAY_ERR_IO;
	}
	(void)fclose(fp);

	return validate_script(script, error);
}
