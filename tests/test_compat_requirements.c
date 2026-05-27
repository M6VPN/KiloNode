/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_compat_requirements.c */

#include <sys/types.h>

#include <string.h>

#include "kilonode/compat_observation_pack.h"
#include "kilonode/compat_requirements.h"

static int test_bad_values(void);
static int test_coverage(void);
static int test_find(void);
static int test_generate(void);
static int test_report(void);
static int test_valid(void);

int
main(void)
{
	if (test_valid() != 0)
		return 1;
	if (test_bad_values() != 0)
		return 1;
	if (test_find() != 0)
		return 1;
	if (test_report() != 0)
		return 1;
	if (test_coverage() != 0)
		return 1;
	if (test_generate() != 0)
		return 1;
	return 0;
}

static int
test_bad_values(void)
{
	const char source_used[] =
	    "name x\nsubject y\nsource-pack manifest.pack\nclean-room true\n"
	    "source-code-used true\nrequirement HELP {\nstatus planned\n"
	    "priority high\n}\n";
	const char bad_status[] =
	    "name x\nsubject y\nsource-pack manifest.pack\nclean-room true\n"
	    "source-code-used false\nrequirement HELP {\nstatus nope\n"
	    "priority high\n}\n";
	const char bad_pack[] =
	    "name x\nsubject y\nsource-pack ../manifest.pack\n"
	    "clean-room true\nsource-code-used false\nrequirement HELP {\n"
	    "status planned\npriority high\n}\n";
	struct kn_compat_requirements req;

	if (kn_compat_requirements_parse_text(source_used, &req, NULL) !=
	    KN_COMPAT_REQ_ERR_INVALID_VALUE)
		return 1;
	if (kn_compat_requirements_parse_text(bad_status, &req, NULL) !=
	    KN_COMPAT_REQ_ERR_INVALID_VALUE)
		return 1;
	return kn_compat_requirements_parse_text(bad_pack, &req, NULL) ==
	    KN_COMPAT_REQ_ERR_INVALID_VALUE ? 0 : 1;
}

static int
test_coverage(void)
{
	struct kn_compat_requirements req;
	struct kn_compat_observation_pack pack;
	char report[KN_COMPAT_REQ_REPORT_MAX];

	if (kn_compat_requirements_parse_file(
	    "../tests/fixtures/compat/linbpq-node/requirements.plan", &req,
	    NULL) != KN_COMPAT_REQ_OK)
		return 1;
	if (kn_compat_observation_pack_parse_file(
	    "../tests/fixtures/compat/linbpq-node/manifest.pack", &pack,
	    NULL) != KN_COMPAT_PACK_OK)
		return 1;
	if (kn_compat_requirements_coverage_report(&req, &pack, report,
	    sizeof(report)) != KN_COMPAT_REQ_OK)
		return 1;
	return strstr(report, "missing=0") != NULL ? 0 : 1;
}

static int
test_find(void)
{
	struct kn_compat_requirements req;

	if (kn_compat_requirements_parse_file(
	    "../tests/fixtures/compat/linbpq-node/requirements.plan", &req,
	    NULL) != KN_COMPAT_REQ_OK)
		return 1;
	if (kn_compat_requirements_find(&req, "HELP") == NULL)
		return 1;
	return kn_compat_requirements_find(&req, "NOPE") == NULL ? 0 : 1;
}

static int
test_generate(void)
{
	struct kn_compat_observation_pack pack;
	char text[KN_COMPAT_REQ_TEXT_MAX];
	struct kn_compat_requirements req;

	if (kn_compat_observation_pack_parse_file(
	    "../tests/fixtures/compat/linbpq-node/manifest.pack", &pack,
	    NULL) != KN_COMPAT_PACK_OK)
		return 1;
	if (kn_compat_requirements_generate_from_pack(&pack, text,
	    sizeof(text)) != KN_COMPAT_REQ_OK)
		return 1;
	if (kn_compat_requirements_parse_text(text, &req, NULL) !=
	    KN_COMPAT_REQ_OK)
		return 1;
	return kn_compat_requirements_find(&req, "CONNECT") != NULL ? 0 : 1;
}

static int
test_report(void)
{
	struct kn_compat_requirements req;
	char report[KN_COMPAT_REQ_REPORT_MAX];

	if (kn_compat_requirements_parse_file(
	    "../tests/fixtures/compat/linbpq-node/requirements.plan", &req,
	    NULL) != KN_COMPAT_REQ_OK)
		return 1;
	if (kn_compat_requirements_report(&req, report, sizeof(report)) !=
	    KN_COMPAT_REQ_OK)
		return 1;
	return strstr(report, "REQUIREMENT command=CONNECT status=blocked") !=
	    NULL ? 0 : 1;
}

static int
test_valid(void)
{
	struct kn_compat_requirements req;

	if (kn_compat_requirements_parse_file(
	    "../tests/fixtures/compat/linbpq-node/requirements.plan", &req,
	    NULL) != KN_COMPAT_REQ_OK)
		return 1;
	if (strcmp(req.name, "linbpq-node-requirements") != 0)
		return 1;
	return req.requirement_count == 12 ? 0 : 1;
}
