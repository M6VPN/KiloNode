/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/tools/kilonodectl.c */

#define _POSIX_C_SOURCE 200809L

#include <sys/types.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "kilonode/control.h"

#define CTL_RESPONSE_BUFSIZ 512

static const char *command_map(const char *);
static int run_command(const char *, const char *);
static void usage(FILE *, const char *);

int
main(int argc, char *argv[])
{
	const char *socket_path;
	const char *command;
	int i;

	socket_path = NULL;
	command = NULL;

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
		if (command != NULL) {
			usage(stderr, argv[0]);
			return 1;
		}
		command = command_map(argv[i]);
		if (command == NULL) {
			usage(stderr, argv[0]);
			return 1;
		}
	}

	if (socket_path == NULL || command == NULL) {
		usage(stderr, argv[0]);
		return 1;
	}

	return run_command(socket_path, command);
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
	fprintf(out, "       %s --socket PATH help\n", argv0);
}
