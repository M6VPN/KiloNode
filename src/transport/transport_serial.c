/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/transport/transport_serial.c */

#define _POSIX_C_SOURCE 200809L

#include <sys/types.h>

#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include "kilonode/transport.h"
#include "kilonode/transport_serial.h"

static uint8_t baud_to_speed(unsigned int, speed_t *);
static enum kn_transport_error serial_raw_configure(int, unsigned int,
	uint8_t);

static uint8_t
baud_to_speed(unsigned int baud, speed_t *speed)
{
	if (speed == NULL)
		return 0;

	switch (baud) {
	case 1200:
		*speed = B1200;
		return 1;
	case 2400:
		*speed = B2400;
		return 1;
	case 4800:
		*speed = B4800;
		return 1;
	case 9600:
		*speed = B9600;
		return 1;
	case 19200:
		*speed = B19200;
		return 1;
	case 38400:
		*speed = B38400;
		return 1;
	case 57600:
		*speed = B57600;
		return 1;
	case 115200:
		*speed = B115200;
		return 1;
	default:
		return 0;
	}
}

uint8_t
kn_transport_serial_baud_valid(unsigned int baud)
{
	speed_t speed;

	return baud_to_speed(baud, &speed);
}

uint8_t
kn_transport_serial_flow_control_parse(const char *input, uint8_t *enabled)
{
	if (input == NULL || enabled == NULL)
		return 0;

	if (strcmp(input, "on") == 0) {
		*enabled = 1;
		return 1;
	}

	if (strcmp(input, "off") == 0) {
		*enabled = 0;
		return 1;
	}

	return 0;
}

enum kn_transport_error
kn_transport_serial_open(struct kn_transport *transport, const char *device,
	unsigned int baud, uint8_t flow_control)
{
	enum kn_transport_error rc;
	int fd;

	if (transport == NULL || device == NULL)
		return KN_TRANSPORT_ERR_INVALID_ARGUMENT;

	if (device[0] == '\0' || kn_transport_serial_baud_valid(baud) == 0)
		return KN_TRANSPORT_ERR_INVALID_CONFIG;

	kn_transport_reset(transport);
	fd = open(device, O_RDWR | O_NOCTTY);
	if (fd < 0) {
		transport->last_errno = errno;
		return KN_TRANSPORT_ERR_OPEN;
	}

	rc = serial_raw_configure(fd, baud, flow_control);
	if (rc != KN_TRANSPORT_OK) {
		transport->last_errno = errno;
		(void)close(fd);
		return rc;
	}

	transport->kind = KN_TRANSPORT_KIND_SERIAL;
	transport->read_fd = fd;
	transport->write_fd = fd;
	transport->listen_fd = -1;
	transport->open = 1;

	return KN_TRANSPORT_OK;
}

static enum kn_transport_error
serial_raw_configure(int fd, unsigned int baud, uint8_t flow_control)
{
	struct termios tio;
	speed_t speed;

	if (baud_to_speed(baud, &speed) == 0)
		return KN_TRANSPORT_ERR_INVALID_CONFIG;

	if (tcgetattr(fd, &tio) != 0)
		return KN_TRANSPORT_ERR_OPEN;

	tio.c_iflag &= (tcflag_t)~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR |
	    IGNCR | ICRNL | IXON | IXOFF | IXANY);
	tio.c_oflag &= (tcflag_t)~OPOST;
	tio.c_lflag &= (tcflag_t)~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
	tio.c_cflag &= (tcflag_t)~(CSIZE | PARENB | CSTOPB);
	tio.c_cflag |= CS8 | CLOCAL | CREAD;

#ifdef CRTSCTS
	if (flow_control != 0)
		tio.c_cflag |= CRTSCTS;
	else
		tio.c_cflag &= (tcflag_t)~CRTSCTS;
#else
	if (flow_control != 0)
		return KN_TRANSPORT_ERR_INVALID_CONFIG;
#endif

	tio.c_cc[VMIN] = 1;
	tio.c_cc[VTIME] = 0;

	if (cfsetispeed(&tio, speed) != 0 || cfsetospeed(&tio, speed) != 0)
		return KN_TRANSPORT_ERR_INVALID_CONFIG;

	if (tcsetattr(fd, TCSANOW, &tio) != 0)
		return KN_TRANSPORT_ERR_OPEN;

	return KN_TRANSPORT_OK;
}
