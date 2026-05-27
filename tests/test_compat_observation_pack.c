/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_compat_observation_pack.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "kilonode/compat_observation_pack.h"

static int test_clean_room_false(void);
static int test_duplicate_fixture(void);
static int test_missing_name(void);
static int test_missing_reference(void);
static int test_overlong_line(void);
static int test_source_code_used_true(void);
static int test_summary(void);
static int test_unknown_key(void);
static int test_unsafe_path(void);
static int test_valid_pack(void);

static const char valid_manifest[] =
    "name linbpq-node\n"
    "subject linbpq\n"
    "type node-command-observations\n"
    "source synthetic-placeholder\n"
    "created 2026-05-27\n"
    "clean-room true\n"
    "source-code-used false\n"
    "fixture help.observation\n"
    "transcript help.transcript\n";

int
main(void)
{
	if (test_valid_pack() != 0)
		return 1;
	if (test_missing_name() != 0)
		return 1;
	if (test_clean_room_false() != 0)
		return 1;
	if (test_source_code_used_true() != 0)
		return 1;
	if (test_unsafe_path() != 0)
		return 1;
	if (test_missing_reference() != 0)
		return 1;
	if (test_duplicate_fixture() != 0)
		return 1;
	if (test_overlong_line() != 0)
		return 1;
	if (test_unknown_key() != 0)
		return 1;
	if (test_summary() != 0)
		return 1;

	return 0;
}

static int
test_clean_room_false(void)
{
	const char text[] =
	    "name x\nsubject y\ntype z\nclean-room false\n"
	    "source-code-used false\nfixture a.observation\n";
	struct kn_compat_observation_pack pack;

	return kn_compat_observation_pack_parse_text(text, &pack, NULL) ==
	    KN_COMPAT_PACK_ERR_INVALID_VALUE ? 0 : 1;
}

static int
test_duplicate_fixture(void)
{
	const char text[] =
	    "name x\nsubject y\ntype z\nclean-room true\n"
	    "source-code-used false\nfixture a.observation\n"
	    "fixture a.observation\n";
	struct kn_compat_observation_pack pack;

	return kn_compat_observation_pack_parse_text(text, &pack, NULL) ==
	    KN_COMPAT_PACK_ERR_INVALID_VALUE ? 0 : 1;
}

static int
test_missing_name(void)
{
	const char text[] =
	    "subject y\ntype z\nclean-room true\nsource-code-used false\n"
	    "fixture a.observation\n";
	struct kn_compat_observation_pack pack;

	return kn_compat_observation_pack_parse_text(text, &pack, NULL) ==
	    KN_COMPAT_PACK_ERR_MISSING_REQUIRED ? 0 : 1;
}

static int
test_missing_reference(void)
{
	const char text[] =
	    "name x\nsubject y\ntype z\nclean-room true\n"
	    "source-code-used false\nfixture missing.observation\n";
	struct kn_compat_observation_pack pack;

	if (kn_compat_observation_pack_parse_text(text, &pack, NULL) !=
	    KN_COMPAT_PACK_OK)
		return 1;

	return kn_compat_observation_pack_validate_refs(&pack, NULL) ==
	    KN_COMPAT_PACK_ERR_REFERENCE ? 0 : 1;
}

static int
test_overlong_line(void)
{
	char text[KN_COMPAT_PACK_LINE_MAX + 32];
	struct kn_compat_observation_pack pack;

	memset(text, 'A', sizeof(text));
	text[sizeof(text) - 2] = '\n';
	text[sizeof(text) - 1] = '\0';

	return kn_compat_observation_pack_parse_text(text, &pack, NULL) ==
	    KN_COMPAT_PACK_ERR_LINE_TOO_LONG ? 0 : 1;
}

static int
test_source_code_used_true(void)
{
	const char text[] =
	    "name x\nsubject y\ntype z\nclean-room true\n"
	    "source-code-used true\nfixture a.observation\n";
	struct kn_compat_observation_pack pack;

	return kn_compat_observation_pack_parse_text(text, &pack, NULL) ==
	    KN_COMPAT_PACK_ERR_INVALID_VALUE ? 0 : 1;
}

static int
test_summary(void)
{
	struct kn_compat_observation_pack pack;
	char report[512];

	if (kn_compat_observation_pack_parse_text(valid_manifest, &pack,
	    NULL) != KN_COMPAT_PACK_OK)
		return 1;
	if (kn_compat_observation_pack_report(&pack, report,
	    sizeof(report)) != KN_COMPAT_PACK_OK)
		return 1;
	if (strstr(report, "PACK name=linbpq-node") == NULL)
		return 1;

	return strstr(report, "PACK TRANSCRIPT path=help.transcript") !=
	    NULL ? 0 : 1;
}

static int
test_unknown_key(void)
{
	const char text[] =
	    "name x\nsubject y\ntype z\nbogus value\n";
	struct kn_compat_observation_pack pack;
	struct kn_compat_pack_error_info error;

	memset(&error, 0, sizeof(error));
	if (kn_compat_observation_pack_parse_text(text, &pack, &error) !=
	    KN_COMPAT_PACK_ERR_UNKNOWN_KEY)
		return 1;

	return error.line == 4 ? 0 : 1;
}

static int
test_unsafe_path(void)
{
	const char text[] =
	    "name x\nsubject y\ntype z\nclean-room true\n"
	    "source-code-used false\nfixture ../bad.observation\n";
	struct kn_compat_observation_pack pack;

	return kn_compat_observation_pack_parse_text(text, &pack, NULL) ==
	    KN_COMPAT_PACK_ERR_INVALID_VALUE ? 0 : 1;
}

static int
test_valid_pack(void)
{
	struct kn_compat_observation_pack pack;

	if (kn_compat_observation_pack_parse_file(
	    "../tests/fixtures/compat/linbpq-node/manifest.pack", &pack,
	    NULL) != KN_COMPAT_PACK_OK)
		return 1;
	if (strcmp(pack.name, "linbpq-node") != 0)
		return 1;
	if (pack.fixture_count != 5 || pack.transcript_count != 3)
		return 1;

	return kn_compat_observation_pack_validate_refs(&pack, NULL) ==
	    KN_COMPAT_PACK_OK ? 0 : 1;
}
