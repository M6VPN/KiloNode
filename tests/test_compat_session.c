/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_compat_session.c */

#include <sys/types.h>

#include <string.h>

#include "kilonode/compat_session.h"

static int test_command_limit(void);
static int test_connection_failure(void);
static int test_invalid_host(void);
static int test_invalid_port(void);
static int test_validation_ok(void);

int
main(void)
{
	if (test_invalid_host() != 0)
		return 1;
	if (test_invalid_port() != 0)
		return 1;
	if (test_command_limit() != 0)
		return 1;
	if (test_connection_failure() != 0)
		return 1;
	if (test_validation_ok() != 0)
		return 1;

	return 0;
}

static int
test_command_limit(void)
{
	struct kn_compat_session_result result;
	char command[KN_COMPAT_SESSION_COMMAND_MAX + 8];

	memset(command, 'A', sizeof(command));
	command[sizeof(command) - 1] = '\0';
	return kn_compat_session_tcp_line("127.0.0.1", "1", command, 1,
	    &result) == KN_COMPAT_SESSION_ERR_COMMAND_TOO_LARGE ? 0 : 1;
}

static int
test_connection_failure(void)
{
	struct kn_compat_session_result result;
	enum kn_compat_session_error rc;

	rc = kn_compat_session_tcp_line("127.0.0.1", "9", "HELP", 1,
	    &result);
	return rc == KN_COMPAT_SESSION_ERR_CONNECT ||
	    rc == KN_COMPAT_SESSION_ERR_TIMEOUT ? 0 : 1;
}

static int
test_invalid_host(void)
{
	return kn_compat_session_validate_endpoint("../bad", "8010") ==
	    KN_COMPAT_SESSION_ERR_INVALID_HOST ? 0 : 1;
}

static int
test_invalid_port(void)
{
	if (kn_compat_session_validate_endpoint("127.0.0.1", "0") !=
	    KN_COMPAT_SESSION_ERR_INVALID_PORT)
		return 1;

	return kn_compat_session_validate_endpoint("127.0.0.1", "bad") ==
	    KN_COMPAT_SESSION_ERR_INVALID_PORT ? 0 : 1;
}

static int
test_validation_ok(void)
{
	return kn_compat_session_validate_endpoint("127.0.0.1", "8010") ==
	    KN_COMPAT_SESSION_OK ? 0 : 1;
}
