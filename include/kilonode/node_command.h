/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/node_command.h */

#ifndef KILONODE_NODE_COMMAND_H
#define KILONODE_NODE_COMMAND_H

#include <sys/types.h>

#include <stdint.h>

#define KN_NODE_COMMAND_ARGS_MAX    512
#define KN_NODE_COMMAND_NAME_MAX    16
#define KN_NODE_COMMAND_PREVIEW_MAX 128

enum kn_node_command_error {
	KN_NODE_COMMAND_OK = 0,
	KN_NODE_COMMAND_ERR_INVALID_ARGUMENT,
	KN_NODE_COMMAND_ERR_EMPTY,
	KN_NODE_COMMAND_ERR_OVERLONG,
	KN_NODE_COMMAND_ERR_CONTROL,
	KN_NODE_COMMAND_ERR_BUFFER
};

struct kn_node_command_input {
	char command[KN_NODE_COMMAND_NAME_MAX];
	char args[KN_NODE_COMMAND_ARGS_MAX];
	char preview[KN_NODE_COMMAND_PREVIEW_MAX];
	size_t command_len;
	size_t args_len;
};

void kn_node_command_input_clear(struct kn_node_command_input *);
const char *kn_node_command_error_name(enum kn_node_command_error);
enum kn_node_command_error kn_node_command_parse(const uint8_t *, size_t,
	size_t, struct kn_node_command_input *);

#endif
