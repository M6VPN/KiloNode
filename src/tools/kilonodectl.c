/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/tools/kilonodectl.c */

#define _POSIX_C_SOURCE 200809L

#include <sys/types.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "kilonode/control.h"

#define CTL_RESPONSE_BUFSIZ KN_CONTROL_QUEUE_MAX

static int command_bbs(char *, size_t, int *, int, char **);
static const char *command_map(const char *);
static int command_rf(char *, size_t, int *, int, char **);
static int command_rx(char *, size_t, int *, int, char **);
static int command_set(char *, size_t, const char *);
static int command_tx_dryrun_ui(char *, size_t, int *, int, char **);
static int command_tx(char *, size_t, int *, int, char **);
static int run_command(const char *, const char *);
static void usage(FILE *, const char *);

int
main(int argc, char *argv[])
{
	const char *socket_path;
	char command[KN_CONTROL_COMMAND_MAX];
	int i;
	int needed;

	socket_path = NULL;
	command[0] = '\0';

	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "--help") == 0) {
			usage(stdout, argv[0]);
			return 0;
		}
		if (strcmp(argv[i], "--socket") == 0) {
			if (i + 1 >= argc) {
				usage(stderr, argv[0]);
				return 1;
			}
			socket_path = argv[++i];
			continue;
		}
		if (command[0] != '\0') {
			usage(stderr, argv[0]);
			return 1;
		}
		if (strcmp(argv[i], "heard") == 0) {
			if (i + 2 < argc && strcmp(argv[i + 1], "--port") == 0) {
				needed = snprintf(command, sizeof(command),
				    "HEARD PORT %s", argv[i + 2]);
				if (needed < 0 ||
				    (size_t)needed >= sizeof(command)) {
					usage(stderr, argv[0]);
					return 1;
				}
				i += 2;
				continue;
			}
			if (command_set(command, sizeof(command), "HEARD") != 0)
				return 1;
			continue;
		}
		if (strcmp(argv[i], "bbs") == 0) {
			if (command_bbs(command, sizeof(command), &i, argc,
			    argv) != 0) {
				usage(stderr, argv[0]);
				return 1;
			}
			continue;
		}
		if (strcmp(argv[i], "rx") == 0) {
			if (command_rx(command, sizeof(command), &i, argc,
			    argv) != 0) {
				usage(stderr, argv[0]);
				return 1;
			}
			continue;
		}
		if (strcmp(argv[i], "rf") == 0) {
			if (command_rf(command, sizeof(command), &i, argc,
			    argv) != 0) {
				usage(stderr, argv[0]);
				return 1;
			}
			continue;
		}
		if (strcmp(argv[i], "tx") == 0) {
			if (command_tx(command, sizeof(command), &i, argc,
			    argv) != 0) {
				usage(stderr, argv[0]);
				return 1;
			}
			continue;
		}
		if (command_set(command, sizeof(command),
		    command_map(argv[i])) != 0) {
			usage(stderr, argv[0]);
			return 1;
		}
	}

	if (socket_path == NULL || command[0] == '\0') {
		usage(stderr, argv[0]);
		return 1;
	}

	return run_command(socket_path, command);
}

static int
command_bbs(char *dst, size_t dst_len, int *index, int argc, char *argv[])
{
	const char *sub;
	const char *filter;
	int needed;

	if (*index + 1 >= argc)
		return 1;
	sub = argv[*index + 1];
	if (strcmp(sub, "status") == 0) {
		*index += 1;
		return command_set(dst, dst_len, "BBS STATUS");
	}
	if (strcmp(sub, "stats") == 0) {
		*index += 1;
		return command_set(dst, dst_len, "BBS STATS");
	}
	if (strcmp(sub, "areas") == 0) {
		*index += 1;
		return command_set(dst, dst_len, "BBS AREAS");
	}
	if (strcmp(sub, "users") == 0) {
		*index += 1;
		return command_set(dst, dst_len, "BBS USERS");
	}
	if (strcmp(sub, "message") == 0) {
		if (*index + 2 >= argc)
			return 1;
		needed = snprintf(dst, dst_len, "BBS MESSAGE %s",
		    argv[*index + 2]);
		if (needed < 0 || (size_t)needed >= dst_len)
			return 1;
		*index += 2;
		return 0;
	}
	if (strcmp(sub, "messages") != 0)
		return 1;
	if (*index + 2 >= argc) {
		*index += 1;
		return command_set(dst, dst_len, "BBS MESSAGES");
	}
	if (strcmp(argv[*index + 2], "--private") == 0) {
		*index += 2;
		return command_set(dst, dst_len, "BBS MESSAGES PRIVATE");
	}
	if (strcmp(argv[*index + 2], "--bulletins") == 0) {
		*index += 2;
		return command_set(dst, dst_len, "BBS MESSAGES BULLETINS");
	}
	if (strcmp(argv[*index + 2], "--area") == 0 ||
	    strcmp(argv[*index + 2], "--to") == 0 ||
	    strcmp(argv[*index + 2], "--from") == 0) {
		if (*index + 3 >= argc)
			return 1;
		if (strcmp(argv[*index + 2], "--area") == 0)
			filter = "AREA";
		else if (strcmp(argv[*index + 2], "--to") == 0)
			filter = "TO";
		else
			filter = "FROM";
		needed = snprintf(dst, dst_len, "BBS MESSAGES %s %s",
		    filter, argv[*index + 3]);
		if (needed < 0 || (size_t)needed >= dst_len)
			return 1;
		*index += 3;
		return 0;
	}
	*index += 1;
	return command_set(dst, dst_len, "BBS MESSAGES");
}

static int
command_rx(char *dst, size_t dst_len, int *index, int argc, char *argv[])
{
	const char *sub;
	const char *filter;
	int needed;

	if (*index + 1 >= argc)
		return 1;

	sub = argv[*index + 1];
	if (strcmp(sub, "status") == 0) {
		*index += 1;
		return command_set(dst, dst_len, "RX STATUS");
	}
	if (strcmp(sub, "event") == 0) {
		if (*index + 2 >= argc)
			return 1;
		needed = snprintf(dst, dst_len, "RX EVENT %s",
		    argv[*index + 2]);
		if (needed < 0 || (size_t)needed >= dst_len)
			return 1;
		*index += 2;
		return 0;
	}
	if (strcmp(sub, "sessions") == 0) {
		if (*index + 2 >= argc) {
			*index += 1;
			return command_set(dst, dst_len, "RX SESSIONS");
		}
		if (strcmp(argv[*index + 2], "--port") == 0 ||
		    strcmp(argv[*index + 2], "--from") == 0) {
			if (*index + 3 >= argc)
				return 1;
			filter = strcmp(argv[*index + 2], "--port") == 0 ?
			    "PORT" : "FROM";
			needed = snprintf(dst, dst_len, "RX SESSIONS %s %s",
			    filter, argv[*index + 3]);
			if (needed < 0 || (size_t)needed >= dst_len)
				return 1;
			*index += 3;
			return 0;
		}
		*index += 1;
		return command_set(dst, dst_len, "RX SESSIONS");
	}
	if (strcmp(sub, "events") != 0)
		return 1;
	if (*index + 2 >= argc) {
		*index += 1;
		return command_set(dst, dst_len, "RX EVENTS");
	}
	if (strcmp(argv[*index + 2], "--limit") == 0 ||
	    strcmp(argv[*index + 2], "--port") == 0 ||
	    strcmp(argv[*index + 2], "--from") == 0 ||
	    strcmp(argv[*index + 2], "--to") == 0) {
		if (*index + 3 >= argc)
			return 1;
		if (strcmp(argv[*index + 2], "--limit") == 0)
			filter = "LIMIT";
		else if (strcmp(argv[*index + 2], "--port") == 0)
			filter = "PORT";
		else if (strcmp(argv[*index + 2], "--from") == 0)
			filter = "FROM";
		else
			filter = "TO";
		needed = snprintf(dst, dst_len, "RX EVENTS %s %s",
		    filter, argv[*index + 3]);
		if (needed < 0 || (size_t)needed >= dst_len)
			return 1;
		*index += 3;
		return 0;
	}
	*index += 1;
	return command_set(dst, dst_len, "RX EVENTS");
}

static const char *
command_map(const char *input)
{
	if (strcmp(input, "ping") == 0)
		return "PING";
	if (strcmp(input, "version") == 0)
		return "VERSION";
	if (strcmp(input, "status") == 0)
		return "STATUS";
	if (strcmp(input, "ports") == 0)
		return "PORTS";
	if (strcmp(input, "stats") == 0)
		return "STATS";
	if (strcmp(input, "help") == 0)
		return "HELP";

	return NULL;
}

static int
command_rf(char *dst, size_t dst_len, int *index, int argc, char *argv[])
{
	const char *sub;
	const char *filter;
	int needed;

	if (*index + 1 >= argc)
		return 1;

	sub = argv[*index + 1];
	if (strcmp(sub, "status") == 0) {
		*index += 1;
		return command_set(dst, dst_len, "RF STATUS");
	}
	if (strcmp(sub, "command") == 0) {
		if (*index + 2 >= argc)
			return 1;
		needed = snprintf(dst, dst_len, "RF COMMAND %s",
		    argv[*index + 2]);
		if (needed < 0 || (size_t)needed >= dst_len)
			return 1;
		*index += 2;
		return 0;
	}
	if (strcmp(sub, "commands") != 0)
		return 1;
	if (*index + 2 >= argc) {
		*index += 1;
		return command_set(dst, dst_len, "RF COMMANDS");
	}
	if (strcmp(argv[*index + 2], "--limit") == 0 ||
	    strcmp(argv[*index + 2], "--port") == 0 ||
	    strcmp(argv[*index + 2], "--from") == 0) {
		if (*index + 3 >= argc)
			return 1;
		if (strcmp(argv[*index + 2], "--limit") == 0)
			filter = "LIMIT";
		else if (strcmp(argv[*index + 2], "--port") == 0)
			filter = "PORT";
		else
			filter = "FROM";
		needed = snprintf(dst, dst_len, "RF COMMANDS %s %s",
		    filter, argv[*index + 3]);
		if (needed < 0 || (size_t)needed >= dst_len)
			return 1;
		*index += 3;
		return 0;
	}
	*index += 1;
	return command_set(dst, dst_len, "RF COMMANDS");
}

static int
command_set(char *dst, size_t dst_len, const char *command)
{
	int needed;

	if (command == NULL)
		return 1;

	needed = snprintf(dst, dst_len, "%s", command);
	if (needed < 0 || (size_t)needed >= dst_len)
		return 1;

	return 0;
}

static int
command_tx(char *dst, size_t dst_len, int *index, int argc, char *argv[])
{
	const char *sub;
	int needed;

	if (*index + 1 >= argc)
		return 1;

	sub = argv[*index + 1];
	if (strcmp(sub, "status") == 0) {
		*index += 1;
		return command_set(dst, dst_len, "TX STATUS");
	}
	if (strcmp(sub, "dispatch-status") == 0) {
		*index += 1;
		return command_set(dst, dst_len, "TX DISPATCH STATUS");
	}
	if (strcmp(sub, "gates") == 0) {
		if (*index + 2 >= argc) {
			*index += 1;
			return command_set(dst, dst_len, "TX GATES");
		}
		if (strcmp(argv[*index + 2], "--port") == 0) {
			if (*index + 3 >= argc)
				return 1;
			needed = snprintf(dst, dst_len, "TX GATES PORT %s",
			    argv[*index + 3]);
			if (needed < 0 || (size_t)needed >= dst_len)
				return 1;
			*index += 3;
			return 0;
		}
		*index += 1;
		return command_set(dst, dst_len, "TX GATES");
	}
	if (strcmp(sub, "dispatch-run") == 0) {
		if (*index + 2 >= argc) {
			*index += 1;
			return command_set(dst, dst_len, "TX DISPATCH RUN");
		}
		if (strcmp(argv[*index + 2], "--port") == 0) {
			if (*index + 3 >= argc)
				return 1;
			needed = snprintf(dst, dst_len,
			    "TX DISPATCH RUN PORT %s", argv[*index + 3]);
			if (needed < 0 || (size_t)needed >= dst_len)
				return 1;
			*index += 3;
			return 0;
		}
		*index += 1;
		return command_set(dst, dst_len, "TX DISPATCH RUN");
	}
	if (strcmp(sub, "frame") == 0) {
		if (*index + 2 >= argc)
			return 1;
		needed = snprintf(dst, dst_len, "TX FRAME %s",
		    argv[*index + 2]);
		if (needed < 0 || (size_t)needed >= dst_len)
			return 1;
		*index += 2;
		return 0;
	}
	if (strcmp(sub, "dryrun-ui") == 0)
		return command_tx_dryrun_ui(dst, dst_len, index, argc, argv);
	if (strcmp(sub, "queue") != 0)
		return 1;
	if (*index + 2 >= argc) {
		*index += 1;
		return command_set(dst, dst_len, "TX QUEUE");
	}
	if (strcmp(argv[*index + 2], "--port") == 0) {
		if (*index + 3 >= argc)
			return 1;
		needed = snprintf(dst, dst_len, "TX QUEUE PORT %s",
		    argv[*index + 3]);
		if (needed < 0 || (size_t)needed >= dst_len)
			return 1;
		*index += 3;
		return 0;
	}
	*index += 1;
	return command_set(dst, dst_len, "TX QUEUE");
}

static int
command_tx_dryrun_ui(char *dst, size_t dst_len, int *index, int argc,
	char *argv[])
{
	const char *port;
	const char *source;
	const char *destination;
	const char *via;
	const char *text;
	int i;
	int needed;

	port = NULL;
	source = NULL;
	destination = NULL;
	via = NULL;
	text = NULL;
	i = *index + 2;
	while (i < argc) {
		if (strcmp(argv[i], "--port") == 0) {
			if (i + 1 >= argc || port != NULL)
				return 1;
			port = argv[i + 1];
			i += 2;
			continue;
		}
		if (strcmp(argv[i], "--from") == 0) {
			if (i + 1 >= argc || source != NULL)
				return 1;
			source = argv[i + 1];
			i += 2;
			continue;
		}
		if (strcmp(argv[i], "--to") == 0) {
			if (i + 1 >= argc || destination != NULL)
				return 1;
			destination = argv[i + 1];
			i += 2;
			continue;
		}
		if (strcmp(argv[i], "--via") == 0) {
			if (i + 1 >= argc || via != NULL)
				return 1;
			via = argv[i + 1];
			i += 2;
			continue;
		}
		if (strcmp(argv[i], "--text") == 0) {
			if (i + 1 >= argc || text != NULL)
				return 1;
			text = argv[i + 1];
			i += 2;
			continue;
		}
		return 1;
	}
	if (port == NULL || source == NULL || destination == NULL ||
	    text == NULL)
		return 1;

	if (via != NULL) {
		needed = snprintf(dst, dst_len,
		    "TX DRYRUN UI PORT %s FROM %s TO %s VIA %s TEXT %s",
		    port, source, destination, via, text);
	} else {
		needed = snprintf(dst, dst_len,
		    "TX DRYRUN UI PORT %s FROM %s TO %s TEXT %s",
		    port, source, destination, text);
	}
	if (needed < 0 || (size_t)needed >= dst_len)
		return 1;

	*index = argc - 1;
	return 0;
}

static int
run_command(const char *socket_path, const char *command)
{
	char request[KN_CONTROL_COMMAND_MAX + 2];
	char response[CTL_RESPONSE_BUFSIZ];
	ssize_t nread;
	int fd;
	int rc;
	int needed;
	uint8_t saw_error;

	if (kn_control_socket_connect(socket_path, &fd) != KN_CONTROL_OK) {
		fprintf(stderr, "connect failed\n");
		return 1;
	}

	needed = snprintf(request, sizeof(request), "%s\n", command);
	if (needed < 0 || (size_t)needed >= sizeof(request)) {
		(void)close(fd);
		return 1;
	}

	if (kn_control_socket_write(fd, request,
	    (size_t)needed) != KN_CONTROL_OK) {
		(void)close(fd);
		return 1;
	}

	rc = 0;
	saw_error = 0;
	for (;;) {
		nread = read(fd, response, sizeof(response) - 1);
		if (nread > 0) {
			response[nread] = '\0';
			if (strncmp(response, "ERR ", 4) == 0)
				saw_error = 1;
			fputs(response, stdout);
			continue;
		}
		if (nread == 0)
			break;
		rc = 1;
		break;
	}

	(void)close(fd);
	if (saw_error != 0)
		return 1;

	return rc;
}

static void
usage(FILE *out, const char *argv0)
{
	fprintf(out, "usage: %s --socket PATH ping\n", argv0);
	fprintf(out, "       %s --socket PATH version\n", argv0);
	fprintf(out, "       %s --socket PATH status\n", argv0);
	fprintf(out, "       %s --socket PATH ports\n", argv0);
	fprintf(out, "       %s --socket PATH stats\n", argv0);
	fprintf(out, "       %s --socket PATH heard\n", argv0);
	fprintf(out, "       %s --socket PATH heard --port NAME\n", argv0);
	fprintf(out, "       %s --socket PATH bbs status\n", argv0);
	fprintf(out, "       %s --socket PATH bbs stats\n", argv0);
	fprintf(out, "       %s --socket PATH bbs areas\n", argv0);
	fprintf(out, "       %s --socket PATH bbs users\n", argv0);
	fprintf(out, "       %s --socket PATH bbs messages\n", argv0);
	fprintf(out, "       %s --socket PATH bbs messages --area AREA\n", argv0);
	fprintf(out, "       %s --socket PATH bbs message ID\n", argv0);
	fprintf(out, "       %s --socket PATH rx status\n", argv0);
	fprintf(out, "       %s --socket PATH rx events\n", argv0);
	fprintf(out, "       %s --socket PATH rx events --from CALLSIGN\n", argv0);
	fprintf(out, "       %s --socket PATH rx event ID\n", argv0);
	fprintf(out, "       %s --socket PATH rx sessions\n", argv0);
	fprintf(out, "       %s --socket PATH rf status\n", argv0);
	fprintf(out, "       %s --socket PATH rf commands\n", argv0);
	fprintf(out, "       %s --socket PATH rf commands --from CALLSIGN\n", argv0);
	fprintf(out, "       %s --socket PATH rf command ID\n", argv0);
	fprintf(out, "       %s --socket PATH tx status\n", argv0);
	fprintf(out, "       %s --socket PATH tx queue\n", argv0);
	fprintf(out, "       %s --socket PATH tx queue --port NAME\n", argv0);
	fprintf(out, "       %s --socket PATH tx frame ID\n", argv0);
	fprintf(out, "       %s --socket PATH tx dryrun-ui --port PORT "
	    "--from CALLSIGN --to CALLSIGN [--via PATH] --text TEXT\n",
	    argv0);
	fprintf(out, "       %s --socket PATH tx dispatch-status\n", argv0);
	fprintf(out, "       %s --socket PATH tx dispatch-run [--port PORT]\n",
	    argv0);
	fprintf(out, "       %s --socket PATH tx gates [--port PORT]\n", argv0);
	fprintf(out, "       %s --socket PATH help\n", argv0);
}
