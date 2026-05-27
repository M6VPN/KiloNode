/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/tools/kilonode-compat.c */

#include <sys/types.h>

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "kilonode/compat_capture.h"
#include "kilonode/compat_observe.h"
#include "kilonode/compat_process.h"
#include "kilonode/compat_report.h"
#include "kilonode/compat_replay.h"
#include "kilonode/compat_session.h"
#include "kilonode/compat_transcript.h"

#define COMPAT_DIR_MAX   128
#define COMPAT_PATH_MAX  512
#define COMPAT_REPORT_MAX 2048

struct compat_dir_entry {
	char path[COMPAT_PATH_MAX];
};

static int command_check_transcript(const char *);
static int command_check_observation(const char *);
static int command_compare_observations(const char *, const char *);
static int command_make_transcript(int, char *[]);
static int command_observe_process(int, char *[]);
static int command_observe_tcp(int, char *[]);
static int command_replay_dir(const char *);
static int command_replay_transcript(const char *);
static int command_report_transcript(const char *);
static int command_show_observation(const char *);
static int compare_entries(const void *, const void *);
static void date_today(char *, size_t);
static int has_transcript_suffix(const char *);
static enum kn_compat_observe_mode mode_from_text(const char *);
static const char *option_value(int, char *[], int *, const char *);
static int parse_observation_file(const char *, struct kn_compat_observation *);
static int parse_file(const char *, struct kn_compat_transcript *);
static void print_observation_error(const char *,
	const struct kn_compat_observe_error_info *);
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
	if (argc < 2) {
		usage();
		return 1;
	}

	if (strcmp(argv[1], "check-transcript") == 0 && argc == 3)
		return command_check_transcript(argv[2]);
	if (strcmp(argv[1], "check-observation") == 0 && argc == 3)
		return command_check_observation(argv[2]);
	if (strcmp(argv[1], "show-observation") == 0 && argc == 3)
		return command_show_observation(argv[2]);
	if (strcmp(argv[1], "compare-observations") == 0 && argc == 4)
		return command_compare_observations(argv[2], argv[3]);
	if (strcmp(argv[1], "make-transcript-from-observation") == 0)
		return command_make_transcript(argc, argv);
	if (strcmp(argv[1], "observe-process") == 0)
		return command_observe_process(argc, argv);
	if (strcmp(argv[1], "observe-tcp") == 0)
		return command_observe_tcp(argc, argv);
	if (strcmp(argv[1], "replay-transcript") == 0 && argc == 3)
		return command_replay_transcript(argv[2]);
	if (strcmp(argv[1], "replay-dir") == 0 && argc == 3)
		return command_replay_dir(argv[2]);
	if (strcmp(argv[1], "report-transcript") == 0 && argc == 3)
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
command_check_observation(const char *path)
{
	struct kn_compat_observation observation;

	if (parse_observation_file(path, &observation) != 0)
		return 1;

	printf("OK observation=%s method=%s mode=%s\n", observation.name,
	    kn_compat_observe_method_name(observation.method),
	    kn_compat_observe_mode_name(observation.mode));
	return 0;
}

static int
command_compare_observations(const char *left_path, const char *right_path)
{
	struct kn_compat_observation left;
	struct kn_compat_observation right;
	char report[COMPAT_REPORT_MAX];

	if (parse_observation_file(left_path, &left) != 0 ||
	    parse_observation_file(right_path, &right) != 0)
		return 1;
	if (kn_compat_capture_compare(&left, &right, report,
	    sizeof(report)) != KN_COMPAT_CAPTURE_OK) {
		fprintf(stderr, "ERR compare-observations\n");
		return 1;
	}
	printf("%s\n", report);
	return 0;
}

static int
command_make_transcript(int argc, char *argv[])
{
	struct kn_compat_observation observation;
	char transcript[KN_COMPAT_CAPTURE_TEXT_MAX];
	const char *output;

	if (argc != 5 || strcmp(argv[3], "--output") != 0) {
		usage();
		return 1;
	}
	output = argv[4];
	if (parse_observation_file(argv[2], &observation) != 0)
		return 1;
	if (kn_compat_capture_transcript_candidate(&observation, transcript,
	    sizeof(transcript)) != KN_COMPAT_CAPTURE_OK) {
		fprintf(stderr, "ERR transcript-candidate\n");
		return 1;
	}
	if (kn_compat_capture_write_text(transcript, output) !=
	    KN_COMPAT_CAPTURE_OK) {
		fprintf(stderr, "ERR write-output\n");
		return 1;
	}
	printf("OK transcript-candidate=%s\n", output);
	return 0;
}

static int
command_observe_process(int argc, char *argv[])
{
	struct kn_compat_observation observation;
	struct kn_compat_process_result result;
	const char *binary;
	const char *config;
	const char *name;
	const char *mode;
	const char *input;
	const char *output;
	unsigned int timeout;
	int i;
	int needed;

	binary = config = name = mode = input = output = NULL;
	timeout = 10;
	for (i = 2; i < argc; i++) {
		if (strcmp(argv[i], "--binary") == 0)
			binary = option_value(argc, argv, &i, "--binary");
		else if (strcmp(argv[i], "--config") == 0)
			config = option_value(argc, argv, &i, "--config");
		else if (strcmp(argv[i], "--name") == 0)
			name = option_value(argc, argv, &i, "--name");
		else if (strcmp(argv[i], "--mode") == 0)
			mode = option_value(argc, argv, &i, "--mode");
		else if (strcmp(argv[i], "--input") == 0)
			input = option_value(argc, argv, &i, "--input");
		else if (strcmp(argv[i], "--output") == 0)
			output = option_value(argc, argv, &i, "--output");
		else if (strcmp(argv[i], "--timeout") == 0)
			timeout = (unsigned int)strtoul(option_value(argc, argv,
			    &i, "--timeout"), NULL, 10);
		else
			return 1;
	}
	if (binary == NULL || config == NULL || name == NULL || mode == NULL ||
	    input == NULL || output == NULL || timeout == 0)
		return 1;

	if (kn_compat_process_run(binary, config, input, timeout, &result) !=
	    KN_COMPAT_PROCESS_OK) {
		fprintf(stderr, "ERR observe-process\n");
		return 1;
	}

	kn_compat_observation_clear(&observation);
	(void)snprintf(observation.name, sizeof(observation.name), "%s", name);
	(void)snprintf(observation.subject, sizeof(observation.subject),
	    "black-box");
	observation.method = KN_COMPAT_OBSERVE_METHOD_PROCESS;
	date_today(observation.date, sizeof(observation.date));
	(void)snprintf(observation.observer, sizeof(observation.observer),
	    "M6VPN");
	(void)snprintf(observation.binary_path, sizeof(observation.binary_path),
	    "%s", binary);
	(void)snprintf(observation.config_path, sizeof(observation.config_path),
	    "%s", config);
	observation.mode = mode_from_text(mode);
	(void)snprintf(observation.input, sizeof(observation.input), "%s",
	    input);
	needed = snprintf(observation.observed,
	    sizeof(observation.observed), "%s%s", result.stdout_text,
	    result.stderr_text);
	if (needed < 0 || (size_t)needed >= sizeof(observation.observed))
		return 1;
	observation.observed_len = (size_t)needed;
	if (kn_compat_capture_write_observation(&observation, output) !=
	    KN_COMPAT_CAPTURE_OK)
		return 1;
	printf("OK observation=%s\n", output);
	return 0;
}

static int
command_observe_tcp(int argc, char *argv[])
{
	struct kn_compat_observation observation;
	struct kn_compat_session_result result;
	const char *host;
	const char *port;
	const char *name;
	const char *mode;
	const char *input;
	const char *output;
	unsigned int timeout;
	int i;
	int needed;

	host = port = name = mode = input = output = NULL;
	timeout = 10;
	for (i = 2; i < argc; i++) {
		if (strcmp(argv[i], "--host") == 0)
			host = option_value(argc, argv, &i, "--host");
		else if (strcmp(argv[i], "--port") == 0)
			port = option_value(argc, argv, &i, "--port");
		else if (strcmp(argv[i], "--name") == 0)
			name = option_value(argc, argv, &i, "--name");
		else if (strcmp(argv[i], "--mode") == 0)
			mode = option_value(argc, argv, &i, "--mode");
		else if (strcmp(argv[i], "--input") == 0)
			input = option_value(argc, argv, &i, "--input");
		else if (strcmp(argv[i], "--output") == 0)
			output = option_value(argc, argv, &i, "--output");
		else if (strcmp(argv[i], "--timeout") == 0)
			timeout = (unsigned int)strtoul(option_value(argc, argv,
			    &i, "--timeout"), NULL, 10);
		else
			return 1;
	}
	if (host == NULL || port == NULL || name == NULL || mode == NULL ||
	    input == NULL || output == NULL || timeout == 0)
		return 1;

	if (kn_compat_session_tcp_line(host, port, input, timeout,
	    &result) != KN_COMPAT_SESSION_OK) {
		fprintf(stderr, "ERR observe-tcp\n");
		return 1;
	}

	kn_compat_observation_clear(&observation);
	(void)snprintf(observation.name, sizeof(observation.name), "%s", name);
	(void)snprintf(observation.subject, sizeof(observation.subject),
	    "black-box");
	observation.method = KN_COMPAT_OBSERVE_METHOD_TCP_LINE;
	date_today(observation.date, sizeof(observation.date));
	(void)snprintf(observation.observer, sizeof(observation.observer),
	    "M6VPN");
	needed = snprintf(observation.connect_target,
	    sizeof(observation.connect_target), "%s:%s", host, port);
	if (needed < 0 ||
	    (size_t)needed >= sizeof(observation.connect_target))
		return 1;
	observation.mode = mode_from_text(mode);
	(void)snprintf(observation.input, sizeof(observation.input), "%s",
	    input);
	(void)snprintf(observation.observed, sizeof(observation.observed),
	    "%s", result.response);
	observation.observed_len = strlen(observation.observed);
	if (kn_compat_capture_write_observation(&observation, output) !=
	    KN_COMPAT_CAPTURE_OK)
		return 1;
	printf("OK observation=%s\n", output);
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
command_show_observation(const char *path)
{
	struct kn_compat_observation observation;
	char report[COMPAT_REPORT_MAX];

	if (parse_observation_file(path, &observation) != 0)
		return 1;
	if (kn_compat_observation_report(&observation, report,
	    sizeof(report)) != KN_COMPAT_OBSERVE_OK) {
		fprintf(stderr, "ERR observation-report\n");
		return 1;
	}
	printf("%s\n", report);
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

static void
date_today(char *buf, size_t bufsiz)
{
	time_t now;
	struct tm *tm;

	now = time(NULL);
	tm = gmtime(&now);
	if (tm == NULL ||
	    strftime(buf, bufsiz, "%Y-%m-%d", tm) == 0)
		(void)snprintf(buf, bufsiz, "unknown");
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

static enum kn_compat_observe_mode
mode_from_text(const char *text)
{
	if (text != NULL && strcmp(text, "tcp-line") == 0)
		return KN_COMPAT_OBSERVE_MODE_TCP_LINE;
	if (text != NULL && strcmp(text, "node-shell") == 0)
		return KN_COMPAT_OBSERVE_MODE_NODE_SHELL;
	if (text != NULL && strcmp(text, "bbs-shell") == 0)
		return KN_COMPAT_OBSERVE_MODE_BBS_SHELL;

	return KN_COMPAT_OBSERVE_MODE_PROCESS_OUTPUT;
}

static const char *
option_value(int argc, char *argv[], int *index, const char *name)
{
	if (*index + 1 >= argc) {
		fprintf(stderr, "ERR missing-value option=%s\n", name);
		return "";
	}
	(*index)++;
	return argv[*index];
}

static int
parse_observation_file(const char *path,
	struct kn_compat_observation *observation)
{
	struct kn_compat_observe_error_info error;
	enum kn_compat_observe_error rc;

	memset(&error, 0, sizeof(error));
	rc = kn_compat_observation_parse_file(path, observation, &error);
	if (rc != KN_COMPAT_OBSERVE_OK) {
		print_observation_error(path, &error);
		return 1;
	}

	return 0;
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
print_observation_error(const char *path,
	const struct kn_compat_observe_error_info *error)
{
	fprintf(stderr, "ERR observation=%s error=%s line=%llu detail=%s\n",
	    path == NULL ? "-" : path,
	    kn_compat_observe_error_name(error->error),
	    (unsigned long long)error->line,
	    error->message[0] == '\0' ? "-" : error->message);
}

static void
usage(void)
{
	fprintf(stderr,
	    "usage: kilonode-compat check-transcript PATH\n"
	    "       kilonode-compat check-observation PATH\n"
	    "       kilonode-compat show-observation PATH\n"
	    "       kilonode-compat replay-transcript PATH\n"
	    "       kilonode-compat replay-dir PATH\n"
	    "       kilonode-compat report-transcript PATH\n"
	    "       kilonode-compat make-transcript-from-observation PATH --output PATH\n"
	    "       kilonode-compat compare-observations A B\n"
	    "       kilonode-compat observe-process --binary PATH --config PATH --name NAME --mode MODE --input TEXT --output PATH [--timeout SECONDS]\n"
	    "       kilonode-compat observe-tcp --host HOST --port PORT --name NAME --mode MODE --input TEXT --output PATH [--timeout SECONDS]\n");
}
