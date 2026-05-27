/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_rf_command.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "kilonode/config.h"
#include "kilonode/rf_command.h"

static void make_port(struct kn_port_stats *);
static void make_rx(struct kn_rx_event *, const char *);
static int test_eligibility(void);
static int test_parse_invalid(void);
static int test_parse_valid(void);

int
main(void)
{
	if (test_parse_valid() != 0)
		return 1;
	if (test_parse_invalid() != 0)
		return 1;
	if (test_eligibility() != 0)
		return 1;

	return 0;
}

static void
make_port(struct kn_port_stats *port)
{
	memset(port, 0, sizeof(*port));
	(void)snprintf(port->name, sizeof(port->name), "kiss0");
	port->enabled = 1;
	port->open = 1;
}

static void
make_rx(struct kn_rx_event *event, const char *destination)
{
	kn_rx_event_clear(event);
	event->id = 12;
	(void)snprintf(event->port_name, sizeof(event->port_name), "kiss0");
	(void)kn_callsign_parse("N0CALL", &event->source);
	(void)kn_callsign_parse(destination, &event->destination);
	event->kind = KN_RX_FRAME_UI;
	event->has_pid = 1;
	event->pid = KN_AX25_PID_NO_LAYER_3;
}

static int
test_eligibility(void)
{
	const uint8_t payload[] = "PING";
	struct kn_config config;
	struct kn_rx_event rx;
	struct kn_port_stats port;
	struct kn_rf_command_event event;

	kn_config_init(&config);
	(void)kn_callsign_parse("M6VPN-1", &config.node.callsign);
	config.node.has_callsign = 1;
	config.rf_command.enabled = 1;
	make_port(&port);
	make_rx(&rx, "M6VPN-1");

	if (kn_rf_command_from_rx(&event, 1, 10, &config, &port, 1, &rx,
	    payload, 4) != KN_RF_COMMAND_OK)
		return 1;
	if (event.command != KN_RF_COMMAND_PING ||
	    event.status != KN_RF_COMMAND_STATUS_OK)
		return 1;

	make_rx(&rx, "APRS");
	return kn_rf_command_from_rx(&event, 2, 10, &config, &port, 1, &rx,
	    payload, 4) == KN_RF_COMMAND_ERR_IGNORED ? 0 : 1;
}

static int
test_parse_invalid(void)
{
	const uint8_t empty[] = "";
	const uint8_t unknown[] = "NOPE";
	const uint8_t bbs[] = "BBS";
	const uint8_t args[] = "PING NOW";
	const uint8_t binary[] = { 'P', 0x01, 'G' };
	enum kn_rf_command_name command;
	enum kn_rf_command_status status;
	char raw[KN_RF_COMMAND_RAW_MAX];

	if (kn_rf_command_parse(empty, 0, 128, &command, raw, sizeof(raw),
	    &status) != KN_RF_COMMAND_OK ||
	    status != KN_RF_COMMAND_STATUS_EMPTY)
		return 1;
	if (kn_rf_command_parse(unknown, 4, 128, &command, raw, sizeof(raw),
	    &status) != KN_RF_COMMAND_OK ||
	    status != KN_RF_COMMAND_STATUS_UNKNOWN)
		return 1;
	if (kn_rf_command_parse(bbs, 3, 128, &command, raw, sizeof(raw),
	    &status) != KN_RF_COMMAND_OK ||
	    status != KN_RF_COMMAND_STATUS_UNKNOWN)
		return 1;
	if (kn_rf_command_parse(args, 8, 128, &command, raw, sizeof(raw),
	    &status) != KN_RF_COMMAND_OK ||
	    status != KN_RF_COMMAND_STATUS_ARGUMENTS)
		return 1;
	if (kn_rf_command_parse(binary, sizeof(binary), 128, &command, raw,
	    sizeof(raw), &status) != KN_RF_COMMAND_OK ||
	    status != KN_RF_COMMAND_STATUS_BINARY)
		return 1;

	return 0;
}

static int
test_parse_valid(void)
{
	const char *commands[] = { "HELP", "info", " PORTS ", "HEARD",
	    "STATS", "PING" };
	enum kn_rf_command_name expected[] = { KN_RF_COMMAND_HELP,
	    KN_RF_COMMAND_INFO, KN_RF_COMMAND_PORTS, KN_RF_COMMAND_HEARD,
	    KN_RF_COMMAND_STATS, KN_RF_COMMAND_PING };
	enum kn_rf_command_name command;
	enum kn_rf_command_status status;
	char raw[KN_RF_COMMAND_RAW_MAX];
	size_t i;

	for (i = 0; i < sizeof(commands) / sizeof(commands[0]); i++) {
		if (kn_rf_command_parse((const uint8_t *)commands[i],
		    strlen(commands[i]), 128, &command, raw, sizeof(raw),
		    &status) != KN_RF_COMMAND_OK)
			return 1;
		if (command != expected[i] ||
		    status != KN_RF_COMMAND_STATUS_OK)
			return 1;
	}

	return 0;
}
