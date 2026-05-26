/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_bbs_store_lock.c */

#define _XOPEN_SOURCE 700

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <errno.h>
#include <stdio.h>
#include <unistd.h>

#include "kilonode/bbs_store_lock.h"

static void cleanup_store(const char *);
static int make_store(char *, size_t);
static int test_exclusive_lock(void);
static int test_invalid_path(void);
static int test_reacquire_after_release(void);

int
main(void)
{
	if (test_exclusive_lock() != 0)
		return 1;
	if (test_reacquire_after_release() != 0)
		return 1;
	if (test_invalid_path() != 0)
		return 1;
	return 0;
}

static void
cleanup_store(const char *dir)
{
	char path[512];

	(void)snprintf(path, sizeof(path), "%s/.kilonode-store.lock", dir);
	(void)unlink(path);
	(void)rmdir(dir);
}

static int
make_store(char *buf, size_t bufsiz)
{
	unsigned int i;
	int needed;

	for (i = 0; i < 100; i++) {
		needed = snprintf(buf, bufsiz, "/tmp/kilonode-lock-%ld-%u",
		    (long)getpid(), i);
		if (needed < 0 || (size_t)needed >= bufsiz)
			return 1;
		if (mkdir(buf, 0700) == 0)
			return 0;
		if (errno != EEXIST)
			return 1;
	}
	return 1;
}

static int
test_exclusive_lock(void)
{
	struct kn_bbs_store_lock first;
	char dir[256];
	pid_t pid;
	int status;
	int rc;

	if (make_store(dir, sizeof(dir)) != 0)
		return 1;
	kn_bbs_store_lock_init(&first);
	rc = kn_bbs_store_lock_exclusive(&first, dir) ==
	    KN_BBS_STORE_LOCK_OK ? 0 : 1;
	if (rc == 0) {
		pid = fork();
		if (pid == 0) {
			struct kn_bbs_store_lock second;

			kn_bbs_store_lock_init(&second);
			_exit(kn_bbs_store_lock_exclusive(&second, dir) ==
			    KN_BBS_STORE_LOCK_ERR_BUSY ? 0 : 1);
		}
		if (pid < 0 || waitpid(pid, &status, 0) < 0 ||
		    !WIFEXITED(status) || WEXITSTATUS(status) != 0)
			rc = 1;
	}
	kn_bbs_store_lock_release(&first);
	cleanup_store(dir);
	return rc;
}

static int
test_invalid_path(void)
{
	struct kn_bbs_store_lock lock;

	kn_bbs_store_lock_init(&lock);
	return kn_bbs_store_lock_exclusive(&lock, "") ==
	    KN_BBS_STORE_LOCK_ERR_INVALID_ARGUMENT ? 0 : 1;
}

static int
test_reacquire_after_release(void)
{
	struct kn_bbs_store_lock lock;
	char dir[256];
	int rc;

	if (make_store(dir, sizeof(dir)) != 0)
		return 1;
	kn_bbs_store_lock_init(&lock);
	rc = kn_bbs_store_lock_exclusive(&lock, dir) == KN_BBS_STORE_LOCK_OK ?
	    0 : 1;
	kn_bbs_store_lock_release(&lock);
	if (rc == 0)
		rc = kn_bbs_store_lock_exclusive(&lock, dir) ==
		    KN_BBS_STORE_LOCK_OK ? 0 : 1;
	kn_bbs_store_lock_release(&lock);
	cleanup_store(dir);
	return rc;
}
