/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/transport/transport_pty.c */

#define _XOPEN_SOURCE 600

#include <sys/types.h>

#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

#include "kilonode/transport.h"
#include "kilonode/transport_pty.h"

static enum kn_transport_error pty_raw_configure_slave(const char *);

enum kn_transport_error
kn_transport_pty_open(struct kn_transport *transport, char *slave_path,
	size_t slave_path_len)
{
	enum kn_transport_error rc;
	char *name;
	int master_fd;
	int needed;

	if (transport == NULL || slave_path == NULL || slave_path_len == 0)
		return KN_TRANSPORT_ERR_INVALID_ARGUMENT;

	kn_transport_reset(transport);
	slave_path[0] = '\0';

	master_fd = posix_openpt(O_RDWR | O_NOCTTY);
	if (master_fd < 0) {
		transport->last_errno = errno;
		return KN_TRANSPORT_ERR_OPEN;
	}

	if (grantpt(master_fd) != 0 || unlockpt(master_fd) != 0) {
		transport->last_errno = errno;
		(void)close(master_fd);
		return KN_TRANSPORT_ERR_OPEN;
	}

	name = ptsname(master_fd);
	if (name == NULL) {
		transport->last_errno = errno;
		(void)close(master_fd);
		return KN_TRANSPORT_ERR_OPEN;
	}

	needed = snprintf(slave_path, slave_path_len, "%s", name);
	if (needed < 0 || (size_t)needed >= slave_path_len) {
		(void)close(master_fd);
		return KN_TRANSPORT_ERR_INVALID_CONFIG;
	}

	rc = pty_raw_configure_slave(slave_path);
	if (rc != KN_TRANSPORT_OK) {
		(void)close(master_fd);
		return rc;
	}

	transport->kind = KN_TRANSPORT_KIND_PTY;
	transport->read_fd = master_fd;
	transport->write_fd = master_fd;
	transport->listen_fd = -1;
	transport->open = 1;

	return KN_TRANSPORT_OK;
}

static enum kn_transport_error
pty_raw_configure_slave(const char *path)
{
	struct termios tio;
	int fd;

	fd = open(path, O_RDWR | O_NOCTTY);
	if (fd < 0)
		return KN_TRANSPORT_ERR_OPEN;

	if (tcgetattr(fd, &tio) != 0) {
		(void)close(fd);
		return KN_TRANSPORT_ERR_OPEN;
	}

	tio.c_iflag &= (tcflag_t)~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR |
	    IGNCR | ICRNL | IXON | IXOFF | IXANY);
	tio.c_oflag &= (tcflag_t)~OPOST;
	tio.c_lflag &= (tcflag_t)~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
	tio.c_cflag &= (tcflag_t)~(CSIZE | PARENB | CSTOPB);
	tio.c_cflag |= CS8 | CLOCAL | CREAD;
	tio.c_cc[VMIN] = 1;
	tio.c_cc[VTIME] = 0;

	if (tcsetattr(fd, TCSANOW, &tio) != 0) {
		(void)close(fd);
		return KN_TRANSPORT_ERR_OPEN;
	}

	(void)close(fd);
	return KN_TRANSPORT_OK;
}
