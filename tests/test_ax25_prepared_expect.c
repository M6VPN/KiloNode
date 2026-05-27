/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_ax25_prepared_expect.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "kilonode/ax25_prepared_expect.h"

static int test_invalid_kind_rejected(void);
static int test_nonzero_tx_writes_rejected(void);
static int test_overlong_line_rejected(void);
static int test_unknown_key_rejected(void);
static int test_valid_file_parses(void);
static int write_fixture(const char *, char *, size_t);

int
main(void)
{
	if (test_valid_file_parses() != 0)
		return 1;
	if (test_unknown_key_rejected() != 0)
		return 1;
	if (test_invalid_kind_rejected() != 0)
		return 1;
	if (test_nonzero_tx_writes_rejected() != 0)
		return 1;
	if (test_overlong_line_rejected() != 0)
		return 1;

	return 0;
}

static int
test_invalid_kind_rejected(void)
{
	struct kn_ax25_prepared_expect_file expected;
	struct kn_ax25_prepared_expect_error_info error;
	char path[128];

	if (write_fixture("capture one.capture {\n"
	    "\tprepared-count 1\n"
	    "\tprepared 1 kind=BAD status=prepared\n"
	    "}\n", path, sizeof(path)) != 0)
		return 1;
	if (kn_ax25_prepared_expect_parse_file(path, &expected, &error) ==
	    KN_AX25_PREPARED_EXPECT_OK)
		return 1;
	(void)unlink(path);

	return error.line == 3 ? 0 : 1;
}

static int
test_nonzero_tx_writes_rejected(void)
{
	struct kn_ax25_prepared_expect_file expected;
	char path[128];

	if (write_fixture("capture one.capture {\n"
	    "\ttx-writes 1\n"
	    "}\n", path, sizeof(path)) != 0)
		return 1;
	if (kn_ax25_prepared_expect_parse_file(path, &expected, NULL) ==
	    KN_AX25_PREPARED_EXPECT_OK)
		return 1;
	(void)unlink(path);

	return 0;
}

static int
test_overlong_line_rejected(void)
{
	struct kn_ax25_prepared_expect_file expected;
	char text[700];
	char path[128];
	size_t i;

	for (i = 0; i + 1 < sizeof(text); i++)
		text[i] = 'a';
	text[sizeof(text) - 1] = '\0';
	if (write_fixture(text, path, sizeof(path)) != 0)
		return 1;
	if (kn_ax25_prepared_expect_parse_file(path, &expected, NULL) ==
	    KN_AX25_PREPARED_EXPECT_OK)
		return 1;
	(void)unlink(path);

	return 0;
}

static int
test_unknown_key_rejected(void)
{
	struct kn_ax25_prepared_expect_file expected;
	char path[128];

	if (write_fixture("capture one.capture {\n"
	    "\tunknown 1\n"
	    "}\n", path, sizeof(path)) != 0)
		return 1;
	if (kn_ax25_prepared_expect_parse_file(path, &expected, NULL) ==
	    KN_AX25_PREPARED_EXPECT_OK)
		return 1;
	(void)unlink(path);

	return 0;
}

static int
test_valid_file_parses(void)
{
	struct kn_ax25_prepared_expect_file expected;
	const struct kn_ax25_prepared_expect_block *block;
	char path[128];

	if (write_fixture("# test\n"
	    "capture kiss-sabm-node.capture {\n"
	    "\tprepared-count 1\n"
	    "\tprepared 1 kind=UA local=M6VPN-1 remote=N0CALL "
	    "action=send-ua status=prepared tx-writes=0\n"
	    "\ttx-writes 0\n"
	    "}\n", path, sizeof(path)) != 0)
		return 1;
	if (kn_ax25_prepared_expect_parse_file(path, &expected, NULL) !=
	    KN_AX25_PREPARED_EXPECT_OK)
		return 1;
	block = kn_ax25_prepared_expect_find(&expected,
	    KN_AX25_PREPARED_EXPECT_BLOCK_CAPTURE, "kiss-sabm-node.capture");
	(void)unlink(path);
	if (block == NULL || block->frame_count != 1)
		return 1;

	return block->frames[0].kind == KN_AX25_FRAME_PLAN_UA ? 0 : 1;
}

static int
write_fixture(const char *text, char *path, size_t pathsiz)
{
	FILE *fp;

	if (snprintf(path, pathsiz, "/tmp/kilonode-prepared-expect-%ld",
	    (long)getpid()) < 0)
		return 1;
	fp = fopen(path, "w");
	if (fp == NULL)
		return 1;
	if (fputs(text, fp) == EOF) {
		(void)fclose(fp);
		return 1;
	}
	(void)fclose(fp);
	return 0;
}
