/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_compat_observe.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "kilonode/compat_observe.h"

static int test_comments_and_block(void);
static int test_missing_required(void);
static int test_overlarge_block(void);
static int test_overlong_line(void);
static int test_path_traversal(void);
static int test_report_sanitizes(void);
static int test_unknown_key(void);
static int test_valid_observation(void);

static const char valid_observation[] =
    "# KiloNode black-box observation v1\n"
    "name blackbox-node-help\n"
    "subject linbpq\n"
    "source synthetic-placeholder\n"
    "method telnet\n"
    "date 2026-05-27\n"
    "observer M6VPN\n"
    "mode node-shell\n"
    "connect 127.0.0.1:8010\n"
    "input HELP\n"
    "observed-begin\n"
    "HELP output\n"
    "observed-end\n"
    "notes captured from running binary only\n";

int
main(void)
{
	if (test_valid_observation() != 0)
		return 1;
	if (test_comments_and_block() != 0)
		return 1;
	if (test_missing_required() != 0)
		return 1;
	if (test_unknown_key() != 0)
		return 1;
	if (test_overlong_line() != 0)
		return 1;
	if (test_overlarge_block() != 0)
		return 1;
	if (test_report_sanitizes() != 0)
		return 1;
	if (test_path_traversal() != 0)
		return 1;

	return 0;
}

static int
test_comments_and_block(void)
{
	struct kn_compat_observation observation;
	struct kn_compat_observe_error_info error;

	memset(&error, 0, sizeof(error));
	if (kn_compat_observation_parse_text(valid_observation, &observation,
	    &error) != KN_COMPAT_OBSERVE_OK)
		return 1;

	return strcmp(observation.observed, "HELP output\n") == 0 ? 0 : 1;
}

static int
test_missing_required(void)
{
	const char text[] =
	    "name missing\n"
	    "method telnet\n";
	struct kn_compat_observation observation;
	struct kn_compat_observe_error_info error;

	memset(&error, 0, sizeof(error));
	return kn_compat_observation_parse_text(text, &observation, &error) ==
	    KN_COMPAT_OBSERVE_ERR_MISSING_REQUIRED ? 0 : 1;
}

static int
test_overlarge_block(void)
{
	char text[KN_COMPAT_OBSERVE_BLOCK_MAX + 1200];
	struct kn_compat_observation observation;
	struct kn_compat_observe_error_info error;
	size_t off;
	size_t i;

	off = 0;
	off += (size_t)snprintf(text + off, sizeof(text) - off,
	    "name big\nsubject x\nmethod process\ndate 2026-05-27\n"
	    "mode process-output\ninput START\nobserved-begin\n");
	while (off + 82 < sizeof(text)) {
		for (i = 0; i < 80; i++)
			text[off++] = 'A';
		text[off++] = '\n';
	}
	text[off] = '\0';
	memset(&error, 0, sizeof(error));

	return kn_compat_observation_parse_text(text, &observation, &error) ==
	    KN_COMPAT_OBSERVE_ERR_TOO_LARGE ? 0 : 1;
}

static int
test_overlong_line(void)
{
	char text[KN_COMPAT_OBSERVE_LINE_MAX + 32];
	struct kn_compat_observation observation;
	struct kn_compat_observe_error_info error;

	memset(text, 'A', sizeof(text));
	text[sizeof(text) - 2] = '\n';
	text[sizeof(text) - 1] = '\0';
	memset(&error, 0, sizeof(error));

	return kn_compat_observation_parse_text(text, &observation, &error) ==
	    KN_COMPAT_OBSERVE_ERR_LINE_TOO_LONG ? 0 : 1;
}

static int
test_path_traversal(void)
{
	const char text[] =
	    "name bad\n"
	    "subject x\n"
	    "method process\n"
	    "date 2026-05-27\n"
	    "mode process-output\n"
	    "input START\n"
	    "packet-capture ../capture.pcap\n"
	    "observed-begin\n"
	    "x\n"
	    "observed-end\n";
	struct kn_compat_observation observation;
	struct kn_compat_observe_error_info error;

	memset(&error, 0, sizeof(error));
	return kn_compat_observation_parse_text(text, &observation, &error) ==
	    KN_COMPAT_OBSERVE_ERR_INVALID_VALUE ? 0 : 1;
}

static int
test_report_sanitizes(void)
{
	struct kn_compat_observation observation;
	char report[512];

	if (kn_compat_observation_parse_text(valid_observation, &observation,
	    NULL) != KN_COMPAT_OBSERVE_OK)
		return 1;
	observation.observed[0] = '\001';
	observation.observed[1] = '\0';
	observation.observed_len = 1;
	if (kn_compat_observation_report(&observation, report,
	    sizeof(report)) != KN_COMPAT_OBSERVE_OK)
		return 1;

	return strstr(report, "\\x01") != NULL ? 0 : 1;
}

static int
test_unknown_key(void)
{
	const char text[] =
	    "name bad\n"
	    "subject x\n"
	    "bogus value\n";
	struct kn_compat_observation observation;
	struct kn_compat_observe_error_info error;

	memset(&error, 0, sizeof(error));
	if (kn_compat_observation_parse_text(text, &observation, &error) !=
	    KN_COMPAT_OBSERVE_ERR_UNKNOWN_KEY)
		return 1;

	return error.line == 3 ? 0 : 1;
}

static int
test_valid_observation(void)
{
	struct kn_compat_observation observation;

	if (kn_compat_observation_parse_text(valid_observation, &observation,
	    NULL) != KN_COMPAT_OBSERVE_OK)
		return 1;
	if (strcmp(observation.name, "blackbox-node-help") != 0)
		return 1;
	if (strcmp(observation.source, "synthetic-placeholder") != 0)
		return 1;
	if (observation.method != KN_COMPAT_OBSERVE_METHOD_TELNET)
		return 1;

	return observation.mode == KN_COMPAT_OBSERVE_MODE_NODE_SHELL ? 0 : 1;
}
