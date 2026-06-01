/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_ax25_loopback_payload.c */

#include <sys/types.h>

#include <stdio.h>

#include "kilonode/ax25_loopback.h"

static const char *fixture_path(const char *);
static int run_fixture(const char *);
static int test_binary_payload(void);
static int test_sequence_mismatch(void);
static int test_text_payload(void);

int
main(void)
{
	if (test_text_payload() != 0)
		return 1;
	if (test_binary_payload() != 0)
		return 1;
	if (test_sequence_mismatch() != 0)
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
test_binary_payload(void)
{
	return run_fixture(fixture_path(
	    "tests/fixtures/ax25-loopback/i-frame-binary-payload.loop"));
}

static int
test_sequence_mismatch(void)
{
	return run_fixture(fixture_path(
	    "tests/fixtures/ax25-loopback/i-frame-sequence-mismatch.loop"));
}

static int
test_text_payload(void)
{
	return run_fixture(fixture_path(
	    "tests/fixtures/ax25-loopback/connect-i-rr-disconnect.loop"));
}
