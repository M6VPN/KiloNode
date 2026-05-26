/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/bbs/message.c */

#include <sys/types.h>

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "kilonode/callsign.h"
#include "kilonode/message.h"

static enum kn_message_error body_len_valid(size_t);
static enum kn_message_error subject_copy(char *, size_t, const char *);

static enum kn_message_error
body_len_valid(size_t body_len)
{
	if (body_len == 0)
		return KN_MESSAGE_ERR_INVALID_BODY;
	if (body_len > KN_MESSAGE_BODY_MAX)
		return KN_MESSAGE_ERR_BODY_TOO_LARGE;

	return KN_MESSAGE_OK;
}

uint8_t
kn_message_area_valid(const char *area)
{
	size_t i;

	if (area == NULL || area[0] == '\0')
		return 0;

	for (i = 0; area[i] != '\0'; i++) {
		if (i >= KN_MESSAGE_AREA_MAX)
			return 0;
		if (!(isupper((unsigned char)area[i]) ||
		    isdigit((unsigned char)area[i]) || area[i] == '_' ||
		    area[i] == '-'))
			return 0;
	}

	return 1;
}

enum kn_message_error
kn_message_bulletin_init(struct kn_message *message, const char *from,
	const char *area, const char *subject, size_t body_len, uint64_t now)
{
	enum kn_message_error rc;

	if (message == NULL || from == NULL || area == NULL || subject == NULL)
		return KN_MESSAGE_ERR_INVALID_ARGUMENT;

	memset(message, 0, sizeof(*message));
	if (kn_callsign_parse(from, &message->from) != 0)
		return KN_MESSAGE_ERR_INVALID_CALLSIGN;
	if (kn_message_area_valid(area) == 0)
		return KN_MESSAGE_ERR_INVALID_AREA;
	rc = subject_copy(message->subject, sizeof(message->subject), subject);
	if (rc != KN_MESSAGE_OK)
		return rc;
	rc = body_len_valid(body_len);
	if (rc != KN_MESSAGE_OK)
		return rc;

	memcpy(message->area, area, strlen(area) + 1);
	message->type = KN_MESSAGE_TYPE_BULLETIN;
	message->created = now;
	message->updated = now;
	message->body_len = body_len;
	return KN_MESSAGE_OK;
}

enum kn_message_error
kn_message_private_init(struct kn_message *message, const char *from,
	const char *to, const char *subject, size_t body_len, uint64_t now)
{
	enum kn_message_error rc;

	if (message == NULL || from == NULL || to == NULL || subject == NULL)
		return KN_MESSAGE_ERR_INVALID_ARGUMENT;

	memset(message, 0, sizeof(*message));
	if (kn_callsign_parse(from, &message->from) != 0)
		return KN_MESSAGE_ERR_INVALID_CALLSIGN;
	if (kn_callsign_parse(to, &message->to) != 0)
		return KN_MESSAGE_ERR_INVALID_CALLSIGN;
	rc = subject_copy(message->subject, sizeof(message->subject), subject);
	if (rc != KN_MESSAGE_OK)
		return rc;
	rc = body_len_valid(body_len);
	if (rc != KN_MESSAGE_OK)
		return rc;

	message->type = KN_MESSAGE_TYPE_PRIVATE;
	message->created = now;
	message->updated = now;
	message->body_len = body_len;
	return KN_MESSAGE_OK;
}

enum kn_message_error
kn_message_summary(const struct kn_message *message, char *buf, size_t bufsiz)
{
	char from[KN_CALLSIGN_MAX + 4];
	char to[KN_CALLSIGN_MAX + 4];
	const char *dest;
	int needed;

	if (message == NULL || buf == NULL || bufsiz == 0)
		return KN_MESSAGE_ERR_INVALID_ARGUMENT;

	if (kn_callsign_format(&message->from, from, sizeof(from)) != 0)
		return KN_MESSAGE_ERR_INVALID_CALLSIGN;
	if (message->type == KN_MESSAGE_TYPE_PRIVATE) {
		if (kn_callsign_format(&message->to, to, sizeof(to)) != 0)
			return KN_MESSAGE_ERR_INVALID_CALLSIGN;
		dest = to;
	} else if (message->type == KN_MESSAGE_TYPE_BULLETIN) {
		dest = message->area;
	} else {
		return KN_MESSAGE_ERR_INVALID_TYPE;
	}

	needed = snprintf(buf, bufsiz,
	    "id=%llu type=%s from=%s to=%s subject=%s body=%llu",
	    (unsigned long long)message->id,
	    kn_message_type_name(message->type), from, dest, message->subject,
	    (unsigned long long)message->body_len);
	if (needed < 0 || (size_t)needed >= bufsiz)
		return KN_MESSAGE_ERR_BUFFER;

	return KN_MESSAGE_OK;
}

const char *
kn_message_type_name(enum kn_message_type type)
{
	switch (type) {
	case KN_MESSAGE_TYPE_PRIVATE:
		return "private";
	case KN_MESSAGE_TYPE_BULLETIN:
		return "bulletin";
	}

	return "unknown";
}

static enum kn_message_error
subject_copy(char *dst, size_t dst_len, const char *subject)
{
	size_t len;
	size_t i;

	len = strlen(subject);
	if (len == 0 || len >= dst_len)
		return KN_MESSAGE_ERR_INVALID_SUBJECT;
	for (i = 0; i < len; i++) {
		if ((unsigned char)subject[i] < 0x20 ||
		    (unsigned char)subject[i] > 0x7e)
			return KN_MESSAGE_ERR_INVALID_SUBJECT;
	}

	memcpy(dst, subject, len + 1);
	return KN_MESSAGE_OK;
}
