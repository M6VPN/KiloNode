/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_compat_capture.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "kilonode/compat_capture.h"

static void observation_base(struct kn_compat_observation *, const char *,
	const char *);
static int test_compare_different(void);
static int test_compare_identical(void);
static int test_invalid_observation_not_converted(void);
static int test_transcript_candidate(void);

int
main(void)
{
	if (test_transcript_candidate() != 0)
		return 1;
	if (test_invalid_observation_not_converted() != 0)
		return 1;
	if (test_compare_identical() != 0)
		return 1;
	if (test_compare_different() != 0)
		return 1;

	return 0;
}

static void
observation_base(struct kn_compat_observation *observation, const char *name,
	const char *observed)
{
	kn_compat_observation_clear(observation);
	(void)snprintf(observation->name, sizeof(observation->name), "%s",
	    name);
	(void)snprintf(observation->subject, sizeof(observation->subject),
	    "linbpq");
	observation->method = KN_COMPAT_OBSERVE_METHOD_TELNET;
	(void)snprintf(observation->date, sizeof(observation->date),
	    "2026-05-27");
	observation->mode = KN_COMPAT_OBSERVE_MODE_NODE_SHELL;
	(void)snprintf(observation->input, sizeof(observation->input),
	    "HELP");
	(void)snprintf(observation->observed, sizeof(observation->observed),
	    "%s", observed);
	observation->observed_len = strlen(observation->observed);
}

static int
test_compare_different(void)
{
	struct kn_compat_observation a;
	struct kn_compat_observation b;
	char report[512];

	observation_base(&a, "a", "one\n");
	observation_base(&b, "b", "two\n");
	if (kn_compat_capture_compare(&a, &b, report, sizeof(report)) !=
	    KN_COMPAT_CAPTURE_OK)
		return 1;

	return strstr(report, "same=false") != NULL ? 0 : 1;
}

static int
test_compare_identical(void)
{
	struct kn_compat_observation a;
	struct kn_compat_observation b;
	char report[512];

	observation_base(&a, "a", "same\n");
	observation_base(&b, "b", "same\n");
	if (kn_compat_capture_compare(&a, &b, report, sizeof(report)) !=
	    KN_COMPAT_CAPTURE_OK)
		return 1;

	return strstr(report, "same=true") != NULL ? 0 : 1;
}

static int
test_invalid_observation_not_converted(void)
{
	char report[128];

	return kn_compat_capture_transcript_candidate(NULL, report,
	    sizeof(report)) == KN_COMPAT_CAPTURE_ERR_INVALID_ARGUMENT ? 0 : 1;
}

static int
test_transcript_candidate(void)
{
	struct kn_compat_observation observation;
	char report[1024];

	observation_base(&observation, "blackbox-node-help", "HELP output\n");
	if (kn_compat_capture_transcript_candidate(&observation, report,
	    sizeof(report)) != KN_COMPAT_CAPTURE_OK)
		return 1;
	if (strstr(report, "source black-box-observation") == NULL)
		return 1;
	if (strstr(report, "expect-observed contains=HELP output") == NULL)
		return 1;

	return strstr(report, "implementation-use=false") != NULL ? 0 : 1;
}
