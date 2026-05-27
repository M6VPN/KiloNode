/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/node/node_command.c */

#include <sys/types.h>

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "kilonode/node_command.h"

static void preview_copy(const uint8_t *, size_t, char *, size_t);

void
kn_node_command_input_clear(struct kn_node_command_input *input)
{
	if (input == NULL)
		return;
	memset(input, 0, sizeof(*input));
}

const char *
kn_node_command_error_name(enum kn_node_command_error error)
{
	switch (error) {
	case KN_NODE_COMMAND_OK:
		return "ok";
	case KN_NODE_COMMAND_ERR_INVALID_ARGUMENT:
		return "invalid-argument";
	case KN_NODE_COMMAND_ERR_EMPTY:
		return "empty";
	case KN_NODE_COMMAND_ERR_OVERLONG:
		return "overlong";
	case KN_NODE_COMMAND_ERR_CONTROL:
		return "control-character";
	case KN_NODE_COMMAND_ERR_BUFFER:
		return "buffer";
	}

	return "unknown";
}

enum kn_node_command_error
kn_node_command_parse(const uint8_t *input, size_t input_len, size_t max_len,
	struct kn_node_command_input *parsed)
{
	size_t start;
	size_t end;
	size_t pos;
	size_t command_end;
	size_t i;

	if (parsed == NULL || (input == NULL && input_len > 0) ||
	    max_len == 0)
		return KN_NODE_COMMAND_ERR_INVALID_ARGUMENT;
	kn_node_command_input_clear(parsed);
	if (input_len > max_len) {
		preview_copy(input, input_len, parsed->preview,
		    sizeof(parsed->preview));
		return KN_NODE_COMMAND_ERR_OVERLONG;
	}

	start = 0;
	while (start < input_len &&
	    (input[start] == ' ' || input[start] == '\t' ||
	    input[start] == '\r' || input[start] == '\n'))
		start++;
	end = input_len;
	while (end > start &&
	    (input[end - 1] == ' ' || input[end - 1] == '\t' ||
	    input[end - 1] == '\r' || input[end - 1] == '\n'))
		end--;
	if (end == start)
		return KN_NODE_COMMAND_ERR_EMPTY;

	preview_copy(input + start, end - start, parsed->preview,
	    sizeof(parsed->preview));
	command_end = start;
	while (command_end < end && input[command_end] != ' ' &&
	    input[command_end] != '\t')
		command_end++;
	if (command_end == start ||
	    command_end - start >= sizeof(parsed->command))
		return KN_NODE_COMMAND_ERR_BUFFER;

	for (i = start; i < command_end; i++) {
		if (input[i] < 0x20 || input[i] > 0x7e)
			return KN_NODE_COMMAND_ERR_CONTROL;
		parsed->command[i - start] =
		    (char)toupper((unsigned char)input[i]);
	}
	parsed->command[i - start] = '\0';
	parsed->command_len = i - start;

	pos = command_end;
	while (pos < end && (input[pos] == ' ' || input[pos] == '\t'))
		pos++;
	if (pos < end) {
		if (end - pos >= sizeof(parsed->args))
			return KN_NODE_COMMAND_ERR_BUFFER;
		for (i = pos; i < end; i++) {
			if (input[i] < 0x20 || input[i] > 0x7e)
				return KN_NODE_COMMAND_ERR_CONTROL;
			parsed->args[i - pos] = (char)input[i];
		}
		parsed->args[i - pos] = '\0';
		parsed->args_len = i - pos;
	}

	return KN_NODE_COMMAND_OK;
}

static void
preview_copy(const uint8_t *input, size_t input_len, char *preview,
	size_t preview_len)
{
	size_t i;
	size_t off;
	int needed;

	if (preview == NULL || preview_len == 0)
		return;
	preview[0] = '\0';
	if (input == NULL)
		return;
	off = 0;
	for (i = 0; i < input_len && off + 1 < preview_len; i++) {
		if (input[i] >= 0x20 && input[i] <= 0x7e) {
			preview[off++] = (char)input[i];
			preview[off] = '\0';
			continue;
		}
		needed = snprintf(preview + off, preview_len - off, "?");
		if (needed < 0 || (size_t)needed >= preview_len - off)
			return;
		off += (size_t)needed;
	}
}
