/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/config.h */

#ifndef KILONODE_CONFIG_H
#define KILONODE_CONFIG_H

#include <sys/types.h>

#include <stdint.h>

#include "kilonode/callsign.h"
#include "kilonode/transport.h"

#define KN_CONFIG_ALIAS_MAX     16
#define KN_CONFIG_ERROR_MAX     160
#define KN_CONFIG_HOST_MAX      128
#define KN_CONFIG_LINE_MAX      512
#define KN_CONFIG_LOCATION_MAX  128
#define KN_CONFIG_MAX_FRAME_MAX 65535
#define KN_CONFIG_MAX_FRAME_MIN 64
#define KN_CONFIG_PATH_MAX      256
#define KN_CONFIG_PORT_MAX      16
#define KN_CONFIG_PORT_NAME_MAX 32

enum kn_config_error {
	KN_CONFIG_OK = 0,
	KN_CONFIG_ERR_INVALID_ARGUMENT,
	KN_CONFIG_ERR_IO,
	KN_CONFIG_ERR_LINE_TOO_LONG,
	KN_CONFIG_ERR_PARSE,
	KN_CONFIG_ERR_UNKNOWN_BLOCK,
	KN_CONFIG_ERR_UNKNOWN_KEY,
	KN_CONFIG_ERR_DUPLICATE_KEY,
	KN_CONFIG_ERR_MISSING_REQUIRED,
	KN_CONFIG_ERR_INVALID_VALUE,
	KN_CONFIG_ERR_DUPLICATE_PORT,
	KN_CONFIG_ERR_TOO_MANY_PORTS
};

enum kn_config_port_type {
	KN_CONFIG_PORT_NONE = 0,
	KN_CONFIG_PORT_STDIO,
	KN_CONFIG_PORT_TCP_CONNECT,
	KN_CONFIG_PORT_TCP_LISTEN,
	KN_CONFIG_PORT_SERIAL,
	KN_CONFIG_PORT_PTY,
	KN_CONFIG_PORT_UNIX_CONNECT,
	KN_CONFIG_PORT_UNIX_LISTEN
};

struct kn_config_node {
	struct kn_callsign callsign;
	char alias[KN_CONFIG_ALIAS_MAX];
	char location[KN_CONFIG_LOCATION_MAX];
	uint8_t has_callsign;
	uint8_t has_alias;
	uint8_t has_location;
};

struct kn_config_control {
	char path[KN_CONFIG_PATH_MAX];
	uint8_t enabled;
	uint8_t has_block;
	uint8_t has_enabled;
	uint8_t has_path;
};

struct kn_config_port {
	char name[KN_CONFIG_PORT_NAME_MAX];
	enum kn_config_port_type type;
	char host[KN_CONFIG_HOST_MAX];
	char port[KN_CONFIG_PORT_NAME_MAX];
	char device[KN_CONFIG_PATH_MAX];
	char path[KN_CONFIG_PATH_MAX];
	unsigned int baud;
	size_t max_frame;
	uint8_t enabled;
	uint8_t flow_control;
	uint8_t has_type;
	uint8_t has_host;
	uint8_t has_port;
	uint8_t has_device;
	uint8_t has_path;
	uint8_t has_baud;
	uint8_t has_max_frame;
	uint8_t has_enabled;
	uint8_t has_flow_control;
};

struct kn_config {
	struct kn_config_node node;
	struct kn_config_control control;
	struct kn_config_port ports[KN_CONFIG_PORT_MAX];
	size_t port_count;
	enum kn_config_error error;
	size_t error_line;
	char error_text[KN_CONFIG_ERROR_MAX];
};

const char *kn_config_error_name(enum kn_config_error);
void kn_config_free(struct kn_config *);
void kn_config_init(struct kn_config *);
enum kn_config_error kn_config_parse_file(const char *, struct kn_config *);
enum kn_config_error kn_config_parse_text(const char *, struct kn_config *);
enum kn_transport_kind kn_config_port_transport_kind(
	const struct kn_config_port *);

#endif
