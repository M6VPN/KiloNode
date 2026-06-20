/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_ax25_connect_dry_run_script.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "kilonode/ax25_connect_dry_run.h"

static int test_invalid_callsign(void);
static int test_invalid_expect(void);
static int test_invalid_params(void);
static int test_missing_header(void);
static int test_valid_script(void);
static int write_temp(const char *, char *, size_t);

int
main(void)
{
	if (test_valid_script() != 0)
		return 1;
	if (test_missing_header() != 0)
		return 1;
	if (test_invalid_callsign() != 0)
		return 1;
	if (test_invalid_params() != 0)
		return 1;
	if (test_invalid_expect() != 0)
		return 1;
	return 0;
}

static int
test_invalid_callsign(void)
{
	char path[128];
	struct kn_ax25_connect_dry_run_script script;
	struct kn_ax25_connect_dry_run_error_info error;

	if (write_temp("name bad\nlocal M6VPN-1\nremote BAD-CALL-TOO-LONG\n"
	    "port kiss0\n", path, sizeof(path)) != 0)
		return 1;
	if (kn_ax25_connect_dry_run_parse_file(path, &script, &error) !=
	    KN_AX25_CONNECT_DRY_RUN_ERR_PARSE) {
		(void)unlink(path);
		return 1;
	}
	(void)unlink(path);
	return error.line == 3 ? 0 : 1;
}

static int
test_invalid_expect(void)
{
	char path[128];
	struct kn_ax25_connect_dry_run_script script;
	struct kn_ax25_connect_dry_run_error_info error;

	if (write_temp("name bad\nlocal M6VPN-1\nremote N0CALL\n"
	    "port kiss0\nexpect tx-writes 1\n", path, sizeof(path)) != 0)
		return 1;
	if (kn_ax25_connect_dry_run_parse_file(path, &script, &error) !=
	    KN_AX25_CONNECT_DRY_RUN_OK) {
		(void)unlink(path);
		return 1;
	}
	if (kn_ax25_connect_dry_run_run_script(&script,
	    &(struct kn_ax25_connect_dry_run_result){0}) !=
	    KN_AX25_CONNECT_DRY_RUN_ERR_MISMATCH) {
		(void)unlink(path);
		return 1;
	}
	(void)unlink(path);
	return 0;
}

static int
test_invalid_params(void)
{
	char path[128];
	struct kn_ax25_connect_dry_run_script script;
	struct kn_ax25_connect_dry_run_error_info error;

	if (write_temp("name bad\nlocal M6VPN-1\nremote N0CALL\n"
	    "port kiss0\nparams modulo=8 window=0 paclen=256 "
	    "t1=3000 t2=1000 t3=300000 n2=3\n", path,
	    sizeof(path)) != 0)
		return 1;
	if (kn_ax25_connect_dry_run_parse_file(path, &script, &error) !=
	    KN_AX25_CONNECT_DRY_RUN_ERR_PARSE) {
		(void)unlink(path);
		return 1;
	}
	(void)unlink(path);
	return error.line == 5 ? 0 : 1;
}

static int
test_missing_header(void)
{
	char path[128];
	struct kn_ax25_connect_dry_run_script script;
	struct kn_ax25_connect_dry_run_error_info error;

	if (write_temp("name bad\nlocal M6VPN-1\nport kiss0\n", path,
	    sizeof(path)) != 0)
		return 1;
	if (kn_ax25_connect_dry_run_parse_file(path, &script, &error) !=
	    KN_AX25_CONNECT_DRY_RUN_ERR_PARSE) {
		(void)unlink(path);
		return 1;
	}
	(void)unlink(path);
	return error.line == 0 ? 0 : 1;
}

static int
test_valid_script(void)
{
	char path[128];
	struct kn_ax25_connect_dry_run_script script;
	struct kn_ax25_connect_dry_run_error_info error;

	if (write_temp("name ok\nlocal M6VPN-1\nremote N0CALL\n"
	    "port kiss0\nparams modulo=8 window=1 paclen=128 "
	    "max-info=256 t1=3000 t2=1000 t3=300000 n2=3\n"
	    "expect planned-state awaiting-connection\n"
	    "expect frame-kind SABM\nexpect bridge blocked\n"
	    "expect connection-created false\nexpect tx-writes 0\n"
	    "expect dispatch-calls 0\nexpect fx25-frames 0\n", path,
	    sizeof(path)) != 0)
		return 1;
	if (kn_ax25_connect_dry_run_parse_file(path, &script, &error) !=
	    KN_AX25_CONNECT_DRY_RUN_OK) {
		(void)unlink(path);
		return 1;
	}
	(void)unlink(path);
	return strcmp(script.name, "ok") == 0 &&
	    script.command_count == 7 && script.params.paclen == 128 ? 0 : 1;
}

static int
write_temp(const char *text, char *path, size_t path_len)
{
	FILE *fp;
	int needed;

	needed = snprintf(path, path_len,
	    "/tmp/kilonode-connect-dry-run-script-%llu.connectdry",
	    (unsigned long long)getpid());
	if (needed < 0 || (size_t)needed >= path_len)
		return 1;
	fp = fopen(path, "w");
	if (fp == NULL)
		return 1;
	(void)fputs(text, fp);
	(void)fclose(fp);
	return 0;
}
