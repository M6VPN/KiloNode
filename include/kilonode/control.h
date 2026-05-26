/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/control.h */

#ifndef KILONODE_CONTROL_H
#define KILONODE_CONTROL_H

#include <sys/types.h>

#include <stdint.h>

#include "kilonode/heard.h"
#include "kilonode/stats.h"

#define KN_CONTROL_COMMAND_MAX 64
#define KN_CONTROL_LINE_MAX    256
#define KN_CONTROL_QUEUE_MAX   4096

enum kn_control_error {
	KN_CONTROL_OK = 0,
	KN_CONTROL_ERR_INVALID_ARGUMENT,
	KN_CONTROL_ERR_INVALID_PATH,
	KN_CONTROL_ERR_OPEN,
	KN_CONTROL_ERR_CONNECT,
	KN_CONTROL_ERR_IO,
	KN_CONTROL_ERR_OVERLONG_COMMAND,
	KN_CONTROL_ERR_UNKNOWN_COMMAND
};

struct kn_control_snapshot {
	const struct kn_daemon_stats *daemon;
	const struct kn_port_stats *ports;
	size_t port_count;
	const struct kn_heard_entry *heard;
	size_t heard_count;
};

struct kn_control_socket {
	int listen_fd;
	int client_fd;
	char path[KN_CONFIG_PATH_MAX];
};

void kn_control_socket_close(struct kn_control_socket *);
enum kn_control_error kn_control_socket_connect(const char *, int *);
int kn_control_socket_fd(const struct kn_control_socket *);
void kn_control_socket_init(struct kn_control_socket *);
enum kn_control_error kn_control_socket_open(struct kn_control_socket *,
	const char *);
enum kn_control_error kn_control_socket_read_command(struct kn_control_socket *,
	char *, size_t, size_t *);
enum kn_control_error kn_control_socket_write(int, const char *, size_t);
enum kn_control_error kn_control_protocol_handle(const char *,
	const struct kn_control_snapshot *, char *, size_t);
uint8_t kn_control_socket_path_valid(const char *);

#endif
