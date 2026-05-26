/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/tools/kilonode-monitor.c */

#include <sys/types.h>

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kilonode/buffer.h"
#include "kilonode/kiss_stream.h"
#include "kilonode/monitor.h"
#include "kilonode/transport.h"
#include "kilonode/transport_stdio.h"
#include "kilonode/transport_tcp.h"

#define MONITOR_LINE_BUFSIZ 512
#define READ_BUFSIZ         2048

enum monitor_mode {
	MONITOR_MODE_NONE = 0,
	MONITOR_MODE_STDIO,
	MONITOR_MODE_TCP_CONNECT,
	MONITOR_MODE_TCP_LISTEN
};

struct monitor_args {
	enum monitor_mode mode;
	const char *host;
	const char *port;
	size_t max_frame;
};

static int monitor_args_parse(int, char **, struct monitor_args *);
static int monitor_open_transport(const struct monitor_args *,
	struct kn_transport *);
static int monitor_pop_frames(struct kn_kiss_stream_parser *,
	struct kn_buffer *);
static int monitor_run(struct kn_transport *, size_t);
static int parse_size_arg(const char *, size_t *);
static void usage(FILE *, const char *);

int
main(int argc, char *argv[])
{
	struct kn_transport transport;
	struct monitor_args args;
	int rc;

	rc = monitor_args_parse(argc, argv, &args);
	if (rc == 2)
		return 0;
	if (rc != 0)
		return rc;

	kn_transport_reset(&transport);
	rc = monitor_open_transport(&args, &transport);
	if (rc != 0)
		return rc;

	rc = monitor_run(&transport, args.max_frame);
	kn_transport_close(&transport);

	return rc;
}

static int
monitor_args_parse(int argc, char **argv, struct monitor_args *args)
{
	int i;

	if (args == NULL)
		return 1;

	memset(args, 0, sizeof(*args));

	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "--help") == 0) {
			usage(stdout, argv[0]);
			return 2;
		} else if (strcmp(argv[i], "--stdio") == 0) {
			if (args->mode != MONITOR_MODE_NONE) {
				usage(stderr, argv[0]);
				return 1;
			}
			args->mode = MONITOR_MODE_STDIO;
		} else if (strcmp(argv[i], "--tcp-connect") == 0) {
			if (args->mode != MONITOR_MODE_NONE || i + 2 >= argc) {
				usage(stderr, argv[0]);
				return 1;
			}
			args->mode = MONITOR_MODE_TCP_CONNECT;
			args->host = argv[++i];
			args->port = argv[++i];
		} else if (strcmp(argv[i], "--tcp-listen") == 0) {
			if (args->mode != MONITOR_MODE_NONE || i + 2 >= argc) {
				usage(stderr, argv[0]);
				return 1;
			}
			args->mode = MONITOR_MODE_TCP_LISTEN;
			args->host = argv[++i];
			args->port = argv[++i];
		} else if (strcmp(argv[i], "--max-frame") == 0) {
			if (i + 1 >= argc ||
			    parse_size_arg(argv[++i], &args->max_frame) != 0) {
				usage(stderr, argv[0]);
				return 1;
			}
		} else {
			usage(stderr, argv[0]);
			return 1;
		}
	}

	if (args->mode == MONITOR_MODE_NONE) {
		usage(stderr, argv[0]);
		return 1;
	}

	return 0;
}

static int
monitor_open_transport(const struct monitor_args *args,
	struct kn_transport *transport)
{
	enum kn_transport_error rc;

	if (args->mode == MONITOR_MODE_STDIO)
		rc = kn_transport_stdio_open(transport);
	else if (args->mode == MONITOR_MODE_TCP_CONNECT)
		rc = kn_transport_tcp_connect_open(transport, args->host,
		    args->port);
	else if (args->mode == MONITOR_MODE_TCP_LISTEN)
		rc = kn_transport_tcp_listen_open(transport, args->host,
		    args->port);
	else
		rc = KN_TRANSPORT_ERR_INVALID_CONFIG;

	if (rc != KN_TRANSPORT_OK) {
		fprintf(stderr, "transport open failed: %s\n",
		    kn_transport_error_name(rc));
		return 1;
	}

	return 0;
}

static int
monitor_pop_frames(struct kn_kiss_stream_parser *parser,
	struct kn_buffer *frame_buf)
{
	struct kn_kiss_stream_frame frame;
	char line[MONITOR_LINE_BUFSIZ];
	enum kn_kiss_stream_error stream_rc;
	enum kn_monitor_error monitor_rc;

	while (kn_kiss_stream_has_frame(parser) != 0) {
		stream_rc = kn_kiss_stream_pop_frame(parser, &frame, frame_buf);
		if (stream_rc != KN_KISS_STREAM_OK)
			return 1;

		monitor_rc = kn_monitor_format_kiss(line, sizeof(line),
		    frame.port, frame.command, frame.payload, frame.payload_len);
		if (monitor_rc != KN_MONITOR_OK)
			printf("PORT %u monitor format error %d\n",
			    (unsigned int)frame.port, (int)monitor_rc);
		else
			printf("%s\n", line);
	}

	return 0;
}

static int
monitor_run(struct kn_transport *transport, size_t max_frame)
{
	struct kn_kiss_stream_parser parser;
	struct kn_buffer frame_buf;
	uint8_t read_buf[READ_BUFSIZ];
	enum kn_kiss_stream_error stream_rc;
	enum kn_transport_error transport_rc;
	size_t read_len;
	int rc;

	if (kn_kiss_stream_init(&parser, max_frame) != KN_KISS_STREAM_OK)
		return 1;
	if (kn_buffer_init(&frame_buf, 0) != 0) {
		kn_kiss_stream_free(&parser);
		return 1;
	}

	rc = 0;
	for (;;) {
		transport_rc = kn_transport_read(transport, read_buf,
		    sizeof(read_buf), &read_len);
		if (transport_rc == KN_TRANSPORT_ERR_EOF)
			break;
		if (transport_rc != KN_TRANSPORT_OK) {
			fprintf(stderr, "transport read failed: %s\n",
			    kn_transport_error_name(transport_rc));
			rc = 1;
			break;
		}

		stream_rc = kn_kiss_stream_feed(&parser, read_buf, read_len);
		if (stream_rc != KN_KISS_STREAM_OK)
			printf("KISS stream error %d\n", (int)stream_rc);

		if (monitor_pop_frames(&parser, &frame_buf) != 0) {
			rc = 1;
			break;
		}
	}

	kn_buffer_free(&frame_buf);
	kn_kiss_stream_free(&parser);
	return rc;
}

static int
parse_size_arg(const char *input, size_t *out)
{
	char *end;
	unsigned long long value;

	if (input == NULL || out == NULL || input[0] == '\0')
		return 1;

	errno = 0;
	value = strtoull(input, &end, 10);
	if (errno != 0 || *end != '\0' || value > SIZE_MAX)
		return 1;

	*out = (size_t)value;
	return 0;
}

static void
usage(FILE *out, const char *argv0)
{
	fprintf(out, "usage: %s --stdio [--max-frame BYTES]\n", argv0);
	fprintf(out, "       %s --tcp-connect HOST PORT [--max-frame BYTES]\n",
	    argv0);
	fprintf(out, "       %s --tcp-listen HOST PORT [--max-frame BYTES]\n",
	    argv0);
}
