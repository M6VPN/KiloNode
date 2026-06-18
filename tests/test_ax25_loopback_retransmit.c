/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_ax25_loopback_retransmit.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "kilonode/ax25_loopback.h"
#include "kilonode/ax25_loopback_retransmit.h"

static const char *fixture_path(const char *);
static int run_fixture(const char *);
static int test_ack(void);
static int test_empty(void);
static int test_full(void);
static int test_fixture_one_frame(void);
static int test_fixture_retry_buffer(void);
static int test_fixture_windowed(void);
static int test_mark_rej_and_replay(void);

int
main(void)
{
	if (test_empty() != 0)
		return 1;
	if (test_ack() != 0)
		return 1;
	if (test_mark_rej_and_replay() != 0)
		return 1;
	if (test_full() != 0)
		return 1;
	if (test_fixture_one_frame() != 0)
		return 1;
	if (test_fixture_windowed() != 0)
		return 1;
	if (test_fixture_retry_buffer() != 0)
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
	return result.retransmit_replayed > 0 ? 0 : 1;
}

static int
test_ack(void)
{
	struct kn_ax25_loopback_retransmit_buffer buffer;
	uint8_t raw[3] = { 0x01, 0x02, 0x03 };
	uint64_t id;
	size_t acked;

	kn_ax25_loopback_retransmit_init(&buffer);
	if (kn_ax25_loopback_retransmit_record(&buffer, 7, 0, 3, 0,
	    raw, sizeof(raw), &id) != KN_AX25_LOOPBACK_RETRANSMIT_OK)
		return 1;
	if (kn_ax25_loopback_retransmit_ack_rr(&buffer, 0, &acked) !=
	    KN_AX25_LOOPBACK_RETRANSMIT_OK)
		return 1;
	if (acked != 1 || buffer.acked != 1)
		return 1;
	return kn_ax25_loopback_retransmit_count_in_flight(&buffer) == 0 ?
	    0 : 1;
}

static int
test_empty(void)
{
	struct kn_ax25_loopback_retransmit_buffer buffer;

	kn_ax25_loopback_retransmit_init(&buffer);
	if (buffer.next_id != 1 || buffer.count != 0)
		return 1;
	return kn_ax25_loopback_retransmit_count_retry_needed(&buffer) == 0 ?
	    0 : 1;
}

static int
test_full(void)
{
	struct kn_ax25_loopback_retransmit_buffer buffer;
	uint8_t raw[1] = { 0x00 };
	uint64_t id;
	size_t i;

	kn_ax25_loopback_retransmit_init(&buffer);
	for (i = 0; i < KN_AX25_LOOPBACK_RETRANSMIT_MAX; i++) {
		if (kn_ax25_loopback_retransmit_record(&buffer,
		    (uint8_t)(i & 7U), 0, 1, i, raw, sizeof(raw), &id) !=
		    KN_AX25_LOOPBACK_RETRANSMIT_OK)
			return 1;
	}
	if (kn_ax25_loopback_retransmit_record(&buffer, 0, 0, 1, i,
	    raw, sizeof(raw), &id) != KN_AX25_LOOPBACK_RETRANSMIT_ERR_FULL)
		return 1;
	return buffer.full == 1 ? 0 : 1;
}

static int
test_fixture_one_frame(void)
{
	return run_fixture(fixture_path(
	    "tests/fixtures/ax25-loopback/rej-replay-one-frame.loop"));
}

static int
test_fixture_retry_buffer(void)
{
	return run_fixture(fixture_path(
	    "tests/fixtures/ax25-loopback/retry-buffer-diagnostic.loop"));
}

static int
test_fixture_windowed(void)
{
	return run_fixture(fixture_path(
	    "tests/fixtures/ax25-loopback/rej-replay-windowed.loop"));
}

static int
test_mark_rej_and_replay(void)
{
	struct kn_ax25_loopback_retransmit_buffer buffer;
	uint8_t raw[3] = { 0xaa, 0xbb, 0xcc };
	uint8_t out[3];
	uint64_t id;
	size_t marked;
	size_t written;

	kn_ax25_loopback_retransmit_init(&buffer);
	if (kn_ax25_loopback_retransmit_record(&buffer, 2, 0, 3, 0,
	    raw, sizeof(raw), &id) != KN_AX25_LOOPBACK_RETRANSMIT_OK)
		return 1;
	if (kn_ax25_loopback_retransmit_mark_rej(&buffer, 2, &marked) !=
	    KN_AX25_LOOPBACK_RETRANSMIT_OK)
		return 1;
	if (marked != 1 || buffer.retry_marked != 1)
		return 1;
	if (kn_ax25_loopback_retransmit_next(&buffer, out, sizeof(out),
	    &written) != KN_AX25_LOOPBACK_RETRANSMIT_OK)
		return 1;
	if (written != sizeof(raw) || memcmp(out, raw, sizeof(raw)) != 0)
		return 1;
	return buffer.replayed == 1 ? 0 : 1;
}
