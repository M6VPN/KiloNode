/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/rf/rf_ignore.c */

#include <sys/types.h>

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "kilonode/rf_ignore.h"

static int callsign_equal(const struct kn_callsign *,
	const struct kn_callsign *);
static enum kn_rf_ignore_error line_parse(struct kn_rf_ignore_list *,
	char *);
static void reason_set(char *, size_t, const char *);

enum kn_rf_ignore_error
kn_rf_ignore_add(struct kn_rf_ignore_list *list,
	const struct kn_callsign *callsign, const char *reason)
{
	size_t i;

	if (list == NULL || callsign == NULL)
		return KN_RF_IGNORE_ERR_INVALID_ARGUMENT;

	for (i = 0; i < list->count; i++) {
		if (callsign_equal(&list->entries[i].callsign, callsign) != 0) {
			reason_set(list->entries[i].reason,
			    sizeof(list->entries[i].reason), reason);
			return KN_RF_IGNORE_OK;
		}
	}

	if (list->count >= KN_RF_IGNORE_MAX_ENTRIES)
		return KN_RF_IGNORE_ERR_FULL;

	list->entries[list->count].callsign = *callsign;
	reason_set(list->entries[list->count].reason,
	    sizeof(list->entries[list->count].reason), reason);
	list->count++;
	return KN_RF_IGNORE_OK;
}

void
kn_rf_ignore_clear(struct kn_rf_ignore_list *list)
{
	if (list == NULL)
		return;

	memset(list, 0, sizeof(*list));
}

size_t
kn_rf_ignore_count(const struct kn_rf_ignore_list *list)
{
	return list == NULL ? 0 : list->count;
}

const struct kn_rf_ignore_entry *
kn_rf_ignore_entries(const struct kn_rf_ignore_list *list)
{
	return list == NULL ? NULL : list->entries;
}

const char *
kn_rf_ignore_error_name(enum kn_rf_ignore_error error)
{
	switch (error) {
	case KN_RF_IGNORE_OK:
		return "ok";
	case KN_RF_IGNORE_ERR_INVALID_ARGUMENT:
		return "invalid-argument";
	case KN_RF_IGNORE_ERR_IO:
		return "io";
	case KN_RF_IGNORE_ERR_PARSE:
		return "parse";
	case KN_RF_IGNORE_ERR_FULL:
		return "full";
	}

	return "unknown";
}

void
kn_rf_ignore_init(struct kn_rf_ignore_list *list)
{
	kn_rf_ignore_clear(list);
}

enum kn_rf_ignore_error
kn_rf_ignore_load_file(struct kn_rf_ignore_list *list, const char *path)
{
	FILE *fp;
	char line[KN_CONFIG_LINE_MAX];
	size_t len;

	if (list == NULL || path == NULL || path[0] == '\0')
		return KN_RF_IGNORE_ERR_INVALID_ARGUMENT;
	if (kn_rf_ignore_path_valid(path) == 0)
		return KN_RF_IGNORE_ERR_INVALID_ARGUMENT;

	fp = fopen(path, "r");
	if (fp == NULL)
		return errno == ENOENT ? KN_RF_IGNORE_OK : KN_RF_IGNORE_ERR_IO;

	while (fgets(line, sizeof(line), fp) != NULL) {
		len = strlen(line);
		if (len > 0 && line[len - 1] != '\n' && !feof(fp)) {
			(void)fclose(fp);
			return KN_RF_IGNORE_ERR_PARSE;
		}
		if (line_parse(list, line) != KN_RF_IGNORE_OK) {
			(void)fclose(fp);
			return KN_RF_IGNORE_ERR_PARSE;
		}
	}

	if (ferror(fp)) {
		(void)fclose(fp);
		return KN_RF_IGNORE_ERR_IO;
	}

	(void)fclose(fp);
	return KN_RF_IGNORE_OK;
}

uint8_t
kn_rf_ignore_path_valid(const char *path)
{
	size_t i;

	if (path == NULL || path[0] == '\0')
		return 0;
	if (strlen(path) >= KN_CONFIG_PATH_MAX)
		return 0;
	if (strstr(path, "..") != NULL)
		return 0;
	for (i = 0; path[i] != '\0'; i++) {
		if ((unsigned char)path[i] < 0x20 ||
		    (unsigned char)path[i] == 0x7f)
			return 0;
	}

	return 1;
}

uint8_t
kn_rf_ignore_source_ignored(const struct kn_rf_ignore_list *list,
	const struct kn_callsign *callsign,
	const struct kn_rf_ignore_entry **entry)
{
	size_t i;

	if (entry != NULL)
		*entry = NULL;
	if (list == NULL || callsign == NULL)
		return 0;

	for (i = 0; i < list->count; i++) {
		if (callsign_equal(&list->entries[i].callsign, callsign) != 0) {
			if (entry != NULL)
				*entry = &list->entries[i];
			return 1;
		}
	}

	return 0;
}

static int
callsign_equal(const struct kn_callsign *a, const struct kn_callsign *b)
{
	return strcmp(a->call, b->call) == 0 && a->ssid == b->ssid;
}

static enum kn_rf_ignore_error
line_parse(struct kn_rf_ignore_list *list, char *line)
{
	struct kn_callsign callsign;
	char call[KN_CALLSIGN_MAX + 4];
	char reason[KN_RF_IGNORE_REASON_MAX];
	char *p;
	char *start;
	size_t len;
	size_t i;

	p = line;
	while (*p == ' ' || *p == '\t')
		p++;
	if (*p == '\0' || *p == '\n' || *p == '#')
		return KN_RF_IGNORE_OK;

	start = p;
	while (*p != '\0' && *p != '\n' && *p != '\r' && *p != ' ' &&
	    *p != '\t' && *p != '#')
		p++;
	len = (size_t)(p - start);
	if (len == 0 || len >= sizeof(call))
		return KN_RF_IGNORE_ERR_PARSE;
	for (i = 0; i < len; i++)
		call[i] = (char)toupper((unsigned char)start[i]);
	call[len] = '\0';
	if (kn_callsign_parse(call, &callsign) != 0)
		return KN_RF_IGNORE_ERR_PARSE;

	while (*p == ' ' || *p == '\t')
		p++;
	if (*p == '\0' || *p == '\n' || *p == '\r' || *p == '#') {
		reason_set(reason, sizeof(reason), "manual");
		return kn_rf_ignore_add(list, &callsign, reason);
	}

	start = p;
	while (*p != '\0' && *p != '\n' && *p != '\r' && *p != '#')
		p++;
	len = (size_t)(p - start);
	while (len > 0 && (start[len - 1] == ' ' || start[len - 1] == '\t'))
		len--;
	if (len == 0)
		reason_set(reason, sizeof(reason), "manual");
	else {
		if (len >= sizeof(reason))
			len = sizeof(reason) - 1;
		memcpy(reason, start, len);
		reason[len] = '\0';
	}

	return kn_rf_ignore_add(list, &callsign, reason);
}

static void
reason_set(char *dst, size_t dst_len, const char *reason)
{
	int needed;

	if (dst == NULL || dst_len == 0)
		return;

	needed = snprintf(dst, dst_len, "%s",
	    reason == NULL || reason[0] == '\0' ? "manual" : reason);
	if (needed < 0 || (size_t)needed >= dst_len)
		dst[0] = '\0';
}
