/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/bbs/bbs_store_lock.c */

#define _POSIX_C_SOURCE 200809L

#include <sys/stat.h>
#include <sys/types.h>

#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "kilonode/bbs_store_lock.h"

static uint8_t path_safe(const char *);

static uint8_t
path_safe(const char *path)
{
	size_t i;

	if (path == NULL || path[0] == '\0')
		return 0;
	for (i = 0; path[i] != '\0'; i++) {
		if ((unsigned char)path[i] < 0x20 ||
		    (unsigned char)path[i] > 0x7e)
			return 0;
	}
	return 1;
}

enum kn_bbs_store_lock_error
kn_bbs_store_lock_exclusive(struct kn_bbs_store_lock *lock,
	const char *store_path)
{
	struct flock fl;
	struct stat st;
	int needed;

	if (lock == NULL || path_safe(store_path) == 0)
		return KN_BBS_STORE_LOCK_ERR_INVALID_ARGUMENT;
	if (lock->locked != 0)
		return KN_BBS_STORE_LOCK_ERR_INVALID_ARGUMENT;
	if (stat(store_path, &st) != 0 || !S_ISDIR(st.st_mode))
		return KN_BBS_STORE_LOCK_ERR_INVALID_ARGUMENT;

	needed = snprintf(lock->path, sizeof(lock->path),
	    "%s/.kilonode-store.lock", store_path);
	if (needed < 0 || (size_t)needed >= sizeof(lock->path))
		return KN_BBS_STORE_LOCK_ERR_BUFFER;

	lock->fd = open(lock->path, O_RDWR | O_CREAT, 0600);
	if (lock->fd < 0)
		return KN_BBS_STORE_LOCK_ERR_IO;

	memset(&fl, 0, sizeof(fl));
	fl.l_type = F_WRLCK;
	fl.l_whence = SEEK_SET;
	if (fcntl(lock->fd, F_SETLK, &fl) != 0) {
		if (errno == EACCES || errno == EAGAIN) {
			kn_bbs_store_lock_release(lock);
			return KN_BBS_STORE_LOCK_ERR_BUSY;
		}
		kn_bbs_store_lock_release(lock);
		return KN_BBS_STORE_LOCK_ERR_IO;
	}
	lock->locked = 1;
	return KN_BBS_STORE_LOCK_OK;
}

const char *
kn_bbs_store_lock_error_name(enum kn_bbs_store_lock_error error)
{
	switch (error) {
	case KN_BBS_STORE_LOCK_OK:
		return "ok";
	case KN_BBS_STORE_LOCK_ERR_INVALID_ARGUMENT:
		return "invalid argument";
	case KN_BBS_STORE_LOCK_ERR_BUSY:
		return "busy";
	case KN_BBS_STORE_LOCK_ERR_IO:
		return "io";
	case KN_BBS_STORE_LOCK_ERR_BUFFER:
		return "buffer";
	}
	return "unknown";
}

void
kn_bbs_store_lock_init(struct kn_bbs_store_lock *lock)
{
	if (lock == NULL)
		return;
	memset(lock, 0, sizeof(*lock));
	lock->fd = -1;
}

void
kn_bbs_store_lock_release(struct kn_bbs_store_lock *lock)
{
	struct flock fl;

	if (lock == NULL)
		return;
	if (lock->fd >= 0) {
		memset(&fl, 0, sizeof(fl));
		fl.l_type = F_UNLCK;
		fl.l_whence = SEEK_SET;
		(void)fcntl(lock->fd, F_SETLK, &fl);
		(void)close(lock->fd);
	}
	kn_bbs_store_lock_init(lock);
}
