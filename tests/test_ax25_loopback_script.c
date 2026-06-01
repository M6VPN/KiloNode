/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_ax25_loopback_script.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "kilonode/ax25_loopback_script.h"

static int test_invalid_command(void);
static int test_invalid_hex(void);
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
	return 0;
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
