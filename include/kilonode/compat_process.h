/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/compat_process.h */

#ifndef KILONODE_COMPAT_PROCESS_H
#define KILONODE_COMPAT_PROCESS_H

#include <sys/types.h>

#include <stdint.h>

#define KN_COMPAT_PROCESS_OUTPUT_MAX 4096

enum kn_compat_process_error {
	KN_COMPAT_PROCESS_OK = 0,
	KN_COMPAT_PROCESS_ERR_INVALID_ARGUMENT,
	KN_COMPAT_PROCESS_ERR_NOT_EXECUTABLE,
	KN_COMPAT_PROCESS_ERR_PIPE,
	KN_COMPAT_PROCESS_ERR_FORK,
	KN_COMPAT_PROCESS_ERR_TIMEOUT,
	KN_COMPAT_PROCESS_ERR_OUTPUT_TOO_LARGE,
	KN_COMPAT_PROCESS_ERR_CHILD
};

struct kn_compat_process_result {
	int exit_status;
	uint8_t timed_out;
	uint8_t terminated;
	char stdout_text[KN_COMPAT_PROCESS_OUTPUT_MAX];
	size_t stdout_len;
	char stderr_text[KN_COMPAT_PROCESS_OUTPUT_MAX];
	size_t stderr_len;
};

void kn_compat_process_result_clear(struct kn_compat_process_result *);
const char *kn_compat_process_error_name(enum kn_compat_process_error);
enum kn_compat_process_error kn_compat_process_run(
	const char *, const char *, const char *, unsigned int,
	struct kn_compat_process_result *);

#endif
