/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_ax25_loopback_script.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "kilonode/ax25_loopback_script.h"

static int test_invalid_command(void);
static int test_invalid_hex(void);
static int test_invalid_paclen(void);
static int test_reassembled_expectation(void);
static int test_replay_buffer(void);
static int test_retransmit_expectation(void);
static int test_segment_flag(void);
static int test_send_rej(void);
static int test_send_i_hex(void);
static int test_send_i_text(void);
static int test_valid_script(void);
static int write_temp(const char *, char *, size_t);

int
main(void)
{
	if (test_valid_script() != 0)
		return 1;
	if (test_send_i_text() != 0)
		return 1;
	if (test_send_i_hex() != 0)
		return 1;
	if (test_invalid_command() != 0)
		return 1;
	if (test_invalid_hex() != 0)
		return 1;
	if (test_segment_flag() != 0)
		return 1;
	if (test_reassembled_expectation() != 0)
		return 1;
	if (test_invalid_paclen() != 0)
		return 1;
	if (test_send_rej() != 0)
		return 1;
	if (test_replay_buffer() != 0)
		return 1;
	if (test_retransmit_expectation() != 0)
		return 1;
	return 0;
}

static int
test_replay_buffer(void)
{
	char path[128];
	struct kn_ax25_loopback_script script;
	struct kn_ax25_loopback_error_info error;

	if (write_temp("name ok\nendpoint-a M6VPN-1\nendpoint-b N0CALL\n"
	    "port kiss0\nreplay-buffer A max-frames=2\n", path,
	    sizeof(path)) != 0)
		return 1;
	if (kn_ax25_loopback_script_parse_file(path, &script, &error) !=
	    KN_AX25_LOOPBACK_OK) {
		(void)unlink(path);
		return 1;
	}
	(void)unlink(path);
	return script.commands[0].type ==
	    KN_AX25_LOOPBACK_SCRIPT_REPLAY_BUFFER &&
	    script.commands[0].endpoint ==
	    KN_AX25_LOOPBACK_SCRIPT_ENDPOINT_A &&
	    script.commands[0].value == 2 ? 0 : 1;
}

static int
test_retransmit_expectation(void)
{
	char path[128];
	struct kn_ax25_loopback_script script;
	struct kn_ax25_loopback_error_info error;

	if (write_temp("name ok\nendpoint-a M6VPN-1\nendpoint-b N0CALL\n"
	    "port kiss0\nexpect A retransmit-buffer=1\n"
	    "expect A retransmit-needed=1\n"
	    "expect A retransmit-acked=1\n"
	    "expect A retransmit-replayed=1\n"
	    "expect A retransmit-full=0\n", path, sizeof(path)) != 0)
		return 1;
	if (kn_ax25_loopback_script_parse_file(path, &script, &error) !=
	    KN_AX25_LOOPBACK_OK) {
		(void)unlink(path);
		return 1;
	}
	(void)unlink(path);
	return script.commands[0].expect ==
	    KN_AX25_LOOPBACK_SCRIPT_EXPECT_RETRANSMIT_BUFFER &&
	    script.commands[1].expect ==
	    KN_AX25_LOOPBACK_SCRIPT_EXPECT_RETRANSMIT_NEEDED &&
	    script.commands[2].expect ==
	    KN_AX25_LOOPBACK_SCRIPT_EXPECT_RETRANSMIT_ACKED &&
	    script.commands[3].expect ==
	    KN_AX25_LOOPBACK_SCRIPT_EXPECT_RETRANSMIT_REPLAYED &&
	    script.commands[4].expect ==
	    KN_AX25_LOOPBACK_SCRIPT_EXPECT_RETRANSMIT_FULL ? 0 : 1;
}

static int
test_send_rej(void)
{
	char path[128];
	struct kn_ax25_loopback_script script;
	struct kn_ax25_loopback_error_info error;

	if (write_temp("name ok\nendpoint-a M6VPN-1\nendpoint-b N0CALL\n"
	    "port kiss0\nevent B send-rej nr=0\n", path,
	    sizeof(path)) != 0)
		return 1;
	if (kn_ax25_loopback_script_parse_file(path, &script, &error) !=
	    KN_AX25_LOOPBACK_OK) {
		(void)unlink(path);
		return 1;
	}
	(void)unlink(path);
	return script.commands[0].event ==
	    KN_AX25_LOOPBACK_SCRIPT_EVENT_SEND_REJ &&
	    script.commands[0].endpoint ==
	    KN_AX25_LOOPBACK_SCRIPT_ENDPOINT_B &&
	    script.commands[0].value == 0 ? 0 : 1;
}

static int
test_invalid_paclen(void)
{
	char path[128];
	struct kn_ax25_loopback_script script;
	struct kn_ax25_loopback_error_info error;

	if (write_temp("name bad\nendpoint-a M6VPN-1\nendpoint-b N0CALL\n"
	    "port kiss0\nparams modulo=8 window=1 paclen=0 t1=3000 "
	    "t2=1000 t3=300000 n2=3\n", path, sizeof(path)) != 0)
		return 1;
	if (kn_ax25_loopback_script_parse_file(path, &script, &error) !=
	    KN_AX25_LOOPBACK_ERR_PARSE) {
		(void)unlink(path);
		return 1;
	}
	(void)unlink(path);
	return error.line == 5 ? 0 : 1;
}

static int
test_reassembled_expectation(void)
{
	char path[128];
	struct kn_ax25_loopback_script script;
	struct kn_ax25_loopback_error_info error;

	if (write_temp("name ok\nendpoint-a M6VPN-1\nendpoint-b N0CALL\n"
	    "port kiss0\nexpect B reassembled=1\n"
	    "expect B last-reassembled-text=\"hello world\"\n"
	    "expect B last-reassembled-hex=0001\n", path,
	    sizeof(path)) != 0)
		return 1;
	if (kn_ax25_loopback_script_parse_file(path, &script, &error) !=
	    KN_AX25_LOOPBACK_OK) {
		(void)unlink(path);
		return 1;
	}
	(void)unlink(path);
	return script.commands[0].expect ==
	    KN_AX25_LOOPBACK_SCRIPT_EXPECT_REASSEMBLED &&
	    script.commands[1].expect ==
	    KN_AX25_LOOPBACK_SCRIPT_EXPECT_LAST_REASSEMBLED_TEXT &&
	    script.commands[2].expect ==
	    KN_AX25_LOOPBACK_SCRIPT_EXPECT_LAST_REASSEMBLED_HEX ? 0 : 1;
}

static int
test_segment_flag(void)
{
	char path[128];
	struct kn_ax25_loopback_script script;
	struct kn_ax25_loopback_error_info error;

	if (write_temp("name ok\nendpoint-a M6VPN-1\nendpoint-b N0CALL\n"
	    "port kiss0\nparams modulo=8 window=1 paclen=4 t1=3000 "
	    "t2=1000 t3=300000 n2=3\n"
	    "event A send-i text=\"hello world\" segment=true\n", path,
	    sizeof(path)) != 0)
		return 1;
	if (kn_ax25_loopback_script_parse_file(path, &script, &error) !=
	    KN_AX25_LOOPBACK_OK) {
		(void)unlink(path);
		return 1;
	}
	(void)unlink(path);
	return script.params.paclen == 4 && script.commands[1].segment != 0 &&
	    script.commands[1].payload_len == 11 ? 0 : 1;
}

static int
test_invalid_command(void)
{
	char path[128];
	struct kn_ax25_loopback_script script;
	struct kn_ax25_loopback_error_info error;

	if (write_temp("name bad\nendpoint-a M6VPN-1\nendpoint-b N0CALL\n"
	    "port kiss0\nunknown\n", path, sizeof(path)) != 0)
		return 1;
	if (kn_ax25_loopback_script_parse_file(path, &script, &error) !=
	    KN_AX25_LOOPBACK_ERR_PARSE) {
		(void)unlink(path);
		return 1;
	}
	(void)unlink(path);
	return error.line == 5 ? 0 : 1;
}

static int
test_invalid_hex(void)
{
	char path[128];
	struct kn_ax25_loopback_script script;
	struct kn_ax25_loopback_error_info error;

	if (write_temp("name bad\nendpoint-a M6VPN-1\nendpoint-b N0CALL\n"
	    "port kiss0\nevent A send-i hex=00f\n", path,
	    sizeof(path)) != 0)
		return 1;
	if (kn_ax25_loopback_script_parse_file(path, &script, &error) !=
	    KN_AX25_LOOPBACK_ERR_PARSE) {
		(void)unlink(path);
		return 1;
	}
	(void)unlink(path);
	return error.line == 5 ? 0 : 1;
}

static int
test_send_i_hex(void)
{
	char path[128];
	struct kn_ax25_loopback_script script;
	struct kn_ax25_loopback_error_info error;

	if (write_temp("name ok\nendpoint-a M6VPN-1\nendpoint-b N0CALL\n"
	    "port kiss0\nevent A send-i hex=00ff41\n", path,
	    sizeof(path)) != 0)
		return 1;
	if (kn_ax25_loopback_script_parse_file(path, &script, &error) !=
	    KN_AX25_LOOPBACK_OK) {
		(void)unlink(path);
		return 1;
	}
	(void)unlink(path);
	return script.commands[0].payload_len == 3 &&
	    script.commands[0].payload[1] == 0xff ? 0 : 1;
}

static int
test_send_i_text(void)
{
	char path[128];
	struct kn_ax25_loopback_script script;
	struct kn_ax25_loopback_error_info error;

	if (write_temp("name ok\nendpoint-a M6VPN-1\nendpoint-b N0CALL\n"
	    "port kiss0\nevent A send-i text=\"hello\"\n"
	    "expect B last-payload-text=\"hello\"\n", path,
	    sizeof(path)) != 0)
		return 1;
	if (kn_ax25_loopback_script_parse_file(path, &script, &error) !=
	    KN_AX25_LOOPBACK_OK) {
		(void)unlink(path);
		return 1;
	}
	(void)unlink(path);
	return script.commands[0].payload_len == 5 &&
	    strcmp(script.commands[1].text, "hello") == 0 ? 0 : 1;
}

static int
test_valid_script(void)
{
	char path[128];
	struct kn_ax25_loopback_script script;
	struct kn_ax25_loopback_error_info error;

	if (write_temp("name ok\nendpoint-a M6VPN-1\nendpoint-b N0CALL\n"
	    "port kiss0\nparams modulo=8 window=1 t1=3000 t2=1000 "
	    "t3=300000 n2=3\n", path, sizeof(path)) != 0)
		return 1;
	if (kn_ax25_loopback_script_parse_file(path, &script, &error) !=
	    KN_AX25_LOOPBACK_OK) {
		(void)unlink(path);
		return 1;
	}
	(void)unlink(path);
	return strcmp(script.name, "ok") == 0 ? 0 : 1;
}

static int
write_temp(const char *text, char *path, size_t path_len)
{
	FILE *fp;
	int needed;

	needed = snprintf(path, path_len,
	    "/tmp/kilonode-loopback-script-%llu.loop",
	    (unsigned long long)getpid());
	if (needed < 0 || (size_t)needed >= path_len)
		return 1;
	fp = fopen(path, "w");
	if (fp == NULL)
		return 1;
	(void)fputs(text, fp);
	(void)fclose(fp);
	return 0;
}
