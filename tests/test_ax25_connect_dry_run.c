/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_ax25_connect_dry_run.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "kilonode/ax25_connect_dry_run.h"

static int test_basic_plan(void);
static int test_mismatch_fails(void);
static int test_modulo_128_plan(void);
static int write_temp(const char *, char *, size_t);

int
main(void)
{
	if (test_basic_plan() != 0)
		return 1;
	if (test_modulo_128_plan() != 0)
		return 1;
	if (test_mismatch_fails() != 0)
		return 1;
	return 0;
}

static int
test_basic_plan(void)
{
	char path[128];
	char report[KN_AX25_CONNECT_DRY_RUN_REPORT_MAX];
	struct kn_ax25_connect_dry_run_result result;
	struct kn_ax25_connect_dry_run_error_info error;

	if (write_temp("name basic\nlocal M6VPN-1\nremote N0CALL\n"
	    "port kiss0\nparams modulo=8 window=1 paclen=256 "
	    "t1=3000 t2=1000 t3=300000 n2=3\n"
	    "expect planned-state awaiting-connection\n"
	    "expect frame-kind SABM\nexpect bridge blocked\n"
	    "expect connection-created false\nexpect tx-writes 0\n"
	    "expect dispatch-calls 0\nexpect fx25-frames 0\n",
	    path, sizeof(path)) != 0)
		return 1;
	if (kn_ax25_connect_dry_run_run_file(path, &result, &error) !=
	    KN_AX25_CONNECT_DRY_RUN_OK) {
		(void)unlink(path);
		return 1;
	}
	if (kn_ax25_connect_dry_run_format_report(&result, report,
	    sizeof(report)) != KN_AX25_CONNECT_DRY_RUN_OK) {
		(void)unlink(path);
		return 1;
	}
	(void)unlink(path);
	if (strstr(report, "tx_writes=0") == NULL ||
	    strstr(report, "dispatch=0") == NULL ||
	    strstr(report, "fx25=0") == NULL)
		return 1;
	return result.pass != 0 && result.connection_created == 0 &&
	    result.bridge_blocked != 0 ? 0 : 1;
}

static int
test_mismatch_fails(void)
{
	char path[128];
	struct kn_ax25_connect_dry_run_result result;
	struct kn_ax25_connect_dry_run_error_info error;

	if (write_temp("name bad\nlocal M6VPN-1\nremote N0CALL\n"
	    "port kiss0\nexpect frame-kind SABME\n", path,
	    sizeof(path)) != 0)
		return 1;
	if (kn_ax25_connect_dry_run_run_file(path, &result, &error) !=
	    KN_AX25_CONNECT_DRY_RUN_ERR_MISMATCH) {
		(void)unlink(path);
		return 1;
	}
	(void)unlink(path);
	return result.first_mismatch_line == 5 &&
	    result.tx_writes == 0 && result.dispatch_calls == 0 ? 0 : 1;
}

static int
test_modulo_128_plan(void)
{
	char path[128];
	struct kn_ax25_connect_dry_run_result result;
	struct kn_ax25_connect_dry_run_error_info error;

	if (write_temp("name sabme\nlocal M6VPN-1\nremote N0CALL\n"
	    "port kiss0\nparams modulo=128 window=1 paclen=256 "
	    "t1=3000 t2=1000 t3=300000 n2=3\n"
	    "expect frame-kind SABME\nexpect tx-writes 0\n", path,
	    sizeof(path)) != 0)
		return 1;
	if (kn_ax25_connect_dry_run_run_file(path, &result, &error) !=
	    KN_AX25_CONNECT_DRY_RUN_OK) {
		(void)unlink(path);
		return 1;
	}
	(void)unlink(path);
	return strcmp(result.frame_kind, "SABME") == 0 &&
	    result.fx25_frames == 0 ? 0 : 1;
}

static int
write_temp(const char *text, char *path, size_t path_len)
{
	FILE *fp;
	int needed;

	needed = snprintf(path, path_len,
	    "/tmp/kilonode-connect-dry-run-%llu.connectdry",
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
