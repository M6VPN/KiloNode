/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/tools/kilonode-compat.c */

#include <sys/types.h>

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "kilonode/ax25_timer_replay.h"
#include "kilonode/ax25_timer_replay_report.h"
#include "kilonode/bench_ax25_diag_replay.h"
#include "kilonode/bench_replay.h"
#include "kilonode/bench_replay_report.h"
#include "kilonode/compat_bench_pack.h"
#include "kilonode/compat_capture.h"
#include "kilonode/compat_capture_report.h"
#include "kilonode/compat_axip_capture.h"
#include "kilonode/compat_command_profile.h"
#include "kilonode/compat_coverage.h"
#include "kilonode/compat_kiss_capture.h"
#include "kilonode/compat_observe.h"
#include "kilonode/compat_observation_pack.h"
#include "kilonode/compat_packet_capture.h"
#include "kilonode/compat_process.h"
#include "kilonode/compat_requirements.h"
#include "kilonode/compat_report.h"
#include "kilonode/compat_replay.h"
#include "kilonode/compat_risk.h"
#include "kilonode/compat_session.h"
#include "kilonode/compat_transcript.h"
#include "kilonode/manual_capture_index.h"
#include "kilonode/manual_capture_replay.h"
#include "kilonode/manual_capture_report.h"
#include "kilonode/manual_capture_workspace.h"

#define COMPAT_DIR_MAX   128
#define COMPAT_PATH_MAX  512
#define COMPAT_REPORT_MAX 2048

struct compat_dir_entry {
	char path[COMPAT_PATH_MAX];
};

static int command_check_transcript(const char *);
static int command_check_ax25_timer_replay(const char *);
static int command_check_bench_pack(const char *);
static int command_check_bench_expected(const char *);
static int command_check_observation(const char *);
static int command_check_capture(const char *);
static int command_check_command_profiles(const char *);
static int command_check_pack(const char *);
static int command_check_requirements(const char *);
static int command_check_risk_register(const char *);
static int command_command_profile_report(const char *);
static int command_compare_observations(const char *, const char *);
static int command_compare_pack_observations(const char *);
static int command_capture_report(const char *);
static int command_bench_coverage(const char *);
static int command_bench_diagnostics_report(const char *);
static int command_bench_pack_report(const char *);
static int command_capture_to_transcript(int, char *[]);
static int command_decode_capture(const char *);
static int command_generate_node_plan(int, char *[]);
static int command_make_transcript(int, char *[]);
static int command_manual_import(int, char *[]);
static int command_manual_list(int, char *[]);
static int command_manual_report(int, char *[]);
static int command_manual_replay(int, char *[]);
static int command_manual_replay_all(int, char *[]);
static int command_manual_summary(int, char *[]);
static int command_manual_validate(int, char *[]);
static int command_manual_workspace_check(const char *);
static int command_manual_workspace_init(const char *);
static int command_observe_process(int, char *[]);
static int command_observe_tcp(int, char *[]);
static int command_replay_dir(const char *);
static int command_replay_ax25_timer_dir(const char *);
static int command_replay_ax25_timer(const char *);
static int command_replay_bench_pack(const char *);
static int command_replay_bench_diagnostics(const char *);
static int command_replay_bench_diagnostics_dir(const char *);
static int command_replay_pack(const char *);
static int command_replay_capture(const char *);
static int command_replay_capture_dir(const char *);
static int command_replay_transcript(const char *);
static int command_report_transcript(const char *);
static int command_ax25_timer_replay_report(const char *);
static int command_requirements_coverage(const char *, const char *);
static int command_requirements_report(const char *);
static int command_risk_report(const char *);
static int command_pack_coverage(const char *);
static int command_pack_report(const char *);
static int command_show_pack(const char *);
static int command_show_observation(const char *);
static int compare_entries(const void *, const void *);
static void date_today(char *, size_t);
static int has_transcript_suffix(const char *);
static int has_capture_suffix(const char *);
static int has_replay_suffix(const char *);
static int decode_capture_file(const char *, struct kn_compat_packet_capture *,
	struct kn_compat_packet_decode *);
static enum kn_compat_observe_mode mode_from_text(const char *);
static const char *option_value(int, char *[], int *, const char *);
static int parse_observation_file(const char *, struct kn_compat_observation *);
static int parse_pack_file(const char *,
	struct kn_compat_observation_pack *);
static int parse_bench_file(const char *, struct kn_compat_bench_pack *);
static int parse_bench_expected_file(const char *,
	struct kn_bench_expected_file *);
static int parse_capture_file(const char *, struct kn_compat_packet_capture *);
static int parse_command_profile_file(const char *,
	struct kn_compat_command_profiles *);
static int parse_file(const char *, struct kn_compat_transcript *);
static int parse_requirements_file(const char *,
	struct kn_compat_requirements *);
static void print_observation_error(const char *,
	const struct kn_compat_observe_error_info *);
static void print_capture_error(const char *,
	const struct kn_compat_packet_capture_error_info *);
static void print_profile_error(const char *,
	const struct kn_compat_profile_error_info *);
static void print_pack_error(const char *,
	const struct kn_compat_pack_error_info *);
static void print_bench_error(const char *,
	const struct kn_compat_bench_error_info *);
static void print_bench_replay_error(const char *,
	const struct kn_bench_replay_error_info *);
static void print_manual_error(const char *,
	const struct kn_manual_capture_error_info *);
static void print_parse_error(const char *,
	const struct kn_compat_transcript_error_info *);
static void print_requirements_error(const char *,
	const struct kn_compat_req_error_info *);
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
	if (strcmp(argv[1], "check-ax25-timer-replay") == 0 && argc == 3)
		return command_check_ax25_timer_replay(argv[2]);
	if (strcmp(argv[1], "check-bench-pack") == 0 && argc == 3)
		return command_check_bench_pack(argv[2]);
	if (strcmp(argv[1], "check-bench-expected") == 0 && argc == 3)
		return command_check_bench_expected(argv[2]);
	if (strcmp(argv[1], "check-observation") == 0 && argc == 3)
		return command_check_observation(argv[2]);
	if (strcmp(argv[1], "check-capture") == 0 && argc == 3)
		return command_check_capture(argv[2]);
	if (strcmp(argv[1], "check-command-profiles") == 0 && argc == 3)
		return command_check_command_profiles(argv[2]);
	if (strcmp(argv[1], "check-pack") == 0 && argc == 3)
		return command_check_pack(argv[2]);
	if (strcmp(argv[1], "check-requirements") == 0 && argc == 3)
		return command_check_requirements(argv[2]);
	if (strcmp(argv[1], "check-risk-register") == 0 && argc == 3)
		return command_check_risk_register(argv[2]);
	if (strcmp(argv[1], "command-profile-report") == 0 && argc == 3)
		return command_command_profile_report(argv[2]);
	if (strcmp(argv[1], "decode-capture") == 0 && argc == 3)
		return command_decode_capture(argv[2]);
	if (strcmp(argv[1], "generate-node-plan") == 0)
		return command_generate_node_plan(argc, argv);
	if (strcmp(argv[1], "capture-report") == 0 && argc == 3)
		return command_capture_report(argv[2]);
	if (strcmp(argv[1], "bench-pack-report") == 0 && argc == 3)
		return command_bench_pack_report(argv[2]);
	if (strcmp(argv[1], "bench-diagnostics-report") == 0 && argc == 3)
		return command_bench_diagnostics_report(argv[2]);
	if (strcmp(argv[1], "bench-coverage") == 0 && argc == 3)
		return command_bench_coverage(argv[2]);
	if (strcmp(argv[1], "capture-to-transcript") == 0)
		return command_capture_to_transcript(argc, argv);
	if (strcmp(argv[1], "show-observation") == 0 && argc == 3)
		return command_show_observation(argv[2]);
	if (strcmp(argv[1], "show-pack") == 0 && argc == 3)
		return command_show_pack(argv[2]);
	if (strcmp(argv[1], "compare-observations") == 0 && argc == 4)
		return command_compare_observations(argv[2], argv[3]);
	if (strcmp(argv[1], "compare-pack-observations") == 0 && argc == 3)
		return command_compare_pack_observations(argv[2]);
	if (strcmp(argv[1], "pack-report") == 0 && argc == 3)
		return command_pack_report(argv[2]);
	if (strcmp(argv[1], "pack-coverage") == 0 && argc == 3)
		return command_pack_coverage(argv[2]);
	if (strcmp(argv[1], "make-transcript-from-observation") == 0)
		return command_make_transcript(argc, argv);
	if (strcmp(argv[1], "manual-workspace-init") == 0 && argc == 3)
		return command_manual_workspace_init(argv[2]);
	if (strcmp(argv[1], "manual-workspace-check") == 0 && argc == 3)
		return command_manual_workspace_check(argv[2]);
	if (strcmp(argv[1], "manual-import") == 0)
		return command_manual_import(argc, argv);
	if (strcmp(argv[1], "manual-list") == 0)
		return command_manual_list(argc, argv);
	if (strcmp(argv[1], "manual-validate") == 0)
		return command_manual_validate(argc, argv);
	if (strcmp(argv[1], "manual-replay") == 0)
		return command_manual_replay(argc, argv);
	if (strcmp(argv[1], "manual-replay-all") == 0)
		return command_manual_replay_all(argc, argv);
	if (strcmp(argv[1], "manual-report") == 0)
		return command_manual_report(argc, argv);
	if (strcmp(argv[1], "manual-summary") == 0)
		return command_manual_summary(argc, argv);
	if (strcmp(argv[1], "observe-process") == 0)
		return command_observe_process(argc, argv);
	if (strcmp(argv[1], "observe-tcp") == 0)
		return command_observe_tcp(argc, argv);
	if (strcmp(argv[1], "replay-transcript") == 0 && argc == 3)
		return command_replay_transcript(argv[2]);
	if (strcmp(argv[1], "run-ax25-timer-replay") == 0 && argc == 3)
		return command_replay_ax25_timer(argv[2]);
	if (strcmp(argv[1], "run-ax25-timer-replay-dir") == 0 && argc == 3)
		return command_replay_ax25_timer_dir(argv[2]);
	if (strcmp(argv[1], "replay-capture") == 0 && argc == 3)
		return command_replay_capture(argv[2]);
	if (strcmp(argv[1], "replay-capture-dir") == 0 && argc == 3)
		return command_replay_capture_dir(argv[2]);
	if (strcmp(argv[1], "replay-bench-pack") == 0 && argc == 3)
		return command_replay_bench_pack(argv[2]);
	if (strcmp(argv[1], "replay-bench-diagnostics") == 0 && argc == 3)
		return command_replay_bench_diagnostics(argv[2]);
	if (strcmp(argv[1], "replay-bench-diagnostics-dir") == 0 &&
	    argc == 3)
		return command_replay_bench_diagnostics_dir(argv[2]);
	if (strcmp(argv[1], "replay-pack") == 0 && argc == 3)
		return command_replay_pack(argv[2]);
	if (strcmp(argv[1], "replay-dir") == 0 && argc == 3)
		return command_replay_dir(argv[2]);
	if (strcmp(argv[1], "report-transcript") == 0 && argc == 3)
		return command_report_transcript(argv[2]);
	if (strcmp(argv[1], "ax25-timer-replay-report") == 0 && argc == 3)
		return command_ax25_timer_replay_report(argv[2]);
	if (strcmp(argv[1], "requirements-coverage") == 0 && argc == 4)
		return command_requirements_coverage(argv[2], argv[3]);
	if (strcmp(argv[1], "requirements-report") == 0 && argc == 3)
		return command_requirements_report(argv[2]);
	if (strcmp(argv[1], "risk-report") == 0 && argc == 3)
		return command_risk_report(argv[2]);

	usage();
	return 1;
}

static int
command_ax25_timer_replay_report(const char *path)
{
	struct kn_ax25_timer_replay_result result;
	struct kn_ax25_timer_replay_error_info error;
	char report[KN_AX25_TIMER_REPLAY_REPORT_MAX];
	enum kn_ax25_timer_replay_error rc;

	rc = kn_ax25_timer_replay_run_file(path, &result, &error);
	if (rc != KN_AX25_TIMER_REPLAY_OK &&
	    rc != KN_AX25_TIMER_REPLAY_ERR_MISMATCH) {
		fprintf(stderr, "ERR ax25-timer-replay path=%s error=%s "
		    "line=%llu detail=%s\n", path,
		    kn_ax25_timer_replay_error_name(rc),
		    (unsigned long long)error.line,
		    error.message[0] == '\0' ? "-" : error.message);
		return 1;
	}
	if (kn_ax25_timer_replay_report_format(&result, report,
	    sizeof(report)) != KN_AX25_TIMER_REPLAY_REPORT_OK)
		return 1;
	printf("%s", report);
	return result.pass != 0 ? 0 : 1;
}

static int
command_bench_diagnostics_report(const char *path)
{
	struct kn_bench_diag_result result;
	char report[KN_BENCH_DIAG_REPORT_MAX];

	if (kn_bench_diag_replay_capture(path, &result) !=
	    KN_BENCH_DIAG_REPLAY_OK) {
		fprintf(stderr, "ERR bench-diagnostics path=%s\n", path);
		return 1;
	}
	if (kn_bench_diag_result_format(&result, report, sizeof(report)) !=
	    KN_BENCH_REPLAY_REPORT_OK)
		return 1;
	printf("%s", report);
	return result.pass != 0 ? 0 : 1;
}

static int
command_bench_coverage(const char *path)
{
	struct kn_compat_bench_pack pack;
	char report[KN_COMPAT_BENCH_REPORT_MAX];

	if (parse_bench_file(path, &pack) != 0)
		return 1;
	if (kn_compat_bench_pack_coverage_report(&pack, report,
	    sizeof(report)) != KN_COMPAT_BENCH_OK) {
		fprintf(stderr, "ERR bench-coverage\n");
		return 1;
	}
	printf("%s", report);
	return 0;
}

static int
command_bench_pack_report(const char *path)
{
	struct kn_compat_bench_pack pack;
	struct kn_compat_bench_error_info error;
	char report[KN_COMPAT_BENCH_REPORT_MAX];

	if (parse_bench_file(path, &pack) != 0)
		return 1;
	memset(&error, 0, sizeof(error));
	if (kn_compat_bench_pack_validate_refs(&pack, &error) !=
	    KN_COMPAT_BENCH_OK) {
		print_bench_error(path, &error);
		return 1;
	}
	if (kn_compat_bench_pack_report(&pack, report,
	    sizeof(report)) != KN_COMPAT_BENCH_OK)
		return 1;
	printf("%s", report);
	return 0;
}

static int
command_check_ax25_timer_replay(const char *path)
{
	struct kn_ax25_timer_replay_script script;
	struct kn_ax25_timer_replay_error_info error;

	if (kn_ax25_timer_replay_script_parse_file(path, &script, &error) !=
	    KN_AX25_TIMER_REPLAY_OK) {
		fprintf(stderr, "ERR ax25-timer-replay path=%s error=%s "
		    "line=%llu detail=%s\n", path,
		    kn_ax25_timer_replay_error_name(error.error),
		    (unsigned long long)error.line,
		    error.message[0] == '\0' ? "-" : error.message);
		return 1;
	}
	printf("OK ax25-timer-replay=%s commands=%llu\n", script.name,
	    (unsigned long long)script.command_count);
	return 0;
}

static int
command_check_bench_pack(const char *path)
{
	struct kn_compat_bench_pack pack;
	struct kn_compat_bench_error_info error;

	if (parse_bench_file(path, &pack) != 0)
		return 1;
	memset(&error, 0, sizeof(error));
	if (kn_compat_bench_pack_validate_refs(&pack, &error) !=
	    KN_COMPAT_BENCH_OK) {
		print_bench_error(path, &error);
		return 1;
	}
	printf("OK bench-pack=%s fixtures=%llu hardware_required=%s "
	    "transmit_required=%s\n", pack.name,
	    (unsigned long long)pack.fixture_count,
	    pack.hardware_required != 0 ? "true" : "false",
	    pack.transmit_required != 0 ? "true" : "false");
	return 0;
}

static int
command_check_bench_expected(const char *path)
{
	struct kn_bench_expected_file expected;

	if (parse_bench_expected_file(path, &expected) != 0)
		return 1;
	printf("OK bench-expected=%s captures=%llu\n", path,
	    (unsigned long long)expected.capture_count);
	return 0;
}

static int
command_check_command_profiles(const char *path)
{
	struct kn_compat_command_profiles profiles;

	if (parse_command_profile_file(path, &profiles) != 0)
		return 1;
	printf("OK command-profiles=%s commands=%llu\n", profiles.name,
	    (unsigned long long)profiles.profile_count);
	return 0;
}

static int
command_check_pack(const char *path)
{
	struct kn_compat_observation_pack pack;
	struct kn_compat_pack_error_info error;

	if (parse_pack_file(path, &pack) != 0)
		return 1;
	memset(&error, 0, sizeof(error));
	if (kn_compat_observation_pack_validate_refs(&pack, &error) !=
	    KN_COMPAT_PACK_OK) {
		print_pack_error(path, &error);
		return 1;
	}
	printf("OK pack=%s fixtures=%llu transcripts=%llu\n", pack.name,
	    (unsigned long long)pack.fixture_count,
	    (unsigned long long)pack.transcript_count);
	return 0;
}

static int
command_check_requirements(const char *path)
{
	struct kn_compat_requirements requirements;

	if (parse_requirements_file(path, &requirements) != 0)
		return 1;
	printf("OK requirements=%s commands=%llu\n", requirements.name,
	    (unsigned long long)requirements.requirement_count);
	return 0;
}

static int
command_check_risk_register(const char *path)
{
	struct kn_compat_risk_register risks;
	FILE *fp;

	fp = fopen(path, "r");
	if (fp == NULL) {
		fprintf(stderr, "ERR risk-register=%s error=open\n", path);
		return 1;
	}
	(void)fclose(fp);
	if (kn_compat_risk_register_default(&risks) != KN_COMPAT_RISK_OK)
		return 1;
	printf("OK risk-register=default source=%s risks=%llu\n", path,
	    (unsigned long long)risks.risk_count);
	return 0;
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
command_command_profile_report(const char *path)
{
	struct kn_compat_command_profiles profiles;
	char report[KN_COMPAT_PROFILE_REPORT_MAX];

	if (parse_command_profile_file(path, &profiles) != 0)
		return 1;
	if (kn_compat_command_profiles_report(&profiles, report,
	    sizeof(report)) != KN_COMPAT_PROFILE_OK) {
		fprintf(stderr, "ERR command-profile-report\n");
		return 1;
	}
	printf("%s", report);
	return 0;
}

static int
command_compare_pack_observations(const char *path)
{
	struct kn_compat_observation_pack pack;
	struct kn_compat_observation left;
	struct kn_compat_observation right;
	char left_path[COMPAT_PATH_MAX];
	char right_path[COMPAT_PATH_MAX];
	char report[COMPAT_REPORT_MAX];
	size_t i;
	int failed;

	if (parse_pack_file(path, &pack) != 0)
		return 1;
	if (pack.fixture_count < 2) {
		fprintf(stderr, "ERR pack-observations-too-few\n");
		return 1;
	}
	failed = 0;
	for (i = 1; i < pack.fixture_count; i++) {
		if (kn_compat_observation_pack_join_path(&pack,
		    pack.fixtures[i - 1].path, left_path,
		    sizeof(left_path)) != KN_COMPAT_PACK_OK ||
		    kn_compat_observation_pack_join_path(&pack,
		    pack.fixtures[i].path, right_path, sizeof(right_path)) !=
		    KN_COMPAT_PACK_OK)
			return 1;
		if (parse_observation_file(left_path, &left) != 0 ||
		    parse_observation_file(right_path, &right) != 0)
			return 1;
		if (kn_compat_capture_compare(&left, &right, report,
		    sizeof(report)) != KN_COMPAT_CAPTURE_OK)
			return 1;
		printf("%s\n", report);
	}

	return failed;
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
command_check_capture(const char *path)
{
	struct kn_compat_packet_capture capture;

	if (parse_capture_file(path, &capture) != 0)
		return 1;

	printf("OK capture=%s method=%s direction=%s frame_bytes=%llu\n",
	    capture.name, kn_compat_packet_method_name(capture.method),
	    kn_compat_packet_direction_name(capture.direction),
	    (unsigned long long)capture.frame_len);
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
command_capture_report(const char *path)
{
	return command_decode_capture(path);
}

static int
command_capture_to_transcript(int argc, char *argv[])
{
	struct kn_compat_packet_capture capture;
	struct kn_compat_packet_decode decode;
	char text[KN_COMPAT_CAPTURE_REPORT_MAX];

	if (argc != 5 || strcmp(argv[3], "--output") != 0) {
		usage();
		return 1;
	}
	if (decode_capture_file(argv[2], &capture, &decode) != 0)
		return 1;
	if (kn_compat_capture_to_transcript(&capture, &decode, text,
	    sizeof(text)) != KN_COMPAT_CAPTURE_REPORT_OK) {
		fprintf(stderr, "ERR capture-to-transcript\n");
		return 1;
	}
	if (kn_compat_capture_write_text(text, argv[4]) !=
	    KN_COMPAT_CAPTURE_OK) {
		fprintf(stderr, "ERR write-output\n");
		return 1;
	}
	printf("OK capture-transcript=%s\n", argv[4]);
	return 0;
}

static int
command_decode_capture(const char *path)
{
	struct kn_compat_packet_capture capture;
	struct kn_compat_packet_decode decode;
	char report[KN_COMPAT_CAPTURE_REPORT_MAX];

	if (decode_capture_file(path, &capture, &decode) != 0)
		return 1;
	if (kn_compat_capture_report_format(&decode, report,
	    sizeof(report)) != KN_COMPAT_CAPTURE_REPORT_OK) {
		fprintf(stderr, "ERR capture-report\n");
		return 1;
	}
	printf("%s", report);
	return 0;
}

static int
command_generate_node_plan(int argc, char *argv[])
{
	struct kn_compat_observation_pack pack;
	char req_text[KN_COMPAT_REQ_TEXT_MAX];
	char profile_text[KN_COMPAT_PROFILE_TEXT_MAX];
	const char *requirements_path;
	const char *profiles_path;
	int i;

	if (argc < 7) {
		usage();
		return 1;
	}
	requirements_path = NULL;
	profiles_path = NULL;
	for (i = 3; i < argc; i++) {
		if (strcmp(argv[i], "--requirements") == 0)
			requirements_path = option_value(argc, argv, &i,
			    "--requirements");
		else if (strcmp(argv[i], "--profiles") == 0)
			profiles_path = option_value(argc, argv, &i,
			    "--profiles");
		else
			return 1;
	}
	if (requirements_path == NULL || profiles_path == NULL)
		return 1;
	if (parse_pack_file(argv[2], &pack) != 0)
		return 1;
	if (kn_compat_requirements_generate_from_pack(&pack, req_text,
	    sizeof(req_text)) != KN_COMPAT_REQ_OK) {
		fprintf(stderr, "ERR generate-requirements\n");
		return 1;
	}
	if (kn_compat_command_profiles_generate_from_pack(&pack, profile_text,
	    sizeof(profile_text)) != KN_COMPAT_PROFILE_OK) {
		fprintf(stderr, "ERR generate-command-profiles\n");
		return 1;
	}
	if (kn_compat_capture_write_text(req_text, requirements_path) !=
	    KN_COMPAT_CAPTURE_OK) {
		fprintf(stderr, "ERR write-requirements\n");
		return 1;
	}
	if (kn_compat_capture_write_text(profile_text, profiles_path) !=
	    KN_COMPAT_CAPTURE_OK) {
		fprintf(stderr, "ERR write-command-profiles\n");
		return 1;
	}
	printf("OK node-plan requirements=%s profiles=%s\n",
	    requirements_path, profiles_path);
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
command_manual_import(int argc, char *argv[])
{
	struct kn_manual_capture_import_request request;
	struct kn_manual_capture_import_result result;
	struct kn_manual_capture_error_info error;
	const char *workspace;
	const char *notes;
	const char *source;
	int i;

	if (argc < 5)
		return 1;
	workspace = notes = source = NULL;
	for (i = 3; i < argc; i++) {
		if (strcmp(argv[i], "--workspace") == 0)
			workspace = option_value(argc, argv, &i, "--workspace");
		else if (strcmp(argv[i], "--notes") == 0)
			notes = option_value(argc, argv, &i, "--notes");
		else if (strcmp(argv[i], "--source") == 0)
			source = option_value(argc, argv, &i, "--source");
		else
			return 1;
	}
	if (workspace == NULL)
		return 1;
	memset(&request, 0, sizeof(request));
	request.source_path = argv[2];
	request.workspace_root = workspace;
	request.notes = notes;
	request.source = KN_MANUAL_CAPTURE_SOURCE_MANUAL;
	if (source != NULL &&
	    kn_manual_capture_source_from_text(source, &request.source) !=
	    KN_MANUAL_CAPTURE_OK)
		return 1;
	memset(&error, 0, sizeof(error));
	if (kn_manual_capture_import(&request, &result, &error) !=
	    KN_MANUAL_CAPTURE_OK) {
		print_manual_error("manual-import", &error);
		return 1;
	}
	printf("OK manual-import id=%u file=%s status=%s\n", result.id,
	    result.file, kn_manual_capture_status_name(result.status));
	return 0;
}

static int
command_manual_list(int argc, char *argv[])
{
	struct kn_manual_capture_index index;
	struct kn_manual_capture_error_info error;
	const char *workspace;
	char report[KN_MANUAL_CAPTURE_REPORT_MAX];

	if (argc != 4 || strcmp(argv[2], "--workspace") != 0)
		return 1;
	workspace = argv[3];
	memset(&error, 0, sizeof(error));
	if (kn_manual_capture_index_load(workspace, &index, &error) !=
	    KN_MANUAL_CAPTURE_OK) {
		print_manual_error("manual-list", &error);
		return 1;
	}
	if (kn_manual_capture_index_format(&index, report, sizeof(report)) !=
	    KN_MANUAL_CAPTURE_OK)
		return 1;
	printf("%s", report);
	return 0;
}

static int
command_manual_report(int argc, char *argv[])
{
	struct kn_manual_capture_replay_result replay;
	struct kn_manual_capture_index index;
	struct kn_manual_capture_entry *entry;
	struct kn_manual_capture_error_info error;
	const char *workspace;
	char report[KN_MANUAL_CAPTURE_REPORT_MAX];
	uint32_t id;

	if (argc != 5 || strcmp(argv[3], "--workspace") != 0)
		return 1;
	id = (uint32_t)strtoul(argv[2], NULL, 10);
	workspace = argv[4];
	memset(&error, 0, sizeof(error));
	if (kn_manual_capture_replay_one(workspace, id, &replay, &error) !=
	    KN_MANUAL_CAPTURE_OK) {
		print_manual_error("manual-report", &error);
		return 1;
	}
	if (kn_manual_capture_index_load(workspace, &index, &error) !=
	    KN_MANUAL_CAPTURE_OK)
		return 1;
	entry = kn_manual_capture_index_find(&index, id);
	if (entry == NULL)
		return 1;
	if (kn_manual_capture_entry_report_format(entry, &replay.diag, report,
	    sizeof(report)) != KN_MANUAL_CAPTURE_OK)
		return 1;
	printf("%s", report);
	return 0;
}

static int
command_manual_replay(int argc, char *argv[])
{
	struct kn_manual_capture_replay_result result;
	struct kn_manual_capture_error_info error;
	const char *workspace;
	char report[KN_BENCH_DIAG_REPORT_MAX];
	uint32_t id;

	if (argc != 5 || strcmp(argv[3], "--workspace") != 0)
		return 1;
	id = (uint32_t)strtoul(argv[2], NULL, 10);
	workspace = argv[4];
	memset(&error, 0, sizeof(error));
	if (kn_manual_capture_replay_one(workspace, id, &result, &error) !=
	    KN_MANUAL_CAPTURE_OK) {
		print_manual_error("manual-replay", &error);
		return 1;
	}
	if (kn_bench_diag_result_format(&result.diag, report,
	    sizeof(report)) != KN_BENCH_REPLAY_REPORT_OK)
		return 1;
	printf("OK manual-replay id=%u replay=%s\n%s", id,
	    kn_manual_capture_replay_status_name(result.replay), report);
	return result.diag.tx_writes_attempted == 0 ? 0 : 1;
}

static int
command_manual_replay_all(int argc, char *argv[])
{
	struct kn_manual_capture_replay_all_result result;
	struct kn_manual_capture_error_info error;
	const char *workspace;
	char report[KN_MANUAL_CAPTURE_REPORT_MAX];

	if (argc != 4 || strcmp(argv[2], "--workspace") != 0)
		return 1;
	workspace = argv[3];
	memset(&error, 0, sizeof(error));
	if (kn_manual_capture_replay_all(workspace, &result, &error) !=
	    KN_MANUAL_CAPTURE_OK) {
		print_manual_error("manual-replay-all", &error);
		return 1;
	}
	if (kn_manual_capture_replay_all_format(&result, report,
	    sizeof(report)) != KN_MANUAL_CAPTURE_OK)
		return 1;
	printf("%s", report);
	return result.tx_writes == 0 ? 0 : 1;
}

static int
command_manual_summary(int argc, char *argv[])
{
	struct kn_manual_capture_index index;
	struct kn_manual_capture_error_info error;
	const char *workspace;
	char report[KN_MANUAL_CAPTURE_REPORT_MAX];

	if (argc != 4 || strcmp(argv[2], "--workspace") != 0)
		return 1;
	workspace = argv[3];
	memset(&error, 0, sizeof(error));
	if (kn_manual_capture_index_load(workspace, &index, &error) !=
	    KN_MANUAL_CAPTURE_OK) {
		print_manual_error("manual-summary", &error);
		return 1;
	}
	if (kn_manual_capture_summary_format(&index, report,
	    sizeof(report)) != KN_MANUAL_CAPTURE_OK)
		return 1;
	printf("%s", report);
	return 0;
}

static int
command_manual_validate(int argc, char *argv[])
{
	struct kn_manual_capture_index index;
	struct kn_manual_capture_error_info error;
	const char *workspace;
	char report[KN_MANUAL_CAPTURE_REPORT_MAX];

	if (argc != 4 || strcmp(argv[2], "--workspace") != 0)
		return 1;
	workspace = argv[3];
	memset(&error, 0, sizeof(error));
	if (kn_manual_capture_validate_all(workspace, &index, &error) !=
	    KN_MANUAL_CAPTURE_OK) {
		print_manual_error("manual-validate", &error);
		return 1;
	}
	if (kn_manual_capture_index_format(&index, report, sizeof(report)) !=
	    KN_MANUAL_CAPTURE_OK)
		return 1;
	printf("%s", report);
	return 0;
}

static int
command_manual_workspace_check(const char *path)
{
	struct kn_manual_capture_workspace workspace;
	struct kn_manual_capture_error_info error;

	memset(&error, 0, sizeof(error));
	if (kn_manual_capture_workspace_check(path, &workspace, &error) !=
	    KN_MANUAL_CAPTURE_OK) {
		print_manual_error(path, &error);
		return 1;
	}
	printf("OK manual-workspace=%s name=%s transmit_required=false\n",
	    path, workspace.name);
	return 0;
}

static int
command_manual_workspace_init(const char *path)
{
	struct kn_manual_capture_error_info error;

	memset(&error, 0, sizeof(error));
	if (kn_manual_capture_workspace_init(path, &error) !=
	    KN_MANUAL_CAPTURE_OK) {
		print_manual_error(path, &error);
		return 1;
	}
	printf("OK manual-workspace-init=%s\n", path);
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
command_requirements_coverage(const char *requirements_path,
	const char *pack_path)
{
	struct kn_compat_requirements requirements;
	struct kn_compat_observation_pack pack;
	char report[KN_COMPAT_REQ_REPORT_MAX];

	if (parse_requirements_file(requirements_path, &requirements) != 0)
		return 1;
	if (parse_pack_file(pack_path, &pack) != 0)
		return 1;
	if (kn_compat_requirements_coverage_report(&requirements, &pack,
	    report, sizeof(report)) != KN_COMPAT_REQ_OK) {
		fprintf(stderr, "ERR requirements-coverage\n");
		return 1;
	}
	printf("%s", report);
	return 0;
}

static int
command_requirements_report(const char *path)
{
	struct kn_compat_requirements requirements;
	char report[KN_COMPAT_REQ_REPORT_MAX];

	if (parse_requirements_file(path, &requirements) != 0)
		return 1;
	if (kn_compat_requirements_report(&requirements, report,
	    sizeof(report)) != KN_COMPAT_REQ_OK) {
		fprintf(stderr, "ERR requirements-report\n");
		return 1;
	}
	printf("%s", report);
	return 0;
}

static int
command_risk_report(const char *path)
{
	struct kn_compat_risk_register risks;
	char report[KN_COMPAT_RISK_REPORT_MAX];
	FILE *fp;

	fp = fopen(path, "r");
	if (fp == NULL) {
		fprintf(stderr, "ERR risk-register=%s error=open\n", path);
		return 1;
	}
	(void)fclose(fp);
	if (kn_compat_risk_register_default(&risks) != KN_COMPAT_RISK_OK)
		return 1;
	if (kn_compat_risk_register_report(&risks, report,
	    sizeof(report)) != KN_COMPAT_RISK_OK) {
		fprintf(stderr, "ERR risk-report\n");
		return 1;
	}
	printf("%s", report);
	return 0;
}

static int
command_replay_ax25_timer(const char *path)
{
	struct kn_ax25_timer_replay_result result;
	struct kn_ax25_timer_replay_error_info error;
	char report[KN_AX25_TIMER_REPLAY_REPORT_MAX];
	enum kn_ax25_timer_replay_error rc;

	rc = kn_ax25_timer_replay_run_file(path, &result, &error);
	if (rc != KN_AX25_TIMER_REPLAY_OK &&
	    rc != KN_AX25_TIMER_REPLAY_ERR_MISMATCH) {
		fprintf(stderr, "ERR ax25-timer-replay path=%s error=%s "
		    "line=%llu detail=%s\n", path,
		    kn_ax25_timer_replay_error_name(rc),
		    (unsigned long long)error.line,
		    error.message[0] == '\0' ? "-" : error.message);
		return 1;
	}
	if (kn_ax25_timer_replay_report_format(&result, report,
	    sizeof(report)) != KN_AX25_TIMER_REPLAY_REPORT_OK)
		return 1;
	printf("%s", report);
	return result.pass != 0 && result.tx_writes_observed == 0 ? 0 : 1;
}

static int
command_replay_ax25_timer_dir(const char *path)
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
		if (has_replay_suffix(entry->d_name) == 0)
			continue;
		if (count >= COMPAT_DIR_MAX) {
			closedir(dir);
			fprintf(stderr, "ERR too-many-replays\n");
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
		if (command_replay_ax25_timer(entries[i].path) != 0)
			failed = 1;
	}
	if (count == 0) {
		fprintf(stderr, "ERR no-replays\n");
		return 1;
	}

	return failed;
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
command_replay_bench_diagnostics(const char *path)
{
	struct kn_compat_bench_pack pack;
	struct kn_bench_expected_file expected;
	struct kn_bench_pack_diag_result pack_result;
	struct kn_bench_diag_result result;
	char expected_path[COMPAT_PATH_MAX];
	char report[KN_BENCH_DIAG_REPORT_MAX];
	FILE *expected_fp;
	int needed;

	if (has_capture_suffix(path) != 0) {
		if (kn_bench_diag_replay_capture(path, &result) !=
		    KN_BENCH_DIAG_REPLAY_OK)
			return 1;
		if (kn_bench_diag_result_format(&result, report,
		    sizeof(report)) != KN_BENCH_REPLAY_REPORT_OK)
			return 1;
		printf("%s", report);
		return result.pass != 0 ? 0 : 1;
	}

	if (parse_bench_file(path, &pack) != 0)
		return 1;
	kn_bench_expected_file_clear(&expected);
	if (pack.base_dir[0] == '\0' || strcmp(pack.base_dir, ".") == 0)
		needed = snprintf(expected_path, sizeof(expected_path),
		    "ax25-diag-replay.expected");
	else
		needed = snprintf(expected_path, sizeof(expected_path),
		    "%s/ax25-diag-replay.expected", pack.base_dir);
	if (needed < 0 || (size_t)needed >= sizeof(expected_path))
		return 1;
	expected_fp = fopen(expected_path, "r");
	if (expected_fp != NULL) {
		(void)fclose(expected_fp);
		if (parse_bench_expected_file(expected_path, &expected) != 0)
			return 1;
	}
	if (kn_bench_replay_pack_diagnostics(&pack,
	    expected.capture_count == 0 ? NULL : &expected,
	    &pack_result) != KN_BENCH_REPLAY_OK &&
	    pack_result.fail_count > 0) {
		if (kn_bench_pack_diag_result_format(&pack_result, report,
		    sizeof(report)) == KN_BENCH_REPLAY_REPORT_OK)
			printf("%s", report);
		return 1;
	}
	if (kn_bench_pack_diag_result_format(&pack_result, report,
	    sizeof(report)) != KN_BENCH_REPLAY_REPORT_OK)
		return 1;
	printf("%s", report);
	return pack_result.total_tx_writes == 0 ? 0 : 1;
}

static int
command_replay_bench_diagnostics_dir(const char *path)
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
		if (has_capture_suffix(entry->d_name) == 0)
			continue;
		if (count >= COMPAT_DIR_MAX) {
			closedir(dir);
			fprintf(stderr, "ERR too-many-captures\n");
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
		if (command_bench_diagnostics_report(entries[i].path) != 0)
			failed = 1;
	}
	if (count == 0) {
		fprintf(stderr, "ERR no-captures\n");
		return 1;
	}

	return failed;
}

static int
command_replay_bench_pack(const char *path)
{
	struct kn_compat_bench_pack pack;
	char report[KN_COMPAT_BENCH_REPORT_MAX];

	if (parse_bench_file(path, &pack) != 0)
		return 1;
	if (kn_compat_bench_pack_replay_report(&pack, report,
	    sizeof(report)) != KN_COMPAT_BENCH_OK) {
		fprintf(stderr, "ERR replay-bench-pack\n");
		return 1;
	}
	printf("%s", report);
	return 0;
}

static int
command_replay_pack(const char *path)
{
	struct kn_compat_observation_pack pack;
	char transcript_path[COMPAT_PATH_MAX];
	size_t i;
	int failed;

	if (parse_pack_file(path, &pack) != 0)
		return 1;
	failed = 0;
	for (i = 0; i < pack.transcript_count; i++) {
		if (kn_compat_observation_pack_join_path(&pack,
		    pack.transcripts[i].path, transcript_path,
		    sizeof(transcript_path)) != KN_COMPAT_PACK_OK)
			return 1;
		if (command_replay_transcript(transcript_path) != 0)
			failed = 1;
	}
	if (pack.transcript_count == 0) {
		fprintf(stderr, "ERR no-pack-transcripts\n");
		return 1;
	}

	return failed;
}

static int
command_replay_capture(const char *path)
{
	struct kn_compat_packet_capture capture;
	struct kn_compat_packet_decode decode;
	char report[KN_COMPAT_CAPTURE_REPORT_MAX];

	if (decode_capture_file(path, &capture, &decode) != 0)
		return 1;
	if (kn_compat_capture_report_format(&decode, report,
	    sizeof(report)) != KN_COMPAT_CAPTURE_REPORT_OK)
		return 1;
	printf("%s", report);

	return decode.passed != 0 ? 0 : 1;
}

static int
command_replay_capture_dir(const char *path)
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
		if (has_capture_suffix(entry->d_name) == 0)
			continue;
		if (count >= COMPAT_DIR_MAX) {
			closedir(dir);
			fprintf(stderr, "ERR too-many-captures\n");
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
		if (command_replay_capture(entries[i].path) != 0)
			failed = 1;
	}
	if (count == 0) {
		fprintf(stderr, "ERR no-captures\n");
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
command_pack_coverage(const char *path)
{
	struct kn_compat_observation_pack pack;
	struct kn_compat_coverage coverage;
	char report[KN_COMPAT_COVERAGE_REPORT_MAX];

	if (parse_pack_file(path, &pack) != 0)
		return 1;
	if (kn_compat_coverage_from_pack(&pack, &coverage) !=
	    KN_COMPAT_COVERAGE_OK) {
		fprintf(stderr, "ERR pack-coverage\n");
		return 1;
	}
	if (kn_compat_coverage_report(&pack, &coverage, report,
	    sizeof(report)) != KN_COMPAT_COVERAGE_OK) {
		fprintf(stderr, "ERR pack-coverage-report\n");
		return 1;
	}
	printf("%s", report);
	return 0;
}

static int
command_pack_report(const char *path)
{
	struct kn_compat_observation_pack pack;
	struct kn_compat_pack_error_info error;
	char report[KN_COMPAT_PACK_REPORT_MAX];

	if (parse_pack_file(path, &pack) != 0)
		return 1;
	memset(&error, 0, sizeof(error));
	if (kn_compat_observation_pack_validate_refs(&pack, &error) !=
	    KN_COMPAT_PACK_OK) {
		print_pack_error(path, &error);
		return 1;
	}
	if (kn_compat_observation_pack_report(&pack, report,
	    sizeof(report)) != KN_COMPAT_PACK_OK)
		return 1;
	printf("%s", report);
	return 0;
}

static int
command_show_pack(const char *path)
{
	struct kn_compat_observation_pack pack;
	char report[KN_COMPAT_PACK_REPORT_MAX];

	if (parse_pack_file(path, &pack) != 0)
		return 1;
	if (kn_compat_observation_pack_report(&pack, report,
	    sizeof(report)) != KN_COMPAT_PACK_OK)
		return 1;
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

static int
has_capture_suffix(const char *name)
{
	const char suffix[] = ".capture";
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
has_replay_suffix(const char *name)
{
	const char suffix[] = ".replay";
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
decode_capture_file(const char *path, struct kn_compat_packet_capture *capture,
	struct kn_compat_packet_decode *decode)
{
	if (parse_capture_file(path, capture) != 0)
		return 1;
	if (capture->method == KN_COMPAT_PACKET_METHOD_KISS) {
		if (kn_compat_kiss_capture_decode(capture, decode) ==
		    KN_COMPAT_KISS_CAPTURE_ERR_DECODE) {
			fprintf(stderr, "ERR capture-decode path=%s\n", path);
			return 1;
		}
		return 0;
	}
	if (capture->method == KN_COMPAT_PACKET_METHOD_AXIP ||
	    capture->method == KN_COMPAT_PACKET_METHOD_AXUDP) {
		if (kn_compat_axip_capture_decode(capture, decode) ==
		    KN_COMPAT_AXIP_CAPTURE_ERR_DECODE) {
			fprintf(stderr, "ERR capture-decode path=%s\n", path);
			return 1;
		}
		return 0;
	}

	fprintf(stderr, "ERR unsupported-capture-method\n");
	return 1;
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
parse_pack_file(const char *path, struct kn_compat_observation_pack *pack)
{
	struct kn_compat_pack_error_info error;
	enum kn_compat_pack_error rc;

	memset(&error, 0, sizeof(error));
	rc = kn_compat_observation_pack_parse_file(path, pack, &error);
	if (rc != KN_COMPAT_PACK_OK) {
		print_pack_error(path, &error);
		return 1;
	}

	return 0;
}

static int
parse_bench_file(const char *path, struct kn_compat_bench_pack *pack)
{
	struct kn_compat_bench_error_info error;
	enum kn_compat_bench_error rc;

	memset(&error, 0, sizeof(error));
	rc = kn_compat_bench_pack_parse_file(path, pack, &error);
	if (rc != KN_COMPAT_BENCH_OK) {
		print_bench_error(path, &error);
		return 1;
	}

	return 0;
}

static int
parse_bench_expected_file(const char *path,
	struct kn_bench_expected_file *expected)
{
	struct kn_bench_replay_error_info error;
	enum kn_bench_replay_error rc;

	memset(&error, 0, sizeof(error));
	rc = kn_bench_expected_parse_file(path, expected, &error);
	if (rc != KN_BENCH_REPLAY_OK) {
		print_bench_replay_error(path, &error);
		return 1;
	}

	return 0;
}

static int
parse_capture_file(const char *path, struct kn_compat_packet_capture *capture)
{
	struct kn_compat_packet_capture_error_info error;
	enum kn_compat_packet_capture_error rc;

	memset(&error, 0, sizeof(error));
	rc = kn_compat_packet_capture_parse_file(path, capture, &error);
	if (rc != KN_COMPAT_PACKET_CAPTURE_OK) {
		print_capture_error(path, &error);
		return 1;
	}

	return 0;
}

static int
parse_command_profile_file(const char *path,
	struct kn_compat_command_profiles *profiles)
{
	struct kn_compat_profile_error_info error;
	enum kn_compat_profile_error rc;

	memset(&error, 0, sizeof(error));
	rc = kn_compat_command_profiles_parse_file(path, profiles, &error);
	if (rc != KN_COMPAT_PROFILE_OK) {
		print_profile_error(path, &error);
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

static int
parse_requirements_file(const char *path,
	struct kn_compat_requirements *requirements)
{
	struct kn_compat_req_error_info error;
	enum kn_compat_req_error rc;

	memset(&error, 0, sizeof(error));
	rc = kn_compat_requirements_parse_file(path, requirements, &error);
	if (rc != KN_COMPAT_REQ_OK) {
		print_requirements_error(path, &error);
		return 1;
	}

	return 0;
}

static void
print_bench_error(const char *path,
	const struct kn_compat_bench_error_info *error)
{
	fprintf(stderr, "ERR bench-pack=%s error=%s line=%llu detail=%s\n",
	    path == NULL ? "-" : path,
	    kn_compat_bench_error_name(error->error),
	    (unsigned long long)error->line,
	    error->message[0] == '\0' ? "-" : error->message);
}

static void
print_bench_replay_error(const char *path,
	const struct kn_bench_replay_error_info *error)
{
	fprintf(stderr, "ERR bench-expected=%s error=%s line=%llu detail=%s\n",
	    path == NULL ? "-" : path,
	    kn_bench_replay_error_name(error->error),
	    (unsigned long long)error->line,
	    error->message[0] == '\0' ? "-" : error->message);
}

static void
print_manual_error(const char *path,
	const struct kn_manual_capture_error_info *error)
{
	fprintf(stderr, "ERR manual-capture=%s error=%s line=%llu detail=%s\n",
	    path == NULL ? "-" : path,
	    error == NULL ? "unknown" : kn_manual_capture_error_name(
	    error->error),
	    error == NULL ? 0ULL : (unsigned long long)error->line,
	    error == NULL || error->message[0] == '\0' ? "-" :
	    error->message);
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
print_capture_error(const char *path,
	const struct kn_compat_packet_capture_error_info *error)
{
	fprintf(stderr, "ERR capture=%s error=%s line=%llu detail=%s\n",
	    path == NULL ? "-" : path,
	    kn_compat_packet_capture_error_name(error->error),
	    (unsigned long long)error->line,
	    error->message[0] == '\0' ? "-" : error->message);
}

static void
print_profile_error(const char *path,
	const struct kn_compat_profile_error_info *error)
{
	fprintf(stderr, "ERR command-profiles=%s error=%s line=%llu "
	    "detail=%s\n",
	    path == NULL ? "-" : path,
	    kn_compat_profile_error_name(error->error),
	    (unsigned long long)error->line,
	    error->message[0] == '\0' ? "-" : error->message);
}

static void
print_pack_error(const char *path,
	const struct kn_compat_pack_error_info *error)
{
	fprintf(stderr, "ERR pack=%s error=%s line=%llu detail=%s\n",
	    path == NULL ? "-" : path,
	    kn_compat_pack_error_name(error->error),
	    (unsigned long long)error->line,
	    error->message[0] == '\0' ? "-" : error->message);
}

static void
print_requirements_error(const char *path,
	const struct kn_compat_req_error_info *error)
{
	fprintf(stderr, "ERR requirements=%s error=%s line=%llu detail=%s\n",
	    path == NULL ? "-" : path,
	    kn_compat_req_error_name(error->error),
	    (unsigned long long)error->line,
	    error->message[0] == '\0' ? "-" : error->message);
}

static void
usage(void)
{
	fprintf(stderr,
	    "usage: kilonode-compat check-transcript PATH\n"
	    "       kilonode-compat check-ax25-timer-replay PATH\n"
	    "       kilonode-compat check-bench-pack PATH\n"
	    "       kilonode-compat check-bench-expected PATH\n"
	    "       kilonode-compat check-observation PATH\n"
	    "       kilonode-compat check-capture PATH\n"
	    "       kilonode-compat check-command-profiles PATH\n"
	    "       kilonode-compat check-pack PATH\n"
	    "       kilonode-compat check-requirements PATH\n"
	    "       kilonode-compat check-risk-register PATH\n"
	    "       kilonode-compat command-profile-report PATH\n"
	    "       kilonode-compat decode-capture PATH\n"
	    "       kilonode-compat generate-node-plan PACK --requirements OUT --profiles OUT\n"
	    "       kilonode-compat replay-capture PATH\n"
	    "       kilonode-compat replay-capture-dir PATH\n"
	    "       kilonode-compat capture-report PATH\n"
	    "       kilonode-compat bench-pack-report PATH\n"
	    "       kilonode-compat replay-bench-pack PATH\n"
	    "       kilonode-compat replay-bench-diagnostics PATH\n"
	    "       kilonode-compat replay-bench-diagnostics-dir PATH\n"
	    "       kilonode-compat bench-diagnostics-report PATH\n"
	    "       kilonode-compat bench-coverage PATH\n"
	    "       kilonode-compat capture-to-transcript PATH --output PATH\n"
	    "       kilonode-compat show-observation PATH\n"
	    "       kilonode-compat show-pack PATH\n"
	    "       kilonode-compat pack-report PATH\n"
	    "       kilonode-compat pack-coverage PATH\n"
	    "       kilonode-compat replay-pack PATH\n"
	    "       kilonode-compat compare-pack-observations PATH\n"
	    "       kilonode-compat replay-transcript PATH\n"
	    "       kilonode-compat run-ax25-timer-replay PATH\n"
	    "       kilonode-compat run-ax25-timer-replay-dir PATH\n"
	    "       kilonode-compat ax25-timer-replay-report PATH\n"
	    "       kilonode-compat replay-dir PATH\n"
	    "       kilonode-compat report-transcript PATH\n"
	    "       kilonode-compat requirements-coverage REQUIREMENTS PACK\n"
	    "       kilonode-compat requirements-report PATH\n"
	    "       kilonode-compat risk-report PATH\n"
	    "       kilonode-compat make-transcript-from-observation PATH --output PATH\n"
	    "       kilonode-compat compare-observations A B\n"
	    "       kilonode-compat manual-workspace-init PATH\n"
	    "       kilonode-compat manual-workspace-check PATH\n"
	    "       kilonode-compat manual-import PATH --workspace PATH [--notes TEXT] [--source manual|synthetic|black-box]\n"
	    "       kilonode-compat manual-list --workspace PATH\n"
	    "       kilonode-compat manual-validate --workspace PATH\n"
	    "       kilonode-compat manual-replay ID --workspace PATH\n"
	    "       kilonode-compat manual-replay-all --workspace PATH\n"
	    "       kilonode-compat manual-report ID --workspace PATH\n"
	    "       kilonode-compat manual-summary --workspace PATH\n"
	    "       kilonode-compat observe-process --binary PATH --config PATH --name NAME --mode MODE --input TEXT --output PATH [--timeout SECONDS]\n"
	    "       kilonode-compat observe-tcp --host HOST --port PORT --name NAME --mode MODE --input TEXT --output PATH [--timeout SECONDS]\n");
}
