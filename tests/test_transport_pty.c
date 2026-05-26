/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_transport_pty.c */

#define _POSIX_C_SOURCE 200809L

#include <sys/types.h>

#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include "kilonode/kiss.h"
#include "kilonode/transport.h"
#include "kilonode/transport_pty.h"

static int test_pty_argument_validation(void);
static int test_pty_create_and_transfer(void);

int
main(void)
{
	if (test_pty_argument_validation() != 0)
		return 1;
	if (test_pty_create_and_transfer() != 0)
		return 1;

	return 0;
}

static int
test_pty_argument_validation(void)
{
	struct kn_transport transport;
	char path[KN_TRANSPORT_PTY_PATH_MAX];

	if (kn_transport_pty_open(NULL, path,
	    sizeof(path)) != KN_TRANSPORT_ERR_INVALID_ARGUMENT)
		return 1;
	if (kn_transport_pty_open(&transport, NULL,
	    sizeof(path)) != KN_TRANSPORT_ERR_INVALID_ARGUMENT)
		return 1;
	if (kn_transport_pty_open(&transport, path,
	    0) != KN_TRANSPORT_ERR_INVALID_ARGUMENT)
		return 1;

	return 0;
}

static int
test_pty_create_and_transfer(void)
{
	struct kn_transport transport;
	char path[KN_TRANSPORT_PTY_PATH_MAX];
	const uint8_t input[] = { KN_KISS_FEND, 0x00, 0x01, KN_KISS_FEND };
	uint8_t output[sizeof(input)];
	size_t read_len;
	int slave_fd;
	enum kn_transport_error rc;

	rc = kn_transport_pty_open(&transport, path, sizeof(path));
	if (rc == KN_TRANSPORT_ERR_OPEN)
		return 0;
	if (rc != KN_TRANSPORT_OK)
		return 1;
	if (path[0] == '\0') {
		kn_transport_close(&transport);
		return 1;
	}

	slave_fd = open(path, O_RDWR | O_NOCTTY);
	if (slave_fd < 0) {
		kn_transport_close(&transport);
		return 1;
	}

	if (write(slave_fd, input, sizeof(input)) != (ssize_t)sizeof(input)) {
		(void)close(slave_fd);
		kn_transport_close(&transport);
		return 1;
	}

	if (kn_transport_read(&transport, output, sizeof(output),
	    &read_len) != KN_TRANSPORT_OK) {
		(void)close(slave_fd);
		kn_transport_close(&transport);
		return 1;
	}

	(void)close(slave_fd);
	kn_transport_close(&transport);

	if (read_len != sizeof(input))
		return 1;

	return memcmp(input, output, sizeof(input)) == 0 ? 0 : 1;
}
