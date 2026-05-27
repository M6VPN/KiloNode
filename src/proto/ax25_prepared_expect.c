/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/proto/ax25_prepared_expect.c */

#include <sys/types.h>

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kilonode/ax25_prepared_expect.h"

#define TOKEN_MAX 32

static enum kn_ax25_prepared_expect_error append_block(
	struct kn_ax25_prepared_expect_file *,
	enum kn_ax25_prepared_expect_block_type, const char *, size_t,
	struct kn_ax25_prepared_expect_block **,
	struct kn_ax25_prepared_expect_error_info *);
static void append_error(struct kn_ax25_prepared_expect_error_info *,
	enum kn_ax25_prepared_expect_error, size_t, const char *);
static enum kn_ax25_prepared_expect_error append_frame(
	struct kn_ax25_prepared_expect_block *,
	const struct kn_ax25_prepared_expect_frame *,
	struct kn_ax25_prepared_expect_error_info *);
static int bool_safe_name(const char *);
static int hex_safe(const char *);
static enum kn_ax25_prepared_expect_error parse_frame_token(
	struct kn_ax25_prepared_expect_frame *, const char *);
static int parse_u64(const char *, uint64_t *);
static int split_tokens(char *, char **, size_t, int *);

static enum kn_ax25_prepared_expect_error
append_block(struct kn_ax25_prepared_expect_file *file,
	enum kn_ax25_prepared_expect_block_type type, const char *name,
	size_t line, struct kn_ax25_prepared_expect_block **block,
	struct kn_ax25_prepared_expect_error_info *error)
{
	int needed;

	if (file == NULL || name == NULL || block == NULL)
		return KN_AX25_PREPARED_EXPECT_ERR_INVALID_ARGUMENT;
	if (file->block_count >= KN_AX25_PREPARED_EXPECT_BLOCK_MAX) {
		append_error(error, KN_AX25_PREPARED_EXPECT_ERR_FULL, line,
		    "too-many-blocks");
		return KN_AX25_PREPARED_EXPECT_ERR_FULL;
	}
	if (bool_safe_name(name) == 0) {
		append_error(error, KN_AX25_PREPARED_EXPECT_ERR_PARSE, line,
		    "unsafe-name");
		return KN_AX25_PREPARED_EXPECT_ERR_PARSE;
	}
	*block = &file->blocks[file->block_count];
	memset(*block, 0, sizeof(**block));
	(*block)->type = type;
	(*block)->line = line;
	needed = snprintf((*block)->name, sizeof((*block)->name), "%s", name);
	if (needed < 0 || (size_t)needed >= sizeof((*block)->name)) {
		append_error(error, KN_AX25_PREPARED_EXPECT_ERR_BUFFER, line,
		    "name-too-long");
		return KN_AX25_PREPARED_EXPECT_ERR_BUFFER;
	}
	file->block_count++;
	return KN_AX25_PREPARED_EXPECT_OK;
}

static void
append_error(struct kn_ax25_prepared_expect_error_info *error,
	enum kn_ax25_prepared_expect_error code, size_t line, const char *msg)
{
	if (error == NULL)
		return;

	error->error = code;
	error->line = line;
	if (msg != NULL)
		(void)snprintf(error->message, sizeof(error->message), "%s",
		    msg);
}

static enum kn_ax25_prepared_expect_error
append_frame(struct kn_ax25_prepared_expect_block *block,
	const struct kn_ax25_prepared_expect_frame *frame,
	struct kn_ax25_prepared_expect_error_info *error)
{
	if (block == NULL || frame == NULL)
		return KN_AX25_PREPARED_EXPECT_ERR_INVALID_ARGUMENT;
	if (block->frame_count >= KN_AX25_PREPARED_EXPECT_FRAME_MAX) {
		append_error(error, KN_AX25_PREPARED_EXPECT_ERR_FULL,
		    frame->line, "too-many-prepared");
		return KN_AX25_PREPARED_EXPECT_ERR_FULL;
	}
	block->frames[block->frame_count] = *frame;
	block->frame_count++;
	return KN_AX25_PREPARED_EXPECT_OK;
}

static int
bool_safe_name(const char *name)
{
	if (name == NULL || name[0] == '\0' || name[0] == '/' ||
	    strstr(name, "..") != NULL || strchr(name, '\\') != NULL)
		return 0;

	return 1;
}

static int
hex_safe(const char *text)
{
	size_t i;

	if (text == NULL || text[0] == '\0')
		return 0;
	for (i = 0; text[i] != '\0'; i++) {
		if (isxdigit((unsigned char)text[i]) == 0)
			return 0;
	}

	return 1;
}

int
kn_ax25_prepared_expect_action_from_text(const char *text,
	enum kn_ax25_action_intent *action)
{
	uint64_t i;

	if (text == NULL || action == NULL)
		return -1;
	for (i = 0; i <= (uint64_t)KN_AX25_ACTION_RETRANSMIT_NEEDED; i++) {
		if (strcmp(text, kn_ax25_action_intent_name(
		    (enum kn_ax25_action_intent)i)) == 0) {
			*action = (enum kn_ax25_action_intent)i;
			return 0;
		}
	}

	return -1;
}

int
kn_ax25_prepared_expect_kind_from_text(const char *text,
	enum kn_ax25_frame_plan_type *kind)
{
	int i;

	if (text == NULL || kind == NULL)
		return -1;
	for (i = 0; i < (int)KN_AX25_FRAME_PLAN_UNKNOWN; i++) {
		if (strcmp(text, kn_ax25_frame_plan_type_name(
		    (enum kn_ax25_frame_plan_type)i)) == 0) {
			*kind = (enum kn_ax25_frame_plan_type)i;
			return 0;
		}
	}

	return -1;
}

int
kn_ax25_prepared_expect_status_from_text(const char *text,
	enum kn_ax25_prepared_frame_status *status)
{
	if (text == NULL || status == NULL)
		return -1;
	if (strcmp(text, "prepared") == 0)
		*status = KN_AX25_PREPARED_FRAME_STATUS_PREPARED;
	else if (strcmp(text, "build-failed") == 0)
		*status = KN_AX25_PREPARED_FRAME_STATUS_BUILD_FAILED;
	else if (strcmp(text, "suppressed") == 0)
		*status = KN_AX25_PREPARED_FRAME_STATUS_SUPPRESSED;
	else if (strcmp(text, "tx-bridge-blocked") == 0)
		*status = KN_AX25_PREPARED_FRAME_STATUS_TX_BRIDGE_BLOCKED;
	else
		return -1;

	return 0;
}

static enum kn_ax25_prepared_expect_error
parse_frame_token(struct kn_ax25_prepared_expect_frame *frame,
	const char *token)
{
	uint64_t value;
	struct kn_callsign callsign;
	int needed;

	if (frame == NULL || token == NULL)
		return KN_AX25_PREPARED_EXPECT_ERR_INVALID_ARGUMENT;
	if (strncmp(token, "kind=", 5) == 0) {
		if (kn_ax25_prepared_expect_kind_from_text(token + 5,
		    &frame->kind) != 0)
			return KN_AX25_PREPARED_EXPECT_ERR_PARSE;
		frame->has_kind = 1;
	} else if (strncmp(token, "action=", 7) == 0) {
		if (kn_ax25_prepared_expect_action_from_text(token + 7,
		    &frame->action) != 0)
			return KN_AX25_PREPARED_EXPECT_ERR_PARSE;
		frame->has_action = 1;
	} else if (strncmp(token, "status=", 7) == 0) {
		if (kn_ax25_prepared_expect_status_from_text(token + 7,
		    &frame->status) != 0)
			return KN_AX25_PREPARED_EXPECT_ERR_PARSE;
		frame->has_status = 1;
	} else if (strncmp(token, "local=", 6) == 0) {
		if (kn_callsign_parse(token + 6, &callsign) != 0)
			return KN_AX25_PREPARED_EXPECT_ERR_PARSE;
		(void)snprintf(frame->local, sizeof(frame->local), "%s",
		    token + 6);
		frame->has_local = 1;
	} else if (strncmp(token, "remote=", 7) == 0) {
		if (kn_callsign_parse(token + 7, &callsign) != 0)
			return KN_AX25_PREPARED_EXPECT_ERR_PARSE;
		(void)snprintf(frame->remote, sizeof(frame->remote), "%s",
		    token + 7);
		frame->has_remote = 1;
	} else if (strncmp(token, "port=", 5) == 0) {
		if (bool_safe_name(token + 5) == 0)
			return KN_AX25_PREPARED_EXPECT_ERR_PARSE;
		needed = snprintf(frame->port, sizeof(frame->port), "%s",
		    token + 5);
		if (needed < 0 || (size_t)needed >= sizeof(frame->port))
			return KN_AX25_PREPARED_EXPECT_ERR_BUFFER;
		frame->has_port = 1;
	} else if (strncmp(token, "ax25-len=", 9) == 0) {
		if (parse_u64(token + 9, &value) != 0)
			return KN_AX25_PREPARED_EXPECT_ERR_PARSE;
		frame->ax25_len = (size_t)value;
		frame->has_ax25_len = 1;
	} else if (strncmp(token, "contains-hex=", 13) == 0) {
		if (hex_safe(token + 13) == 0)
			return KN_AX25_PREPARED_EXPECT_ERR_PARSE;
		if (snprintf(frame->hex_prefix, sizeof(frame->hex_prefix),
		    "%s", token + 13) < 0)
			return KN_AX25_PREPARED_EXPECT_ERR_BUFFER;
		frame->has_hex_prefix = 1;
	} else if (strncmp(token, "tx-writes=", 10) == 0) {
		if (parse_u64(token + 10, &value) != 0 || value != 0)
			return KN_AX25_PREPARED_EXPECT_ERR_PARSE;
		frame->tx_writes = value;
		frame->has_tx_writes = 1;
	} else {
		return KN_AX25_PREPARED_EXPECT_ERR_PARSE;
	}

	return KN_AX25_PREPARED_EXPECT_OK;
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
	char *comment;
	char *token;
	size_t used;

	if (line == NULL || tokens == NULL || count == NULL)
		return -1;
	comment = strchr(line, '#');
	if (comment != NULL)
		*comment = '\0';
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

void
kn_ax25_prepared_expect_clear(struct kn_ax25_prepared_expect_file *file)
{
	if (file == NULL)
		return;

	memset(file, 0, sizeof(*file));
}

void
kn_ax25_prepared_expect_error_clear(
	struct kn_ax25_prepared_expect_error_info *error)
{
	if (error == NULL)
		return;

	memset(error, 0, sizeof(*error));
}

const char *
kn_ax25_prepared_expect_error_name(
	enum kn_ax25_prepared_expect_error error)
{
	switch (error) {
	case KN_AX25_PREPARED_EXPECT_OK:
		return "ok";
	case KN_AX25_PREPARED_EXPECT_ERR_INVALID_ARGUMENT:
		return "invalid-argument";
	case KN_AX25_PREPARED_EXPECT_ERR_IO:
		return "io";
	case KN_AX25_PREPARED_EXPECT_ERR_PARSE:
		return "parse";
	case KN_AX25_PREPARED_EXPECT_ERR_FULL:
		return "full";
	case KN_AX25_PREPARED_EXPECT_ERR_BUFFER:
		return "buffer";
	}

	return "unknown";
}

const struct kn_ax25_prepared_expect_block *
kn_ax25_prepared_expect_find(const struct kn_ax25_prepared_expect_file *file,
	enum kn_ax25_prepared_expect_block_type type, const char *name)
{
	size_t i;

	if (file == NULL || name == NULL)
		return NULL;
	for (i = 0; i < file->block_count; i++) {
		if (file->blocks[i].type == type &&
		    strcmp(file->blocks[i].name, name) == 0)
			return &file->blocks[i];
	}

	return NULL;
}

enum kn_ax25_prepared_expect_error
kn_ax25_prepared_expect_parse_file(const char *path,
	struct kn_ax25_prepared_expect_file *file,
	struct kn_ax25_prepared_expect_error_info *error)
{
	FILE *fp;
	char line[KN_AX25_PREPARED_EXPECT_LINE_MAX];
	char work[KN_AX25_PREPARED_EXPECT_LINE_MAX];
	char *tokens[TOKEN_MAX];
	struct kn_ax25_prepared_expect_block *block;
	struct kn_ax25_prepared_expect_frame frame;
	enum kn_ax25_prepared_expect_block_type type;
	uint64_t value;
	size_t line_no;
	size_t len;
	int count;
	int i;

	if (path == NULL || file == NULL)
		return KN_AX25_PREPARED_EXPECT_ERR_INVALID_ARGUMENT;
	kn_ax25_prepared_expect_clear(file);
	kn_ax25_prepared_expect_error_clear(error);
	fp = fopen(path, "r");
	if (fp == NULL) {
		append_error(error, KN_AX25_PREPARED_EXPECT_ERR_IO, 0,
		    "open-failed");
		return KN_AX25_PREPARED_EXPECT_ERR_IO;
	}
	block = NULL;
	line_no = 0;
	while (fgets(line, sizeof(line), fp) != NULL) {
		line_no++;
		len = strlen(line);
		if (len > 0 && line[len - 1] != '\n' && !feof(fp)) {
			(void)fclose(fp);
			append_error(error, KN_AX25_PREPARED_EXPECT_ERR_PARSE,
			    line_no, "line-too-long");
			return KN_AX25_PREPARED_EXPECT_ERR_PARSE;
		}
		(void)snprintf(work, sizeof(work), "%s", line);
		if (split_tokens(work, tokens, TOKEN_MAX, &count) != 0) {
			(void)fclose(fp);
			append_error(error, KN_AX25_PREPARED_EXPECT_ERR_PARSE,
			    line_no, "tokenize");
			return KN_AX25_PREPARED_EXPECT_ERR_PARSE;
		}
		if (count == 0)
			continue;
		if (strcmp(tokens[0], "}") == 0 && count == 1) {
			if (block == NULL)
				goto invalid;
			block = NULL;
			continue;
		}
		if ((strcmp(tokens[0], "capture") == 0 ||
		    strcmp(tokens[0], "replay") == 0) && count == 3 &&
		    strcmp(tokens[2], "{") == 0) {
			if (block != NULL)
				goto invalid;
			type = strcmp(tokens[0], "capture") == 0 ?
			    KN_AX25_PREPARED_EXPECT_BLOCK_CAPTURE :
			    KN_AX25_PREPARED_EXPECT_BLOCK_REPLAY;
			if (append_block(file, type, tokens[1], line_no,
			    &block, error) != KN_AX25_PREPARED_EXPECT_OK) {
				(void)fclose(fp);
				return error == NULL ?
				    KN_AX25_PREPARED_EXPECT_ERR_PARSE :
				    error->error;
			}
			continue;
		}
		if (block == NULL)
			goto invalid;
		if (strcmp(tokens[0], "prepared-count") == 0 && count == 2) {
			if (parse_u64(tokens[1], &value) != 0)
				goto invalid;
			block->prepared_count = (size_t)value;
			block->has_prepared_count = 1;
			continue;
		}
		if (strcmp(tokens[0], "tx-writes") == 0 && count == 2) {
			if (parse_u64(tokens[1], &value) != 0 || value != 0)
				goto invalid;
			block->tx_writes = value;
			block->has_tx_writes = 1;
			continue;
		}
		if (strcmp(tokens[0], "prepared") == 0 && count >= 3) {
			memset(&frame, 0, sizeof(frame));
			frame.line = line_no;
			if (parse_u64(tokens[1], &value) != 0 || value == 0)
				goto invalid;
			frame.order = (size_t)value;
			for (i = 2; i < count; i++) {
				if (parse_frame_token(&frame, tokens[i]) !=
				    KN_AX25_PREPARED_EXPECT_OK)
					goto invalid;
			}
			if (append_frame(block, &frame, error) !=
			    KN_AX25_PREPARED_EXPECT_OK) {
				(void)fclose(fp);
				return error == NULL ?
				    KN_AX25_PREPARED_EXPECT_ERR_FULL :
				    error->error;
			}
			continue;
		}
invalid:
		(void)fclose(fp);
		append_error(error, KN_AX25_PREPARED_EXPECT_ERR_PARSE,
		    line_no, "invalid-line");
		return KN_AX25_PREPARED_EXPECT_ERR_PARSE;
	}
	(void)fclose(fp);
	if (block != NULL) {
		append_error(error, KN_AX25_PREPARED_EXPECT_ERR_PARSE,
		    block->line, "unterminated-block");
		return KN_AX25_PREPARED_EXPECT_ERR_PARSE;
	}

	return KN_AX25_PREPARED_EXPECT_OK;
}
