/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_daemon_config.c */

#include <sys/types.h>

#include <string.h>

#include "kilonode/config.h"
#include "kilonode/transport.h"

static int test_disabled_port_mapping(void);
static int test_max_frame_mapping(void);
static int test_serial_mapping(void);
static int test_tcp_connect_mapping(void);
static int test_tcp_listen_mapping(void);
static int test_unix_listen_mapping(void);

int
main(void)
{
	if (test_tcp_listen_mapping() != 0)
		return 1;
	if (test_tcp_connect_mapping() != 0)
		return 1;
	if (test_serial_mapping() != 0)
		return 1;
	if (test_unix_listen_mapping() != 0)
		return 1;
	if (test_disabled_port_mapping() != 0)
		return 1;
	if (test_max_frame_mapping() != 0)
		return 1;

	return 0;
}

static int
test_disabled_port_mapping(void)
{
	struct kn_config config;
	const char text[] =
		"node {\n"
		"callsign M6VPN-1\n"
		"}\n"
		"port kiss0 {\n"
		"type pty\n"
		"enabled false\n"
		"}\n";

	if (kn_config_parse_text(text, &config) != KN_CONFIG_OK)
		return 1;

	return config.ports[0].enabled == 0 ? 0 : 1;
}

static int
test_max_frame_mapping(void)
{
	struct kn_config config;
	const char text[] =
		"node {\n"
		"callsign M6VPN-1\n"
		"}\n"
		"port kiss0 {\n"
		"type pty\n"
		"max-frame 4096\n"
		"}\n";

	if (kn_config_parse_text(text, &config) != KN_CONFIG_OK)
		return 1;

	return config.ports[0].max_frame == 4096 ? 0 : 1;
}

static int
test_serial_mapping(void)
{
	struct kn_config config;
	const char text[] =
		"node {\n"
		"callsign M6VPN-1\n"
		"}\n"
		"port kiss0 {\n"
		"type serial\n"
		"device /dev/ttyUSB0\n"
		"baud 9600\n"
		"flow-control off\n"
		"}\n";

	if (kn_config_parse_text(text, &config) != KN_CONFIG_OK)
		return 1;
	if (kn_config_port_transport_kind(&config.ports[0]) !=
	    KN_TRANSPORT_KIND_SERIAL)
		return 1;
	if (strcmp(config.ports[0].device, "/dev/ttyUSB0") != 0)
		return 1;

	return config.ports[0].baud == 9600 ? 0 : 1;
}

static int
test_tcp_connect_mapping(void)
{
	struct kn_config config;
	const char text[] =
		"node {\n"
		"callsign M6VPN-1\n"
		"}\n"
		"port kiss0 {\n"
		"type tcp-connect\n"
		"host 127.0.0.1\n"
		"port 8001\n"
		"}\n";

	if (kn_config_parse_text(text, &config) != KN_CONFIG_OK)
		return 1;

	return kn_config_port_transport_kind(&config.ports[0]) ==
	    KN_TRANSPORT_KIND_TCP_CLIENT ? 0 : 1;
}

static int
test_tcp_listen_mapping(void)
{
	struct kn_config config;
	const char text[] =
		"node {\n"
		"callsign M6VPN-1\n"
		"}\n"
		"port kiss0 {\n"
		"type tcp-listen\n"
		"host 127.0.0.1\n"
		"port 8001\n"
		"}\n";

	if (kn_config_parse_text(text, &config) != KN_CONFIG_OK)
		return 1;

	return kn_config_port_transport_kind(&config.ports[0]) ==
	    KN_TRANSPORT_KIND_TCP_SERVER ? 0 : 1;
}

static int
test_unix_listen_mapping(void)
{
	struct kn_config config;
	const char text[] =
		"node {\n"
		"callsign M6VPN-1\n"
		"}\n"
		"port kiss0 {\n"
		"type unix-listen\n"
		"path /tmp/kilonode/kiss.sock\n"
		"}\n";

	if (kn_config_parse_text(text, &config) != KN_CONFIG_OK)
		return 1;
	if (kn_config_port_transport_kind(&config.ports[0]) !=
	    KN_TRANSPORT_KIND_UNIX_SERVER)
		return 1;

	return strcmp(config.ports[0].path, "/tmp/kilonode/kiss.sock") == 0 ?
	    0 : 1;
}
