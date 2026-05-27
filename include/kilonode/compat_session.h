/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/compat_session.h */

#ifndef KILONODE_COMPAT_SESSION_H
#define KILONODE_COMPAT_SESSION_H

#include <sys/types.h>

#define KN_COMPAT_SESSION_COMMAND_MAX 256
#define KN_COMPAT_SESSION_RESPONSE_MAX 4096

enum kn_compat_session_error {
	KN_COMPAT_SESSION_OK = 0,
	KN_COMPAT_SESSION_ERR_INVALID_ARGUMENT,
	KN_COMPAT_SESSION_ERR_INVALID_HOST,
	KN_COMPAT_SESSION_ERR_INVALID_PORT,
	KN_COMPAT_SESSION_ERR_COMMAND_TOO_LARGE,
	KN_COMPAT_SESSION_ERR_CONNECT,
	KN_COMPAT_SESSION_ERR_TIMEOUT,
	KN_COMPAT_SESSION_ERR_IO,
	KN_COMPAT_SESSION_ERR_RESPONSE_TOO_LARGE
};

struct kn_compat_session_result {
	char response[KN_COMPAT_SESSION_RESPONSE_MAX];
	size_t response_len;
};

void kn_compat_session_result_clear(struct kn_compat_session_result *);
const char *kn_compat_session_error_name(enum kn_compat_session_error);
enum kn_compat_session_error kn_compat_session_tcp_line(
	const char *, const char *, const char *, unsigned int,
	struct kn_compat_session_result *);
enum kn_compat_session_error kn_compat_session_validate_endpoint(
	const char *, const char *);

#endif
