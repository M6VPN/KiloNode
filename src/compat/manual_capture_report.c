/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/compat/manual_capture_report.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "kilonode/manual_capture_report.h"

static void count_status(const struct kn_manual_capture_index *, size_t *,
	size_t *, size_t *, size_t *, size_t *, size_t *, size_t *, size_t *);

enum kn_manual_capture_error
kn_manual_capture_summary_format(const struct kn_manual_capture_index *index,
	char *buf, size_t bufsiz)
{
	size_t unchecked;
	size_t valid;
	size_t invalid;
	size_t unsupported;
	size_t not_run;
	size_t pass;
	size_t fail;
	size_t replay_unsupported;
	int needed;

	if (index == NULL || buf == NULL || bufsiz == 0)
		return KN_MANUAL_CAPTURE_ERR_INVALID_ARGUMENT;
	count_status(index, &unchecked, &valid, &invalid, &unsupported,
	    &not_run, &pass, &fail, &replay_unsupported);
	needed = snprintf(buf, bufsiz,
	    "MANUAL-SUMMARY count=%llu unchecked=%llu valid=%llu "
	    "invalid=%llu unsupported=%llu not_run=%llu pass=%llu "
	    "fail=%llu replay_unsupported=%llu\n",
	    (unsigned long long)index->entry_count,
	    (unsigned long long)unchecked, (unsigned long long)valid,
	    (unsigned long long)invalid, (unsigned long long)unsupported,
	    (unsigned long long)not_run, (unsigned long long)pass,
	    (unsigned long long)fail,
	    (unsigned long long)replay_unsupported);
	if (needed < 0 || (size_t)needed >= bufsiz)
		return KN_MANUAL_CAPTURE_ERR_BUFFER;
	return KN_MANUAL_CAPTURE_OK;
}

enum kn_manual_capture_error
kn_manual_capture_entry_report_format(
	const struct kn_manual_capture_entry *entry,
	const struct kn_bench_diag_result *diag, char *buf, size_t bufsiz)
{
	int needed;

	if (entry == NULL || buf == NULL || bufsiz == 0)
		return KN_MANUAL_CAPTURE_ERR_INVALID_ARGUMENT;
	needed = snprintf(buf, bufsiz,
	    "MANUAL-REPORT id=%u file=%s method=%s status=%s replay=%s "
	    "state=%s tx_writes=%llu unsupported=%s\n",
	    entry->id, entry->file, entry->method,
	    kn_manual_capture_status_name(entry->status),
	    kn_manual_capture_replay_status_name(entry->replay),
	    diag == NULL ? "not-run" : diag->final_state,
	    diag == NULL ? 0ULL :
	    (unsigned long long)diag->tx_writes_attempted,
	    diag != NULL && diag->unsupported != 0 ? "true" : "false");
	if (needed < 0 || (size_t)needed >= bufsiz)
		return KN_MANUAL_CAPTURE_ERR_BUFFER;
	return KN_MANUAL_CAPTURE_OK;
}

enum kn_manual_capture_error
kn_manual_capture_replay_all_format(
	const struct kn_manual_capture_replay_all_result *result, char *buf,
	size_t bufsiz)
{
	int needed;

	if (result == NULL || buf == NULL || bufsiz == 0)
		return KN_MANUAL_CAPTURE_ERR_INVALID_ARGUMENT;
	needed = snprintf(buf, bufsiz,
	    "MANUAL-REPLAY count=%llu pass=%llu fail=%llu unsupported=%llu "
	    "tx_writes=%llu\n",
	    (unsigned long long)result->count,
	    (unsigned long long)result->pass_count,
	    (unsigned long long)result->fail_count,
	    (unsigned long long)result->unsupported_count,
	    (unsigned long long)result->tx_writes);
	if (needed < 0 || (size_t)needed >= bufsiz)
		return KN_MANUAL_CAPTURE_ERR_BUFFER;
	return KN_MANUAL_CAPTURE_OK;
}

static void
count_status(const struct kn_manual_capture_index *index, size_t *unchecked,
	size_t *valid, size_t *invalid, size_t *unsupported, size_t *not_run,
	size_t *pass, size_t *fail, size_t *replay_unsupported)
{
	size_t i;

	*unchecked = *valid = *invalid = *unsupported = 0;
	*not_run = *pass = *fail = *replay_unsupported = 0;
	for (i = 0; i < index->entry_count; i++) {
		if (index->entries[i].status ==
		    KN_MANUAL_CAPTURE_STATUS_UNCHECKED)
			(*unchecked)++;
		else if (index->entries[i].status ==
		    KN_MANUAL_CAPTURE_STATUS_VALID)
			(*valid)++;
		else if (index->entries[i].status ==
		    KN_MANUAL_CAPTURE_STATUS_INVALID)
			(*invalid)++;
		else
			(*unsupported)++;
		if (index->entries[i].replay ==
		    KN_MANUAL_CAPTURE_REPLAY_NOT_RUN)
			(*not_run)++;
		else if (index->entries[i].replay ==
		    KN_MANUAL_CAPTURE_REPLAY_PASS)
			(*pass)++;
		else if (index->entries[i].replay ==
		    KN_MANUAL_CAPTURE_REPLAY_FAIL)
			(*fail)++;
		else
			(*replay_unsupported)++;
	}
}
