/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_message.c */

#include <sys/types.h>

#include <string.h>

#include "kilonode/message.h"

static int test_binary_body_length(void);
static int test_empty_subject(void);
static int test_invalid_area(void);
static int test_invalid_from_callsign(void);
static int test_invalid_private_to_callsign(void);
static int test_overlong_subject(void);
static int test_summary(void);
static int test_valid_bulletin(void);
static int test_valid_private(void);
static int test_zero_body(void);

int
main(void)
{
	if (test_valid_private() != 0)
		return 1;
	if (test_valid_bulletin() != 0)
		return 1;
	if (test_invalid_from_callsign() != 0)
		return 1;
	if (test_invalid_private_to_callsign() != 0)
		return 1;
	if (test_invalid_area() != 0)
		return 1;
	if (test_empty_subject() != 0)
		return 1;
	if (test_overlong_subject() != 0)
		return 1;
	if (test_zero_body() != 0)
		return 1;
	if (test_binary_body_length() != 0)
		return 1;
	if (test_summary() != 0)
		return 1;

	return 0;
}

static int
test_binary_body_length(void)
{
	struct kn_message message;

	if (kn_message_private_init(&message, "M6VPN-1", "N0CALL",
	    "Binary", 3, 1) != KN_MESSAGE_OK)
		return 1;

	return message.body_len == 3 ? 0 : 1;
}

static int
test_empty_subject(void)
{
	struct kn_message message;

	return kn_message_private_init(&message, "M6VPN-1", "N0CALL", "",
	    1, 1) == KN_MESSAGE_ERR_INVALID_SUBJECT ? 0 : 1;
}

static int
test_invalid_area(void)
{
	struct kn_message message;

	if (kn_message_bulletin_init(&message, "M6VPN-1", "../BAD",
	    "Subject", 1, 1) != KN_MESSAGE_ERR_INVALID_AREA)
		return 1;

	return kn_message_area_valid("GENERAL") != 0 ? 0 : 1;
}

static int
test_invalid_from_callsign(void)
{
	struct kn_message message;

	return kn_message_private_init(&message, "bad", "N0CALL", "Subject",
	    1, 1) == KN_MESSAGE_ERR_INVALID_CALLSIGN ? 0 : 1;
}

static int
test_invalid_private_to_callsign(void)
{
	struct kn_message message;

	return kn_message_private_init(&message, "M6VPN-1", "bad", "Subject",
	    1, 1) == KN_MESSAGE_ERR_INVALID_CALLSIGN ? 0 : 1;
}

static int
test_overlong_subject(void)
{
	struct kn_message message;
	char subject[KN_MESSAGE_SUBJECT_MAX + 2];

	memset(subject, 'A', sizeof(subject));
	subject[sizeof(subject) - 1] = '\0';
	return kn_message_private_init(&message, "M6VPN-1", "N0CALL",
	    subject, 1, 1) == KN_MESSAGE_ERR_INVALID_SUBJECT ? 0 : 1;
}

static int
test_summary(void)
{
	struct kn_message message;
	char summary[256];

	if (kn_message_private_init(&message, "M6VPN-1", "N0CALL",
	    "Subject", 4, 1) != KN_MESSAGE_OK)
		return 1;
	message.id = 7;
	if (kn_message_summary(&message, summary, sizeof(summary)) !=
	    KN_MESSAGE_OK)
		return 1;

	return strcmp(summary,
	    "id=7 type=private from=M6VPN-1 to=N0CALL subject=Subject body=4") ==
	    0 ? 0 : 1;
}

static int
test_valid_bulletin(void)
{
	struct kn_message message;

	if (kn_message_bulletin_init(&message, "M6VPN-1", "GENERAL",
	    "Notice", 12, 10) != KN_MESSAGE_OK)
		return 1;
	if (message.type != KN_MESSAGE_TYPE_BULLETIN)
		return 1;
	if (strcmp(message.area, "GENERAL") != 0)
		return 1;

	return message.created == 10 ? 0 : 1;
}

static int
test_valid_private(void)
{
	struct kn_message message;

	if (kn_message_private_init(&message, "M6VPN-1", "N0CALL",
	    "Subject", 12, 10) != KN_MESSAGE_OK)
		return 1;
	if (message.type != KN_MESSAGE_TYPE_PRIVATE)
		return 1;
	if (strcmp(message.from.call, "M6VPN") != 0)
		return 1;

	return message.from.ssid == 1 ? 0 : 1;
}

static int
test_zero_body(void)
{
	struct kn_message message;

	return kn_message_private_init(&message, "M6VPN-1", "N0CALL",
	    "Subject", 0, 1) == KN_MESSAGE_ERR_INVALID_BODY ? 0 : 1;
}
