/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_compat_coverage.c */

#include <sys/types.h>

#include <string.h>

#include "kilonode/compat_coverage.h"

static int test_clear(void);
static int test_report(void);
static int test_status_names(void);
static int test_valid_pack_coverage(void);

int
main(void)
{
	if (test_clear() != 0)
		return 1;
	if (test_status_names() != 0)
		return 1;
	if (test_valid_pack_coverage() != 0)
		return 1;
	if (test_report() != 0)
		return 1;

	return 0;
}

static int
test_clear(void)
{
	struct kn_compat_coverage coverage;

	memset(&coverage, 0xff, sizeof(coverage));
	kn_compat_coverage_clear(&coverage);

	return coverage.count == 0 ? 0 : 1;
}

static int
test_report(void)
{
	struct kn_compat_observation_pack pack;
	struct kn_compat_coverage coverage;
	char report[KN_COMPAT_COVERAGE_REPORT_MAX];

	if (kn_compat_observation_pack_parse_file(
	    "../tests/fixtures/compat/linbpq-node/manifest.pack", &pack,
	    NULL) != KN_COMPAT_PACK_OK)
		return 1;
	if (kn_compat_coverage_from_pack(&pack, &coverage) !=
	    KN_COMPAT_COVERAGE_OK)
		return 1;
	if (kn_compat_coverage_report(&pack, &coverage, report,
	    sizeof(report)) != KN_COMPAT_COVERAGE_OK)
		return 1;
	if (strstr(report, "COVERAGE pack=linbpq-node") == NULL)
		return 1;

	return strstr(report, "COVERAGE COMMAND name=HELP") != NULL ? 0 : 1;
}

static int
test_status_names(void)
{
	if (strcmp(kn_compat_coverage_status_name(
	    KN_COMPAT_COVERAGE_SYNTHETIC), "synthetic") != 0)
		return 1;
	if (strcmp(kn_compat_coverage_status_name(
	    KN_COMPAT_COVERAGE_COMPATIBILITY_PLANNED),
	    "compatibility-planned") != 0)
		return 1;

	return strcmp(kn_compat_coverage_status_name(99), "unknown") == 0 ?
	    0 : 1;
}

static int
test_valid_pack_coverage(void)
{
	struct kn_compat_observation_pack pack;
	struct kn_compat_coverage coverage;
	size_t i;
	int help_seen;
	int unknown_seen;
	int connect_planned;

	if (kn_compat_observation_pack_parse_file(
	    "../tests/fixtures/compat/linbpq-node/manifest.pack", &pack,
	    NULL) != KN_COMPAT_PACK_OK)
		return 1;
	if (kn_compat_coverage_from_pack(&pack, &coverage) !=
	    KN_COMPAT_COVERAGE_OK)
		return 1;

	help_seen = unknown_seen = connect_planned = 0;
	for (i = 0; i < coverage.count; i++) {
		if (strcmp(coverage.entries[i].command, "HELP") == 0 &&
		    coverage.entries[i].synthetic != 0 &&
		    coverage.entries[i].transcript_candidate != 0)
			help_seen = 1;
		if (strcmp(coverage.entries[i].command, "UNKNOWN") == 0 &&
		    coverage.entries[i].synthetic != 0)
			unknown_seen = 1;
		if (strcmp(coverage.entries[i].command, "CONNECT") == 0 &&
		    coverage.entries[i].status ==
		    KN_COMPAT_COVERAGE_COMPATIBILITY_PLANNED)
			connect_planned = 1;
	}

	return help_seen != 0 && unknown_seen != 0 &&
	    connect_planned != 0 ? 0 : 1;
}
