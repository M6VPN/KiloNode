/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_transport.c */

#define _POSIX_C_SOURCE 200809L

#include <sys/types.h>

#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include "kilonode/transport.h"
#include "kilonode/transport_stdio.h"
#include "kilonode/transport_tcp.h"

static int test_eof_read(void);
static int test_invalid_config(void);
static int test_invalid_io_args(void);
static int test_kind_names(void);
static int test_pipe_read_write(void);
static int test_stdio_validation(void);
static int test_tcp_port_validation(void);

int
main(void)
{
	if (test_kind_names() != 0)
		return 1;
	if (test_stdio_validation() != 0)
		return 1;
	if (test_tcp_port_validation() != 0)
		return 1;
	if (test_invalid_config() != 0)
		return 1;
	if (test_invalid_io_args() != 0)
		return 1;
	if (test_pipe_read_write() != 0)
		return 1;
	if (test_eof_read() != 0)
		return 1;

	return 0;
}

static int
test_eof_read(void)
{
	struct kn_transport transport;
	uint8_t buf[8];
	size_t read_len;
	int fds[2];
	enum kn_transport_error rc;

	if (pipe(fds) != 0)
		return 1;

	(void)close(fds[1]);
	kn_transport_reset(&transport);
	transport.kind = KN_TRANSPORT_KIND_STDIO;
	transport.read_fd = fds[0];
	transport.write_fd = -1;
	transport.listen_fd = -1;
	transport.open = 1;

	rc = kn_transport_read(&transport, buf, sizeof(buf), &read_len);
	kn_transport_close(&transport);

	return rc == KN_TRANSPORT_ERR_EOF && read_len == 0 ? 0 : 1;
}

static int
test_invalid_config(void)
{
	struct kn_transport transport;

	if (kn_transport_tcp_connect_open(&transport, "127.0.0.1",
	    "0") != KN_TRANSPORT_ERR_INVALID_CONFIG)
		return 1;

	if (kn_transport_tcp_listen_open(&transport, "127.0.0.1",
	    "bad") != KN_TRANSPORT_ERR_INVALID_CONFIG)
		return 1;

	return 0;
}

static int
test_invalid_io_args(void)
{
	struct kn_transport transport;
	uint8_t buf[4];
	size_t read_len;

	kn_transport_reset(&transport);

	if (kn_transport_read(NULL, buf, sizeof(buf),
	    &read_len) != KN_TRANSPORT_ERR_INVALID_ARGUMENT)
		return 1;
	if (kn_transport_read(&transport, buf, sizeof(buf),
	    &read_len) != KN_TRANSPORT_ERR_NOT_OPEN)
		return 1;
	if (kn_transport_write(NULL, buf,
	    sizeof(buf)) != KN_TRANSPORT_ERR_INVALID_ARGUMENT)
		return 1;
	if (kn_transport_write(&transport, buf,
	    sizeof(buf)) != KN_TRANSPORT_ERR_NOT_OPEN)
		return 1;

	return 0;
}

static int
test_kind_names(void)
{
	if (strcmp(kn_transport_kind_name(KN_TRANSPORT_KIND_STDIO),
	    "stdio") != 0)
		return 1;
	if (strcmp(kn_transport_kind_name(KN_TRANSPORT_KIND_TCP_CLIENT),
	    "tcp-client") != 0)
		return 1;
	if (strcmp(kn_transport_kind_name(KN_TRANSPORT_KIND_TCP_SERVER),
	    "tcp-server") != 0)
		return 1;
	if (kn_transport_kind_valid(KN_TRANSPORT_KIND_NONE) != 0)
		return 1;
	if (kn_transport_kind_valid(KN_TRANSPORT_KIND_STDIO) == 0)
		return 1;

	return 0;
}

static int
test_pipe_read_write(void)
{
	struct kn_transport transport;
	const uint8_t input[] = { 0x01, 0x02, 0x03 };
	uint8_t output[sizeof(input)];
	size_t read_len;
	int fds[2];

	if (pipe(fds) != 0)
		return 1;

	kn_transport_reset(&transport);
	transport.kind = KN_TRANSPORT_KIND_STDIO;
	transport.read_fd = fds[0];
	transport.write_fd = fds[1];
	transport.listen_fd = -1;
	transport.open = 1;

	if (kn_transport_write(&transport, input,
	    sizeof(input)) != KN_TRANSPORT_OK) {
		kn_transport_close(&transport);
		return 1;
	}

	if (kn_transport_read(&transport, output, sizeof(output),
	    &read_len) != KN_TRANSPORT_OK) {
		kn_transport_close(&transport);
		return 1;
	}

	kn_transport_close(&transport);

	if (read_len != sizeof(input))
		return 1;

	return memcmp(input, output, sizeof(input)) == 0 ? 0 : 1;
}

static int
test_stdio_validation(void)
{
	return kn_transport_stdio_open(NULL) ==
	    KN_TRANSPORT_ERR_INVALID_ARGUMENT ? 0 : 1;
}

static int
test_tcp_port_validation(void)
{
	if (kn_transport_tcp_port_valid("1") == 0)
		return 1;
	if (kn_transport_tcp_port_valid("65535") == 0)
		return 1;
	if (kn_transport_tcp_port_valid("0") != 0)
		return 1;
	if (kn_transport_tcp_port_valid("65536") != 0)
		return 1;
	if (kn_transport_tcp_port_valid("abc") != 0)
		return 1;
	if (kn_transport_tcp_port_valid("") != 0)
		return 1;

	return 0;
}
