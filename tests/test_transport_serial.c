/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_transport_serial.c */

#include <sys/types.h>

#include <stdint.h>
#include <stddef.h>

#include "kilonode/transport.h"
#include "kilonode/transport_serial.h"

static int test_bad_device_failure(void);
static int test_baud_validation(void);
static int test_flow_control_parse(void);
static int test_serial_argument_validation(void);

int
main(void)
{
	if (test_baud_validation() != 0)
		return 1;
	if (test_flow_control_parse() != 0)
		return 1;
	if (test_serial_argument_validation() != 0)
		return 1;
	if (test_bad_device_failure() != 0)
		return 1;

	return 0;
}

static int
test_bad_device_failure(void)
{
	struct kn_transport transport;

	if (kn_transport_serial_open(&transport,
	    "/tmp/kilonode-missing-serial-device", 9600,
	    0) != KN_TRANSPORT_ERR_OPEN)
		return 1;

	return 0;
}

static int
test_baud_validation(void)
{
	const unsigned int valid[] = {
		1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200
	};
	size_t i;

	for (i = 0; i < sizeof(valid) / sizeof(valid[0]); i++) {
		if (kn_transport_serial_baud_valid(valid[i]) == 0)
			return 1;
	}

	if (kn_transport_serial_baud_valid(300) != 0)
		return 1;
	if (kn_transport_serial_baud_valid(230400) != 0)
		return 1;

	return 0;
}

static int
test_flow_control_parse(void)
{
	uint8_t enabled;

	enabled = 99;
	if (kn_transport_serial_flow_control_parse("on", &enabled) == 0 ||
	    enabled != 1)
		return 1;

	enabled = 99;
	if (kn_transport_serial_flow_control_parse("off", &enabled) == 0 ||
	    enabled != 0)
		return 1;

	if (kn_transport_serial_flow_control_parse("bad", &enabled) != 0)
		return 1;
	if (kn_transport_serial_flow_control_parse(NULL, &enabled) != 0)
		return 1;
	if (kn_transport_serial_flow_control_parse("on", NULL) != 0)
		return 1;

	return 0;
}

static int
test_serial_argument_validation(void)
{
	struct kn_transport transport;

	if (kn_transport_serial_open(NULL, "/dev/null", 9600,
	    0) != KN_TRANSPORT_ERR_INVALID_ARGUMENT)
		return 1;
	if (kn_transport_serial_open(&transport, NULL, 9600,
	    0) != KN_TRANSPORT_ERR_INVALID_ARGUMENT)
		return 1;
	if (kn_transport_serial_open(&transport, "", 9600,
	    0) != KN_TRANSPORT_ERR_INVALID_CONFIG)
		return 1;
	if (kn_transport_serial_open(&transport, "/dev/null", 12345,
	    0) != KN_TRANSPORT_ERR_INVALID_CONFIG)
		return 1;

	return 0;
}
