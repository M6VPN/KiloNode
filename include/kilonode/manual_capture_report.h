/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/manual_capture_report.h */

#ifndef KILONODE_MANUAL_CAPTURE_REPORT_H
#define KILONODE_MANUAL_CAPTURE_REPORT_H

#include <sys/types.h>

#include "kilonode/manual_capture_replay.h"

enum kn_manual_capture_error kn_manual_capture_summary_format(
	const struct kn_manual_capture_index *, char *, size_t);
enum kn_manual_capture_error kn_manual_capture_entry_report_format(
	const struct kn_manual_capture_entry *,
	const struct kn_bench_diag_result *, char *, size_t);
enum kn_manual_capture_error kn_manual_capture_replay_all_format(
	const struct kn_manual_capture_replay_all_result *, char *, size_t);

#endif
