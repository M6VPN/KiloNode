/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_transport_unix.c */

#define _POSIX_C_SOURCE 200809L

#include <sys/types.h>
#include <sys/wait.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "kilonode/kiss.h"
#include "kilonode/transport.h"
#include "kilonode/transport_unix.h"

static int child_connect_write(const char *, const uint8_t *, size_t);
static int test_unix_argument_validation(void);
static int test_unix_connect_missing(void);
static int test_unix_loopback_transfer(void);
static int test_unix_path_validation(void);

int
main(void)
{
	if (test_unix_path_validation() != 0)
		return 1;
	if (test_unix_argument_validation() != 0)
		return 1;
	if (test_unix_connect_missing() != 0)
		return 1;
	if (test_unix_loopback_transfer() != 0)
		return 1;

	return 0;
}

static int
child_connect_write(const char *path, const uint8_t *input, size_t input_len)
{
	struct kn_transport transport;
	size_t i;
	enum kn_transport_error rc;

	for (i = 0; i < 200000; i++) {
		rc = kn_transport_unix_connect_open(&transport, path);
		if (rc == KN_TRANSPORT_OK) {
			if (kn_transport_write(&transport, input,
			    input_len) != KN_TRANSPORT_OK) {
				kn_transport_close(&transport);
				return 1;
			}
			kn_transport_close(&transport);
			return 0;
		}
	}

	return 1;
}

static int
test_unix_argument_validation(void)
{
	struct kn_transport transport;

	if (kn_transport_unix_connect_open(NULL,
	    "/tmp/kilonode.sock") != KN_TRANSPORT_ERR_INVALID_ARGUMENT)
		return 1;
	if (kn_transport_unix_connect_open(&transport,
	    NULL) != KN_TRANSPORT_ERR_INVALID_ARGUMENT)
		return 1;
	if (kn_transport_unix_listen_open(NULL,
	    "/tmp/kilonode.sock") != KN_TRANSPORT_ERR_INVALID_ARGUMENT)
		return 1;
	if (kn_transport_unix_listen_open(&transport,
	    NULL) != KN_TRANSPORT_ERR_INVALID_ARGUMENT)
		return 1;

	return 0;
}

static int
test_unix_connect_missing(void)
{
	struct kn_transport transport;
	char path[128];

	(void)snprintf(path, sizeof(path), "/tmp/kilonode-missing-%ld.sock",
	    (long)getpid());
	(void)unlink(path);

	if (kn_transport_unix_connect_open(&transport,
	    path) != KN_TRANSPORT_ERR_CONNECT)
		return 1;

	return 0;
}

static int
test_unix_loopback_transfer(void)
{
	struct kn_transport server;
	const uint8_t input[] = { KN_KISS_FEND, 0x00, 0x7a, KN_KISS_FEND };
	uint8_t output[sizeof(input)];
	char path[128];
	size_t read_len;
	int status;
	pid_t pid;

	(void)snprintf(path, sizeof(path), "/tmp/kilonode-test-%ld.sock",
	    (long)getpid());
	(void)unlink(path);

	pid = fork();
	if (pid < 0)
		return 1;

	if (pid == 0)
		_exit(child_connect_write(path, input, sizeof(input)) == 0 ?
		    0 : 1);

	if (kn_transport_unix_listen_open(&server, path) != KN_TRANSPORT_OK) {
		(void)waitpid(pid, &status, 0);
		(void)unlink(path);
		return 1;
	}

	if (kn_transport_read(&server, output, sizeof(output),
	    &read_len) != KN_TRANSPORT_OK) {
		kn_transport_close(&server);
		(void)waitpid(pid, &status, 0);
		(void)unlink(path);
		return 1;
	}

	kn_transport_close(&server);
	(void)waitpid(pid, &status, 0);
	(void)unlink(path);

	if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
		return 1;
	if (read_len != sizeof(input))
		return 1;

	return memcmp(input, output, sizeof(input)) == 0 ? 0 : 1;
}

static int
test_unix_path_validation(void)
{
	char long_path[200];

	memset(long_path, 'a', sizeof(long_path));
	long_path[sizeof(long_path) - 1] = '\0';

	if (kn_transport_unix_path_valid("/tmp/kilonode.sock") == 0)
		return 1;
	if (kn_transport_unix_path_valid("") != 0)
		return 1;
	if (kn_transport_unix_path_valid(long_path) != 0)
		return 1;

	return 0;
}
