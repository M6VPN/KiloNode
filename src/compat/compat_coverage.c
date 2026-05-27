/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/compat/compat_coverage.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "kilonode/compat_coverage.h"
#include "kilonode/compat_observe.h"
#include "kilonode/compat_transcript.h"
#include "kilonode/rf_command.h"

static void apply_command(struct kn_compat_coverage *, const char *,
	uint8_t, uint8_t);
static struct kn_compat_coverage_entry *find_entry(
	struct kn_compat_coverage *, const char *);
static void init_defaults(struct kn_compat_coverage *);
static const char *input_command(const char *, char *, size_t);

void
kn_compat_coverage_clear(struct kn_compat_coverage *coverage)
{
	if (coverage == NULL)
		return;
	memset(coverage, 0, sizeof(*coverage));
}

const char *
kn_compat_coverage_error_name(enum kn_compat_coverage_error error)
{
	switch (error) {
	case KN_COMPAT_COVERAGE_OK:
		return "ok";
	case KN_COMPAT_COVERAGE_ERR_INVALID_ARGUMENT:
		return "invalid-argument";
	case KN_COMPAT_COVERAGE_ERR_BUFFER:
		return "buffer";
	case KN_COMPAT_COVERAGE_ERR_REFERENCE:
		return "reference";
	}

	return "unknown";
}

const char *
kn_compat_coverage_status_name(enum kn_compat_coverage_status status)
{
	switch (status) {
	case KN_COMPAT_COVERAGE_NOT_STARTED:
		return "not-started";
	case KN_COMPAT_COVERAGE_SYNTHETIC:
		return "synthetic";
	case KN_COMPAT_COVERAGE_OBSERVED:
		return "observed";
	case KN_COMPAT_COVERAGE_TRANSCRIPT_CANDIDATE:
		return "transcript-candidate";
	case KN_COMPAT_COVERAGE_IMPLEMENTED_KILONODE_NATIVE:
		return "implemented-kilonode-native";
	case KN_COMPAT_COVERAGE_COMPATIBILITY_PLANNED:
		return "compatibility-planned";
	case KN_COMPAT_COVERAGE_OUT_OF_SCOPE_THIS_MILESTONE:
		return "out-of-scope-this-milestone";
	}

	return "unknown";
}

enum kn_compat_coverage_error
kn_compat_coverage_from_pack(const struct kn_compat_observation_pack *pack,
	struct kn_compat_coverage *coverage)
{
	struct kn_compat_observation observation;
	struct kn_compat_transcript transcript;
	char path[KN_COMPAT_PACK_FIELD_MAX * 2];
	char command[32];
	size_t i;

	if (pack == NULL || coverage == NULL)
		return KN_COMPAT_COVERAGE_ERR_INVALID_ARGUMENT;

	init_defaults(coverage);
	for (i = 0; i < pack->fixture_count; i++) {
		if (kn_compat_observation_pack_join_path(pack,
		    pack->fixtures[i].path, path, sizeof(path)) !=
		    KN_COMPAT_PACK_OK)
			return KN_COMPAT_COVERAGE_ERR_REFERENCE;
		if (kn_compat_observation_parse_file(path, &observation,
		    NULL) != KN_COMPAT_OBSERVE_OK)
			return KN_COMPAT_COVERAGE_ERR_REFERENCE;
		apply_command(coverage,
		    input_command(observation.input, command,
		    sizeof(command)),
		    strcmp(observation.source, "synthetic-placeholder") == 0 ?
		    1 : 0,
		    strcmp(observation.source, "manual-black-box") == 0 ?
		    1 : 0);
	}
	for (i = 0; i < pack->transcript_count; i++) {
		struct kn_compat_coverage_entry *entry;

		if (kn_compat_observation_pack_join_path(pack,
		    pack->transcripts[i].path, path, sizeof(path)) !=
		    KN_COMPAT_PACK_OK)
			return KN_COMPAT_COVERAGE_ERR_REFERENCE;
		if (kn_compat_transcript_parse_file(path, &transcript,
		    NULL) != KN_COMPAT_TRANSCRIPT_OK)
			return KN_COMPAT_COVERAGE_ERR_REFERENCE;
		apply_command(coverage,
		    kn_rf_command_name_string(transcript.expect_command), 1, 0);
		entry = find_entry(coverage,
		    kn_rf_command_name_string(transcript.expect_command));
		if (entry != NULL) {
			entry->transcript_candidate = 1;
			entry->replayable = 1;
			entry->status =
			    KN_COMPAT_COVERAGE_TRANSCRIPT_CANDIDATE;
		}
	}

	return KN_COMPAT_COVERAGE_OK;
}

enum kn_compat_coverage_error
kn_compat_coverage_report(const struct kn_compat_observation_pack *pack,
	const struct kn_compat_coverage *coverage, char *buf, size_t bufsiz)
{
	size_t off;
	size_t i;
	size_t synthetic;
	size_t observed;
	size_t transcripts;
	int needed;

	if (pack == NULL || coverage == NULL || buf == NULL || bufsiz == 0)
		return KN_COMPAT_COVERAGE_ERR_INVALID_ARGUMENT;

	synthetic = observed = transcripts = 0;
	for (i = 0; i < coverage->count; i++) {
		if (coverage->entries[i].synthetic != 0)
			synthetic++;
		if (coverage->entries[i].manual_black_box != 0)
			observed++;
		if (coverage->entries[i].transcript_candidate != 0)
			transcripts++;
	}
	needed = snprintf(buf, bufsiz,
	    "COVERAGE pack=%s commands=%llu synthetic=%llu observed=%llu "
	    "transcripts=%llu\n",
	    pack->name, (unsigned long long)coverage->count,
	    (unsigned long long)synthetic, (unsigned long long)observed,
	    (unsigned long long)transcripts);
	if (needed < 0 || (size_t)needed >= bufsiz)
		return KN_COMPAT_COVERAGE_ERR_BUFFER;
	off = (size_t)needed;

	for (i = 0; i < coverage->count; i++) {
		needed = snprintf(buf + off, bufsiz - off,
		    "COVERAGE COMMAND name=%s status=%s synthetic=%s "
		    "manual=%s transcript=%s replayable=%s notes=\"%s\"\n",
		    coverage->entries[i].command,
		    kn_compat_coverage_status_name(coverage->entries[i].status),
		    coverage->entries[i].synthetic != 0 ? "true" : "false",
		    coverage->entries[i].manual_black_box != 0 ? "true" :
		    "false",
		    coverage->entries[i].transcript_candidate != 0 ? "true" :
		    "false",
		    coverage->entries[i].replayable != 0 ? "true" : "false",
		    coverage->entries[i].notes);
		if (needed < 0 || (size_t)needed >= bufsiz - off)
			return KN_COMPAT_COVERAGE_ERR_BUFFER;
		off += (size_t)needed;
	}

	return KN_COMPAT_COVERAGE_OK;
}

static void
apply_command(struct kn_compat_coverage *coverage, const char *command,
	uint8_t synthetic, uint8_t manual)
{
	struct kn_compat_coverage_entry *entry;

	entry = find_entry(coverage, command);
	if (entry == NULL)
		entry = find_entry(coverage, "UNKNOWN");
	if (entry == NULL)
		return;
	entry->observed = 1;
	if (synthetic != 0)
		entry->synthetic = 1;
	if (manual != 0)
		entry->manual_black_box = 1;
	if (manual != 0)
		entry->status = KN_COMPAT_COVERAGE_OBSERVED;
	else if (entry->transcript_candidate != 0)
		entry->status = KN_COMPAT_COVERAGE_TRANSCRIPT_CANDIDATE;
	else
		entry->status = KN_COMPAT_COVERAGE_SYNTHETIC;
}

static struct kn_compat_coverage_entry *
find_entry(struct kn_compat_coverage *coverage, const char *command)
{
	size_t i;

	if (coverage == NULL || command == NULL)
		return NULL;
	for (i = 0; i < coverage->count; i++) {
		if (strcmp(coverage->entries[i].command, command) == 0)
			return &coverage->entries[i];
	}

	return NULL;
}

static void
init_defaults(struct kn_compat_coverage *coverage)
{
	static const char *commands[KN_COMPAT_COVERAGE_COMMANDS] = {
		"HELP", "INFO", "PORTS", "USERS", "HEARD", "STATS",
		"BBS", "BYE", "UNKNOWN", "CONNECT", "NODES", "ROUTES"
	};
	size_t i;

	kn_compat_coverage_clear(coverage);
	coverage->count = KN_COMPAT_COVERAGE_COMMANDS;
	for (i = 0; i < coverage->count; i++) {
		(void)snprintf(coverage->entries[i].command,
		    sizeof(coverage->entries[i].command), "%s", commands[i]);
		coverage->entries[i].status =
		    KN_COMPAT_COVERAGE_NOT_STARTED;
		(void)snprintf(coverage->entries[i].notes,
		    sizeof(coverage->entries[i].notes), "compatibility deferred");
	}
	find_entry(coverage, "HELP")->status =
	    KN_COMPAT_COVERAGE_IMPLEMENTED_KILONODE_NATIVE;
	find_entry(coverage, "INFO")->status =
	    KN_COMPAT_COVERAGE_IMPLEMENTED_KILONODE_NATIVE;
	find_entry(coverage, "PORTS")->status =
	    KN_COMPAT_COVERAGE_IMPLEMENTED_KILONODE_NATIVE;
	find_entry(coverage, "HEARD")->status =
	    KN_COMPAT_COVERAGE_IMPLEMENTED_KILONODE_NATIVE;
	find_entry(coverage, "STATS")->status =
	    KN_COMPAT_COVERAGE_IMPLEMENTED_KILONODE_NATIVE;
	find_entry(coverage, "CONNECT")->status =
	    KN_COMPAT_COVERAGE_COMPATIBILITY_PLANNED;
	find_entry(coverage, "NODES")->status =
	    KN_COMPAT_COVERAGE_COMPATIBILITY_PLANNED;
	find_entry(coverage, "ROUTES")->status =
	    KN_COMPAT_COVERAGE_COMPATIBILITY_PLANNED;
	find_entry(coverage, "BBS")->status =
	    KN_COMPAT_COVERAGE_COMPATIBILITY_PLANNED;
}

static const char *
input_command(const char *input, char *buf, size_t bufsiz)
{
	size_t i;

	if (bufsiz == 0)
		return "";
	buf[0] = '\0';
	if (input == NULL || input[0] == '\0')
		return "UNKNOWN";
	for (i = 0; input[i] != '\0' && i + 1 < bufsiz; i++) {
		if (input[i] == ' ' || input[i] == '\t' ||
		    input[i] == '\r' || input[i] == '\n')
			break;
		if (input[i] >= 'a' && input[i] <= 'z')
			buf[i] = (char)(input[i] - 'a' + 'A');
		else
			buf[i] = input[i];
	}
	buf[i] = '\0';
	return buf[0] == '\0' ? "UNKNOWN" : buf;
}
