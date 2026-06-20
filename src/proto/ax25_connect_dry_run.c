/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/proto/ax25_connect_dry_run.c */

#include <sys/types.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kilonode/ax25_connect_dry_run.h"
#include "kilonode/callsign.h"

static int append_command(struct kn_ax25_connect_dry_run_script *,
	const struct kn_ax25_connect_dry_run_command *);
static void error_set(struct kn_ax25_connect_dry_run_error_info *,
	enum kn_ax25_connect_dry_run_error, size_t, const char *);
static int expect_from_tokens(char *[], size_t,
	struct kn_ax25_connect_dry_run_command *);
static const char *frame_kind_from_params(const struct kn_ax25_params *);
static int parse_params(char *[], size_t, struct kn_ax25_params *);
static int parse_u64(const char *, uint64_t *);
static int split_tokens(char *, char *[], size_t, size_t *);
static int state_from_text(const char *, enum kn_ax25_connection_state *);
static char *trim(char *);

static int
append_command(struct kn_ax25_connect_dry_run_script *script,
	const struct kn_ax25_connect_dry_run_command *command)
{
	if (script->command_count >= KN_AX25_CONNECT_DRY_RUN_COMMAND_MAX)
		return -1;
	script->commands[script->command_count++] = *command;
	return 0;
}

static void
error_set(struct kn_ax25_connect_dry_run_error_info *error,
	enum kn_ax25_connect_dry_run_error rc, size_t line,
	const char *message)
{
	int needed;

	if (error == NULL)
		return;
	memset(error, 0, sizeof(*error));
	error->error = rc;
	error->line = line;
	if (message == NULL)
		message = "";
	needed = snprintf(error->message, sizeof(error->message), "%s",
	    message);
	if (needed < 0 || (size_t)needed >= sizeof(error->message))
		error->message[sizeof(error->message) - 1] = '\0';
}

static int
expect_from_tokens(char *tokens[], size_t count,
	struct kn_ax25_connect_dry_run_command *command)
{
	uint64_t value;

	if (count != 3)
		return -1;
	if (strcmp(tokens[1], "planned-state") == 0) {
		command->expect =
		    KN_AX25_CONNECT_DRY_RUN_EXPECT_PLANNED_STATE;
		return state_from_text(tokens[2], &command->state);
	}
	if (strcmp(tokens[1], "frame-kind") == 0) {
		if (strcmp(tokens[2], "SABM") != 0 &&
		    strcmp(tokens[2], "SABME") != 0)
			return -1;
		command->expect = KN_AX25_CONNECT_DRY_RUN_EXPECT_FRAME_KIND;
		(void)snprintf(command->text, sizeof(command->text), "%s",
		    tokens[2]);
		return 0;
	}
	if (strcmp(tokens[1], "bridge") == 0) {
		if (strcmp(tokens[2], "blocked") != 0)
			return -1;
		command->expect = KN_AX25_CONNECT_DRY_RUN_EXPECT_BRIDGE;
		command->value = 1;
		return 0;
	}
	if (strcmp(tokens[1], "connection-created") == 0) {
		if (strcmp(tokens[2], "false") == 0)
			value = 0;
		else if (strcmp(tokens[2], "true") == 0)
			value = 1;
		else
			return -1;
		command->expect =
		    KN_AX25_CONNECT_DRY_RUN_EXPECT_CONNECTION_CREATED;
		command->value = value;
		return 0;
	}
	if (strcmp(tokens[1], "tx-writes") == 0) {
		if (parse_u64(tokens[2], &value) != 0)
			return -1;
		command->expect = KN_AX25_CONNECT_DRY_RUN_EXPECT_TX_WRITES;
		command->value = value;
		return 0;
	}
	if (strcmp(tokens[1], "dispatch-calls") == 0) {
		if (parse_u64(tokens[2], &value) != 0)
			return -1;
		command->expect =
		    KN_AX25_CONNECT_DRY_RUN_EXPECT_DISPATCH_CALLS;
		command->value = value;
		return 0;
	}
	if (strcmp(tokens[1], "fx25-frames") == 0) {
		if (parse_u64(tokens[2], &value) != 0)
			return -1;
		command->expect = KN_AX25_CONNECT_DRY_RUN_EXPECT_FX25_FRAMES;
		command->value = value;
		return 0;
	}
	return -1;
}

static const char *
frame_kind_from_params(const struct kn_ax25_params *params)
{
	if (params->modulo_mode == KN_AX25_MODULO_128)
		return "SABME";
	return "SABM";
}

static int
parse_params(char *tokens[], size_t count, struct kn_ax25_params *params)
{
	size_t i;
	uint64_t value;

	if (params == NULL || count < 2)
		return -1;
	kn_ax25_params_default(params);
	params->allow_connected_mode = 1;
	for (i = 1; i < count; i++) {
		if (strncmp(tokens[i], "modulo=", 7) == 0) {
			if (strcmp(tokens[i] + 7, "8") == 0)
				params->modulo_mode = KN_AX25_MODULO_8;
			else if (strcmp(tokens[i] + 7, "128") == 0)
				params->modulo_mode = KN_AX25_MODULO_128;
			else
				return -1;
		} else if (strncmp(tokens[i], "window=", 7) == 0) {
			if (parse_u64(tokens[i] + 7, &value) != 0 ||
			    value > 255)
				return -1;
			params->window_size = (uint8_t)value;
		} else if (strncmp(tokens[i], "paclen=", 7) == 0) {
			if (parse_u64(tokens[i] + 7, &value) != 0 ||
			    (uint64_t)(size_t)value != value)
				return -1;
			params->paclen = (size_t)value;
		} else if (strncmp(tokens[i], "max-info=", 9) == 0) {
			if (parse_u64(tokens[i] + 9, &value) != 0 ||
			    (uint64_t)(size_t)value != value)
				return -1;
			params->max_info_len = (size_t)value;
		} else if (strncmp(tokens[i], "t1=", 3) == 0) {
			if (parse_u64(tokens[i] + 3, &value) != 0 ||
			    value > UINT32_MAX)
				return -1;
			params->t1_ms = (uint32_t)value;
		} else if (strncmp(tokens[i], "t2=", 3) == 0) {
			if (parse_u64(tokens[i] + 3, &value) != 0 ||
			    value > UINT32_MAX)
				return -1;
			params->t2_ms = (uint32_t)value;
		} else if (strncmp(tokens[i], "t3=", 3) == 0) {
			if (parse_u64(tokens[i] + 3, &value) != 0 ||
			    value > UINT32_MAX)
				return -1;
			params->t3_ms = (uint32_t)value;
		} else if (strncmp(tokens[i], "n2=", 3) == 0) {
			if (parse_u64(tokens[i] + 3, &value) != 0 ||
			    value > 255)
				return -1;
			params->n2_retry_count = (uint8_t)value;
		} else {
			return -1;
		}
	}
	return kn_ax25_params_validate(params) == KN_AX25_PARAMS_OK ? 0 : -1;
}

static int
parse_u64(const char *text, uint64_t *value)
{
	char *end;
	unsigned long long parsed;

	if (text == NULL || text[0] == '\0' || value == NULL)
		return -1;
	errno = 0;
	parsed = strtoull(text, &end, 10);
	if (errno != 0 || end == text || *end != '\0')
		return -1;
	*value = (uint64_t)parsed;
	return 0;
}

static int
split_tokens(char *line, char *tokens[], size_t max_tokens, size_t *count)
{
	char *p;

	if (line == NULL || tokens == NULL || count == NULL ||
	    max_tokens == 0)
		return -1;
	*count = 0;
	p = line;
	while (*p != '\0') {
		while (*p == ' ' || *p == '\t') {
			*p = '\0';
			p++;
		}
		if (*p == '\0')
			break;
		if (*count >= max_tokens)
			return -1;
		tokens[(*count)++] = p;
		while (*p != '\0' && *p != ' ' && *p != '\t')
			p++;
	}
	return 0;
}

static int
state_from_text(const char *text, enum kn_ax25_connection_state *state)
{
	enum kn_ax25_connection_state i;

	for (i = KN_AX25_CONNECTION_DISABLED;
	    i <= KN_AX25_CONNECTION_TIMER_RECOVERY; i++) {
		if (strcmp(text, kn_ax25_connection_state_name(i)) == 0) {
			*state = i;
			return 0;
		}
	}
	return -1;
}

static char *
trim(char *line)
{
	size_t len;

	while (*line == ' ' || *line == '\t')
		line++;
	len = strlen(line);
	while (len > 0 && (line[len - 1] == '\n' ||
	    line[len - 1] == '\r' || line[len - 1] == ' ' ||
	    line[len - 1] == '\t')) {
		line[len - 1] = '\0';
		len--;
	}
	return line;
}

const char *
kn_ax25_connect_dry_run_error_name(
	enum kn_ax25_connect_dry_run_error error)
{
	switch (error) {
	case KN_AX25_CONNECT_DRY_RUN_OK:
		return "ok";
	case KN_AX25_CONNECT_DRY_RUN_ERR_INVALID_ARGUMENT:
		return "invalid-argument";
	case KN_AX25_CONNECT_DRY_RUN_ERR_PARSE:
		return "parse";
	case KN_AX25_CONNECT_DRY_RUN_ERR_IO:
		return "io";
	case KN_AX25_CONNECT_DRY_RUN_ERR_MISMATCH:
		return "mismatch";
	case KN_AX25_CONNECT_DRY_RUN_ERR_BUFFER:
		return "buffer";
	}
	return "unknown";
}

enum kn_ax25_connect_dry_run_error
kn_ax25_connect_dry_run_format_report(
	const struct kn_ax25_connect_dry_run_result *result, char *buf,
	size_t bufsiz)
{
	int needed;

	if (result == NULL || buf == NULL || bufsiz == 0)
		return KN_AX25_CONNECT_DRY_RUN_ERR_INVALID_ARGUMENT;
	needed = snprintf(buf, bufsiz,
	    "AX25-CONNECT-DRY-RUN name=%s result=%s local=%s remote=%s "
	    "port=%s planned_state=%s frame=%s bridge=%s "
	    "connection_created=%s tx_writes=%llu dispatch=%llu "
	    "fx25=%llu mismatches=%llu first_mismatch_line=%llu last=%s\n",
	    result->name,
	    result->pass != 0 ? "pass" : "fail",
	    result->local,
	    result->remote,
	    result->port,
	    kn_ax25_connection_state_name(result->planned_state),
	    result->frame_kind,
	    result->bridge_blocked != 0 ? "blocked" : "allowed",
	    result->connection_created != 0 ? "true" : "false",
	    (unsigned long long)result->tx_writes,
	    (unsigned long long)result->dispatch_calls,
	    (unsigned long long)result->fx25_frames,
	    (unsigned long long)result->mismatch_count,
	    (unsigned long long)result->first_mismatch_line,
	    result->last_error[0] == '\0' ? "-" : result->last_error);
	if (needed < 0 || (size_t)needed >= bufsiz)
		return KN_AX25_CONNECT_DRY_RUN_ERR_BUFFER;
	return KN_AX25_CONNECT_DRY_RUN_OK;
}

enum kn_ax25_connect_dry_run_error
kn_ax25_connect_dry_run_parse_file(const char *path,
	struct kn_ax25_connect_dry_run_script *script,
	struct kn_ax25_connect_dry_run_error_info *error)
{
	FILE *fp;
	char line[KN_AX25_CONNECT_DRY_RUN_LINE_MAX + 2];
	char *tokens[16];
	char *text;
	size_t line_no;
	size_t count;
	struct kn_ax25_connect_dry_run_command command;
	struct kn_callsign callsign;

	if (path == NULL || script == NULL)
		return KN_AX25_CONNECT_DRY_RUN_ERR_INVALID_ARGUMENT;
	memset(script, 0, sizeof(*script));
	kn_ax25_params_default(&script->params);
	script->params.allow_connected_mode = 1;
	fp = fopen(path, "r");
	if (fp == NULL) {
		error_set(error, KN_AX25_CONNECT_DRY_RUN_ERR_IO, 0, "open");
		return KN_AX25_CONNECT_DRY_RUN_ERR_IO;
	}
	line_no = 0;
	while (fgets(line, sizeof(line), fp) != NULL) {
		line_no++;
		if (strchr(line, '\n') == NULL && !feof(fp)) {
			(void)fclose(fp);
			error_set(error, KN_AX25_CONNECT_DRY_RUN_ERR_PARSE,
			    line_no, "line-too-long");
			return KN_AX25_CONNECT_DRY_RUN_ERR_PARSE;
		}
		text = trim(line);
		if (text[0] == '\0' || text[0] == '#')
			continue;
		if (split_tokens(text, tokens, 16, &count) != 0)
			goto parse_error;
		memset(&command, 0, sizeof(command));
		command.line = line_no;
		if (strcmp(tokens[0], "name") == 0 && count == 2) {
			(void)snprintf(script->name, sizeof(script->name),
			    "%s", tokens[1]);
			continue;
		}
		if (strcmp(tokens[0], "local") == 0 && count == 2) {
			if (kn_callsign_parse(tokens[1], &callsign) != 0)
				goto parse_error;
			(void)snprintf(script->local, sizeof(script->local),
			    "%s", tokens[1]);
			continue;
		}
		if (strcmp(tokens[0], "remote") == 0 && count == 2) {
			if (kn_callsign_parse(tokens[1], &callsign) != 0)
				goto parse_error;
			(void)snprintf(script->remote, sizeof(script->remote),
			    "%s", tokens[1]);
			continue;
		}
		if (strcmp(tokens[0], "port") == 0 && count == 2) {
			if (strlen(tokens[1]) >= sizeof(script->port))
				goto parse_error;
			(void)snprintf(script->port, sizeof(script->port),
			    "%s", tokens[1]);
			continue;
		}
		if (strcmp(tokens[0], "params") == 0) {
			if (parse_params(tokens, count, &script->params) != 0)
				goto parse_error;
			continue;
		}
		if (strcmp(tokens[0], "expect") == 0) {
			if (expect_from_tokens(tokens, count, &command) != 0)
				goto parse_error;
			if (append_command(script, &command) != 0)
				goto parse_error;
			continue;
		}
parse_error:
		(void)fclose(fp);
		error_set(error, KN_AX25_CONNECT_DRY_RUN_ERR_PARSE, line_no,
		    "invalid-command");
		return KN_AX25_CONNECT_DRY_RUN_ERR_PARSE;
	}
	(void)fclose(fp);
	if (script->name[0] == '\0' || script->local[0] == '\0' ||
	    script->remote[0] == '\0' || script->port[0] == '\0') {
		error_set(error, KN_AX25_CONNECT_DRY_RUN_ERR_PARSE, 0,
		    "missing-header");
		return KN_AX25_CONNECT_DRY_RUN_ERR_PARSE;
	}
	error_set(error, KN_AX25_CONNECT_DRY_RUN_OK, 0, "ok");
	return KN_AX25_CONNECT_DRY_RUN_OK;
}

enum kn_ax25_connect_dry_run_error
kn_ax25_connect_dry_run_run_file(const char *path,
	struct kn_ax25_connect_dry_run_result *result,
	struct kn_ax25_connect_dry_run_error_info *error)
{
	struct kn_ax25_connect_dry_run_script script;
	enum kn_ax25_connect_dry_run_error rc;

	rc = kn_ax25_connect_dry_run_parse_file(path, &script, error);
	if (rc != KN_AX25_CONNECT_DRY_RUN_OK)
		return rc;
	return kn_ax25_connect_dry_run_run_script(&script, result);
}

enum kn_ax25_connect_dry_run_error
kn_ax25_connect_dry_run_run_script(
	const struct kn_ax25_connect_dry_run_script *script,
	struct kn_ax25_connect_dry_run_result *result)
{
	const struct kn_ax25_connect_dry_run_command *command;
	size_t i;

	if (script == NULL || result == NULL)
		return KN_AX25_CONNECT_DRY_RUN_ERR_INVALID_ARGUMENT;
	memset(result, 0, sizeof(*result));
	(void)snprintf(result->name, sizeof(result->name), "%s",
	    script->name);
	(void)snprintf(result->local, sizeof(result->local), "%s",
	    script->local);
	(void)snprintf(result->remote, sizeof(result->remote), "%s",
	    script->remote);
	(void)snprintf(result->port, sizeof(result->port), "%s",
	    script->port);
	result->planned_state = KN_AX25_CONNECTION_AWAITING_CONNECTION;
	(void)snprintf(result->frame_kind, sizeof(result->frame_kind), "%s",
	    frame_kind_from_params(&script->params));
	result->bridge_blocked = 1;
	result->connection_created = 0;
	result->tx_writes = 0;
	result->dispatch_calls = 0;
	result->fx25_frames = 0;
	(void)snprintf(result->last_error, sizeof(result->last_error),
	    "ok");
	for (i = 0; i < script->command_count; i++) {
		command = &script->commands[i];
		if (command->expect ==
		    KN_AX25_CONNECT_DRY_RUN_EXPECT_PLANNED_STATE &&
		    command->state != result->planned_state)
			goto mismatch;
		if (command->expect ==
		    KN_AX25_CONNECT_DRY_RUN_EXPECT_FRAME_KIND &&
		    strcmp(command->text, result->frame_kind) != 0)
			goto mismatch;
		if (command->expect == KN_AX25_CONNECT_DRY_RUN_EXPECT_BRIDGE &&
		    command->value != result->bridge_blocked)
			goto mismatch;
		if (command->expect ==
		    KN_AX25_CONNECT_DRY_RUN_EXPECT_CONNECTION_CREATED &&
		    command->value != result->connection_created)
			goto mismatch;
		if (command->expect ==
		    KN_AX25_CONNECT_DRY_RUN_EXPECT_TX_WRITES &&
		    command->value != result->tx_writes)
			goto mismatch;
		if (command->expect ==
		    KN_AX25_CONNECT_DRY_RUN_EXPECT_DISPATCH_CALLS &&
		    command->value != result->dispatch_calls)
			goto mismatch;
		if (command->expect ==
		    KN_AX25_CONNECT_DRY_RUN_EXPECT_FX25_FRAMES &&
		    command->value != result->fx25_frames)
			goto mismatch;
		continue;
mismatch:
		result->mismatch_count++;
		if (result->first_mismatch_line == 0)
			result->first_mismatch_line = command->line;
		(void)snprintf(result->last_error, sizeof(result->last_error),
		    "expectation-mismatch");
	}
	result->pass = result->mismatch_count == 0 ? 1 : 0;
	return result->pass != 0 ? KN_AX25_CONNECT_DRY_RUN_OK :
	    KN_AX25_CONNECT_DRY_RUN_ERR_MISMATCH;
}
