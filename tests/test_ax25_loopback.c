/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_ax25_loopback.c */

#include <sys/types.h>

#include <stdio.h>
#include <unistd.h>

#include "kilonode/ax25_loopback.h"

static int run_fixture(const char *);
static int test_connect_disconnect(void);
static int test_mismatch_fails(void);
static const char *fixture_path(const char *);

int
main(void)
{
	if (test_connect_disconnect() != 0)
		return 1;
	if (test_mismatch_fails() != 0)
		return 1;
	return 0;
}

static const char *
fixture_path(const char *path)
{
	FILE *fp;
	static char fallback[256];
	int needed;

	fp = fopen(path, "r");
	if (fp != NULL) {
		(void)fclose(fp);
		return path;
	}
	needed = snprintf(fallback, sizeof(fallback), "../%s", path);
	if (needed < 0 || (size_t)needed >= sizeof(fallback))
		return path;
	return fallback;
}

static int
run_fixture(const char *path)
{
	struct kn_ax25_loopback_result result;
	struct kn_ax25_loopback_error_info error;

	if (kn_ax25_loopback_run_file(path, &result, &error) !=
	    KN_AX25_LOOPBACK_OK)
		return 1;
	return result.pass != 0 && result.real_tx_queue_writes == 0 &&
	    result.dispatch_calls == 0 && result.fx25_frames_generated == 0 ?
	    0 : 1;
}

static int
test_connect_disconnect(void)
{
	return run_fixture(fixture_path(
	    "tests/fixtures/ax25-loopback/connect-disconnect.loop"));
}

static int
test_mismatch_fails(void)
{
	char path[128];
	FILE *fp;
	struct kn_ax25_loopback_result result;
	struct kn_ax25_loopback_error_info error;
	int needed;

	needed = snprintf(path, sizeof(path),
	    "/tmp/kilonode-loopback-mismatch-%llu.loop",
	    (unsigned long long)getpid());
	if (needed < 0 || (size_t)needed >= sizeof(path))
		return 1;
	fp = fopen(path, "w");
	if (fp == NULL)
		return 1;
	(void)fputs("name mismatch\nendpoint-a M6VPN-1\n"
	    "endpoint-b N0CALL\nport kiss0\nexpect A state=connected\n",
	    fp);
	(void)fclose(fp);
	if (kn_ax25_loopback_run_file(path, &result, &error) !=
	    KN_AX25_LOOPBACK_ERR_MISMATCH) {
		(void)unlink(path);
		return 1;
	}
	(void)unlink(path);
	return result.mismatch_count == 1 ? 0 : 1;
}
