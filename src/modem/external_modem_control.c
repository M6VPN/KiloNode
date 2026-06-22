/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/modem/external_modem_control.c */

#include <sys/types.h>

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "kilonode/external_modem_control.h"

static enum kn_external_modem_control_error append_format(char *, size_t,
	size_t *, const char *, ...);
static enum kn_external_modem_control_error format_modem(const char *,
	const struct kn_external_modem_status_table *, char *, size_t);
static enum kn_external_modem_control_error format_modem_profiles(char *,
	size_t);
static enum kn_external_modem_control_error format_modem_profile(const char *,
	char *, size_t);
static enum kn_external_modem_control_error format_modems(
	const struct kn_external_modem_status_table *, char *, size_t);

static enum kn_external_modem_control_error
append_format(char *buf, size_t bufsiz, size_t *offset, const char *fmt, ...)
{
	va_list args;
	int needed;

	if (buf == NULL || offset == NULL || fmt == NULL || bufsiz == 0)
		return KN_EXTERNAL_MODEM_CONTROL_ERR_INVALID_ARGUMENT;
	if (*offset >= bufsiz)
		return KN_EXTERNAL_MODEM_CONTROL_ERR_BUFFER;

	va_start(args, fmt);
	needed = vsnprintf(buf + *offset, bufsiz - *offset, fmt, args);
	va_end(args);
	if (needed < 0 || (size_t)needed >= bufsiz - *offset)
		return KN_EXTERNAL_MODEM_CONTROL_ERR_BUFFER;

	*offset += (size_t)needed;
	return KN_EXTERNAL_MODEM_CONTROL_OK;
}

static enum kn_external_modem_control_error
format_modem(const char *name,
	const struct kn_external_modem_status_table *table, char *buf,
	size_t bufsiz)
{
	const struct kn_external_modem_status_entry *entry;
	char line[256];
	size_t offset;

	if (name == NULL || table == NULL || buf == NULL || bufsiz == 0)
		return KN_EXTERNAL_MODEM_CONTROL_ERR_INVALID_ARGUMENT;

	entry = kn_external_modem_status_lookup(table, name);
	if (entry == NULL) {
		if (snprintf(buf, bufsiz, "ERR modem-not-found\n") < 0)
			return KN_EXTERNAL_MODEM_CONTROL_ERR_BUFFER;
		return KN_EXTERNAL_MODEM_CONTROL_ERR_UNKNOWN_COMMAND;
	}
	if (kn_external_modem_status_format(entry, line, sizeof(line)) !=
	    KN_EXTERNAL_MODEM_OK)
		return KN_EXTERNAL_MODEM_CONTROL_ERR_BUFFER;

	offset = 0;
	if (append_format(buf, bufsiz, &offset, "OK MODEM name=%s\n",
	    entry->name) != KN_EXTERNAL_MODEM_CONTROL_OK)
		return KN_EXTERNAL_MODEM_CONTROL_ERR_BUFFER;
	if (append_format(buf, bufsiz, &offset, "%s\n", line) !=
	    KN_EXTERNAL_MODEM_CONTROL_OK)
		return KN_EXTERNAL_MODEM_CONTROL_ERR_BUFFER;
	return append_format(buf, bufsiz, &offset, "END\n");
}

static enum kn_external_modem_control_error
format_modem_profiles(char *buf, size_t bufsiz)
{
	const struct kn_external_modem_profile *profiles;
	char line[256];
	size_t count;
	size_t i;
	size_t offset;

	if (buf == NULL || bufsiz == 0)
		return KN_EXTERNAL_MODEM_CONTROL_ERR_INVALID_ARGUMENT;

	profiles = kn_external_modem_profiles(&count);
	offset = 0;
	if (append_format(buf, bufsiz, &offset,
	    "OK MODEM PROFILES count=%llu\n", (unsigned long long)count) !=
	    KN_EXTERNAL_MODEM_CONTROL_OK)
		return KN_EXTERNAL_MODEM_CONTROL_ERR_BUFFER;
	for (i = 0; i < count; i++) {
		if (kn_external_modem_profile_format(&profiles[i], line,
		    sizeof(line)) != KN_EXTERNAL_MODEM_OK)
			return KN_EXTERNAL_MODEM_CONTROL_ERR_BUFFER;
		if (append_format(buf, bufsiz, &offset, "%s\n", line) !=
		    KN_EXTERNAL_MODEM_CONTROL_OK)
			return KN_EXTERNAL_MODEM_CONTROL_ERR_BUFFER;
	}

	return append_format(buf, bufsiz, &offset, "END\n");
}

static enum kn_external_modem_control_error
format_modem_profile(const char *type_name, char *buf, size_t bufsiz)
{
	const struct kn_external_modem_profile *profile;
	enum kn_external_modem_type type;
	char line[256];
	size_t offset;

	if (type_name == NULL || buf == NULL || bufsiz == 0)
		return KN_EXTERNAL_MODEM_CONTROL_ERR_INVALID_ARGUMENT;

	type = kn_external_modem_type_parse(type_name);
	profile = kn_external_modem_profile_for_type(type);
	if (profile == NULL) {
		if (snprintf(buf, bufsiz, "ERR modem-profile-not-found\n") < 0)
			return KN_EXTERNAL_MODEM_CONTROL_ERR_BUFFER;
		return KN_EXTERNAL_MODEM_CONTROL_ERR_UNKNOWN_COMMAND;
	}
	if (kn_external_modem_profile_format(profile, line, sizeof(line)) !=
	    KN_EXTERNAL_MODEM_OK)
		return KN_EXTERNAL_MODEM_CONTROL_ERR_BUFFER;

	offset = 0;
	if (append_format(buf, bufsiz, &offset, "OK MODEM PROFILE type=%s\n",
	    kn_external_modem_type_name(profile->type)) !=
	    KN_EXTERNAL_MODEM_CONTROL_OK)
		return KN_EXTERNAL_MODEM_CONTROL_ERR_BUFFER;
	if (append_format(buf, bufsiz, &offset, "%s\n", line) !=
	    KN_EXTERNAL_MODEM_CONTROL_OK)
		return KN_EXTERNAL_MODEM_CONTROL_ERR_BUFFER;
	return append_format(buf, bufsiz, &offset, "END\n");
}

static enum kn_external_modem_control_error
format_modems(const struct kn_external_modem_status_table *table, char *buf,
	size_t bufsiz)
{
	char line[256];
	size_t i;
	size_t offset;

	if (table == NULL || buf == NULL || bufsiz == 0)
		return KN_EXTERNAL_MODEM_CONTROL_ERR_INVALID_ARGUMENT;

	offset = 0;
	if (append_format(buf, bufsiz, &offset, "OK MODEMS count=%llu\n",
	    (unsigned long long)table->count) !=
	    KN_EXTERNAL_MODEM_CONTROL_OK)
		return KN_EXTERNAL_MODEM_CONTROL_ERR_BUFFER;
	for (i = 0; i < table->count; i++) {
		if (kn_external_modem_status_format(&table->entries[i], line,
		    sizeof(line)) != KN_EXTERNAL_MODEM_OK)
			return KN_EXTERNAL_MODEM_CONTROL_ERR_BUFFER;
		if (append_format(buf, bufsiz, &offset, "%s\n", line) !=
		    KN_EXTERNAL_MODEM_CONTROL_OK)
			return KN_EXTERNAL_MODEM_CONTROL_ERR_BUFFER;
	}

	return append_format(buf, bufsiz, &offset, "END\n");
}

enum kn_external_modem_control_error
kn_external_modem_control_format(const char *command,
	const struct kn_external_modem_status_table *table, char *buf,
	size_t bufsiz)
{
	struct kn_external_modem_status_table empty;

	if (command == NULL || buf == NULL || bufsiz == 0)
		return KN_EXTERNAL_MODEM_CONTROL_ERR_INVALID_ARGUMENT;

	if (table == NULL) {
		memset(&empty, 0, sizeof(empty));
		table = &empty;
	}

	if (strcmp(command, "MODEMS") == 0)
		return format_modems(table, buf, bufsiz);
	if (strcmp(command, "MODEM PROFILES") == 0)
		return format_modem_profiles(buf, bufsiz);
	if (strncmp(command, "MODEM PROFILE ", 14) == 0) {
		if (command[14] == '\0' || strchr(command + 14, ' ') != NULL)
			goto malformed;
		return format_modem_profile(command + 14, buf, bufsiz);
	}
	if (strncmp(command, "MODEM ", 6) == 0) {
		if (command[6] == '\0' || strchr(command + 6, ' ') != NULL)
			goto malformed;
		return format_modem(command + 6, table, buf, bufsiz);
	}

malformed:
	if (snprintf(buf, bufsiz, "ERR invalid-modem-command\n") < 0)
		return KN_EXTERNAL_MODEM_CONTROL_ERR_BUFFER;
	return KN_EXTERNAL_MODEM_CONTROL_ERR_UNKNOWN_COMMAND;
}
