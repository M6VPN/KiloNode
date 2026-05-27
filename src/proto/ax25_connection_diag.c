/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/proto/ax25_connection_diag.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "kilonode/ax25_connection_diag.h"

static enum kn_ax25_connection_diag_error append_text(char *, size_t,
	size_t *, const char *);

static enum kn_ax25_connection_diag_error
append_text(char *buf, size_t bufsiz, size_t *used, const char *text)
{
	size_t len;

	if (buf == NULL || used == NULL || text == NULL || *used >= bufsiz)
		return KN_AX25_CONNECTION_DIAG_ERR_INVALID_ARGUMENT;

	len = strlen(text);
	if (len >= bufsiz - *used)
		return KN_AX25_CONNECTION_DIAG_ERR_BUFFER;
	memcpy(buf + *used, text, len + 1);
	*used += len;
	return KN_AX25_CONNECTION_DIAG_OK;
}

enum kn_ax25_connection_diag_error
kn_ax25_connection_diag_format_actions(
	const struct kn_ax25_action_list *actions, char *buf, size_t bufsiz)
{
	if (actions == NULL || buf == NULL || bufsiz == 0)
		return KN_AX25_CONNECTION_DIAG_ERR_INVALID_ARGUMENT;
	if (kn_ax25_action_list_format(actions, buf, bufsiz) !=
	    KN_AX25_ACTION_OK)
		return KN_AX25_CONNECTION_DIAG_ERR_BUFFER;

	return KN_AX25_CONNECTION_DIAG_OK;
}

enum kn_ax25_connection_diag_error
kn_ax25_connection_diag_format_frame_plans(
	const struct kn_ax25_frame_plan_list *plans, char *buf, size_t bufsiz)
{
	char item[256];
	size_t i;
	size_t used;

	if (plans == NULL || buf == NULL || bufsiz == 0)
		return KN_AX25_CONNECTION_DIAG_ERR_INVALID_ARGUMENT;

	buf[0] = '\0';
	used = 0;
	if (plans->count == 0)
		return append_text(buf, bufsiz, &used, "none");

	for (i = 0; i < plans->count; i++) {
		if (i > 0 && append_text(buf, bufsiz, &used, ";") !=
		    KN_AX25_CONNECTION_DIAG_OK)
			return KN_AX25_CONNECTION_DIAG_ERR_BUFFER;
		if (kn_ax25_frame_plan_format(&plans->plans[i], item,
		    sizeof(item)) != KN_AX25_FRAME_PLAN_OK)
			return KN_AX25_CONNECTION_DIAG_ERR_BUFFER;
		if (append_text(buf, bufsiz, &used, item) !=
		    KN_AX25_CONNECTION_DIAG_OK)
			return KN_AX25_CONNECTION_DIAG_ERR_BUFFER;
	}

	return KN_AX25_CONNECTION_DIAG_OK;
}

enum kn_ax25_connection_diag_error
kn_ax25_connection_diag_format_key(const struct kn_ax25_connection_key *key,
	char *buf, size_t bufsiz)
{
	if (key == NULL || buf == NULL || bufsiz == 0)
		return KN_AX25_CONNECTION_DIAG_ERR_INVALID_ARGUMENT;
	if (kn_ax25_connection_key_format(key, buf, bufsiz) !=
	    KN_AX25_CONNECTION_KEY_OK)
		return KN_AX25_CONNECTION_DIAG_ERR_BUFFER;

	return KN_AX25_CONNECTION_DIAG_OK;
}

enum kn_ax25_connection_diag_error
kn_ax25_connection_diag_format_record(
	const struct kn_ax25_connection_record *record, char *buf,
	size_t bufsiz)
{
	char key[192];
	int needed;

	if (record == NULL || buf == NULL || bufsiz == 0)
		return KN_AX25_CONNECTION_DIAG_ERR_INVALID_ARGUMENT;
	if (kn_ax25_connection_key_format(&record->key, key,
	    sizeof(key)) != KN_AX25_CONNECTION_KEY_OK)
		return KN_AX25_CONNECTION_DIAG_ERR_BUFFER;

	needed = snprintf(buf, bufsiz,
	    "AX25 CONN %s state=%s last=%s actions=%llu plans=%llu "
	    "errors=%llu",
	    key, kn_ax25_connection_state_name(record->connection.state),
	    kn_ax25_connection_event_name(record->last_event_kind),
	    (unsigned long long)record->last_actions.count,
	    (unsigned long long)record->last_plans.count,
	    (unsigned long long)record->counters.protocol_errors);
	if (needed < 0 || (size_t)needed >= bufsiz)
		return KN_AX25_CONNECTION_DIAG_ERR_BUFFER;

	return KN_AX25_CONNECTION_DIAG_OK;
}

enum kn_ax25_connection_diag_error
kn_ax25_connection_diag_format_state(
	const struct kn_ax25_connection *connection, char *buf, size_t bufsiz)
{
	if (connection == NULL || buf == NULL || bufsiz == 0)
		return KN_AX25_CONNECTION_DIAG_ERR_INVALID_ARGUMENT;
	if (kn_ax25_connection_format(connection, buf, bufsiz) !=
	    KN_AX25_CONNECTION_OK)
		return KN_AX25_CONNECTION_DIAG_ERR_BUFFER;

	return KN_AX25_CONNECTION_DIAG_OK;
}

enum kn_ax25_connection_diag_error
kn_ax25_connection_diag_format_table(
	const struct kn_ax25_connection_table *table, char *buf, size_t bufsiz)
{
	int needed;

	if (table == NULL || buf == NULL || bufsiz == 0)
		return KN_AX25_CONNECTION_DIAG_ERR_INVALID_ARGUMENT;

	needed = snprintf(buf, bufsiz, "AX25 TABLE count=%llu max=%llu",
	    (unsigned long long)table->count,
	    (unsigned long long)table->max_connections);
	if (needed < 0 || (size_t)needed >= bufsiz)
		return KN_AX25_CONNECTION_DIAG_ERR_BUFFER;

	return KN_AX25_CONNECTION_DIAG_OK;
}
