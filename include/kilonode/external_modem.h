/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/external_modem.h */

#ifndef KILONODE_EXTERNAL_MODEM_H
#define KILONODE_EXTERNAL_MODEM_H

#include <sys/types.h>

#include <stdint.h>

#define KN_EXTERNAL_MODEM_MAX       16
#define KN_EXTERNAL_MODEM_NAME_MAX  32
#define KN_EXTERNAL_MODEM_HOST_MAX  128
#define KN_EXTERNAL_MODEM_NOTES_MAX 128
#define KN_EXTERNAL_MODEM_REASON_MAX 128

enum kn_external_modem_error {
	KN_EXTERNAL_MODEM_OK = 0,
	KN_EXTERNAL_MODEM_ERR_INVALID_ARGUMENT,
	KN_EXTERNAL_MODEM_ERR_INVALID_VALUE,
	KN_EXTERNAL_MODEM_ERR_NOT_FOUND,
	KN_EXTERNAL_MODEM_ERR_DUPLICATE,
	KN_EXTERNAL_MODEM_ERR_FULL,
	KN_EXTERNAL_MODEM_ERR_BUFFER
};

enum kn_external_modem_mode {
	KN_EXTERNAL_MODEM_MODE_NONE = 0,
	KN_EXTERNAL_MODEM_MODE_DISABLED,
	KN_EXTERNAL_MODEM_MODE_STATUS_ONLY,
	KN_EXTERNAL_MODEM_MODE_RECEIVE_ONLY,
	KN_EXTERNAL_MODEM_MODE_PLANNED
};

enum kn_external_modem_state {
	KN_EXTERNAL_MODEM_STATE_DISABLED = 0,
	KN_EXTERNAL_MODEM_STATE_CONFIGURED,
	KN_EXTERNAL_MODEM_STATE_PLANNED,
	KN_EXTERNAL_MODEM_STATE_UNAVAILABLE,
	KN_EXTERNAL_MODEM_STATE_UNKNOWN
};

enum kn_external_modem_type {
	KN_EXTERNAL_MODEM_TYPE_NONE = 0,
	KN_EXTERNAL_MODEM_TYPE_KISS_TCP,
	KN_EXTERNAL_MODEM_TYPE_MERCURY_OFDM,
	KN_EXTERNAL_MODEM_TYPE_VARA_HF,
	KN_EXTERNAL_MODEM_TYPE_VARA_FM,
	KN_EXTERNAL_MODEM_TYPE_ARDOP,
	KN_EXTERNAL_MODEM_TYPE_GENERIC_TCP
};

struct kn_external_modem_config {
	char name[KN_EXTERNAL_MODEM_NAME_MAX];
	enum kn_external_modem_type type;
	enum kn_external_modem_mode mode;
	char host[KN_EXTERNAL_MODEM_HOST_MAX];
	uint16_t port;
	char notes[KN_EXTERNAL_MODEM_NOTES_MAX];
	uint8_t enabled;
	uint8_t auto_start;
	uint8_t tx_enabled;
	uint8_t connect_enabled;
	uint8_t has_type;
	uint8_t has_mode;
	uint8_t has_host;
	uint8_t has_port;
	uint8_t has_notes;
	uint8_t has_enabled;
	uint8_t has_auto_start;
	uint8_t has_tx_enabled;
	uint8_t has_connect_enabled;
};

void kn_external_modem_config_defaults(struct kn_external_modem_config *);
enum kn_external_modem_error kn_external_modem_config_validate(
	const struct kn_external_modem_config *);
const char *kn_external_modem_error_name(enum kn_external_modem_error);
uint8_t kn_external_modem_host_valid(const char *);
uint8_t kn_external_modem_name_valid(const char *);
const char *kn_external_modem_mode_name(enum kn_external_modem_mode);
enum kn_external_modem_mode kn_external_modem_mode_parse(const char *);
const char *kn_external_modem_state_name(enum kn_external_modem_state);
const char *kn_external_modem_type_name(enum kn_external_modem_type);
enum kn_external_modem_type kn_external_modem_type_parse(const char *);

#endif
