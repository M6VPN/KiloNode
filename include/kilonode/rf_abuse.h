/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/rf_abuse.h */

#ifndef KILONODE_RF_ABUSE_H
#define KILONODE_RF_ABUSE_H

#include <sys/types.h>

#include <stdint.h>

#include "kilonode/callsign.h"
#include "kilonode/config.h"
#include "kilonode/rf_ignore.h"

#define KN_RF_ABUSE_MAX_SOURCES 128
#define KN_RF_ABUSE_REASON_MAX  64

enum kn_rf_abuse_error {
	KN_RF_ABUSE_OK = 0,
	KN_RF_ABUSE_ERR_INVALID_ARGUMENT,
	KN_RF_ABUSE_ERR_IGNORED,
	KN_RF_ABUSE_ERR_RATE_LIMITED,
	KN_RF_ABUSE_ERR_REPLY_RATE_LIMITED,
	KN_RF_ABUSE_ERR_FULL
};

struct kn_rf_abuse_source {
	struct kn_callsign callsign;
	uint64_t first_seen;
	uint64_t last_seen;
	uint64_t command_window_start;
	size_t command_count;
	uint64_t reply_window_start;
	size_t reply_count;
	uint64_t reject_window_start;
	size_t reject_count;
	uint64_t accepted_count;
	uint64_t rejected_count;
	uint64_t replies;
	uint8_t ignored;
	uint64_t ignore_until;
	char last_reason[KN_RF_ABUSE_REASON_MAX];
};

struct kn_rf_abuse_state {
	struct kn_rf_abuse_source sources[KN_RF_ABUSE_MAX_SOURCES];
	size_t max_sources;
	size_t count;
};

enum kn_rf_abuse_error kn_rf_abuse_check_command(
	struct kn_rf_abuse_state *, const struct kn_config_rf_command *,
	const struct kn_rf_ignore_list *, const struct kn_callsign *,
	uint64_t, char *, size_t);
enum kn_rf_abuse_error kn_rf_abuse_check_reply(
	struct kn_rf_abuse_state *, const struct kn_config_rf_command *,
	const struct kn_callsign *, uint64_t, char *, size_t);
void kn_rf_abuse_clear(struct kn_rf_abuse_state *);
size_t kn_rf_abuse_count(const struct kn_rf_abuse_state *);
const char *kn_rf_abuse_error_name(enum kn_rf_abuse_error);
const struct kn_rf_abuse_source *kn_rf_abuse_find(
	const struct kn_rf_abuse_state *, const struct kn_callsign *);
void kn_rf_abuse_init(struct kn_rf_abuse_state *, size_t);
enum kn_rf_abuse_error kn_rf_abuse_record_accepted(
	struct kn_rf_abuse_state *, const struct kn_callsign *, uint64_t);
enum kn_rf_abuse_error kn_rf_abuse_record_rejected(
	struct kn_rf_abuse_state *, const struct kn_config_rf_command *,
	const struct kn_callsign *, uint64_t, const char *);
enum kn_rf_abuse_error kn_rf_abuse_record_reply(
	struct kn_rf_abuse_state *, const struct kn_callsign *, uint64_t);
const struct kn_rf_abuse_source *kn_rf_abuse_sources(
	const struct kn_rf_abuse_state *);
uint8_t kn_rf_abuse_source_ignored(struct kn_rf_abuse_state *,
	const struct kn_callsign *, uint64_t, char *, size_t);

#endif
