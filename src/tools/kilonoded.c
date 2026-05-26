/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/tools/kilonoded.c */

#include <sys/types.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "kilonode/config.h"
#include "kilonode/daemon.h"

struct daemon_args {
	const char *config_path;
	uint8_t check_config;
	uint8_t foreground;
};

static int args_parse(int, char **, struct daemon_args *);
static void usage(FILE *, const char *);

int
main(int argc, char *argv[])
{
	struct daemon_args args;
	struct kn_config config;
	enum kn_config_error config_rc;
	enum kn_daemon_error daemon_rc;
	int rc;

	rc = args_parse(argc, argv, &args);
	if (rc == 2)
		return 0;
	if (rc != 0)
		return rc;

	config_rc = kn_config_parse_file(args.config_path, &config);
	if (config_rc != KN_CONFIG_OK) {
		fprintf(stderr, "config error: %s line %zu: %s\n",
		    kn_config_error_name(config_rc), config.error_line,
		    config.error_text);
		return 1;
	}

	if (args.check_config != 0) {
		printf("config ok\n");
		kn_config_free(&config);
		return 0;
	}

	if (args.foreground == 0) {
		fprintf(stderr, "background daemon mode is not implemented yet\n");
		kn_config_free(&config);
		return 1;
	}

	daemon_rc = kn_daemon_run_foreground(&config);
	kn_config_free(&config);
	if (daemon_rc != KN_DAEMON_OK)
		return 1;

	return 0;
}

static int
args_parse(int argc, char **argv, struct daemon_args *args)
{
	int i;

	memset(args, 0, sizeof(*args));

	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "--help") == 0) {
			usage(stdout, argv[0]);
			return 2;
		} else if (strcmp(argv[i], "--config") == 0) {
			if (i + 1 >= argc) {
				usage(stderr, argv[0]);
				return 1;
			}
			args->config_path = argv[++i];
		} else if (strcmp(argv[i], "--check-config") == 0) {
			args->check_config = 1;
		} else if (strcmp(argv[i], "--foreground") == 0) {
			args->foreground = 1;
		} else {
			usage(stderr, argv[0]);
			return 1;
		}
	}

	if (args->config_path == NULL) {
		usage(stderr, argv[0]);
		return 1;
	}

	return 0;
}

static void
usage(FILE *out, const char *argv0)
{
	fprintf(out, "usage: %s --config PATH --check-config\n", argv0);
	fprintf(out, "       %s --config PATH --foreground\n", argv0);
}
