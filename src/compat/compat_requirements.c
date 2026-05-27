/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/compat/compat_requirements.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "kilonode/compat_coverage.h"
#include "kilonode/compat_requirements.h"

#define REQ_FIELD_NAME             0x0001u
#define REQ_FIELD_SUBJECT          0x0002u
#define REQ_FIELD_SOURCE_PACK      0x0004u
#define REQ_FIELD_CLEAN_ROOM       0x0008u
#define REQ_FIELD_SOURCE_CODE_USED 0x0010u
#define REQ_BLOCK_STATUS           0x0001u
#define REQ_BLOCK_PRIORITY         0x0002u
#define REQ_BLOCK_OBSERVED         0x0004u
#define REQ_BLOCK_MODE             0x0008u
#define REQ_BLOCK_NOTES            0x0010u

static enum kn_compat_req_error append_requirement(char *, size_t, size_t *,
	const char *, enum kn_compat_req_status, enum kn_compat_req_priority,
	const char *, const char *);
static enum kn_compat_req_error bool_parse(const char *, uint8_t *);
static void error_set(struct kn_compat_req_error_info *,
	enum kn_compat_req_error, size_t, const char *);
static enum kn_compat_req_error header_set(struct kn_compat_requirements *,
	uint32_t *, const char *, const char *, size_t,
	struct kn_compat_req_error_info *);
static enum kn_compat_req_error priority_parse(const char *,
	enum kn_compat_req_priority *);
static enum kn_compat_req_error requirement_finish(
	struct kn_compat_requirement *, uint32_t, size_t,
	struct kn_compat_req_error_info *);
static enum kn_compat_req_error requirement_set(
	struct kn_compat_requirement *, uint32_t *, const char *,
	const char *, size_t, struct kn_compat_req_error_info *);
static enum kn_compat_req_error status_parse(const char *,
	enum kn_compat_req_status *);
static uint8_t field_seen(uint32_t *, uint32_t);
static char *trim(char *);
static uint8_t unsafe_ref(const char *);

void
kn_compat_requirements_clear(struct kn_compat_requirements *requirements)
{
	if (requirements == NULL)
		return;
	memset(requirements, 0, sizeof(*requirements));
}

const char *
kn_compat_req_error_name(enum kn_compat_req_error error)
{
	switch (error) {
	case KN_COMPAT_REQ_OK:
		return "ok";
	case KN_COMPAT_REQ_ERR_INVALID_ARGUMENT:
		return "invalid-argument";
	case KN_COMPAT_REQ_ERR_IO:
		return "io";
	case KN_COMPAT_REQ_ERR_LINE_TOO_LONG:
		return "line-too-long";
	case KN_COMPAT_REQ_ERR_PARSE:
		return "parse";
	case KN_COMPAT_REQ_ERR_DUPLICATE_KEY:
		return "duplicate-key";
	case KN_COMPAT_REQ_ERR_UNKNOWN_KEY:
		return "unknown-key";
	case KN_COMPAT_REQ_ERR_MISSING_REQUIRED:
		return "missing-required";
	case KN_COMPAT_REQ_ERR_INVALID_VALUE:
		return "invalid-value";
	case KN_COMPAT_REQ_ERR_TOO_MANY:
		return "too-many";
	case KN_COMPAT_REQ_ERR_REFERENCE:
		return "reference";
	case KN_COMPAT_REQ_ERR_BUFFER:
		return "buffer";
	}

	return "unknown";
}

const char *
kn_compat_req_priority_name(enum kn_compat_req_priority priority)
{
	switch (priority) {
	case KN_COMPAT_REQ_PRIORITY_LOW:
		return "low";
	case KN_COMPAT_REQ_PRIORITY_MEDIUM:
		return "medium";
	case KN_COMPAT_REQ_PRIORITY_HIGH:
		return "high";
	case KN_COMPAT_REQ_PRIORITY_CRITICAL:
		return "critical";
	}

	return "unknown";
}

const char *
kn_compat_req_status_name(enum kn_compat_req_status status)
{
	switch (status) {
	case KN_COMPAT_REQ_STATUS_PLANNED:
		return "planned";
	case KN_COMPAT_REQ_STATUS_BLOCKED:
		return "blocked";
	case KN_COMPAT_REQ_STATUS_NEEDS_OBSERVATION:
		return "needs-observation";
	case KN_COMPAT_REQ_STATUS_READY_FOR_DESIGN:
		return "ready-for-design";
	case KN_COMPAT_REQ_STATUS_READY_FOR_IMPLEMENTATION:
		return "ready-for-implementation";
	case KN_COMPAT_REQ_STATUS_IMPLEMENTED_NATIVE:
		return "implemented-native";
	case KN_COMPAT_REQ_STATUS_IMPLEMENTED_COMPATIBLE:
		return "implemented-compatible";
	case KN_COMPAT_REQ_STATUS_OUT_OF_SCOPE:
		return "out-of-scope";
	}

	return "unknown";
}

const struct kn_compat_requirement *
kn_compat_requirements_find(const struct kn_compat_requirements *requirements,
	const char *command)
{
	size_t i;

	if (requirements == NULL || command == NULL)
		return NULL;
	for (i = 0; i < requirements->requirement_count; i++) {
		if (strcmp(requirements->requirements[i].command, command) == 0)
			return &requirements->requirements[i];
	}

	return NULL;
}

enum kn_compat_req_error
kn_compat_requirements_parse_file(const char *path,
	struct kn_compat_requirements *requirements,
	struct kn_compat_req_error_info *info)
{
	FILE *fp;
	char text[KN_COMPAT_REQ_TEXT_MAX + 1];
	size_t len;
	int ch;
	enum kn_compat_req_error rc;

	if (path == NULL || requirements == NULL)
		return KN_COMPAT_REQ_ERR_INVALID_ARGUMENT;
	fp = fopen(path, "r");
	if (fp == NULL) {
		error_set(info, KN_COMPAT_REQ_ERR_IO, 0, "open failed");
		return KN_COMPAT_REQ_ERR_IO;
	}
	len = 0;
	while ((ch = fgetc(fp)) != EOF) {
		if (len >= KN_COMPAT_REQ_TEXT_MAX) {
			(void)fclose(fp);
			error_set(info, KN_COMPAT_REQ_ERR_INVALID_VALUE, 0,
			    "requirements too large");
			return KN_COMPAT_REQ_ERR_INVALID_VALUE;
		}
		text[len++] = (char)ch;
	}
	if (ferror(fp)) {
		(void)fclose(fp);
		error_set(info, KN_COMPAT_REQ_ERR_IO, 0, "read failed");
		return KN_COMPAT_REQ_ERR_IO;
	}
	(void)fclose(fp);
	text[len] = '\0';
	rc = kn_compat_requirements_parse_text(text, requirements, info);
	if (rc == KN_COMPAT_REQ_OK)
		(void)snprintf(requirements->path, sizeof(requirements->path),
		    "%s", path);

	return rc;
}

enum kn_compat_req_error
kn_compat_requirements_parse_text(const char *text,
	struct kn_compat_requirements *requirements,
	struct kn_compat_req_error_info *info)
{
	struct kn_compat_requirement current;
	char line[KN_COMPAT_REQ_LINE_MAX];
	char *key;
	char *value;
	size_t i;
	size_t line_len;
	size_t line_no;
	uint32_t header_seen;
	uint32_t block_seen;
	uint8_t in_block;
	enum kn_compat_req_error rc;

	if (text == NULL || requirements == NULL)
		return KN_COMPAT_REQ_ERR_INVALID_ARGUMENT;
	kn_compat_requirements_clear(requirements);
	if (info != NULL)
		memset(info, 0, sizeof(*info));
	memset(&current, 0, sizeof(current));
	header_seen = block_seen = 0;
	in_block = 0;
	line_no = 1;
	line_len = 0;

	for (i = 0;; i++) {
		if (text[i] != '\0' && text[i] != '\n') {
			if (line_len + 1 >= sizeof(line)) {
				error_set(info, KN_COMPAT_REQ_ERR_LINE_TOO_LONG,
				    line_no, "line too long");
				return KN_COMPAT_REQ_ERR_LINE_TOO_LONG;
			}
			line[line_len++] = text[i];
			continue;
		}
		line[line_len] = '\0';
		key = trim(line);
		if (key[0] != '\0' && key[0] != '#') {
			if (strcmp(key, "}") == 0) {
				if (in_block == 0) {
					error_set(info, KN_COMPAT_REQ_ERR_PARSE,
					    line_no, "unexpected block end");
					return KN_COMPAT_REQ_ERR_PARSE;
				}
				rc = requirement_finish(&current, block_seen,
				    line_no, info);
				if (rc != KN_COMPAT_REQ_OK)
					return rc;
				requirements->requirements[
				    requirements->requirement_count++] =
				    current;
				memset(&current, 0, sizeof(current));
				block_seen = 0;
				in_block = 0;
			} else if (in_block != 0) {
				value = key;
				while (*value != '\0' && *value != ' ' &&
				    *value != '\t')
					value++;
				if (*value == '\0')
					goto parse;
				*value++ = '\0';
				rc = requirement_set(&current, &block_seen,
				    key, trim(value), line_no, info);
				if (rc != KN_COMPAT_REQ_OK)
					return rc;
			} else if (strncmp(key, "requirement ", 12) == 0) {
				value = trim(key + 12);
				if (requirements->requirement_count >=
				    KN_COMPAT_REQ_MAX)
					return KN_COMPAT_REQ_ERR_TOO_MANY;
				if (value[0] == '\0' ||
				    value[strlen(value) - 1] != '{')
					goto parse;
				value[strlen(value) - 1] = '\0';
				value = trim(value);
				if (kn_compat_requirements_find(requirements,
				    value) != NULL)
					return KN_COMPAT_REQ_ERR_DUPLICATE_KEY;
				if (strlen(value) >= sizeof(current.command))
					return KN_COMPAT_REQ_ERR_INVALID_VALUE;
				(void)snprintf(current.command,
				    sizeof(current.command), "%s", value);
				in_block = 1;
			} else {
				value = key;
				while (*value != '\0' && *value != ' ' &&
				    *value != '\t')
					value++;
				if (*value == '\0')
					goto parse;
				*value++ = '\0';
				rc = header_set(requirements, &header_seen,
				    key, trim(value), line_no, info);
				if (rc != KN_COMPAT_REQ_OK)
					return rc;
			}
		}
		if (text[i] == '\0')
			break;
		line_no++;
		line_len = 0;
	}
	if (in_block != 0)
		goto parse;
	if ((header_seen & (REQ_FIELD_NAME | REQ_FIELD_SUBJECT |
	    REQ_FIELD_SOURCE_PACK | REQ_FIELD_CLEAN_ROOM |
	    REQ_FIELD_SOURCE_CODE_USED)) != (REQ_FIELD_NAME |
	    REQ_FIELD_SUBJECT | REQ_FIELD_SOURCE_PACK |
	    REQ_FIELD_CLEAN_ROOM | REQ_FIELD_SOURCE_CODE_USED)) {
		error_set(info, KN_COMPAT_REQ_ERR_MISSING_REQUIRED, 0,
		    "missing required field");
		return KN_COMPAT_REQ_ERR_MISSING_REQUIRED;
	}

	return requirements->requirement_count > 0 ? KN_COMPAT_REQ_OK :
	    KN_COMPAT_REQ_ERR_MISSING_REQUIRED;

parse:
	error_set(info, KN_COMPAT_REQ_ERR_PARSE, line_no, "parse");
	return KN_COMPAT_REQ_ERR_PARSE;
}

enum kn_compat_req_error
kn_compat_requirements_report(const struct kn_compat_requirements *requirements,
	char *buf, size_t bufsiz)
{
	size_t counts[8];
	size_t priorities[4];
	size_t i;
	size_t off;
	int needed;

	if (requirements == NULL || buf == NULL || bufsiz == 0)
		return KN_COMPAT_REQ_ERR_INVALID_ARGUMENT;
	memset(counts, 0, sizeof(counts));
	memset(priorities, 0, sizeof(priorities));
	for (i = 0; i < requirements->requirement_count; i++) {
		counts[requirements->requirements[i].status]++;
		priorities[requirements->requirements[i].priority]++;
	}
	needed = snprintf(buf, bufsiz,
	    "REQUIREMENTS name=%s subject=%s count=%llu planned=%llu "
	    "blocked=%llu needs_observation=%llu high=%llu critical=%llu\n",
	    requirements->name, requirements->subject,
	    (unsigned long long)requirements->requirement_count,
	    (unsigned long long)counts[KN_COMPAT_REQ_STATUS_PLANNED],
	    (unsigned long long)counts[KN_COMPAT_REQ_STATUS_BLOCKED],
	    (unsigned long long)counts[
	    KN_COMPAT_REQ_STATUS_NEEDS_OBSERVATION],
	    (unsigned long long)priorities[KN_COMPAT_REQ_PRIORITY_HIGH],
	    (unsigned long long)priorities[
	    KN_COMPAT_REQ_PRIORITY_CRITICAL]);
	if (needed < 0 || (size_t)needed >= bufsiz)
		return KN_COMPAT_REQ_ERR_BUFFER;
	off = (size_t)needed;
	for (i = 0; i < requirements->requirement_count; i++) {
		needed = snprintf(buf + off, bufsiz - off,
		    "REQUIREMENT command=%s status=%s priority=%s "
		    "observed=%s mode=%s notes=\"%s\"\n",
		    requirements->requirements[i].command,
		    kn_compat_req_status_name(requirements->requirements[i].
		    status),
		    kn_compat_req_priority_name(requirements->requirements[i].
		    priority),
		    requirements->requirements[i].observed,
		    requirements->requirements[i].mode,
		    requirements->requirements[i].notes);
		if (needed < 0 || (size_t)needed >= bufsiz - off)
			return KN_COMPAT_REQ_ERR_BUFFER;
		off += (size_t)needed;
	}

	return KN_COMPAT_REQ_OK;
}

enum kn_compat_req_error
kn_compat_requirements_coverage_report(
	const struct kn_compat_requirements *requirements,
	const struct kn_compat_observation_pack *pack, char *buf, size_t bufsiz)
{
	struct kn_compat_coverage coverage;
	const struct kn_compat_requirement *req;
	size_t i;
	size_t missing;
	size_t off;
	int needed;

	if (requirements == NULL || pack == NULL || buf == NULL || bufsiz == 0)
		return KN_COMPAT_REQ_ERR_INVALID_ARGUMENT;
	if (kn_compat_coverage_from_pack(pack, &coverage) !=
	    KN_COMPAT_COVERAGE_OK)
		return KN_COMPAT_REQ_ERR_REFERENCE;
	missing = 0;
	for (i = 0; i < coverage.count; i++) {
		if (kn_compat_requirements_find(requirements,
		    coverage.entries[i].command) == NULL)
			missing++;
	}
	needed = snprintf(buf, bufsiz,
	    "REQUIREMENTS-COVERAGE requirements=%s pack=%s missing=%llu\n",
	    requirements->name, pack->name, (unsigned long long)missing);
	if (needed < 0 || (size_t)needed >= bufsiz)
		return KN_COMPAT_REQ_ERR_BUFFER;
	off = (size_t)needed;
	for (i = 0; i < coverage.count; i++) {
		req = kn_compat_requirements_find(requirements,
		    coverage.entries[i].command);
		needed = snprintf(buf + off, bufsiz - off,
		    "REQUIREMENTS-COVERAGE command=%s requirement=%s "
		    "coverage=%s synthetic=%s manual=%s\n",
		    coverage.entries[i].command, req != NULL ? "true" :
		    "false",
		    kn_compat_coverage_status_name(coverage.entries[i].status),
		    coverage.entries[i].synthetic != 0 ? "true" : "false",
		    coverage.entries[i].manual_black_box != 0 ? "true" :
		    "false");
		if (needed < 0 || (size_t)needed >= bufsiz - off)
			return KN_COMPAT_REQ_ERR_BUFFER;
		off += (size_t)needed;
	}

	return missing == 0 ? KN_COMPAT_REQ_OK :
	    KN_COMPAT_REQ_ERR_REFERENCE;
}

enum kn_compat_req_error
kn_compat_requirements_generate_from_pack(
	const struct kn_compat_observation_pack *pack, char *buf, size_t bufsiz)
{
	size_t off;
	enum kn_compat_req_error rc;

	if (pack == NULL || buf == NULL || bufsiz == 0)
		return KN_COMPAT_REQ_ERR_INVALID_ARGUMENT;
	(void)snprintf(buf, bufsiz,
	    "# KiloNode compatibility requirements v1\n"
	    "name %s-requirements\n"
	    "subject %s\n"
	    "source-pack manifest.pack\n"
	    "clean-room true\n"
	    "source-code-used false\n\n",
	    pack->name, pack->subject);
	off = strlen(buf);
#define ADD_REQ(cmd, st, prio, obs) do { \
	rc = append_requirement(buf, bufsiz, &off, (cmd), (st), (prio), \
	    (obs), "node-command"); \
	if (rc != KN_COMPAT_REQ_OK) return rc; \
} while (0)
	ADD_REQ("HELP", KN_COMPAT_REQ_STATUS_NEEDS_OBSERVATION,
	    KN_COMPAT_REQ_PRIORITY_HIGH, "synthetic");
	ADD_REQ("INFO", KN_COMPAT_REQ_STATUS_NEEDS_OBSERVATION,
	    KN_COMPAT_REQ_PRIORITY_HIGH, "synthetic");
	ADD_REQ("PORTS", KN_COMPAT_REQ_STATUS_NEEDS_OBSERVATION,
	    KN_COMPAT_REQ_PRIORITY_HIGH, "synthetic");
	ADD_REQ("USERS", KN_COMPAT_REQ_STATUS_NEEDS_OBSERVATION,
	    KN_COMPAT_REQ_PRIORITY_MEDIUM, "synthetic");
	ADD_REQ("HEARD", KN_COMPAT_REQ_STATUS_IMPLEMENTED_NATIVE,
	    KN_COMPAT_REQ_PRIORITY_HIGH, "planned");
	ADD_REQ("STATS", KN_COMPAT_REQ_STATUS_IMPLEMENTED_NATIVE,
	    KN_COMPAT_REQ_PRIORITY_MEDIUM, "planned");
	ADD_REQ("BYE", KN_COMPAT_REQ_STATUS_PLANNED,
	    KN_COMPAT_REQ_PRIORITY_MEDIUM, "planned");
	ADD_REQ("UNKNOWN", KN_COMPAT_REQ_STATUS_NEEDS_OBSERVATION,
	    KN_COMPAT_REQ_PRIORITY_HIGH, "synthetic");
	ADD_REQ("BBS", KN_COMPAT_REQ_STATUS_BLOCKED,
	    KN_COMPAT_REQ_PRIORITY_HIGH, "planned");
	ADD_REQ("CONNECT", KN_COMPAT_REQ_STATUS_BLOCKED,
	    KN_COMPAT_REQ_PRIORITY_CRITICAL, "planned");
	ADD_REQ("NODES", KN_COMPAT_REQ_STATUS_BLOCKED,
	    KN_COMPAT_REQ_PRIORITY_HIGH, "planned");
	ADD_REQ("ROUTES", KN_COMPAT_REQ_STATUS_BLOCKED,
	    KN_COMPAT_REQ_PRIORITY_HIGH, "planned");
#undef ADD_REQ
	return KN_COMPAT_REQ_OK;
}

static enum kn_compat_req_error
append_requirement(char *buf, size_t bufsiz, size_t *off, const char *command,
	enum kn_compat_req_status status, enum kn_compat_req_priority priority,
	const char *observed, const char *mode)
{
	int needed;

	if (*off >= bufsiz)
		return KN_COMPAT_REQ_ERR_BUFFER;
	needed = snprintf(buf + *off, bufsiz - *off,
	    "requirement %s {\n"
	    "\tstatus %s\n"
	    "\tpriority %s\n"
	    "\tobserved %s\n"
	    "\tmode %s\n"
	    "\tnotes Needs manual black-box observations before implementation.\n"
	    "}\n\n",
	    command, kn_compat_req_status_name(status),
	    kn_compat_req_priority_name(priority), observed, mode);
	if (needed < 0 || (size_t)needed >= bufsiz - *off)
		return KN_COMPAT_REQ_ERR_BUFFER;
	*off += (size_t)needed;
	return KN_COMPAT_REQ_OK;
}

static enum kn_compat_req_error
bool_parse(const char *value, uint8_t *out)
{
	if (strcmp(value, "true") == 0)
		*out = 1;
	else if (strcmp(value, "false") == 0)
		*out = 0;
	else
		return KN_COMPAT_REQ_ERR_INVALID_VALUE;
	return KN_COMPAT_REQ_OK;
}

static void
error_set(struct kn_compat_req_error_info *info,
	enum kn_compat_req_error error, size_t line, const char *message)
{
	if (info == NULL)
		return;
	info->error = error;
	info->line = line;
	(void)snprintf(info->message, sizeof(info->message), "%s",
	    message == NULL ? "" : message);
}

static enum kn_compat_req_error
header_set(struct kn_compat_requirements *requirements, uint32_t *seen,
	const char *key, const char *value, size_t line,
	struct kn_compat_req_error_info *info)
{
	char *dst;
	size_t dst_len;
	uint32_t flag;
	uint8_t b;

	dst = NULL;
	dst_len = 0;
	flag = 0;
	if (strcmp(key, "name") == 0) {
		dst = requirements->name;
		dst_len = sizeof(requirements->name);
		flag = REQ_FIELD_NAME;
	} else if (strcmp(key, "subject") == 0) {
		dst = requirements->subject;
		dst_len = sizeof(requirements->subject);
		flag = REQ_FIELD_SUBJECT;
	} else if (strcmp(key, "source-pack") == 0) {
		if (unsafe_ref(value) != 0)
			goto invalid;
		dst = requirements->source_pack;
		dst_len = sizeof(requirements->source_pack);
		flag = REQ_FIELD_SOURCE_PACK;
	} else if (strcmp(key, "clean-room") == 0) {
		if (field_seen(seen, REQ_FIELD_CLEAN_ROOM) != 0)
			goto duplicate;
		if (bool_parse(value, &b) != KN_COMPAT_REQ_OK || b == 0)
			goto invalid;
		requirements->clean_room = b;
		return KN_COMPAT_REQ_OK;
	} else if (strcmp(key, "source-code-used") == 0) {
		if (field_seen(seen, REQ_FIELD_SOURCE_CODE_USED) != 0)
			goto duplicate;
		if (bool_parse(value, &b) != KN_COMPAT_REQ_OK || b != 0)
			goto invalid;
		requirements->source_code_used = b;
		return KN_COMPAT_REQ_OK;
	} else {
		error_set(info, KN_COMPAT_REQ_ERR_UNKNOWN_KEY, line, key);
		return KN_COMPAT_REQ_ERR_UNKNOWN_KEY;
	}
	if (field_seen(seen, flag) != 0)
		goto duplicate;
	if (strlen(value) >= dst_len)
		goto invalid;
	(void)snprintf(dst, dst_len, "%s", value);
	return KN_COMPAT_REQ_OK;

duplicate:
	error_set(info, KN_COMPAT_REQ_ERR_DUPLICATE_KEY, line, key);
	return KN_COMPAT_REQ_ERR_DUPLICATE_KEY;
invalid:
	error_set(info, KN_COMPAT_REQ_ERR_INVALID_VALUE, line, key);
	return KN_COMPAT_REQ_ERR_INVALID_VALUE;
}

static enum kn_compat_req_error
priority_parse(const char *value, enum kn_compat_req_priority *priority)
{
	if (strcmp(value, "low") == 0)
		*priority = KN_COMPAT_REQ_PRIORITY_LOW;
	else if (strcmp(value, "medium") == 0)
		*priority = KN_COMPAT_REQ_PRIORITY_MEDIUM;
	else if (strcmp(value, "high") == 0)
		*priority = KN_COMPAT_REQ_PRIORITY_HIGH;
	else if (strcmp(value, "critical") == 0)
		*priority = KN_COMPAT_REQ_PRIORITY_CRITICAL;
	else
		return KN_COMPAT_REQ_ERR_INVALID_VALUE;
	return KN_COMPAT_REQ_OK;
}

static enum kn_compat_req_error
requirement_finish(struct kn_compat_requirement *req, uint32_t seen,
	size_t line, struct kn_compat_req_error_info *info)
{
	if ((seen & (REQ_BLOCK_STATUS | REQ_BLOCK_PRIORITY)) !=
	    (REQ_BLOCK_STATUS | REQ_BLOCK_PRIORITY)) {
		error_set(info, KN_COMPAT_REQ_ERR_MISSING_REQUIRED, line,
		    req->command);
		return KN_COMPAT_REQ_ERR_MISSING_REQUIRED;
	}
	if (req->observed[0] == '\0')
		(void)snprintf(req->observed, sizeof(req->observed),
		    "planned");
	if (req->mode[0] == '\0')
		(void)snprintf(req->mode, sizeof(req->mode), "node-command");
	return KN_COMPAT_REQ_OK;
}

static enum kn_compat_req_error
requirement_set(struct kn_compat_requirement *req, uint32_t *seen,
	const char *key, const char *value, size_t line,
	struct kn_compat_req_error_info *info)
{
	char *dst;
	size_t dst_len;
	uint32_t flag;

	dst = NULL;
	dst_len = 0;
	flag = 0;
	if (strcmp(key, "status") == 0) {
		if (field_seen(seen, REQ_BLOCK_STATUS) != 0)
			goto duplicate;
		if (status_parse(value, &req->status) != KN_COMPAT_REQ_OK)
			goto invalid;
		req->has_status = 1;
		return KN_COMPAT_REQ_OK;
	} else if (strcmp(key, "priority") == 0) {
		if (field_seen(seen, REQ_BLOCK_PRIORITY) != 0)
			goto duplicate;
		if (priority_parse(value, &req->priority) !=
		    KN_COMPAT_REQ_OK)
			goto invalid;
		req->has_priority = 1;
		return KN_COMPAT_REQ_OK;
	} else if (strcmp(key, "observed") == 0) {
		dst = req->observed;
		dst_len = sizeof(req->observed);
		flag = REQ_BLOCK_OBSERVED;
	} else if (strcmp(key, "mode") == 0) {
		dst = req->mode;
		dst_len = sizeof(req->mode);
		flag = REQ_BLOCK_MODE;
	} else if (strcmp(key, "notes") == 0) {
		dst = req->notes;
		dst_len = sizeof(req->notes);
		flag = REQ_BLOCK_NOTES;
	} else {
		error_set(info, KN_COMPAT_REQ_ERR_UNKNOWN_KEY, line, key);
		return KN_COMPAT_REQ_ERR_UNKNOWN_KEY;
	}
	if (field_seen(seen, flag) != 0)
		goto duplicate;
	if (strlen(value) >= dst_len)
		goto invalid;
	(void)snprintf(dst, dst_len, "%s", value);
	return KN_COMPAT_REQ_OK;

duplicate:
	error_set(info, KN_COMPAT_REQ_ERR_DUPLICATE_KEY, line, key);
	return KN_COMPAT_REQ_ERR_DUPLICATE_KEY;
invalid:
	error_set(info, KN_COMPAT_REQ_ERR_INVALID_VALUE, line, key);
	return KN_COMPAT_REQ_ERR_INVALID_VALUE;
}

static enum kn_compat_req_error
status_parse(const char *value, enum kn_compat_req_status *status)
{
	if (strcmp(value, "planned") == 0)
		*status = KN_COMPAT_REQ_STATUS_PLANNED;
	else if (strcmp(value, "blocked") == 0)
		*status = KN_COMPAT_REQ_STATUS_BLOCKED;
	else if (strcmp(value, "needs-observation") == 0)
		*status = KN_COMPAT_REQ_STATUS_NEEDS_OBSERVATION;
	else if (strcmp(value, "ready-for-design") == 0)
		*status = KN_COMPAT_REQ_STATUS_READY_FOR_DESIGN;
	else if (strcmp(value, "ready-for-implementation") == 0)
		*status = KN_COMPAT_REQ_STATUS_READY_FOR_IMPLEMENTATION;
	else if (strcmp(value, "implemented-native") == 0)
		*status = KN_COMPAT_REQ_STATUS_IMPLEMENTED_NATIVE;
	else if (strcmp(value, "implemented-compatible") == 0)
		*status = KN_COMPAT_REQ_STATUS_IMPLEMENTED_COMPATIBLE;
	else if (strcmp(value, "out-of-scope") == 0)
		*status = KN_COMPAT_REQ_STATUS_OUT_OF_SCOPE;
	else
		return KN_COMPAT_REQ_ERR_INVALID_VALUE;
	return KN_COMPAT_REQ_OK;
}

static uint8_t
field_seen(uint32_t *seen, uint32_t flag)
{
	if ((*seen & flag) != 0)
		return 1;
	*seen |= flag;
	return 0;
}

static char *
trim(char *s)
{
	char *end;

	while (*s == ' ' || *s == '\t' || *s == '\r')
		s++;
	end = s + strlen(s);
	while (end > s &&
	    (end[-1] == ' ' || end[-1] == '\t' || end[-1] == '\r'))
		*--end = '\0';
	return s;
}

static uint8_t
unsafe_ref(const char *path)
{
	if (path == NULL || path[0] == '\0' || path[0] == '/')
		return 1;
	if (strstr(path, "..") != NULL || strchr(path, '\\') != NULL)
		return 1;
	return 0;
}
