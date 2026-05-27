/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/proto/ax25_action.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "kilonode/ax25_action.h"

static enum kn_ax25_action_error append_text(char *, size_t, size_t *,
	const char *);

static enum kn_ax25_action_error
append_text(char *buf, size_t bufsiz, size_t *used, const char *text)
{
	size_t len;

	if (buf == NULL || used == NULL || text == NULL || *used >= bufsiz)
		return KN_AX25_ACTION_ERR_INVALID_ARGUMENT;

	len = strlen(text);
	if (len >= bufsiz - *used)
		return KN_AX25_ACTION_ERR_BUFFER;

	memcpy(buf + *used, text, len + 1);
	*used += len;
	return KN_AX25_ACTION_OK;
}

enum kn_ax25_action_error
kn_ax25_action_list_append(struct kn_ax25_action_list *list,
	enum kn_ax25_action_intent intent)
{
	return kn_ax25_action_list_append_sequence(list, intent, 0);
}

enum kn_ax25_action_error
kn_ax25_action_list_append_sequence(struct kn_ax25_action_list *list,
	enum kn_ax25_action_intent intent, uint8_t sequence)
{
	if (list == NULL)
		return KN_AX25_ACTION_ERR_INVALID_ARGUMENT;
	if (list->count >= KN_AX25_ACTION_LIST_MAX)
		return KN_AX25_ACTION_ERR_FULL;

	list->actions[list->count].intent = intent;
	list->actions[list->count].sequence = sequence;
	list->count++;
	return KN_AX25_ACTION_OK;
}

void
kn_ax25_action_list_clear(struct kn_ax25_action_list *list)
{
	if (list == NULL)
		return;

	memset(list, 0, sizeof(*list));
}

enum kn_ax25_action_error
kn_ax25_action_list_format(const struct kn_ax25_action_list *list,
	char *buf, size_t bufsiz)
{
	char item[64];
	size_t i;
	size_t used;
	int needed;

	if (list == NULL || buf == NULL || bufsiz == 0)
		return KN_AX25_ACTION_ERR_INVALID_ARGUMENT;

	buf[0] = '\0';
	used = 0;
	if (list->count == 0)
		return append_text(buf, bufsiz, &used, "none");

	for (i = 0; i < list->count; i++) {
		if (i > 0 &&
		    append_text(buf, bufsiz, &used, ",") !=
		    KN_AX25_ACTION_OK)
			return KN_AX25_ACTION_ERR_BUFFER;

		if (list->actions[i].intent == KN_AX25_ACTION_SEND_RR ||
		    list->actions[i].intent == KN_AX25_ACTION_SEND_RNR ||
		    list->actions[i].intent == KN_AX25_ACTION_SEND_REJ ||
		    list->actions[i].intent == KN_AX25_ACTION_DELIVER_I_PAYLOAD ||
		    list->actions[i].intent == KN_AX25_ACTION_RETRANSMIT_NEEDED) {
			needed = snprintf(item, sizeof(item), "%s:%u",
			    kn_ax25_action_intent_name(
			    list->actions[i].intent),
			    (unsigned int)list->actions[i].sequence);
		} else {
			needed = snprintf(item, sizeof(item), "%s",
			    kn_ax25_action_intent_name(
			    list->actions[i].intent));
		}
		if (needed < 0 || (size_t)needed >= sizeof(item))
			return KN_AX25_ACTION_ERR_BUFFER;
		if (append_text(buf, bufsiz, &used, item) !=
		    KN_AX25_ACTION_OK)
			return KN_AX25_ACTION_ERR_BUFFER;
	}

	return KN_AX25_ACTION_OK;
}

const char *
kn_ax25_action_intent_name(enum kn_ax25_action_intent intent)
{
	switch (intent) {
	case KN_AX25_ACTION_NONE:
		return "none";
	case KN_AX25_ACTION_SEND_SABM:
		return "send-sabm";
	case KN_AX25_ACTION_SEND_SABME:
		return "send-sabme";
	case KN_AX25_ACTION_SEND_UA:
		return "send-ua";
	case KN_AX25_ACTION_SEND_DM:
		return "send-dm";
	case KN_AX25_ACTION_SEND_DISC:
		return "send-disc";
	case KN_AX25_ACTION_SEND_RR:
		return "send-rr";
	case KN_AX25_ACTION_SEND_RNR:
		return "send-rnr";
	case KN_AX25_ACTION_SEND_REJ:
		return "send-rej";
	case KN_AX25_ACTION_SEND_FRMR:
		return "send-frmr";
	case KN_AX25_ACTION_DELIVER_I_PAYLOAD:
		return "deliver-i-payload";
	case KN_AX25_ACTION_START_T1:
		return "start-t1";
	case KN_AX25_ACTION_STOP_T1:
		return "stop-t1";
	case KN_AX25_ACTION_START_T3:
		return "start-t3";
	case KN_AX25_ACTION_STOP_T3:
		return "stop-t3";
	case KN_AX25_ACTION_RESET_RETRY_COUNT:
		return "reset-retry-count";
	case KN_AX25_ACTION_INCREMENT_RETRY_COUNT:
		return "increment-retry-count";
	case KN_AX25_ACTION_ENTER_CONNECTED:
		return "enter-connected";
	case KN_AX25_ACTION_ENTER_DISCONNECTED:
		return "enter-disconnected";
	case KN_AX25_ACTION_PROTOCOL_ERROR:
		return "protocol-error";
	case KN_AX25_ACTION_RETRANSMIT_NEEDED:
		return "retransmit-needed";
	}

	return "unknown";
}
