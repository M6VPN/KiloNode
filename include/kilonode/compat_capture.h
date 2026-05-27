/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/compat_capture.h */

#ifndef KILONODE_COMPAT_CAPTURE_H
#define KILONODE_COMPAT_CAPTURE_H

#include <sys/types.h>

#include "kilonode/compat_observe.h"

#define KN_COMPAT_CAPTURE_TEXT_MAX 8192

enum kn_compat_capture_error {
	KN_COMPAT_CAPTURE_OK = 0,
	KN_COMPAT_CAPTURE_ERR_INVALID_ARGUMENT,
	KN_COMPAT_CAPTURE_ERR_BUFFER,
	KN_COMPAT_CAPTURE_ERR_IO,
	KN_COMPAT_CAPTURE_ERR_UNSAFE_PATH
};

const char *kn_compat_capture_error_name(enum kn_compat_capture_error);
enum kn_compat_capture_error kn_compat_capture_compare(
	const struct kn_compat_observation *,
	const struct kn_compat_observation *, char *, size_t);
enum kn_compat_capture_error kn_compat_capture_transcript_candidate(
	const struct kn_compat_observation *, char *, size_t);
enum kn_compat_capture_error kn_compat_capture_write_observation(
	const struct kn_compat_observation *, const char *);
enum kn_compat_capture_error kn_compat_capture_write_text(
	const char *, const char *);

#endif
