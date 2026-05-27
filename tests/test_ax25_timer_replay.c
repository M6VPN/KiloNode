/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_ax25_timer_replay.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "kilonode/ax25_timer_replay.h"

static int run_fixture(const char *);
static const char *fixture_path(const char *);
static int test_connect_timeout_exhaust(void);
static int test_connect_timeout_retry(void);
static int test_connect_ua_stops_t1(void);
static int test_connected_t3(void);
static int test_disconnect_timeout_retry(void);
static int test_mismatch_fails(void);
static int test_t2_placeholder(void);

int
main(void)
{
	if (test_connect_timeout_retry() != 0)
		return 1;
	if (test_connect_timeout_exhaust() != 0)
		return 1;
	if (test_connect_ua_stops_t1() != 0)
		return 1;
	if (test_connected_t3() != 0)
		return 1;
	if (test_disconnect_timeout_retry() != 0)
		return 1;
	if (test_t2_placeholder() != 0)
		return 1;
	if (test_mismatch_fails() != 0)
		return 1;

	return 0;
}

static int
fixture_exists(const char *path)
{
	FILE *fp;

	fp = fopen(path, "r");
	if (fp == NULL)
		return 0;
	(void)fclose(fp);
	return 1;
}

static const char *
fixture_path(const char *path)
{
	static char fallback[256];
	int needed;

	if (fixture_exists(path) != 0)
		return path;
	needed = snprintf(fallback, sizeof(fallback), "../%s", path);
	if (needed < 0 || (size_t)needed >= sizeof(fallback))
		return path;
	return fallback;
}

static int
run_fixture(const char *path)
{
	struct kn_ax25_timer_replay_result result;
	struct kn_ax25_timer_replay_error_info error;

	if (kn_ax25_timer_replay_run_file(path, &result, &error) !=
	    KN_AX25_TIMER_REPLAY_OK)
		return 1;
	if (result.pass == 0 || result.tx_writes_observed != 0 ||
	    result.prepared_tx_writes_observed != 0)
		return 1;

	return 0;
}

static int
test_connect_timeout_exhaust(void)
{
	return run_fixture(
	    fixture_path(
	    "tests/fixtures/ax25-timer/connect-timeout-exhaust.replay"));
}

static int
test_connect_timeout_retry(void)
{
	return run_fixture(
	    fixture_path(
	    "tests/fixtures/ax25-timer/connect-timeout-retry.replay"));
}

static int
test_connect_ua_stops_t1(void)
{
	return run_fixture(
	    fixture_path(
	    "tests/fixtures/ax25-timer/connect-ua-stops-t1.replay"));
}

static int
test_connected_t3(void)
{
	return run_fixture(
	    fixture_path(
	    "tests/fixtures/ax25-timer/connected-t3-poll.replay"));
}

static int
test_disconnect_timeout_retry(void)
{
	return run_fixture(
	    fixture_path(
	    "tests/fixtures/ax25-timer/disconnect-timeout-retry.replay"));
}

static int
test_mismatch_fails(void)
{
	char path[128];
	FILE *fp;
	struct kn_ax25_timer_replay_result result;
	struct kn_ax25_timer_replay_error_info error;
	int needed;

	needed = snprintf(path, sizeof(path),
	    "/tmp/kilonode-timer-replay-mismatch-%llu.replay",
	    (unsigned long long)getpid());
	if (needed < 0 || (size_t)needed >= sizeof(path))
		return 1;
	fp = fopen(path, "w");
	if (fp == NULL)
		return 1;
	(void)fputs("name mismatch\nnode M6VPN-1\nremote N0CALL\n"
	    "port kiss0\nevent local-connect\nexpect state=connected\n",
	    fp);
	(void)fclose(fp);
	if (kn_ax25_timer_replay_run_file(path, &result, &error) !=
	    KN_AX25_TIMER_REPLAY_ERR_MISMATCH) {
		(void)unlink(path);
		return 1;
	}
	(void)unlink(path);
	return result.mismatch_count == 1 && result.tx_writes_observed == 0 &&
	    result.prepared_tx_writes_observed == 0 ? 0 : 1;
}

static int
test_t2_placeholder(void)
{
	return run_fixture(fixture_path(
	    "tests/fixtures/ax25-timer/t2-placeholder.replay"));
}
