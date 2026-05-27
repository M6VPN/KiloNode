/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/tools/kilonode-compat.c */

#include <sys/types.h>

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kilonode/compat_report.h"
#include "kilonode/compat_replay.h"
#include "kilonode/compat_transcript.h"

#define COMPAT_DIR_MAX   128
#define COMPAT_PATH_MAX  512
#define COMPAT_REPORT_MAX 2048

struct compat_dir_entry {
	char path[COMPAT_PATH_MAX];
};

static int command_check_transcript(const char *);
static int command_replay_dir(const char *);
static int command_replay_transcript(const char *);
static int command_report_transcript(const char *);
static int compare_entries(const void *, const void *);
static int has_transcript_suffix(const char *);
static int parse_file(const char *, struct kn_compat_transcript *);
static void print_parse_error(const char *,
	const struct kn_compat_transcript_error_info *);
static void usage(void);

int
main(int argc, char *argv[])
{
	if (argc == 2 && strcmp(argv[1], "--help") == 0) {
		usage();
		return 0;
	}
	if (argc != 3) {
		usage();
		return 1;
	}

	if (strcmp(argv[1], "check-transcript") == 0)
		return command_check_transcript(argv[2]);
	if (strcmp(argv[1], "replay-transcript") == 0)
		return command_replay_transcript(argv[2]);
	if (strcmp(argv[1], "replay-dir") == 0)
		return command_replay_dir(argv[2]);
	if (strcmp(argv[1], "report-transcript") == 0)
		return command_report_transcript(argv[2]);

	usage();
	return 1;
}

static int
command_check_transcript(const char *path)
{
	struct kn_compat_transcript transcript;

	if (parse_file(path, &transcript) != 0)
		return 1;

	printf("OK transcript=%s mode=%s\n", transcript.name,
	    kn_compat_mode_name(transcript.mode));
	return 0;
}

static int
command_replay_dir(const char *path)
{
	struct compat_dir_entry entries[COMPAT_DIR_MAX];
	DIR *dir;
	struct dirent *entry;
	size_t count;
	size_t i;
	int failed;
	int needed;

	dir = opendir(path);
	if (dir == NULL) {
		fprintf(stderr, "ERR open-dir path=%s\n", path);
		return 1;
	}

	count = 0;
	while ((entry = readdir(dir)) != NULL) {
		if (has_transcript_suffix(entry->d_name) == 0)
			continue;
		if (count >= COMPAT_DIR_MAX) {
			closedir(dir);
			fprintf(stderr, "ERR too-many-transcripts\n");
			return 1;
		}
		needed = snprintf(entries[count].path,
		    sizeof(entries[count].path), "%s/%s", path,
		    entry->d_name);
		if (needed < 0 ||
		    (size_t)needed >= sizeof(entries[count].path)) {
			closedir(dir);
			fprintf(stderr, "ERR path-too-long\n");
			return 1;
		}
		count++;
	}
	closedir(dir);

	qsort(entries, count, sizeof(entries[0]), compare_entries);

	failed = 0;
	for (i = 0; i < count; i++) {
		if (command_replay_transcript(entries[i].path) != 0)
			failed = 1;
	}

	if (count == 0) {
		fprintf(stderr, "ERR no-transcripts\n");
		return 1;
	}

	return failed;
}

static int
command_replay_transcript(const char *path)
{
	struct kn_compat_transcript transcript;
	struct kn_compat_replay_result result;
	char report[COMPAT_REPORT_MAX];
	enum kn_compat_replay_error replay_rc;

	if (parse_file(path, &transcript) != 0)
		return 1;

	replay_rc = kn_compat_replay_transcript(&transcript, &result);
	if (kn_compat_report_format_text(&result, report, sizeof(report)) !=
	    KN_COMPAT_REPORT_OK) {
		fprintf(stderr, "ERR report-format\n");
		return 1;
	}
	printf("%s", report);

	return replay_rc == KN_COMPAT_REPLAY_OK ? 0 : 1;
}

static int
command_report_transcript(const char *path)
{
	struct kn_compat_transcript transcript;
	char report[COMPAT_REPORT_MAX];

	if (parse_file(path, &transcript) != 0)
		return 1;
	if (kn_compat_transcript_report(&transcript, report,
	    sizeof(report)) != KN_COMPAT_TRANSCRIPT_OK) {
		fprintf(stderr, "ERR transcript-report\n");
		return 1;
	}

	printf("%s", report);
	return 0;
}

static int
compare_entries(const void *left, const void *right)
{
	const struct compat_dir_entry *a;
	const struct compat_dir_entry *b;

	a = left;
	b = right;
	return strcmp(a->path, b->path);
}

static int
has_transcript_suffix(const char *name)
{
	const char suffix[] = ".transcript";
	size_t name_len;
	size_t suffix_len;

	if (name == NULL)
		return 0;
	name_len = strlen(name);
	suffix_len = strlen(suffix);
	if (name_len < suffix_len)
		return 0;

	return strcmp(name + name_len - suffix_len, suffix) == 0 ? 1 : 0;
}

static int
parse_file(const char *path, struct kn_compat_transcript *transcript)
{
	struct kn_compat_transcript_error_info error;
	enum kn_compat_transcript_error rc;

	memset(&error, 0, sizeof(error));
	rc = kn_compat_transcript_parse_file(path, transcript, &error);
	if (rc != KN_COMPAT_TRANSCRIPT_OK) {
		print_parse_error(path, &error);
		return 1;
	}

	return 0;
}

static void
print_parse_error(const char *path,
	const struct kn_compat_transcript_error_info *error)
{
	fprintf(stderr, "ERR transcript=%s error=%s line=%llu detail=%s\n",
	    path == NULL ? "-" : path,
	    kn_compat_transcript_error_name(error->error),
	    (unsigned long long)error->line,
	    error->message[0] == '\0' ? "-" : error->message);
}

static void
usage(void)
{
	fprintf(stderr,
	    "usage: kilonode-compat check-transcript PATH\n"
	    "       kilonode-compat replay-transcript PATH\n"
	    "       kilonode-compat replay-dir PATH\n"
	    "       kilonode-compat report-transcript PATH\n");
}
