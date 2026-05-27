/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_ax25_timer_replay_script.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "kilonode/ax25_timer_replay_script.h"

static int parse_fixture(const char *);
static int test_invalid_callsign(void);
static int test_invalid_expectation(void);
static int test_invalid_params(void);
static int test_missing_name(void);
static int test_overlong_line(void);
static int test_unknown_command(void);
static int test_valid_script(void);
static int write_temp(const char *, char *, size_t);
static const char *fixture_path(const char *);

int
main(void)
{
	if (test_valid_script() != 0)
		return 1;
	if (test_missing_name() != 0)
		return 1;
	if (test_invalid_callsign() != 0)
		return 1;
	if (test_unknown_command() != 0)
		return 1;
	if (test_invalid_params() != 0)
		return 1;
	if (test_invalid_expectation() != 0)
		return 1;
	if (test_overlong_line() != 0)
		return 1;

	return 0;
}

static int
parse_fixture(const char *path)
{
	struct kn_ax25_timer_replay_script script;
	struct kn_ax25_timer_replay_error_info error;

	return kn_ax25_timer_replay_script_parse_file(path, &script,
	    &error) == KN_AX25_TIMER_REPLAY_OK ? 0 : 1;
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
test_invalid_callsign(void)
{
	char path[128];
	struct kn_ax25_timer_replay_script script;
	struct kn_ax25_timer_replay_error_info error;
	const char text[] =
	    "name invalid-callsign\n"
	    "node BAD!CALL\n"
	    "remote N0CALL\n"
	    "port kiss0\n";

	if (write_temp(text, path, sizeof(path)) != 0)
		return 1;
	if (kn_ax25_timer_replay_script_parse_file(path, &script, &error) !=
	    KN_AX25_TIMER_REPLAY_ERR_PARSE) {
		(void)unlink(path);
		return 1;
	}
	(void)unlink(path);
	return error.line == 0 ? 0 : 1;
}

static int
test_invalid_expectation(void)
{
	char path[128];
	struct kn_ax25_timer_replay_script script;
	struct kn_ax25_timer_replay_error_info error;
	const char text[] =
	    "name invalid-expect\n"
	    "node M6VPN-1\n"
	    "remote N0CALL\n"
	    "port kiss0\n"
	    "expect unknown=1\n";

	if (write_temp(text, path, sizeof(path)) != 0)
		return 1;
	if (kn_ax25_timer_replay_script_parse_file(path, &script, &error) !=
	    KN_AX25_TIMER_REPLAY_ERR_PARSE) {
		(void)unlink(path);
		return 1;
	}
	(void)unlink(path);
	return error.line == 5 ? 0 : 1;
}

static int
test_invalid_params(void)
{
	char path[128];
	struct kn_ax25_timer_replay_script script;
	struct kn_ax25_timer_replay_error_info error;
	const char text[] =
	    "name invalid-params\n"
	    "node M6VPN-1\n"
	    "remote N0CALL\n"
	    "port kiss0\n"
	    "params modulo=8 window=0 t1=1 t2=1 t3=1 n2=1\n";

	if (write_temp(text, path, sizeof(path)) != 0)
		return 1;
	if (kn_ax25_timer_replay_script_parse_file(path, &script, &error) !=
	    KN_AX25_TIMER_REPLAY_ERR_PARSE) {
		(void)unlink(path);
		return 1;
	}
	(void)unlink(path);
	return error.line == 5 ? 0 : 1;
}

static int
test_missing_name(void)
{
	char path[128];
	struct kn_ax25_timer_replay_script script;
	struct kn_ax25_timer_replay_error_info error;
	const char text[] =
	    "node M6VPN-1\n"
	    "remote N0CALL\n"
	    "port kiss0\n";

	if (write_temp(text, path, sizeof(path)) != 0)
		return 1;
	if (kn_ax25_timer_replay_script_parse_file(path, &script, &error) !=
	    KN_AX25_TIMER_REPLAY_ERR_PARSE) {
		(void)unlink(path);
		return 1;
	}
	(void)unlink(path);
	return strcmp(error.message, "missing-name") == 0 ? 0 : 1;
}

static int
test_overlong_line(void)
{
	char path[128];
	FILE *fp;
	struct kn_ax25_timer_replay_script script;
	struct kn_ax25_timer_replay_error_info error;
	size_t i;
	int needed;

	needed = snprintf(path, sizeof(path),
	    "/tmp/kilonode-timer-replay-long-%llu.replay",
	    (unsigned long long)getpid());
	if (needed < 0 || (size_t)needed >= sizeof(path))
		return 1;
	fp = fopen(path, "w");
	if (fp == NULL)
		return 1;
	for (i = 0; i < KN_AX25_TIMER_REPLAY_LINE_MAX + 4U; i++)
		(void)fputc('x', fp);
	(void)fputc('\n', fp);
	(void)fclose(fp);
	if (kn_ax25_timer_replay_script_parse_file(path, &script, &error) !=
	    KN_AX25_TIMER_REPLAY_ERR_PARSE) {
		(void)unlink(path);
		return 1;
	}
	(void)unlink(path);
	return strcmp(error.message, "line-too-long") == 0 ? 0 : 1;
}

static int
test_unknown_command(void)
{
	char path[128];
	struct kn_ax25_timer_replay_script script;
	struct kn_ax25_timer_replay_error_info error;
	const char text[] =
	    "name unknown-command\n"
	    "node M6VPN-1\n"
	    "remote N0CALL\n"
	    "port kiss0\n"
	    "shell rm\n";

	if (write_temp(text, path, sizeof(path)) != 0)
		return 1;
	if (kn_ax25_timer_replay_script_parse_file(path, &script, &error) !=
	    KN_AX25_TIMER_REPLAY_ERR_PARSE) {
		(void)unlink(path);
		return 1;
	}
	(void)unlink(path);
	return error.line == 5 ? 0 : 1;
}

static int
test_valid_script(void)
{
	return parse_fixture(
	    fixture_path(
	    "tests/fixtures/ax25-timer/connect-timeout-retry.replay"));
}

static int
write_temp(const char *text, char *path, size_t path_len)
{
	FILE *fp;
	int needed;

	needed = snprintf(path, path_len,
	    "/tmp/kilonode-timer-replay-%llu.replay",
	    (unsigned long long)getpid());
	if (needed < 0 || (size_t)needed >= path_len)
		return 1;
	fp = fopen(path, "w");
	if (fp == NULL)
		return 1;
	if (fputs(text, fp) < 0) {
		(void)fclose(fp);
		return 1;
	}
	(void)fclose(fp);
	return 0;
}
