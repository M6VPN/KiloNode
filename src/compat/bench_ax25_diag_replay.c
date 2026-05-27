/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/compat/bench_ax25_diag_replay.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "kilonode/ax25.h"
#include "kilonode/ax25_connection.h"
#include "kilonode/ax25_rx_feed.h"
#include "kilonode/bench_ax25_diag_replay.h"
#include "kilonode/buffer.h"
#include "kilonode/kiss_stream.h"

static void add_mismatch(struct kn_bench_diag_result *, const char *);
static enum kn_bench_diag_replay_error decode_ax25(
	const struct kn_compat_packet_capture *, struct kn_ax25_frame *);
static enum kn_bench_diag_replay_error decode_kiss(
	const struct kn_compat_packet_capture *, struct kn_ax25_frame *);
static const char *final_state_name(const struct kn_ax25_runtime *);
static uint8_t planned_placeholder(const char *);
static void replay_runtime_init(struct kn_ax25_runtime *);
static void result_from_runtime(struct kn_bench_diag_result *,
	const struct kn_ax25_runtime *);

const char *
kn_bench_diag_replay_error_name(enum kn_bench_diag_replay_error error)
{
	switch (error) {
	case KN_BENCH_DIAG_REPLAY_OK:
		return "ok";
	case KN_BENCH_DIAG_REPLAY_ERR_INVALID_ARGUMENT:
		return "invalid-argument";
	case KN_BENCH_DIAG_REPLAY_ERR_UNSUPPORTED:
		return "unsupported";
	case KN_BENCH_DIAG_REPLAY_ERR_PARSE:
		return "parse";
	case KN_BENCH_DIAG_REPLAY_ERR_DECODE:
		return "decode";
	case KN_BENCH_DIAG_REPLAY_ERR_RUNTIME:
		return "runtime";
	case KN_BENCH_DIAG_REPLAY_ERR_MISMATCH:
		return "mismatch";
	}

	return "unknown";
}

void
kn_bench_diag_result_clear(struct kn_bench_diag_result *result)
{
	if (result == NULL)
		return;

	memset(result, 0, sizeof(*result));
	result->pass = 1;
	(void)snprintf(result->final_state, sizeof(result->final_state),
	    "none");
}

enum kn_bench_diag_replay_error
kn_bench_diag_replay_capture(const char *path,
	struct kn_bench_diag_result *result)
{
	struct kn_compat_packet_capture capture;

	if (path == NULL || result == NULL)
		return KN_BENCH_DIAG_REPLAY_ERR_INVALID_ARGUMENT;
	if (kn_compat_packet_capture_parse_file(path, &capture, NULL) !=
	    KN_COMPAT_PACKET_CAPTURE_OK)
		return KN_BENCH_DIAG_REPLAY_ERR_PARSE;

	return kn_bench_diag_replay_capture_parsed(path, &capture, result);
}

enum kn_bench_diag_replay_error
kn_bench_diag_replay_capture_parsed(const char *path,
	const struct kn_compat_packet_capture *capture,
	struct kn_bench_diag_result *result)
{
	struct kn_ax25_runtime runtime;
	struct kn_ax25_rx_feed_options options;
	struct kn_ax25_rx_feed_result feed_result;
	struct kn_ax25_frame frame;
	struct kn_callsign local;
	const char *port;
	enum kn_bench_diag_replay_error rc;

	if (capture == NULL || result == NULL)
		return KN_BENCH_DIAG_REPLAY_ERR_INVALID_ARGUMENT;

	kn_bench_diag_result_clear(result);
	(void)snprintf(result->capture_name, sizeof(result->capture_name),
	    "%s", capture->name);
	(void)snprintf(result->capture_path, sizeof(result->capture_path),
	    "%s", path == NULL ? capture->name : path);
	result->method = capture->method;
	result->frames_parsed = 1;

	if (planned_placeholder(path) != 0 ||
	    planned_placeholder(capture->name) != 0) {
		result->unsupported = 1;
		result->pass = 1;
		add_mismatch(result, "planned-placeholder");
		return KN_BENCH_DIAG_REPLAY_OK;
	}

	rc = decode_ax25(capture, &frame);
	if (rc != KN_BENCH_DIAG_REPLAY_OK) {
		result->pass = 0;
		result->malformed_frames = 1;
		add_mismatch(result, "decode");
		return rc;
	}
	result->frames_decoded = 1;

	replay_runtime_init(&runtime);
	kn_ax25_rx_feed_options_from_runtime(&runtime, &options);
	(void)kn_callsign_parse("M6VPN-1", &local);
	port = capture->port[0] != '\0' ? capture->port : "bench0";
	(void)kn_ax25_rx_feed_frame(&runtime, &options, port, &local, &frame,
	    capture->timestamp == 0 ? 1 : capture->timestamp, &feed_result);
	result_from_runtime(result, &runtime);
	kn_ax25_runtime_free(&runtime);

	if (result->tx_writes_attempted != 0) {
		result->pass = 0;
		add_mismatch(result, "tx-writes");
		return KN_BENCH_DIAG_REPLAY_ERR_MISMATCH;
	}

	return KN_BENCH_DIAG_REPLAY_OK;
}

static void
add_mismatch(struct kn_bench_diag_result *result, const char *text)
{
	if (result == NULL ||
	    result->mismatch_count >= KN_BENCH_DIAG_MISMATCH_MAX)
		return;
	(void)snprintf(result->mismatches[result->mismatch_count].text,
	    sizeof(result->mismatches[result->mismatch_count].text), "%s",
	    text == NULL ? "-" : text);
	result->mismatch_count++;
}

static enum kn_bench_diag_replay_error
decode_ax25(const struct kn_compat_packet_capture *capture,
	struct kn_ax25_frame *frame)
{
	if (capture->method == KN_COMPAT_PACKET_METHOD_KISS)
		return decode_kiss(capture, frame);
	if (capture->method == KN_COMPAT_PACKET_METHOD_AXIP ||
	    capture->method == KN_COMPAT_PACKET_METHOD_AXUDP) {
		return kn_ax25_frame_decode(capture->frame,
		    capture->frame_len, frame) == KN_AX25_OK ?
		    KN_BENCH_DIAG_REPLAY_OK :
		    KN_BENCH_DIAG_REPLAY_ERR_DECODE;
	}

	return KN_BENCH_DIAG_REPLAY_ERR_UNSUPPORTED;
}

static enum kn_bench_diag_replay_error
decode_kiss(const struct kn_compat_packet_capture *capture,
	struct kn_ax25_frame *frame)
{
	struct kn_kiss_stream_parser parser;
	struct kn_kiss_stream_frame kiss_frame;
	struct kn_buffer out;
	enum kn_bench_diag_replay_error rc;

	if (kn_kiss_stream_init(&parser, KN_COMPAT_CAPTURE_FRAME_MAX) !=
	    KN_KISS_STREAM_OK)
		return KN_BENCH_DIAG_REPLAY_ERR_DECODE;
	if (kn_buffer_init(&out, 0) != 0) {
		kn_kiss_stream_free(&parser);
		return KN_BENCH_DIAG_REPLAY_ERR_DECODE;
	}
	rc = KN_BENCH_DIAG_REPLAY_OK;
	if (kn_kiss_stream_feed(&parser, capture->frame,
	    capture->frame_len) != KN_KISS_STREAM_OK ||
	    kn_kiss_stream_pop_frame(&parser, &kiss_frame, &out) !=
	    KN_KISS_STREAM_OK ||
	    kn_ax25_frame_decode(kiss_frame.payload, kiss_frame.payload_len,
	    frame) != KN_AX25_OK)
		rc = KN_BENCH_DIAG_REPLAY_ERR_DECODE;

	kn_buffer_free(&out);
	kn_kiss_stream_free(&parser);
	return rc;
}

static const char *
final_state_name(const struct kn_ax25_runtime *runtime)
{
	const struct kn_ax25_connection_record *record;

	record = kn_ax25_runtime_get_connection(runtime, 0);
	if (record == NULL)
		return "none";

	return kn_ax25_connection_state_name(record->connection.state);
}

static uint8_t
planned_placeholder(const char *path)
{
	if (path == NULL)
		return 0;
	if (strstr(path, "fx25") != NULL)
		return 1;
	if (strstr(path, "sequence") != NULL)
		return 1;

	return 0;
}

static void
replay_runtime_init(struct kn_ax25_runtime *runtime)
{
	struct kn_ax25_live_options live;

	kn_ax25_runtime_init(runtime);
	(void)kn_ax25_runtime_set_enabled(runtime, 1, 0);
	memset(&live, 0, sizeof(live));
	live.live_rx_feed = 1;
	live.live_rx_create_connections = 1;
	live.live_rx_retain_frame_plans = 1;
	(void)kn_ax25_runtime_set_live_options(runtime, &live);
}

static void
result_from_runtime(struct kn_bench_diag_result *result,
	const struct kn_ax25_runtime *runtime)
{
	result->ui_ignored = runtime->live_counters.ui_ignored;
	result->connected_frames_accepted =
	    runtime->live_counters.frames_accepted;
	result->frames_ignored = runtime->live_counters.frames_ignored;
	result->malformed_frames = runtime->live_counters.frames_malformed;
	result->connections_created = runtime->counters.connections_created;
	result->final_connections = kn_ax25_runtime_connection_count(runtime);
	result->frame_plans_retained =
	    runtime->live_counters.frame_plans_retained;
	result->tx_writes_attempted =
	    runtime->live_counters.tx_queue_writes_attempted;
	(void)snprintf(result->final_state, sizeof(result->final_state),
	    "%s", final_state_name(runtime));
}
