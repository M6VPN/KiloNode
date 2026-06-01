/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/proto/ax25_loopback_script.c */

#include <sys/types.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kilonode/ax25_loopback_script.h"

static int append_command(struct kn_ax25_loopback_script *,
	const struct kn_ax25_loopback_script_command *);
static int endpoint_from_text(const char *,
	enum kn_ax25_loopback_script_endpoint *);
static void error_set(struct kn_ax25_loopback_error_info *,
	enum kn_ax25_loopback_error, size_t, const char *);
static int event_from_text(const char *,
	enum kn_ax25_loopback_script_event *);
static int expect_from_tokens(char *[], size_t,
	struct kn_ax25_loopback_script_command *);
static int hex_nibble(char);
static int parse_hex_payload(const char *, uint8_t *, size_t, size_t *);
static int parse_params(char *[], size_t, struct kn_ax25_params *);
static int parse_payload_tokens(char *[], size_t,
	struct kn_ax25_loopback_script_command *);
static int parse_u64(const char *, uint64_t *);
static const char *strip_quotes(const char *);
static int state_from_text(const char *, enum kn_ax25_connection_state *);
static char *trim(char *);

static int
append_command(struct kn_ax25_loopback_script *script,
	const struct kn_ax25_loopback_script_command *command)
{
	if (script->command_count >= KN_AX25_LOOPBACK_SCRIPT_COMMAND_MAX)
		return -1;
	script->commands[script->command_count++] = *command;
	return 0;
}

static int
endpoint_from_text(const char *text,
	enum kn_ax25_loopback_script_endpoint *endpoint)
{
	if (strcmp(text, "A") == 0) {
		*endpoint = KN_AX25_LOOPBACK_SCRIPT_ENDPOINT_A;
		return 0;
	}
	if (strcmp(text, "B") == 0) {
		*endpoint = KN_AX25_LOOPBACK_SCRIPT_ENDPOINT_B;
		return 0;
	}
	if (strcmp(text, "both") == 0) {
		*endpoint = KN_AX25_LOOPBACK_SCRIPT_ENDPOINT_BOTH;
		return 0;
	}
	return -1;
}

static void
error_set(struct kn_ax25_loopback_error_info *error,
	enum kn_ax25_loopback_error rc, size_t line, const char *message)
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
event_from_text(const char *text, enum kn_ax25_loopback_script_event *event)
{
	if (strcmp(text, "local-connect") == 0) {
		*event = KN_AX25_LOOPBACK_SCRIPT_EVENT_LOCAL_CONNECT;
		return 0;
	}
	if (strcmp(text, "local-disconnect") == 0) {
		*event = KN_AX25_LOOPBACK_SCRIPT_EVENT_LOCAL_DISCONNECT;
		return 0;
	}
	if (strcmp(text, "send-i") == 0) {
		*event = KN_AX25_LOOPBACK_SCRIPT_EVENT_SEND_I;
		return 0;
	}
	return -1;
}

static int
expect_from_tokens(char *tokens[], size_t count,
	struct kn_ax25_loopback_script_command *command)
{
	uint64_t value;

	if (count < 2)
		return -1;
	command->type = KN_AX25_LOOPBACK_SCRIPT_EXPECT;
	if (endpoint_from_text(tokens[1], &command->endpoint) == 0) {
		if (count != 3)
			return -1;
		if (strncmp(tokens[2], "state=", 6) == 0) {
			command->expect = KN_AX25_LOOPBACK_SCRIPT_EXPECT_STATE;
			return state_from_text(tokens[2] + 6, &command->state);
		}
		if (strncmp(tokens[2], "delivered=", 10) == 0) {
			command->expect =
			    KN_AX25_LOOPBACK_SCRIPT_EXPECT_DELIVERED;
			if (parse_u64(tokens[2] + 10, &command->value) != 0)
				return -1;
			return 0;
		}
		if (strncmp(tokens[2], "rejected=", 9) == 0) {
			command->expect =
			    KN_AX25_LOOPBACK_SCRIPT_EXPECT_REJECTED;
			if (parse_u64(tokens[2] + 9, &command->value) != 0)
				return -1;
			return 0;
		}
		if (strncmp(tokens[2], "last-payload-text=", 18) == 0) {
			command->expect =
			    KN_AX25_LOOPBACK_SCRIPT_EXPECT_LAST_PAYLOAD_TEXT;
			(void)snprintf(command->text, sizeof(command->text),
			    "%s", strip_quotes(tokens[2] + 18));
			return 0;
		}
		if (strncmp(tokens[2], "last-payload-hex=", 17) == 0) {
			command->expect =
			    KN_AX25_LOOPBACK_SCRIPT_EXPECT_LAST_PAYLOAD_HEX;
			(void)snprintf(command->text, sizeof(command->text),
			    "%s", tokens[2] + 17);
			return 0;
		}
		return -1;
	}
	command->endpoint = KN_AX25_LOOPBACK_SCRIPT_ENDPOINT_NONE;
	if (count != 3)
		return -1;
	if (strcmp(tokens[1], "prepared-count") == 0) {
		command->expect =
		    KN_AX25_LOOPBACK_SCRIPT_EXPECT_PREPARED_COUNT;
	} else if (strcmp(tokens[1], "transferred") == 0) {
		command->expect = KN_AX25_LOOPBACK_SCRIPT_EXPECT_TRANSFERRED;
	} else if (strcmp(tokens[1], "tx-writes") == 0) {
		command->expect = KN_AX25_LOOPBACK_SCRIPT_EXPECT_TX_WRITES;
	} else if (strcmp(tokens[1], "dispatch-calls") == 0) {
		command->expect =
		    KN_AX25_LOOPBACK_SCRIPT_EXPECT_DISPATCH_CALLS;
	} else if (strcmp(tokens[1], "fx25-frames") == 0) {
		command->expect = KN_AX25_LOOPBACK_SCRIPT_EXPECT_FX25_FRAMES;
	} else {
		return -1;
	}
	if (parse_u64(tokens[2], &value) != 0)
		return -1;
	command->value = value;
	return 0;
}

static int
hex_nibble(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	return -1;
}

static int
parse_hex_payload(const char *text, uint8_t *out, size_t out_len,
	size_t *written)
{
	size_t len;
	size_t i;
	int hi;
	int lo;

	if (text == NULL || out == NULL || written == NULL)
		return -1;
	len = strlen(text);
	if ((len & 1U) != 0 || len / 2 > out_len)
		return -1;
	for (i = 0; i < len / 2; i++) {
		hi = hex_nibble(text[i * 2]);
		lo = hex_nibble(text[i * 2 + 1]);
		if (hi < 0 || lo < 0)
			return -1;
		out[i] = (uint8_t)((hi << 4) | lo);
	}
	*written = len / 2;
	return 0;
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
			else
				return -1;
		} else if (strncmp(tokens[i], "window=", 7) == 0) {
			if (parse_u64(tokens[i] + 7, &value) != 0 ||
			    value > 255)
				return -1;
			params->window_size = (uint8_t)value;
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
parse_payload_tokens(char *tokens[], size_t count,
	struct kn_ax25_loopback_script_command *command)
{
	uint64_t value;
	const char *text;
	size_t i;
	size_t len;
	uint8_t saw_payload;

	if (tokens == NULL || command == NULL)
		return -1;
	saw_payload = 0;
	for (i = 3; i < count; i++) {
		if (strncmp(tokens[i], "text=", 5) == 0) {
			text = strip_quotes(tokens[i] + 5);
			len = strlen(text);
			if (len > sizeof(command->payload))
				return -1;
			memcpy(command->payload, text, len);
			command->payload_len = len;
			(void)snprintf(command->text,
			    sizeof(command->text), "%s", text);
			saw_payload = 1;
		} else if (strncmp(tokens[i], "hex=", 4) == 0) {
			if (parse_hex_payload(tokens[i] + 4, command->payload,
			    sizeof(command->payload),
			    &command->payload_len) != 0)
				return -1;
			command->payload_is_hex = 1;
			(void)snprintf(command->text,
			    sizeof(command->text), "%s", tokens[i] + 4);
			saw_payload = 1;
		} else if (strncmp(tokens[i], "len=", 4) == 0) {
			if (parse_u64(tokens[i] + 4, &value) != 0 ||
			    value != 0)
				return -1;
			command->payload_len = 0;
			saw_payload = 1;
		} else if (strncmp(tokens[i], "ns=", 3) == 0) {
			if (parse_u64(tokens[i] + 3, &value) != 0 ||
			    value > 7)
				return -1;
			command->ns_override = (uint8_t)value;
			command->use_ns_override = 1;
		} else {
			return -1;
		}
	}
	return saw_payload != 0 ? 0 : -1;
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

static const char *
strip_quotes(const char *text)
{
	size_t len;
	static char stripped[KN_AX25_LOOPBACK_SCRIPT_TEXT_MAX];

	if (text == NULL)
		return "";
	len = strlen(text);
	if (len >= 2 && text[0] == '"' && text[len - 1] == '"') {
		if (len - 2 >= sizeof(stripped))
			return text;
		memcpy(stripped, text + 1, len - 2);
		stripped[len - 2] = '\0';
		return stripped;
	}
	return text;
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
kn_ax25_loopback_error_name(enum kn_ax25_loopback_error error)
{
	switch (error) {
	case KN_AX25_LOOPBACK_OK:
		return "ok";
	case KN_AX25_LOOPBACK_ERR_INVALID_ARGUMENT:
		return "invalid-argument";
	case KN_AX25_LOOPBACK_ERR_PARSE:
		return "parse";
	case KN_AX25_LOOPBACK_ERR_IO:
		return "io";
	case KN_AX25_LOOPBACK_ERR_UNSUPPORTED:
		return "unsupported";
	case KN_AX25_LOOPBACK_ERR_MISMATCH:
		return "mismatch";
	case KN_AX25_LOOPBACK_ERR_INTERNAL:
		return "internal";
	}
	return "unknown";
}

enum kn_ax25_loopback_error
kn_ax25_loopback_script_parse_file(const char *path,
	struct kn_ax25_loopback_script *script,
	struct kn_ax25_loopback_error_info *error)
{
	FILE *fp;
	char line[KN_AX25_LOOPBACK_SCRIPT_LINE_MAX + 2];
	char *tokens[16];
	char *text;
	size_t line_no;
	size_t count;
	struct kn_ax25_loopback_script_command command;
	struct kn_callsign callsign;
	uint64_t value;

	if (path == NULL || script == NULL)
		return KN_AX25_LOOPBACK_ERR_INVALID_ARGUMENT;
	memset(script, 0, sizeof(*script));
	kn_ax25_params_default(&script->params);
	script->params.allow_connected_mode = 1;
	fp = fopen(path, "r");
	if (fp == NULL) {
		error_set(error, KN_AX25_LOOPBACK_ERR_IO, 0, "open");
		return KN_AX25_LOOPBACK_ERR_IO;
	}
	line_no = 0;
	while (fgets(line, sizeof(line), fp) != NULL) {
		line_no++;
		if (strchr(line, '\n') == NULL && !feof(fp)) {
			(void)fclose(fp);
			error_set(error, KN_AX25_LOOPBACK_ERR_PARSE, line_no,
			    "line-too-long");
			return KN_AX25_LOOPBACK_ERR_PARSE;
		}
		text = trim(line);
		if (text[0] == '\0' || text[0] == '#')
			continue;
		count = 0;
		for (tokens[count] = strtok(text, " \t");
		    tokens[count] != NULL && count + 1 < 16;
		    tokens[count] = strtok(NULL, " \t"))
			count++;
		if (count == 0)
			continue;
		memset(&command, 0, sizeof(command));
		command.line = line_no;
		if (strcmp(tokens[0], "name") == 0 && count == 2) {
			(void)snprintf(script->name, sizeof(script->name),
			    "%s", tokens[1]);
			continue;
		}
		if (strcmp(tokens[0], "endpoint-a") == 0 && count == 2) {
			if (kn_callsign_parse(tokens[1], &callsign) != 0)
				goto parse_error;
			(void)snprintf(script->endpoint_a,
			    sizeof(script->endpoint_a), "%s", tokens[1]);
			continue;
		}
		if (strcmp(tokens[0], "endpoint-b") == 0 && count == 2) {
			if (kn_callsign_parse(tokens[1], &callsign) != 0)
				goto parse_error;
			(void)snprintf(script->endpoint_b,
			    sizeof(script->endpoint_b), "%s", tokens[1]);
			continue;
		}
		if (strcmp(tokens[0], "port") == 0 && count == 2) {
			(void)snprintf(script->port, sizeof(script->port),
			    "%s", tokens[1]);
			continue;
		}
		if (strcmp(tokens[0], "params") == 0) {
			if (parse_params(tokens, count, &script->params) != 0)
				goto parse_error;
			command.type = KN_AX25_LOOPBACK_SCRIPT_PARAMS;
			command.params = script->params;
			if (append_command(script, &command) != 0)
				goto parse_error;
			continue;
		}
		if (strcmp(tokens[0], "now") == 0 && count == 2) {
			if (parse_u64(tokens[1], &value) != 0)
				goto parse_error;
			command.type = KN_AX25_LOOPBACK_SCRIPT_NOW;
			command.value = value;
			if (append_command(script, &command) != 0)
				goto parse_error;
			continue;
		}
		if (strcmp(tokens[0], "advance") == 0 && count == 2) {
			if (parse_u64(tokens[1], &value) != 0)
				goto parse_error;
			command.type = KN_AX25_LOOPBACK_SCRIPT_ADVANCE;
			command.value = value;
			if (append_command(script, &command) != 0)
				goto parse_error;
			continue;
		}
		if (strcmp(tokens[0], "event") == 0 && count >= 3) {
			if (endpoint_from_text(tokens[1],
			    &command.endpoint) != 0 ||
			    event_from_text(tokens[2], &command.event) != 0)
				goto parse_error;
			command.type = KN_AX25_LOOPBACK_SCRIPT_EVENT;
			if (command.event == KN_AX25_LOOPBACK_SCRIPT_EVENT_SEND_I) {
				if (parse_payload_tokens(tokens, count,
				    &command) != 0)
					goto parse_error;
			} else if (count != 3) {
				goto parse_error;
			}
			if (append_command(script, &command) != 0)
				goto parse_error;
			continue;
		}
		if (strcmp(tokens[0], "process-timers") == 0 && count == 2) {
			if (endpoint_from_text(tokens[1],
			    &command.endpoint) != 0)
				goto parse_error;
			command.type = KN_AX25_LOOPBACK_SCRIPT_PROCESS_TIMERS;
			if (append_command(script, &command) != 0)
				goto parse_error;
			continue;
		}
		if (strcmp(tokens[0], "transfer") == 0 && count == 2) {
			command.type = KN_AX25_LOOPBACK_SCRIPT_TRANSFER;
			if (strcmp(tokens[1], "A-to-B") == 0) {
				command.endpoint =
				    KN_AX25_LOOPBACK_SCRIPT_ENDPOINT_A;
				command.endpoint_to =
				    KN_AX25_LOOPBACK_SCRIPT_ENDPOINT_B;
			} else if (strcmp(tokens[1], "B-to-A") == 0) {
				command.endpoint =
				    KN_AX25_LOOPBACK_SCRIPT_ENDPOINT_B;
				command.endpoint_to =
				    KN_AX25_LOOPBACK_SCRIPT_ENDPOINT_A;
			} else {
				goto parse_error;
			}
			if (append_command(script, &command) != 0)
				goto parse_error;
			continue;
		}
		if (strcmp(tokens[0], "run-until-idle") == 0 && count == 2) {
			if (strncmp(tokens[1], "max-steps=", 10) != 0 ||
			    parse_u64(tokens[1] + 10, &value) != 0 ||
			    value == 0)
				goto parse_error;
			command.type =
			    KN_AX25_LOOPBACK_SCRIPT_RUN_UNTIL_IDLE;
			command.value = value;
			if (append_command(script, &command) != 0)
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
		error_set(error, KN_AX25_LOOPBACK_ERR_PARSE, line_no,
		    "invalid-command");
		return KN_AX25_LOOPBACK_ERR_PARSE;
	}
	(void)fclose(fp);
	if (script->name[0] == '\0' || script->endpoint_a[0] == '\0' ||
	    script->endpoint_b[0] == '\0' || script->port[0] == '\0') {
		error_set(error, KN_AX25_LOOPBACK_ERR_PARSE, 0,
		    "missing-header");
		return KN_AX25_LOOPBACK_ERR_PARSE;
	}
	error_set(error, KN_AX25_LOOPBACK_OK, 0, "ok");
	return KN_AX25_LOOPBACK_OK;
}
