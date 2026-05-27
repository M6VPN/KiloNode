/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/rf_ignore.h */

#ifndef KILONODE_RF_IGNORE_H
#define KILONODE_RF_IGNORE_H

#include <sys/types.h>

#include "kilonode/callsign.h"
#include "kilonode/config.h"

#define KN_RF_IGNORE_MAX_ENTRIES 64
#define KN_RF_IGNORE_REASON_MAX  64

enum kn_rf_ignore_error {
	KN_RF_IGNORE_OK = 0,
	KN_RF_IGNORE_ERR_INVALID_ARGUMENT,
	KN_RF_IGNORE_ERR_IO,
	KN_RF_IGNORE_ERR_PARSE,
	KN_RF_IGNORE_ERR_FULL
};

struct kn_rf_ignore_entry {
	struct kn_callsign callsign;
	char reason[KN_RF_IGNORE_REASON_MAX];
};

struct kn_rf_ignore_list {
	struct kn_rf_ignore_entry entries[KN_RF_IGNORE_MAX_ENTRIES];
	size_t count;
};

enum kn_rf_ignore_error kn_rf_ignore_add(struct kn_rf_ignore_list *,
	const struct kn_callsign *, const char *);
void kn_rf_ignore_clear(struct kn_rf_ignore_list *);
size_t kn_rf_ignore_count(const struct kn_rf_ignore_list *);
const struct kn_rf_ignore_entry *kn_rf_ignore_entries(
	const struct kn_rf_ignore_list *);
const char *kn_rf_ignore_error_name(enum kn_rf_ignore_error);
void kn_rf_ignore_init(struct kn_rf_ignore_list *);
enum kn_rf_ignore_error kn_rf_ignore_load_file(struct kn_rf_ignore_list *,
	const char *);
uint8_t kn_rf_ignore_path_valid(const char *);
uint8_t kn_rf_ignore_source_ignored(const struct kn_rf_ignore_list *,
	const struct kn_callsign *, const struct kn_rf_ignore_entry **);

#endif
