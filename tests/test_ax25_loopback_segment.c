/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_ax25_loopback_segment.c */

#include <sys/types.h>

#include <stdio.h>

#include "kilonode/ax25_loopback.h"

static const char *fixture_path(const char *);
static int run_fixture(const char *);
static int test_binary_segmented(void);
static int test_boundary(void);
static int test_text_segmented(void);
static int test_window_one(void);

int
main(void)
{
	if (test_text_segmented() != 0)
		return 1;
	if (test_binary_segmented() != 0)
		return 1;
	if (test_boundary() != 0)
		return 1;
	if (test_window_one() != 0)
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
	if (result.pass == 0 || result.real_tx_queue_writes != 0 ||
	    result.dispatch_calls != 0 || result.fx25_frames_generated != 0)
		return 1;
	return result.segments_sent == result.segments_received ? 0 : 1;
}

static int
test_binary_segmented(void)
{
	return run_fixture(fixture_path(
	    "tests/fixtures/ax25-loopback/segmented-binary-payload.loop"));
}

static int
test_boundary(void)
{
	return run_fixture(fixture_path(
	    "tests/fixtures/ax25-loopback/paclen-boundary.loop"));
}

static int
test_text_segmented(void)
{
	return run_fixture(fixture_path(
	    "tests/fixtures/ax25-loopback/segmented-text-payload.loop"));
}

static int
test_window_one(void)
{
	return run_fixture(fixture_path(
	    "tests/fixtures/ax25-loopback/window-one-segmented.loop"));
}
