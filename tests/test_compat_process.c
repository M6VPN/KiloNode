/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_compat_process.c */

#include <sys/types.h>

#include <string.h>

#include "kilonode/compat_process.h"

static int test_missing_binary(void);
static int test_non_executable(void);
static int test_output_bounded(void);
static int test_stdout_stderr_capture(void);
static int test_timeout(void);

int
main(void)
{
	if (test_missing_binary() != 0)
		return 1;
	if (test_non_executable() != 0)
		return 1;
	if (test_stdout_stderr_capture() != 0)
		return 1;
	if (test_timeout() != 0)
		return 1;
	if (test_output_bounded() != 0)
		return 1;

	return 0;
}

static int
test_missing_binary(void)
{
	struct kn_compat_process_result result;

	return kn_compat_process_run("/no/such/kilonode-helper", NULL, "",
	    1, &result) == KN_COMPAT_PROCESS_ERR_NOT_EXECUTABLE ? 0 : 1;
}

static int
test_non_executable(void)
{
	struct kn_compat_process_result result;

	return kn_compat_process_run("../tests/fixtures/compat/"
	    "blackbox-node-help.observation", NULL, "", 1, &result) ==
	    KN_COMPAT_PROCESS_ERR_NOT_EXECUTABLE ? 0 : 1;
}

static int
test_output_bounded(void)
{
	struct kn_compat_process_result result;

	return kn_compat_process_run("./compat_process_helper", "large", "",
	    1, &result) == KN_COMPAT_PROCESS_ERR_OUTPUT_TOO_LARGE ? 0 : 1;
}

static int
test_stdout_stderr_capture(void)
{
	struct kn_compat_process_result result;

	if (kn_compat_process_run("./compat_process_helper", NULL,
	    "hello\n", 2, &result) != KN_COMPAT_PROCESS_OK)
		return 1;
	if (strstr(result.stdout_text, "helper stdout") == NULL)
		return 1;
	if (strstr(result.stderr_text, "helper stderr") == NULL)
		return 1;

	return strstr(result.stdout_text, "input=hello") != NULL ? 0 : 1;
}

static int
test_timeout(void)
{
	struct kn_compat_process_result result;

	if (kn_compat_process_run("./compat_process_helper", "slow", "",
	    1, &result) != KN_COMPAT_PROCESS_ERR_TIMEOUT)
		return 1;

	return result.timed_out != 0 && result.terminated != 0 ? 0 : 1;
}
