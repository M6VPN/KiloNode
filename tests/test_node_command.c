/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_node_command.c */

#include <sys/types.h>

#include <string.h>

#include "kilonode/node_command.h"

static int test_invalid(void);
static int test_parse_args(void);
static int test_parse_empty(void);
static int test_parse_no_args(void);
static int test_reject_control(void);
static int test_reject_overlong(void);

int
main(void)
{
	if (test_parse_no_args() != 0)
		return 1;
	if (test_parse_args() != 0)
		return 1;
	if (test_reject_overlong() != 0)
		return 1;
	if (test_reject_control() != 0)
		return 1;
	if (test_parse_empty() != 0)
		return 1;
	if (test_invalid() != 0)
		return 1;
	return 0;
}

static int
test_invalid(void)
{
	const uint8_t input[] = "HELP";

	return kn_node_command_parse(input, strlen((const char *)input), 128,
	    NULL) == KN_NODE_COMMAND_ERR_INVALID_ARGUMENT ? 0 : 1;
}

static int
test_parse_args(void)
{
	const uint8_t input[] = " heard port kiss0 ";
	struct kn_node_command_input parsed;

	if (kn_node_command_parse(input, strlen((const char *)input), 128,
	    &parsed) != KN_NODE_COMMAND_OK)
		return 1;
	if (strcmp(parsed.command, "HEARD") != 0)
		return 1;
	if (strcmp(parsed.args, "port kiss0") != 0)
		return 1;
	return strcmp(parsed.preview, "heard port kiss0") == 0 ? 0 : 1;
}

static int
test_parse_empty(void)
{
	const uint8_t input[] = " \t\r\n ";
	struct kn_node_command_input parsed;

	return kn_node_command_parse(input, strlen((const char *)input), 128,
	    &parsed) == KN_NODE_COMMAND_ERR_EMPTY ? 0 : 1;
}

static int
test_parse_no_args(void)
{
	const uint8_t input[] = " ping ";
	struct kn_node_command_input parsed;

	if (kn_node_command_parse(input, strlen((const char *)input), 128,
	    &parsed) != KN_NODE_COMMAND_OK)
		return 1;
	if (strcmp(parsed.command, "PING") != 0)
		return 1;
	if (parsed.args_len != 0)
		return 1;
	return strcmp(parsed.preview, "ping") == 0 ? 0 : 1;
}

static int
test_reject_control(void)
{
	const uint8_t input[] = { 'P', 'I', 0x01, 'G' };
	struct kn_node_command_input parsed;

	return kn_node_command_parse(input, sizeof(input), 128, &parsed) ==
	    KN_NODE_COMMAND_ERR_CONTROL ? 0 : 1;
}

static int
test_reject_overlong(void)
{
	const uint8_t input[] = "HELP";
	struct kn_node_command_input parsed;

	return kn_node_command_parse(input, strlen((const char *)input), 3,
	    &parsed) == KN_NODE_COMMAND_ERR_OVERLONG ? 0 : 1;
}
