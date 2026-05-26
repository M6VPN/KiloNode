/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/core/config.c */

#include <sys/types.h>

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kilonode/config.h"
#include "kilonode/control.h"
#include "kilonode/kiss_stream.h"
#include "kilonode/transport_serial.h"
#include "kilonode/transport_tcp.h"
#include "kilonode/transport_unix.h"

#define TOKEN_MAX 4

enum parser_block {
	BLOCK_NONE = 0,
	BLOCK_NODE,
	BLOCK_CONTROL,
	BLOCK_PORT
};

struct parser {
	struct kn_config *config;
	enum parser_block block;
	struct kn_config_port *port;
};

static enum kn_config_error config_validate(struct kn_config *);
static enum kn_config_error copy_field(char *, size_t, const char *,
	struct kn_config *, size_t);
static uint8_t key_seen(uint8_t *, struct kn_config *, size_t);
static enum kn_config_error line_parse(struct parser *, char *, size_t);
static enum kn_config_error node_key_set(struct kn_config *, char **, size_t,
	size_t);
static enum kn_config_error parse_bool(const char *, uint8_t *);
static enum kn_config_error parse_line_tokens(char *, char **, size_t *,
	struct kn_config *, size_t);
static enum kn_config_error parse_size_value(const char *, size_t *);
static enum kn_config_error port_key_set(struct parser *, char **, size_t,
	size_t);
static enum kn_config_port_type port_type_parse(const char *);
static enum kn_config_error set_error(struct kn_config *, enum kn_config_error,
	size_t, const char *);
static char *skip_ws(char *);

static enum kn_config_error
config_validate(struct kn_config *config)
{
	size_t i;
	struct kn_config_port *port;

	if (config->node.has_callsign == 0)
		return set_error(config, KN_CONFIG_ERR_MISSING_REQUIRED, 0,
		    "missing node callsign");

	if (config->control.has_block != 0 && config->control.enabled != 0 &&
	    config->control.has_path == 0)
		return set_error(config, KN_CONFIG_ERR_MISSING_REQUIRED, 0,
		    "missing control path");

	for (i = 0; i < config->port_count; i++) {
		port = &config->ports[i];
		if (port->has_type == 0)
			return set_error(config, KN_CONFIG_ERR_MISSING_REQUIRED, 0,
			    "missing port type");

		switch (port->type) {
		case KN_CONFIG_PORT_STDIO:
		case KN_CONFIG_PORT_PTY:
			break;
		case KN_CONFIG_PORT_TCP_CONNECT:
		case KN_CONFIG_PORT_TCP_LISTEN:
			if (port->has_host == 0 || port->has_port == 0)
				return set_error(config,
				    KN_CONFIG_ERR_MISSING_REQUIRED, 0,
				    "missing tcp host or port");
			break;
		case KN_CONFIG_PORT_SERIAL:
			if (port->has_device == 0 || port->has_baud == 0)
				return set_error(config,
				    KN_CONFIG_ERR_MISSING_REQUIRED, 0,
				    "missing serial device or baud");
			break;
		case KN_CONFIG_PORT_UNIX_CONNECT:
		case KN_CONFIG_PORT_UNIX_LISTEN:
			if (port->has_path == 0)
				return set_error(config,
				    KN_CONFIG_ERR_MISSING_REQUIRED, 0,
				    "missing unix socket path");
			break;
		case KN_CONFIG_PORT_NONE:
			return set_error(config, KN_CONFIG_ERR_INVALID_VALUE, 0,
			    "invalid port type");
		}
	}

	return KN_CONFIG_OK;
}

static enum kn_config_error
copy_field(char *dst, size_t dst_len, const char *src, struct kn_config *config,
	size_t line_no)
{
	size_t src_len;

	src_len = strlen(src);
	if (src_len == 0 || src_len >= dst_len)
		return set_error(config, KN_CONFIG_ERR_INVALID_VALUE, line_no,
		    "invalid field length");

	memcpy(dst, src, src_len + 1);
	return KN_CONFIG_OK;
}

const char *
kn_config_error_name(enum kn_config_error error)
{
	switch (error) {
	case KN_CONFIG_OK:
		return "ok";
	case KN_CONFIG_ERR_INVALID_ARGUMENT:
		return "invalid argument";
	case KN_CONFIG_ERR_IO:
		return "io";
	case KN_CONFIG_ERR_LINE_TOO_LONG:
		return "line too long";
	case KN_CONFIG_ERR_PARSE:
		return "parse";
	case KN_CONFIG_ERR_UNKNOWN_BLOCK:
		return "unknown block";
	case KN_CONFIG_ERR_UNKNOWN_KEY:
		return "unknown key";
	case KN_CONFIG_ERR_DUPLICATE_KEY:
		return "duplicate key";
	case KN_CONFIG_ERR_MISSING_REQUIRED:
		return "missing required";
	case KN_CONFIG_ERR_INVALID_VALUE:
		return "invalid value";
	case KN_CONFIG_ERR_DUPLICATE_PORT:
		return "duplicate port";
	case KN_CONFIG_ERR_TOO_MANY_PORTS:
		return "too many ports";
	}

	return "unknown";
}

void
kn_config_free(struct kn_config *config)
{
	if (config == NULL)
		return;

	kn_config_init(config);
}

void
kn_config_init(struct kn_config *config)
{
	if (config == NULL)
		return;

	memset(config, 0, sizeof(*config));
}

enum kn_config_error
kn_config_parse_file(const char *path, struct kn_config *config)
{
	FILE *fp;
	char *text;
	char line[KN_CONFIG_LINE_MAX];
	size_t cap;
	size_t len;
	size_t line_len;
	enum kn_config_error rc;

	if (path == NULL || config == NULL)
		return KN_CONFIG_ERR_INVALID_ARGUMENT;

	fp = fopen(path, "r");
	if (fp == NULL) {
		kn_config_init(config);
		return set_error(config, KN_CONFIG_ERR_IO, 0, "open failed");
	}

	text = NULL;
	cap = 0;
	len = 0;
	while (fgets(line, sizeof(line), fp) != NULL) {
		line_len = strlen(line);
		if (line_len > 0 && line[line_len - 1] != '\n' &&
		    !feof(fp)) {
			(void)fclose(fp);
			free(text);
			kn_config_init(config);
			return set_error(config, KN_CONFIG_ERR_LINE_TOO_LONG, 0,
			    "line too long");
		}
		if (len + line_len + 1 < len) {
			(void)fclose(fp);
			free(text);
			kn_config_init(config);
			return set_error(config, KN_CONFIG_ERR_INVALID_VALUE, 0,
			    "config too large");
		}
		if (len + line_len + 1 > cap) {
			char *new_text;
			size_t new_cap;

			new_cap = cap == 0 ? 1024 : cap * 2;
			while (new_cap < len + line_len + 1)
				new_cap *= 2;
			new_text = realloc(text, new_cap);
			if (new_text == NULL) {
				(void)fclose(fp);
				free(text);
				kn_config_init(config);
				return set_error(config, KN_CONFIG_ERR_IO, 0,
				    "memory allocation failed");
			}
			text = new_text;
			cap = new_cap;
		}
		memcpy(text + len, line, line_len);
		len += line_len;
		text[len] = '\0';
	}

	if (ferror(fp)) {
		(void)fclose(fp);
		free(text);
		kn_config_init(config);
		return set_error(config, KN_CONFIG_ERR_IO, 0, "read failed");
	}

	(void)fclose(fp);
	rc = kn_config_parse_text(text == NULL ? "" : text, config);
	free(text);
	return rc;
}

enum kn_config_error
kn_config_parse_text(const char *text, struct kn_config *config)
{
	struct parser parser;
	char line[KN_CONFIG_LINE_MAX];
	size_t i;
	size_t line_len;
	size_t line_no;
	enum kn_config_error rc;

	if (text == NULL || config == NULL)
		return KN_CONFIG_ERR_INVALID_ARGUMENT;

	kn_config_init(config);
	memset(&parser, 0, sizeof(parser));
	parser.config = config;
	line_no = 1;
	line_len = 0;

	for (i = 0;; i++) {
		if (text[i] != '\0' && text[i] != '\n') {
			if (line_len + 1 >= sizeof(line))
				return set_error(config, KN_CONFIG_ERR_LINE_TOO_LONG,
				    line_no, "line too long");
			line[line_len++] = text[i];
			continue;
		}

		line[line_len] = '\0';
		rc = line_parse(&parser, line, line_no);
		if (rc != KN_CONFIG_OK)
			return rc;

		if (text[i] == '\0')
			break;

		line_no++;
		line_len = 0;
	}

	if (parser.block != BLOCK_NONE)
		return set_error(config, KN_CONFIG_ERR_PARSE, line_no,
		    "unclosed block");

	rc = config_validate(config);
	if (rc != KN_CONFIG_OK)
		return rc;

	config->error = KN_CONFIG_OK;
	config->error_line = 0;
	config->error_text[0] = '\0';
	return KN_CONFIG_OK;
}

enum kn_transport_kind
kn_config_port_transport_kind(const struct kn_config_port *port)
{
	if (port == NULL)
		return KN_TRANSPORT_KIND_NONE;

	switch (port->type) {
	case KN_CONFIG_PORT_STDIO:
		return KN_TRANSPORT_KIND_STDIO;
	case KN_CONFIG_PORT_TCP_CONNECT:
		return KN_TRANSPORT_KIND_TCP_CLIENT;
	case KN_CONFIG_PORT_TCP_LISTEN:
		return KN_TRANSPORT_KIND_TCP_SERVER;
	case KN_CONFIG_PORT_SERIAL:
		return KN_TRANSPORT_KIND_SERIAL;
	case KN_CONFIG_PORT_PTY:
		return KN_TRANSPORT_KIND_PTY;
	case KN_CONFIG_PORT_UNIX_CONNECT:
		return KN_TRANSPORT_KIND_UNIX_CLIENT;
	case KN_CONFIG_PORT_UNIX_LISTEN:
		return KN_TRANSPORT_KIND_UNIX_SERVER;
	case KN_CONFIG_PORT_NONE:
		return KN_TRANSPORT_KIND_NONE;
	}

	return KN_TRANSPORT_KIND_NONE;
}

static uint8_t
key_seen(uint8_t *seen, struct kn_config *config, size_t line_no)
{
	if (*seen != 0) {
		(void)set_error(config, KN_CONFIG_ERR_DUPLICATE_KEY, line_no,
		    "duplicate key");
		return 1;
	}

	*seen = 1;
	return 0;
}

static enum kn_config_error
line_parse(struct parser *parser, char *line, size_t line_no)
{
	char *tokens[TOKEN_MAX];
	size_t token_count;
	size_t i;
	struct kn_config_port *port;
	enum kn_config_error rc;

	rc = parse_line_tokens(line, tokens, &token_count, parser->config,
	    line_no);
	if (rc != KN_CONFIG_OK || token_count == 0)
		return rc;

	if (strcmp(tokens[0], "}") == 0) {
		if (token_count != 1 || parser->block == BLOCK_NONE)
			return set_error(parser->config, KN_CONFIG_ERR_PARSE,
			    line_no, "unexpected block close");
		parser->block = BLOCK_NONE;
		parser->port = NULL;
		return KN_CONFIG_OK;
	}

	if (parser->block == BLOCK_NODE)
		return node_key_set(parser->config, tokens, token_count, line_no);

	if (parser->block == BLOCK_CONTROL) {
		if (token_count != 2)
			return set_error(parser->config, KN_CONFIG_ERR_PARSE,
			    line_no, "invalid control key");
		if (strcmp(tokens[0], "enabled") == 0) {
			if (key_seen(&parser->config->control.has_enabled,
			    parser->config, line_no) != 0)
				return parser->config->error;
			if (parse_bool(tokens[1],
			    &parser->config->control.enabled) != KN_CONFIG_OK)
				return set_error(parser->config,
				    KN_CONFIG_ERR_INVALID_VALUE, line_no,
				    "invalid control enabled value");
			return KN_CONFIG_OK;
		}
		if (strcmp(tokens[0], "path") == 0) {
			if (key_seen(&parser->config->control.has_path,
			    parser->config, line_no) != 0)
				return parser->config->error;
			if (kn_control_socket_path_valid(tokens[1]) == 0)
				return set_error(parser->config,
				    KN_CONFIG_ERR_INVALID_VALUE, line_no,
				    "invalid control path");
			return copy_field(parser->config->control.path,
			    sizeof(parser->config->control.path), tokens[1],
			    parser->config, line_no);
		}
		return set_error(parser->config, KN_CONFIG_ERR_UNKNOWN_KEY,
		    line_no, "unknown control key");
	}

	if (parser->block == BLOCK_PORT)
		return port_key_set(parser, tokens, token_count, line_no);

	if (strcmp(tokens[0], "node") == 0) {
		if (token_count != 2 || strcmp(tokens[1], "{") != 0)
			return set_error(parser->config, KN_CONFIG_ERR_PARSE,
			    line_no, "invalid node block");
		parser->block = BLOCK_NODE;
		return KN_CONFIG_OK;
	}

	if (strcmp(tokens[0], "control") == 0) {
		if (token_count != 2 || strcmp(tokens[1], "{") != 0)
			return set_error(parser->config, KN_CONFIG_ERR_PARSE,
			    line_no, "invalid control block");
		if (parser->config->control.has_block != 0)
			return set_error(parser->config,
			    KN_CONFIG_ERR_DUPLICATE_KEY, line_no,
			    "duplicate control block");
		parser->config->control.has_block = 1;
		parser->config->control.enabled = 0;
		parser->block = BLOCK_CONTROL;
		return KN_CONFIG_OK;
	}

	if (strcmp(tokens[0], "port") == 0) {
		if (token_count != 3 || strcmp(tokens[2], "{") != 0)
			return set_error(parser->config, KN_CONFIG_ERR_PARSE,
			    line_no, "invalid port block");
		if (parser->config->port_count >= KN_CONFIG_PORT_MAX)
			return set_error(parser->config,
			    KN_CONFIG_ERR_TOO_MANY_PORTS, line_no,
			    "too many ports");
		for (i = 0; i < parser->config->port_count; i++) {
			if (strcmp(parser->config->ports[i].name, tokens[1]) == 0)
				return set_error(parser->config,
				    KN_CONFIG_ERR_DUPLICATE_PORT, line_no,
				    "duplicate port");
		}
		port = &parser->config->ports[parser->config->port_count++];
		memset(port, 0, sizeof(*port));
		port->enabled = 1;
		port->max_frame = KN_KISS_STREAM_DEFAULT_MAX_FRAME;
		rc = copy_field(port->name, sizeof(port->name), tokens[1],
		    parser->config, line_no);
		if (rc != KN_CONFIG_OK)
			return rc;
		parser->block = BLOCK_PORT;
		parser->port = port;
		return KN_CONFIG_OK;
	}

	return set_error(parser->config, KN_CONFIG_ERR_UNKNOWN_BLOCK, line_no,
	    "unknown block");
}

static enum kn_config_error
node_key_set(struct kn_config *config, char **tokens, size_t token_count,
	size_t line_no)
{
	if (token_count != 2)
		return set_error(config, KN_CONFIG_ERR_PARSE, line_no,
		    "invalid node key");

	if (strcmp(tokens[0], "callsign") == 0) {
		if (key_seen(&config->node.has_callsign, config, line_no) != 0)
			return config->error;
		if (kn_callsign_parse(tokens[1], &config->node.callsign) != 0)
			return set_error(config, KN_CONFIG_ERR_INVALID_VALUE,
			    line_no, "invalid callsign");
		return KN_CONFIG_OK;
	}

	if (strcmp(tokens[0], "alias") == 0) {
		if (key_seen(&config->node.has_alias, config, line_no) != 0)
			return config->error;
		return copy_field(config->node.alias, sizeof(config->node.alias),
		    tokens[1], config, line_no);
	}

	if (strcmp(tokens[0], "location") == 0) {
		if (key_seen(&config->node.has_location, config, line_no) != 0)
			return config->error;
		return copy_field(config->node.location,
		    sizeof(config->node.location), tokens[1], config, line_no);
	}

	return set_error(config, KN_CONFIG_ERR_UNKNOWN_KEY, line_no,
	    "unknown node key");
}

static enum kn_config_error
parse_bool(const char *input, uint8_t *out)
{
	if (strcmp(input, "true") == 0 || strcmp(input, "on") == 0) {
		*out = 1;
		return KN_CONFIG_OK;
	}

	if (strcmp(input, "false") == 0 || strcmp(input, "off") == 0) {
		*out = 0;
		return KN_CONFIG_OK;
	}

	return KN_CONFIG_ERR_INVALID_VALUE;
}

static enum kn_config_error
parse_line_tokens(char *line, char **tokens, size_t *token_count,
	struct kn_config *config, size_t line_no)
{
	char *p;
	size_t count;
	uint8_t quoted;

	count = 0;
	p = skip_ws(line);

	while (*p != '\0') {
		if (*p == '#')
			break;
		if (count >= TOKEN_MAX)
			return set_error(config, KN_CONFIG_ERR_PARSE, line_no,
			    "too many tokens");

		quoted = 0;
		if (*p == '"') {
			quoted = 1;
			p++;
		}
		tokens[count++] = p;

		while (*p != '\0') {
			if (quoted != 0) {
				if (*p == '"')
					break;
			} else if (*p == '#' || *p == ' ' || *p == '\t' ||
			    *p == '\r') {
				break;
			}
			p++;
		}

		if (quoted != 0) {
			if (*p != '"')
				return set_error(config, KN_CONFIG_ERR_PARSE,
				    line_no, "unterminated string");
			*p++ = '\0';
		} else if (*p == '#') {
			*p = '\0';
			break;
		} else if (*p != '\0') {
			*p++ = '\0';
		}

		p = skip_ws(p);
	}

	*token_count = count;
	return KN_CONFIG_OK;
}

static enum kn_config_error
parse_size_value(const char *input, size_t *out)
{
	char *end;
	unsigned long value;

	errno = 0;
	value = strtoul(input, &end, 10);
	if (errno != 0 || *end != '\0')
		return KN_CONFIG_ERR_INVALID_VALUE;

	if (value < KN_CONFIG_MAX_FRAME_MIN || value > KN_CONFIG_MAX_FRAME_MAX)
		return KN_CONFIG_ERR_INVALID_VALUE;

	*out = (size_t)value;
	return KN_CONFIG_OK;
}

static enum kn_config_error
port_key_set(struct parser *parser, char **tokens, size_t token_count,
	size_t line_no)
{
	struct kn_config_port *port;
	unsigned long baud;
	char *end;

	if (token_count != 2)
		return set_error(parser->config, KN_CONFIG_ERR_PARSE, line_no,
		    "invalid port key");

	port = parser->port;

	if (strcmp(tokens[0], "type") == 0) {
		if (key_seen(&port->has_type, parser->config, line_no) != 0)
			return parser->config->error;
		port->type = port_type_parse(tokens[1]);
		if (port->type == KN_CONFIG_PORT_NONE)
			return set_error(parser->config, KN_CONFIG_ERR_INVALID_VALUE,
			    line_no, "invalid port type");
		return KN_CONFIG_OK;
	}

	if (strcmp(tokens[0], "host") == 0) {
		if (key_seen(&port->has_host, parser->config, line_no) != 0)
			return parser->config->error;
		return copy_field(port->host, sizeof(port->host), tokens[1],
		    parser->config, line_no);
	}

	if (strcmp(tokens[0], "port") == 0) {
		if (key_seen(&port->has_port, parser->config, line_no) != 0)
			return parser->config->error;
		if (kn_transport_tcp_port_valid(tokens[1]) == 0)
			return set_error(parser->config, KN_CONFIG_ERR_INVALID_VALUE,
			    line_no, "invalid tcp port");
		return copy_field(port->port, sizeof(port->port), tokens[1],
		    parser->config, line_no);
	}

	if (strcmp(tokens[0], "device") == 0) {
		if (key_seen(&port->has_device, parser->config, line_no) != 0)
			return parser->config->error;
		return copy_field(port->device, sizeof(port->device), tokens[1],
		    parser->config, line_no);
	}

	if (strcmp(tokens[0], "path") == 0) {
		if (key_seen(&port->has_path, parser->config, line_no) != 0)
			return parser->config->error;
		if (kn_transport_unix_path_valid(tokens[1]) == 0)
			return set_error(parser->config, KN_CONFIG_ERR_INVALID_VALUE,
			    line_no, "invalid path");
		return copy_field(port->path, sizeof(port->path), tokens[1],
		    parser->config, line_no);
	}

	if (strcmp(tokens[0], "baud") == 0) {
		if (key_seen(&port->has_baud, parser->config, line_no) != 0)
			return parser->config->error;
		errno = 0;
		baud = strtoul(tokens[1], &end, 10);
		if (errno != 0 || *end != '\0' || baud > UINT32_MAX ||
		    kn_transport_serial_baud_valid((unsigned int)baud) == 0)
			return set_error(parser->config, KN_CONFIG_ERR_INVALID_VALUE,
			    line_no, "invalid baud");
		port->baud = (unsigned int)baud;
		return KN_CONFIG_OK;
	}

	if (strcmp(tokens[0], "flow-control") == 0) {
		if (key_seen(&port->has_flow_control, parser->config,
		    line_no) != 0)
			return parser->config->error;
		if (kn_transport_serial_flow_control_parse(tokens[1],
		    &port->flow_control) == 0)
			return set_error(parser->config, KN_CONFIG_ERR_INVALID_VALUE,
			    line_no, "invalid flow-control");
		return KN_CONFIG_OK;
	}

	if (strcmp(tokens[0], "max-frame") == 0) {
		if (key_seen(&port->has_max_frame, parser->config,
		    line_no) != 0)
			return parser->config->error;
		if (parse_size_value(tokens[1], &port->max_frame) != KN_CONFIG_OK)
			return set_error(parser->config, KN_CONFIG_ERR_INVALID_VALUE,
			    line_no, "invalid max-frame");
		return KN_CONFIG_OK;
	}

	if (strcmp(tokens[0], "enabled") == 0) {
		if (key_seen(&port->has_enabled, parser->config, line_no) != 0)
			return parser->config->error;
		if (parse_bool(tokens[1], &port->enabled) != KN_CONFIG_OK)
			return set_error(parser->config, KN_CONFIG_ERR_INVALID_VALUE,
			    line_no, "invalid enabled value");
		return KN_CONFIG_OK;
	}

	return set_error(parser->config, KN_CONFIG_ERR_UNKNOWN_KEY, line_no,
	    "unknown port key");
}

static enum kn_config_port_type
port_type_parse(const char *input)
{
	if (strcmp(input, "stdio") == 0)
		return KN_CONFIG_PORT_STDIO;
	if (strcmp(input, "tcp-connect") == 0)
		return KN_CONFIG_PORT_TCP_CONNECT;
	if (strcmp(input, "tcp-listen") == 0)
		return KN_CONFIG_PORT_TCP_LISTEN;
	if (strcmp(input, "serial") == 0)
		return KN_CONFIG_PORT_SERIAL;
	if (strcmp(input, "pty") == 0)
		return KN_CONFIG_PORT_PTY;
	if (strcmp(input, "unix-connect") == 0)
		return KN_CONFIG_PORT_UNIX_CONNECT;
	if (strcmp(input, "unix-listen") == 0)
		return KN_CONFIG_PORT_UNIX_LISTEN;

	return KN_CONFIG_PORT_NONE;
}

static enum kn_config_error
set_error(struct kn_config *config, enum kn_config_error error, size_t line_no,
	const char *text)
{
	int needed;

	config->error = error;
	config->error_line = line_no;
	needed = snprintf(config->error_text, sizeof(config->error_text),
	    "%s", text == NULL ? "" : text);
	if (needed < 0 ||
	    (size_t)needed >= sizeof(config->error_text))
		config->error_text[0] = '\0';

	return error;
}

static char *
skip_ws(char *p)
{
	while (*p == ' ' || *p == '\t' || *p == '\r')
		p++;

	return p;
}
