/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/bbs/bbs_store_maintenance.c */

#define _POSIX_C_SOURCE 200809L

#include <sys/stat.h>
#include <sys/types.h>

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "kilonode/bbs_read_state.h"
#include "kilonode/bbs_store_lock.h"
#include "kilonode/bbs_store_maintenance.h"
#include "kilonode/bbs_user.h"
#include "kilonode/callsign.h"
#include "kilonode/message_index.h"

#define MAINT_BODY_MAX 65536
#define MAINT_COPY_BUFSIZ 8192
#define MAINT_INDEX_LINE_MAX 512
#define MAINT_META_MAX 1024
#define MAINT_ID_MAX 99999999ULL

struct raw_message {
	uint64_t id;
	uint64_t body_len;
	uint8_t type;
	uint8_t deleted;
	char from[KN_CALLSIGN_MAX + 4];
	char dest[KN_MESSAGE_AREA_MAX + 1];
	char subject[KN_MESSAGE_SUBJECT_MAX + 1];
};

static enum kn_bbs_store_maintenance_error add_finding(
	struct kn_bbs_store_finding *, size_t, size_t *,
	enum kn_bbs_store_finding_severity, const char *, const char *,
	const char *, uint64_t);
static enum kn_bbs_store_maintenance_error copy_dir(const char *,
	const char *, const char *);
static enum kn_bbs_store_maintenance_error copy_file(const char *,
	const char *);
static enum kn_bbs_store_maintenance_error dedup_read_dir(const char *,
	struct kn_bbs_store_finding *, size_t, size_t *);
static uint8_t dir_exists(const char *);
static enum kn_bbs_store_maintenance_error ensure_dir(const char *);
static enum kn_bbs_store_maintenance_error export_manifest(const char *,
	const char *, const struct kn_bbs_store_stats *);
static uint8_t file_exists(const char *);
static uint8_t filename_id(const char *, const char *, uint64_t *);
static enum kn_bbs_store_maintenance_error join_path(const char *,
	const char *, char *, size_t);
static enum kn_bbs_store_maintenance_error msg_path(const char *, uint64_t,
	const char *, char *, size_t);
static enum kn_bbs_store_maintenance_error next_id_read_path(const char *,
	uint64_t *);
static enum kn_bbs_store_maintenance_error next_id_write_path(const char *,
	uint64_t);
static enum kn_bbs_store_maintenance_error parse_index_duplicates(
	const char *, struct kn_bbs_store_finding *, size_t, size_t *);
static enum kn_bbs_store_maintenance_error parse_raw_meta_path(const char *,
	struct raw_message *);
static uint8_t path_safe(const char *);
static enum kn_bbs_store_maintenance_error rebuild_indexes(const char *);
static enum kn_bbs_store_maintenance_error read_file(const char *, char *,
	size_t, size_t *);
static enum kn_bbs_store_maintenance_error scan_messages(const char *,
	struct kn_bbs_store_stats *, struct kn_bbs_store_finding *, size_t,
	size_t *, uint64_t *);
static enum kn_bbs_store_maintenance_error scan_read_state(const char *,
	struct kn_bbs_store_finding *, size_t, size_t *);
static enum kn_bbs_store_maintenance_error scan_users(const char *,
	struct kn_bbs_store_finding *, size_t, size_t *,
	struct kn_bbs_store_stats *);
static uint8_t store_child_ok(const char *);
static enum kn_bbs_store_maintenance_error write_read_state_ids(const char *,
	uint64_t *, size_t);

static enum kn_bbs_store_maintenance_error
add_finding(struct kn_bbs_store_finding *findings, size_t max_findings,
	size_t *count, enum kn_bbs_store_finding_severity severity,
	const char *code, const char *path, const char *message, uint64_t id)
{
	struct kn_bbs_store_finding *finding;
	int needed;

	if (count == NULL)
		return KN_BBS_STORE_MAINTENANCE_ERR_INVALID_ARGUMENT;
	if (*count < max_findings && findings != NULL) {
		finding = &findings[*count];
		memset(finding, 0, sizeof(*finding));
		finding->severity = severity;
		finding->message_id = id;
		needed = snprintf(finding->code, sizeof(finding->code), "%s",
		    code);
		if (needed < 0 || (size_t)needed >= sizeof(finding->code))
			return KN_BBS_STORE_MAINTENANCE_ERR_BUFFER;
		needed = snprintf(finding->path, sizeof(finding->path), "%s",
		    path == NULL ? "" : path);
		if (needed < 0 || (size_t)needed >= sizeof(finding->path))
			return KN_BBS_STORE_MAINTENANCE_ERR_BUFFER;
		needed = snprintf(finding->message, sizeof(finding->message),
		    "%s", message);
		if (needed < 0 || (size_t)needed >= sizeof(finding->message))
			return KN_BBS_STORE_MAINTENANCE_ERR_BUFFER;
	}
	(*count)++;
	return KN_BBS_STORE_MAINTENANCE_OK;
}

static enum kn_bbs_store_maintenance_error
copy_dir(const char *source_root, const char *dest_root, const char *name)
{
	struct dirent *entry;
	DIR *dir;
	char source[KN_MESSAGE_STORE_PATH_MAX];
	char dest[KN_MESSAGE_STORE_PATH_MAX];
	char source_file[KN_MESSAGE_STORE_PATH_MAX];
	char dest_file[KN_MESSAGE_STORE_PATH_MAX];
	struct stat st;
	enum kn_bbs_store_maintenance_error rc;

	rc = join_path(source_root, name, source, sizeof(source));
	if (rc != KN_BBS_STORE_MAINTENANCE_OK)
		return rc;
	rc = join_path(dest_root, name, dest, sizeof(dest));
	if (rc != KN_BBS_STORE_MAINTENANCE_OK)
		return rc;
	rc = ensure_dir(dest);
	if (rc != KN_BBS_STORE_MAINTENANCE_OK)
		return rc;
	dir = opendir(source);
	if (dir == NULL)
		return KN_BBS_STORE_MAINTENANCE_ERR_IO;
	while ((entry = readdir(dir)) != NULL) {
		if (entry->d_name[0] == '.')
			continue;
		if (store_child_ok(entry->d_name) == 0) {
			(void)closedir(dir);
			return KN_BBS_STORE_MAINTENANCE_ERR_CORRUPT;
		}
		rc = join_path(source, entry->d_name, source_file,
		    sizeof(source_file));
		if (rc != KN_BBS_STORE_MAINTENANCE_OK) {
			(void)closedir(dir);
			return rc;
		}
		rc = join_path(dest, entry->d_name, dest_file,
		    sizeof(dest_file));
		if (rc != KN_BBS_STORE_MAINTENANCE_OK) {
			(void)closedir(dir);
			return rc;
		}
		if (lstat(source_file, &st) != 0) {
			(void)closedir(dir);
			return KN_BBS_STORE_MAINTENANCE_ERR_IO;
		}
		if (!S_ISREG(st.st_mode))
			continue;
		rc = copy_file(source_file, dest_file);
		if (rc != KN_BBS_STORE_MAINTENANCE_OK) {
			(void)closedir(dir);
			return rc;
		}
	}
	(void)closedir(dir);
	return KN_BBS_STORE_MAINTENANCE_OK;
}

static enum kn_bbs_store_maintenance_error
copy_file(const char *source, const char *dest)
{
	uint8_t buf[MAINT_COPY_BUFSIZ];
	ssize_t nread;
	ssize_t nwritten;
	size_t done;
	int in_fd;
	int out_fd;

	in_fd = open(source, O_RDONLY);
	if (in_fd < 0)
		return KN_BBS_STORE_MAINTENANCE_ERR_IO;
	out_fd = open(dest, O_WRONLY | O_CREAT | O_EXCL, 0600);
	if (out_fd < 0) {
		(void)close(in_fd);
		return KN_BBS_STORE_MAINTENANCE_ERR_IO;
	}
	for (;;) {
		nread = read(in_fd, buf, sizeof(buf));
		if (nread == 0)
			break;
		if (nread < 0) {
			if (errno == EINTR)
				continue;
			(void)close(in_fd);
			(void)close(out_fd);
			return KN_BBS_STORE_MAINTENANCE_ERR_IO;
		}
		done = 0;
		while (done < (size_t)nread) {
			nwritten = write(out_fd, buf + done,
			    (size_t)nread - done);
			if (nwritten > 0) {
				done += (size_t)nwritten;
				continue;
			}
			if (nwritten < 0 && errno == EINTR)
				continue;
			(void)close(in_fd);
			(void)close(out_fd);
			return KN_BBS_STORE_MAINTENANCE_ERR_IO;
		}
	}
	(void)fsync(out_fd);
	if (close(in_fd) != 0 || close(out_fd) != 0)
		return KN_BBS_STORE_MAINTENANCE_ERR_IO;
	return KN_BBS_STORE_MAINTENANCE_OK;
}

static enum kn_bbs_store_maintenance_error
dedup_read_dir(const char *path, struct kn_bbs_store_finding *findings,
	size_t max_findings, size_t *count)
{
	struct dirent *entry;
	DIR *dir;
	char read_dir[KN_MESSAGE_STORE_PATH_MAX];
	char file[KN_MESSAGE_STORE_PATH_MAX];
	char line[64];
	uint64_t ids[KN_BBS_READ_STATE_MAX];
	char *end;
	unsigned long long id;
	size_t id_count;
	size_t i;
	uint8_t changed;
	FILE *fp;
	enum kn_bbs_store_maintenance_error rc;

	rc = join_path(path, "read", read_dir, sizeof(read_dir));
	if (rc != KN_BBS_STORE_MAINTENANCE_OK)
		return rc;
	dir = opendir(read_dir);
	if (dir == NULL)
		return KN_BBS_STORE_MAINTENANCE_ERR_IO;
	while ((entry = readdir(dir)) != NULL) {
		if (entry->d_name[0] == '.')
			continue;
		rc = join_path(read_dir, entry->d_name, file, sizeof(file));
		if (rc != KN_BBS_STORE_MAINTENANCE_OK) {
			(void)closedir(dir);
			return rc;
		}
		fp = fopen(file, "r");
		if (fp == NULL)
			continue;
		id_count = 0;
		changed = 0;
		while (fgets(line, sizeof(line), fp) != NULL) {
			errno = 0;
			id = strtoull(line, &end, 10);
			if (errno != 0 || id == 0)
				continue;
			for (i = 0; i < id_count; i++) {
				if (ids[i] == (uint64_t)id)
					break;
			}
			if (i < id_count) {
				changed = 1;
				continue;
			}
			if (id_count < KN_BBS_READ_STATE_MAX)
				ids[id_count++] = (uint64_t)id;
		}
		(void)fclose(fp);
		if (changed != 0) {
			rc = write_read_state_ids(file, ids, id_count);
			if (rc != KN_BBS_STORE_MAINTENANCE_OK) {
				(void)closedir(dir);
				return rc;
			}
			rc = add_finding(findings, max_findings, count,
			    KN_BBS_STORE_FINDING_INFO, "read-dedup", file,
			    "deduplicated read-state file", 0);
			if (rc != KN_BBS_STORE_MAINTENANCE_OK) {
				(void)closedir(dir);
				return rc;
			}
		}
	}
	(void)closedir(dir);
	return KN_BBS_STORE_MAINTENANCE_OK;
}

static uint8_t
dir_exists(const char *path)
{
	struct stat st;

	return stat(path, &st) == 0 && S_ISDIR(st.st_mode) ? 1 : 0;
}

static enum kn_bbs_store_maintenance_error
ensure_dir(const char *path)
{
	struct stat st;

	if (mkdir(path, 0700) == 0)
		return KN_BBS_STORE_MAINTENANCE_OK;
	if (errno != EEXIST)
		return KN_BBS_STORE_MAINTENANCE_ERR_IO;
	if (stat(path, &st) != 0 || !S_ISDIR(st.st_mode))
		return KN_BBS_STORE_MAINTENANCE_ERR_IO;
	return KN_BBS_STORE_MAINTENANCE_OK;
}

static enum kn_bbs_store_maintenance_error
export_manifest(const char *source, const char *dest,
	const struct kn_bbs_store_stats *stats)
{
	char path[KN_MESSAGE_STORE_PATH_MAX];
	FILE *fp;
	enum kn_bbs_store_maintenance_error rc;

	rc = join_path(dest, "manifest.txt", path, sizeof(path));
	if (rc != KN_BBS_STORE_MAINTENANCE_OK)
		return rc;
	fp = fopen(path, "wx");
	if (fp == NULL)
		return KN_BBS_STORE_MAINTENANCE_ERR_IO;
	if (fprintf(fp,
	    "format KiloNode-store-export-v1\n"
	    "source %s\n"
	    "messages %llu\n"
	    "private %llu\n"
	    "bulletins %llu\n"
	    "deleted %llu\n"
	    "users %llu\n"
	    "read-state-files %llu\n"
	    "areas %llu\n"
	    "body-bytes %llu\n"
	    "newest-message-id %llu\n"
	    "next-id %llu\n",
	    source, (unsigned long long)stats->total_messages,
	    (unsigned long long)stats->private_messages,
	    (unsigned long long)stats->bulletins,
	    (unsigned long long)stats->deleted_messages,
	    (unsigned long long)stats->users,
	    (unsigned long long)stats->read_state_files,
	    (unsigned long long)stats->bulletin_areas,
	    (unsigned long long)stats->total_body_bytes,
	    (unsigned long long)stats->newest_message_id,
	    (unsigned long long)stats->next_id) < 0) {
		(void)fclose(fp);
		return KN_BBS_STORE_MAINTENANCE_ERR_IO;
	}
	return fclose(fp) == 0 ? KN_BBS_STORE_MAINTENANCE_OK :
	    KN_BBS_STORE_MAINTENANCE_ERR_IO;
}

static uint8_t
file_exists(const char *path)
{
	struct stat st;

	return stat(path, &st) == 0 && S_ISREG(st.st_mode) ? 1 : 0;
}

static uint8_t
filename_id(const char *name, const char *suffix, uint64_t *id_out)
{
	char digits[9];
	char *end;
	size_t suffix_len;
	size_t i;
	unsigned long value;

	suffix_len = strlen(suffix);
	if (strlen(name) != 8 + suffix_len)
		return 0;
	for (i = 0; i < 8; i++) {
		if (name[i] < '0' || name[i] > '9')
			return 0;
		digits[i] = name[i];
	}
	digits[8] = '\0';
	if (strcmp(name + 8, suffix) != 0)
		return 0;
	errno = 0;
	value = strtoul(digits, &end, 10);
	if (errno != 0 || *end != '\0' || value == 0)
		return 0;
	*id_out = (uint64_t)value;
	return 1;
}

static enum kn_bbs_store_maintenance_error
join_path(const char *base, const char *name, char *buf, size_t bufsiz)
{
	int needed;

	if (base == NULL || name == NULL || buf == NULL || bufsiz == 0 ||
	    path_safe(base) == 0 || store_child_ok(name) == 0)
		return KN_BBS_STORE_MAINTENANCE_ERR_INVALID_ARGUMENT;
	needed = snprintf(buf, bufsiz, "%s/%s", base, name);
	if (needed < 0 || (size_t)needed >= bufsiz)
		return KN_BBS_STORE_MAINTENANCE_ERR_BUFFER;
	return KN_BBS_STORE_MAINTENANCE_OK;
}

static enum kn_bbs_store_maintenance_error
msg_path(const char *root, uint64_t id, const char *suffix, char *buf,
	size_t bufsiz)
{
	char name[32];
	char msg_dir[KN_MESSAGE_STORE_PATH_MAX];
	int needed;
	enum kn_bbs_store_maintenance_error rc;

	if (id == 0 || id > MAINT_ID_MAX)
		return KN_BBS_STORE_MAINTENANCE_ERR_INVALID_ARGUMENT;
	needed = snprintf(name, sizeof(name), "%08llu.%s",
	    (unsigned long long)id, suffix);
	if (needed < 0 || (size_t)needed >= sizeof(name))
		return KN_BBS_STORE_MAINTENANCE_ERR_BUFFER;
	rc = join_path(root, "msg", msg_dir, sizeof(msg_dir));
	if (rc != KN_BBS_STORE_MAINTENANCE_OK)
		return rc;
	return join_path(msg_dir, name, buf, bufsiz);
}

static enum kn_bbs_store_maintenance_error
next_id_read_path(const char *path, uint64_t *next_id)
{
	char meta[KN_MESSAGE_STORE_PATH_MAX];
	char next[KN_MESSAGE_STORE_PATH_MAX];
	char buf[64];
	char *end;
	size_t len;
	unsigned long long value;
	enum kn_bbs_store_maintenance_error rc;

	rc = join_path(path, "meta", meta, sizeof(meta));
	if (rc != KN_BBS_STORE_MAINTENANCE_OK)
		return rc;
	rc = join_path(meta, "next-id", next, sizeof(next));
	if (rc != KN_BBS_STORE_MAINTENANCE_OK)
		return rc;
	rc = read_file(next, buf, sizeof(buf), &len);
	if (rc != KN_BBS_STORE_MAINTENANCE_OK)
		return rc;
	buf[len] = '\0';
	errno = 0;
	value = strtoull(buf, &end, 10);
	if (errno != 0 || value == 0 || value > MAINT_ID_MAX)
		return KN_BBS_STORE_MAINTENANCE_ERR_CORRUPT;
	if (*end == '\n')
		end++;
	if (*end != '\0')
		return KN_BBS_STORE_MAINTENANCE_ERR_CORRUPT;
	*next_id = (uint64_t)value;
	return KN_BBS_STORE_MAINTENANCE_OK;
}

static enum kn_bbs_store_maintenance_error
next_id_write_path(const char *path, uint64_t next_id)
{
	char meta[KN_MESSAGE_STORE_PATH_MAX];
	char next[KN_MESSAGE_STORE_PATH_MAX];
	char tmp[KN_MESSAGE_STORE_PATH_MAX + 8];
	char buf[64];
	FILE *fp;
	int needed;
	enum kn_bbs_store_maintenance_error rc;

	rc = join_path(path, "meta", meta, sizeof(meta));
	if (rc != KN_BBS_STORE_MAINTENANCE_OK)
		return rc;
	rc = join_path(meta, "next-id", next, sizeof(next));
	if (rc != KN_BBS_STORE_MAINTENANCE_OK)
		return rc;
	needed = snprintf(tmp, sizeof(tmp), "%s.tmp", next);
	if (needed < 0 || (size_t)needed >= sizeof(tmp))
		return KN_BBS_STORE_MAINTENANCE_ERR_BUFFER;
	needed = snprintf(buf, sizeof(buf), "%llu\n",
	    (unsigned long long)next_id);
	if (needed < 0 || (size_t)needed >= sizeof(buf))
		return KN_BBS_STORE_MAINTENANCE_ERR_BUFFER;
	fp = fopen(tmp, "w");
	if (fp == NULL)
		return KN_BBS_STORE_MAINTENANCE_ERR_IO;
	if (fwrite(buf, 1, (size_t)needed, fp) != (size_t)needed ||
	    fflush(fp) != 0 || fsync(fileno(fp)) != 0 || fclose(fp) != 0) {
		(void)unlink(tmp);
		return KN_BBS_STORE_MAINTENANCE_ERR_IO;
	}
	if (rename(tmp, next) != 0) {
		(void)unlink(tmp);
		return KN_BBS_STORE_MAINTENANCE_ERR_IO;
	}
	return KN_BBS_STORE_MAINTENANCE_OK;
}

static enum kn_bbs_store_maintenance_error
parse_index_duplicates(const char *path, struct kn_bbs_store_finding *findings,
	size_t max_findings, size_t *count)
{
	uint64_t ids[4096];
	FILE *fp;
	char line[MAINT_INDEX_LINE_MAX];
	char *end;
	unsigned long long id;
	size_t id_count;
	size_t i;
	enum kn_bbs_store_maintenance_error rc;

	fp = fopen(path, "r");
	if (fp == NULL)
		return KN_BBS_STORE_MAINTENANCE_ERR_IO;
	id_count = 0;
	while (fgets(line, sizeof(line), fp) != NULL) {
		if (strchr(line, '\n') == NULL) {
			(void)fclose(fp);
			return add_finding(findings, max_findings, count,
			    KN_BBS_STORE_FINDING_ERROR, "index-corrupt", path,
			    "index line is malformed", 0);
		}
		errno = 0;
		id = strtoull(line, &end, 10);
		if (errno != 0 || id == 0 || *end != '|') {
			(void)fclose(fp);
			return add_finding(findings, max_findings, count,
			    KN_BBS_STORE_FINDING_ERROR, "index-corrupt", path,
			    "index entry is malformed", 0);
		}
		for (i = 0; i < id_count; i++) {
			if (ids[i] == (uint64_t)id) {
				rc = add_finding(findings, max_findings, count,
				    KN_BBS_STORE_FINDING_WARNING,
				    "index-duplicate", path,
				    "index contains duplicate message ID",
				    (uint64_t)id);
				if (rc != KN_BBS_STORE_MAINTENANCE_OK) {
					(void)fclose(fp);
					return rc;
				}
				break;
			}
		}
		if (i == id_count && id_count < 4096)
			ids[id_count++] = (uint64_t)id;
	}
	if (ferror(fp)) {
		(void)fclose(fp);
		return KN_BBS_STORE_MAINTENANCE_ERR_IO;
	}
	(void)fclose(fp);
	return KN_BBS_STORE_MAINTENANCE_OK;
}

static enum kn_bbs_store_maintenance_error
parse_raw_meta_path(const char *path, struct raw_message *message)
{
	char buf[MAINT_META_MAX];
	char copy[MAINT_META_MAX];
	char *line;
	char *saveptr;
	char *value;
	char *end;
	unsigned long long number;
	size_t len;
	uint8_t have_id;
	uint8_t have_type;
	uint8_t have_from;
	uint8_t have_dest;
	uint8_t have_subject;
	uint8_t have_body;
	enum kn_bbs_store_maintenance_error rc;
	struct kn_callsign call;

	rc = read_file(path, buf, sizeof(buf), &len);
	if (rc != KN_BBS_STORE_MAINTENANCE_OK)
		return rc;
	memset(message, 0, sizeof(*message));
	memcpy(copy, buf, len + 1);
	have_id = 0;
	have_type = 0;
	have_from = 0;
	have_dest = 0;
	have_subject = 0;
	have_body = 0;
	line = strtok_r(copy, "\n", &saveptr);
	while (line != NULL) {
		value = strchr(line, ' ');
		if (value == NULL)
			return KN_BBS_STORE_MAINTENANCE_ERR_CORRUPT;
		*value++ = '\0';
		if (strcmp(line, "id") == 0 || strcmp(line, "body-length") == 0) {
			errno = 0;
			number = strtoull(value, &end, 10);
			if (errno != 0 || *end != '\0')
				return KN_BBS_STORE_MAINTENANCE_ERR_CORRUPT;
			if (strcmp(line, "id") == 0) {
				message->id = (uint64_t)number;
				have_id = 1;
			} else {
				message->body_len = (uint64_t)number;
				have_body = 1;
			}
		} else if (strcmp(line, "type") == 0) {
			if (strcmp(value, "private") == 0)
				message->type = KN_MESSAGE_TYPE_PRIVATE;
			else if (strcmp(value, "bulletin") == 0)
				message->type = KN_MESSAGE_TYPE_BULLETIN;
			else
				return KN_BBS_STORE_MAINTENANCE_ERR_CORRUPT;
			have_type = 1;
		} else if (strcmp(line, "from") == 0) {
			if (kn_callsign_parse(value, &call) != 0)
				return KN_BBS_STORE_MAINTENANCE_ERR_CORRUPT;
			memcpy(message->from, value, strlen(value) + 1);
			have_from = 1;
		} else if (strcmp(line, "to") == 0) {
			if (strlen(value) >= sizeof(message->dest))
				return KN_BBS_STORE_MAINTENANCE_ERR_CORRUPT;
			memcpy(message->dest, value, strlen(value) + 1);
			have_dest = 1;
		} else if (strcmp(line, "subject") == 0) {
			if (strlen(value) == 0 || strlen(value) >=
			    sizeof(message->subject))
				return KN_BBS_STORE_MAINTENANCE_ERR_CORRUPT;
			memcpy(message->subject, value, strlen(value) + 1);
			have_subject = 1;
		} else if (strcmp(line, "deleted") == 0) {
			if (strcmp(value, "0") != 0 && strcmp(value, "1") != 0)
				return KN_BBS_STORE_MAINTENANCE_ERR_CORRUPT;
			message->deleted = (uint8_t)(value[0] - '0');
		}
		line = strtok_r(NULL, "\n", &saveptr);
	}
	if (have_id == 0 || have_type == 0 || have_from == 0 ||
	    have_dest == 0 || have_subject == 0 || have_body == 0)
		return KN_BBS_STORE_MAINTENANCE_ERR_CORRUPT;
	if (message->type == KN_MESSAGE_TYPE_PRIVATE &&
	    kn_callsign_parse(message->dest, &call) != 0)
		return KN_BBS_STORE_MAINTENANCE_ERR_CORRUPT;
	if (message->type == KN_MESSAGE_TYPE_BULLETIN &&
	    kn_message_area_valid(message->dest) == 0)
		return KN_BBS_STORE_MAINTENANCE_ERR_CORRUPT;
	return KN_BBS_STORE_MAINTENANCE_OK;
}

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

static enum kn_bbs_store_maintenance_error
rebuild_indexes(const char *path)
{
	struct kn_message_store store;
	enum kn_bbs_store_maintenance_error rc;

	kn_message_store_init(&store);
	if (kn_message_store_open(&store, path, MAINT_BODY_MAX) !=
	    KN_MESSAGE_STORE_OK)
		return KN_BBS_STORE_MAINTENANCE_ERR_CORRUPT;
	rc = kn_message_index_rebuild(&store) == KN_MESSAGE_INDEX_OK ?
	    KN_BBS_STORE_MAINTENANCE_OK : KN_BBS_STORE_MAINTENANCE_ERR_IO;
	kn_message_store_close(&store);
	return rc;
}

static enum kn_bbs_store_maintenance_error
read_file(const char *path, char *buf, size_t bufsiz, size_t *len_out)
{
	FILE *fp;
	size_t len;

	if (bufsiz == 0)
		return KN_BBS_STORE_MAINTENANCE_ERR_INVALID_ARGUMENT;
	fp = fopen(path, "r");
	if (fp == NULL)
		return KN_BBS_STORE_MAINTENANCE_ERR_IO;
	len = fread(buf, 1, bufsiz - 1, fp);
	if (ferror(fp)) {
		(void)fclose(fp);
		return KN_BBS_STORE_MAINTENANCE_ERR_IO;
	}
	if (len == bufsiz - 1 && fgetc(fp) != EOF) {
		(void)fclose(fp);
		return KN_BBS_STORE_MAINTENANCE_ERR_BUFFER;
	}
	(void)fclose(fp);
	buf[len] = '\0';
	*len_out = len;
	return KN_BBS_STORE_MAINTENANCE_OK;
}

static enum kn_bbs_store_maintenance_error
scan_messages(const char *path, struct kn_bbs_store_stats *stats,
	struct kn_bbs_store_finding *findings, size_t max_findings,
	size_t *count, uint64_t *max_id)
{
	struct dirent *entry;
	DIR *dir;
	char msg_dir[KN_MESSAGE_STORE_PATH_MAX];
	char file[KN_MESSAGE_STORE_PATH_MAX];
	char body[KN_MESSAGE_STORE_PATH_MAX];
	struct stat st;
	struct raw_message msg;
	uint64_t id;
	enum kn_bbs_store_maintenance_error rc;

	*max_id = 0;
	rc = join_path(path, "msg", msg_dir, sizeof(msg_dir));
	if (rc != KN_BBS_STORE_MAINTENANCE_OK)
		return rc;
	dir = opendir(msg_dir);
	if (dir == NULL)
		return KN_BBS_STORE_MAINTENANCE_OK;
	while ((entry = readdir(dir)) != NULL) {
		if (entry->d_name[0] == '.')
			continue;
		rc = join_path(msg_dir, entry->d_name, file, sizeof(file));
		if (rc != KN_BBS_STORE_MAINTENANCE_OK) {
			(void)closedir(dir);
			return rc;
		}
		if (filename_id(entry->d_name, ".body", &id) != 0)
			continue;
		if (filename_id(entry->d_name, ".meta", &id) == 0) {
			rc = add_finding(findings, max_findings, count,
			    KN_BBS_STORE_FINDING_ERROR, "invalid-filename",
			    file, "invalid message filename", 0);
			if (rc != KN_BBS_STORE_MAINTENANCE_OK) {
				(void)closedir(dir);
				return rc;
			}
			continue;
		}
		if (id > *max_id)
			*max_id = id;
		rc = parse_raw_meta_path(file, &msg);
		if (rc != KN_BBS_STORE_MAINTENANCE_OK) {
			rc = add_finding(findings, max_findings, count,
			    KN_BBS_STORE_FINDING_ERROR, "metadata-corrupt",
			    file, "message metadata is corrupt", id);
			if (rc != KN_BBS_STORE_MAINTENANCE_OK) {
				(void)closedir(dir);
				return rc;
			}
			continue;
		}
		stats->total_messages++;
		if (msg.id > stats->newest_message_id)
			stats->newest_message_id = msg.id;
		if (msg.type == KN_MESSAGE_TYPE_PRIVATE)
			stats->private_messages++;
		else
			stats->bulletins++;
		if (msg.deleted != 0)
			stats->deleted_messages++;
		rc = msg_path(path, id, "body", body, sizeof(body));
		if (rc != KN_BBS_STORE_MAINTENANCE_OK) {
			(void)closedir(dir);
			return rc;
		}
		if (stat(body, &st) != 0) {
			if (msg.deleted == 0) {
				rc = add_finding(findings, max_findings, count,
				    KN_BBS_STORE_FINDING_ERROR,
				    "body-missing", body,
				    "message body is missing", id);
				if (rc != KN_BBS_STORE_MAINTENANCE_OK) {
					(void)closedir(dir);
					return rc;
				}
			}
			continue;
		}
		if ((uint64_t)st.st_size != msg.body_len) {
			rc = add_finding(findings, max_findings, count,
			    KN_BBS_STORE_FINDING_ERROR, "body-length",
			    body, "message body length mismatch", id);
			if (rc != KN_BBS_STORE_MAINTENANCE_OK) {
				(void)closedir(dir);
				return rc;
			}
		}
		stats->total_body_bytes += (uint64_t)st.st_size;
	}
	(void)closedir(dir);
	return KN_BBS_STORE_MAINTENANCE_OK;
}

static enum kn_bbs_store_maintenance_error
scan_read_state(const char *path, struct kn_bbs_store_finding *findings,
	size_t max_findings, size_t *count)
{
	struct dirent *entry;
	DIR *dir;
	char read_dir[KN_MESSAGE_STORE_PATH_MAX];
	char file[KN_MESSAGE_STORE_PATH_MAX];
	char line[64];
	uint64_t ids[KN_BBS_READ_STATE_MAX];
	char call[KN_CALLSIGN_MAX + 4];
	char *end;
	unsigned long long id;
	size_t id_count;
	size_t name_len;
	size_t i;
	FILE *fp;
	struct kn_callsign parsed;
	enum kn_bbs_store_maintenance_error rc;

	rc = join_path(path, "read", read_dir, sizeof(read_dir));
	if (rc != KN_BBS_STORE_MAINTENANCE_OK)
		return rc;
	dir = opendir(read_dir);
	if (dir == NULL)
		return KN_BBS_STORE_MAINTENANCE_OK;
	while ((entry = readdir(dir)) != NULL) {
		if (entry->d_name[0] == '.')
			continue;
		name_len = strlen(entry->d_name);
		if (name_len <= 5 || strcmp(entry->d_name + name_len - 5,
		    ".read") != 0 || name_len - 5 >= sizeof(call)) {
			rc = add_finding(findings, max_findings, count,
			    KN_BBS_STORE_FINDING_ERROR, "read-name",
			    entry->d_name, "read-state filename is invalid", 0);
			if (rc != KN_BBS_STORE_MAINTENANCE_OK) {
				(void)closedir(dir);
				return rc;
			}
			continue;
		}
		memcpy(call, entry->d_name, name_len - 5);
		call[name_len - 5] = '\0';
		if (kn_callsign_parse(call, &parsed) != 0) {
			rc = add_finding(findings, max_findings, count,
			    KN_BBS_STORE_FINDING_ERROR, "read-callsign",
			    entry->d_name, "read-state callsign is invalid", 0);
			if (rc != KN_BBS_STORE_MAINTENANCE_OK) {
				(void)closedir(dir);
				return rc;
			}
			continue;
		}
		rc = join_path(read_dir, entry->d_name, file, sizeof(file));
		if (rc != KN_BBS_STORE_MAINTENANCE_OK) {
			(void)closedir(dir);
			return rc;
		}
		fp = fopen(file, "r");
		if (fp == NULL)
			continue;
		id_count = 0;
		while (fgets(line, sizeof(line), fp) != NULL) {
			errno = 0;
			id = strtoull(line, &end, 10);
			if (errno != 0 || id == 0 || (*end != '\n' &&
			    *end != '\0')) {
				rc = add_finding(findings, max_findings, count,
				    KN_BBS_STORE_FINDING_ERROR,
				    "read-corrupt", file,
				    "read-state entry is corrupt", 0);
				if (rc != KN_BBS_STORE_MAINTENANCE_OK) {
					(void)fclose(fp);
					(void)closedir(dir);
					return rc;
				}
				break;
			}
			for (i = 0; i < id_count; i++) {
				if (ids[i] == (uint64_t)id)
					break;
			}
			if (i < id_count) {
				rc = add_finding(findings, max_findings, count,
				    KN_BBS_STORE_FINDING_WARNING,
				    "read-duplicate", file,
				    "read-state has duplicate message ID",
				    (uint64_t)id);
				if (rc != KN_BBS_STORE_MAINTENANCE_OK) {
					(void)fclose(fp);
					(void)closedir(dir);
					return rc;
				}
			} else if (id_count < KN_BBS_READ_STATE_MAX) {
				ids[id_count++] = (uint64_t)id;
			}
		}
		(void)fclose(fp);
	}
	(void)closedir(dir);
	return KN_BBS_STORE_MAINTENANCE_OK;
}

static enum kn_bbs_store_maintenance_error
scan_users(const char *path, struct kn_bbs_store_finding *findings,
	size_t max_findings, size_t *count, struct kn_bbs_store_stats *stats)
{
	struct dirent *entry;
	DIR *dir;
	char users[KN_MESSAGE_STORE_PATH_MAX];
	char file[KN_MESSAGE_STORE_PATH_MAX];
	char buf[512];
	size_t len;
	enum kn_bbs_store_maintenance_error rc;

	rc = join_path(path, "users", users, sizeof(users));
	if (rc != KN_BBS_STORE_MAINTENANCE_OK)
		return rc;
	dir = opendir(users);
	if (dir == NULL)
		return KN_BBS_STORE_MAINTENANCE_OK;
	while ((entry = readdir(dir)) != NULL) {
		if (entry->d_name[0] == '.')
			continue;
		if (strstr(entry->d_name, ".user") == NULL)
			continue;
		stats->users++;
		rc = join_path(users, entry->d_name, file, sizeof(file));
		if (rc != KN_BBS_STORE_MAINTENANCE_OK) {
			(void)closedir(dir);
			return rc;
		}
		rc = read_file(file, buf, sizeof(buf), &len);
		(void)len;
		if (rc != KN_BBS_STORE_MAINTENANCE_OK ||
		    strstr(buf, "call ") == NULL) {
			rc = add_finding(findings, max_findings, count,
			    KN_BBS_STORE_FINDING_ERROR, "user-corrupt", file,
			    "user file is corrupt", 0);
			if (rc != KN_BBS_STORE_MAINTENANCE_OK) {
				(void)closedir(dir);
				return rc;
			}
		}
	}
	(void)closedir(dir);
	return KN_BBS_STORE_MAINTENANCE_OK;
}

static uint8_t
store_child_ok(const char *name)
{
	size_t i;

	if (name == NULL || name[0] == '\0')
		return 0;
	for (i = 0; name[i] != '\0'; i++) {
		if ((unsigned char)name[i] < 0x20 ||
		    (unsigned char)name[i] > 0x7e)
			return 0;
		if (name[i] == '/')
			return 0;
	}
	return 1;
}

static enum kn_bbs_store_maintenance_error
write_read_state_ids(const char *path, uint64_t *ids, size_t count)
{
	char tmp[KN_MESSAGE_STORE_PATH_MAX + 8];
	FILE *fp;
	size_t i;
	int needed;

	needed = snprintf(tmp, sizeof(tmp), "%s.tmp", path);
	if (needed < 0 || (size_t)needed >= sizeof(tmp))
		return KN_BBS_STORE_MAINTENANCE_ERR_BUFFER;
	fp = fopen(tmp, "w");
	if (fp == NULL)
		return KN_BBS_STORE_MAINTENANCE_ERR_IO;
	for (i = 0; i < count; i++) {
		if (fprintf(fp, "%llu\n", (unsigned long long)ids[i]) < 0) {
			(void)fclose(fp);
			(void)unlink(tmp);
			return KN_BBS_STORE_MAINTENANCE_ERR_IO;
		}
	}
	if (fflush(fp) != 0 || fsync(fileno(fp)) != 0 || fclose(fp) != 0) {
		(void)unlink(tmp);
		return KN_BBS_STORE_MAINTENANCE_ERR_IO;
	}
	if (rename(tmp, path) != 0) {
		(void)unlink(tmp);
		return KN_BBS_STORE_MAINTENANCE_ERR_IO;
	}
	return KN_BBS_STORE_MAINTENANCE_OK;
}

enum kn_bbs_store_maintenance_error
kn_bbs_store_check(const char *path, struct kn_bbs_store_finding *findings,
	size_t max_findings, size_t *count_out, size_t *error_count_out)
{
	char child[KN_MESSAGE_STORE_PATH_MAX];
	char index[KN_MESSAGE_STORE_PATH_MAX];
	const char *dirs[] = { "meta", "msg", "index", "users", "read" };
	const char *indexes[] = { "all.idx", "private.idx", "bulletin.idx" };
	struct kn_bbs_store_stats stats;
	size_t count;
	size_t errors;
	size_t i;
	uint64_t max_id;
	uint64_t next_id;
	enum kn_bbs_store_maintenance_error rc;

	if (path_safe(path) == 0 || count_out == NULL ||
	    error_count_out == NULL)
		return KN_BBS_STORE_MAINTENANCE_ERR_INVALID_ARGUMENT;
	memset(&stats, 0, sizeof(stats));
	count = 0;
	errors = 0;
	if (dir_exists(path) == 0) {
		(void)add_finding(findings, max_findings, &count,
		    KN_BBS_STORE_FINDING_ERROR, "root-missing", path,
		    "store root is missing", 0);
		*count_out = count;
		*error_count_out = 1;
		return KN_BBS_STORE_MAINTENANCE_OK;
	}
	for (i = 0; i < sizeof(dirs) / sizeof(dirs[0]); i++) {
		rc = join_path(path, dirs[i], child, sizeof(child));
		if (rc != KN_BBS_STORE_MAINTENANCE_OK)
			return rc;
		if (dir_exists(child) == 0)
			(void)add_finding(findings, max_findings, &count,
			    KN_BBS_STORE_FINDING_ERROR, "dir-missing", child,
			    "managed directory is missing", 0);
	}
	rc = next_id_read_path(path, &next_id);
	if (rc != KN_BBS_STORE_MAINTENANCE_OK) {
		(void)add_finding(findings, max_findings, &count,
		    KN_BBS_STORE_FINDING_ERROR, "next-id", path,
		    "next-id is missing or corrupt", 0);
		next_id = 0;
	}
	rc = scan_messages(path, &stats, findings, max_findings, &count,
	    &max_id);
	if (rc != KN_BBS_STORE_MAINTENANCE_OK)
		return rc;
	if (next_id != 0 && next_id <= max_id)
		(void)add_finding(findings, max_findings, &count,
		    KN_BBS_STORE_FINDING_ERROR, "next-id-low", path,
		    "next-id is not greater than existing message IDs", max_id);
	for (i = 0; i < sizeof(indexes) / sizeof(indexes[0]); i++) {
		rc = join_path(path, "index", child, sizeof(child));
		if (rc != KN_BBS_STORE_MAINTENANCE_OK)
			return rc;
		rc = join_path(child, indexes[i], index, sizeof(index));
		if (rc != KN_BBS_STORE_MAINTENANCE_OK)
			return rc;
		if (file_exists(index) == 0) {
			(void)add_finding(findings, max_findings, &count,
			    KN_BBS_STORE_FINDING_ERROR, "index-missing",
			    index, "required index is missing", 0);
			continue;
		}
		rc = parse_index_duplicates(index, findings, max_findings,
		    &count);
		if (rc != KN_BBS_STORE_MAINTENANCE_OK)
			return rc;
	}
	rc = scan_users(path, findings, max_findings, &count, &stats);
	if (rc != KN_BBS_STORE_MAINTENANCE_OK)
		return rc;
	rc = scan_read_state(path, findings, max_findings, &count);
	if (rc != KN_BBS_STORE_MAINTENANCE_OK)
		return rc;
	for (i = 0; i < count && i < max_findings && findings != NULL; i++) {
		if (findings[i].severity == KN_BBS_STORE_FINDING_ERROR)
			errors++;
	}
	*count_out = count;
	*error_count_out = errors;
	return KN_BBS_STORE_MAINTENANCE_OK;
}

const char *
kn_bbs_store_maintenance_error_name(
	enum kn_bbs_store_maintenance_error error)
{
	switch (error) {
	case KN_BBS_STORE_MAINTENANCE_OK:
		return "ok";
	case KN_BBS_STORE_MAINTENANCE_ERR_INVALID_ARGUMENT:
		return "invalid argument";
	case KN_BBS_STORE_MAINTENANCE_ERR_IO:
		return "io";
	case KN_BBS_STORE_MAINTENANCE_ERR_LOCK:
		return "lock";
	case KN_BBS_STORE_MAINTENANCE_ERR_BUFFER:
		return "buffer";
	case KN_BBS_STORE_MAINTENANCE_ERR_CORRUPT:
		return "corrupt";
	}
	return "unknown";
}

const char *
kn_bbs_store_finding_severity_name(enum kn_bbs_store_finding_severity severity)
{
	switch (severity) {
	case KN_BBS_STORE_FINDING_INFO:
		return "info";
	case KN_BBS_STORE_FINDING_WARNING:
		return "warning";
	case KN_BBS_STORE_FINDING_ERROR:
		return "error";
	}
	return "unknown";
}

enum kn_bbs_store_maintenance_error
kn_bbs_store_export(const char *path, const char *dest)
{
	struct kn_bbs_store_lock lock;
	struct kn_bbs_store_stats stats;
	struct dirent *entry;
	DIR *dir;
	size_t path_len;
	enum kn_bbs_store_maintenance_error rc;

	if (path_safe(path) == 0 || path_safe(dest) == 0)
		return KN_BBS_STORE_MAINTENANCE_ERR_INVALID_ARGUMENT;
	path_len = strlen(path);
	if (strncmp(dest, path, path_len) == 0 &&
	    (dest[path_len] == '/' || dest[path_len] == '\0'))
		return KN_BBS_STORE_MAINTENANCE_ERR_INVALID_ARGUMENT;
	if (mkdir(dest, 0700) != 0 && errno != EEXIST)
		return KN_BBS_STORE_MAINTENANCE_ERR_IO;
	dir = opendir(dest);
	if (dir == NULL)
		return KN_BBS_STORE_MAINTENANCE_ERR_IO;
	while ((entry = readdir(dir)) != NULL) {
		if (entry->d_name[0] != '.') {
			(void)closedir(dir);
			return KN_BBS_STORE_MAINTENANCE_ERR_INVALID_ARGUMENT;
		}
	}
	(void)closedir(dir);
	kn_bbs_store_lock_init(&lock);
	if (kn_bbs_store_lock_exclusive(&lock, path) !=
	    KN_BBS_STORE_LOCK_OK)
		return KN_BBS_STORE_MAINTENANCE_ERR_LOCK;
	rc = kn_bbs_store_stats(path, &stats);
	if (rc == KN_BBS_STORE_MAINTENANCE_OK)
		rc = copy_dir(path, dest, "meta");
	if (rc == KN_BBS_STORE_MAINTENANCE_OK)
		rc = copy_dir(path, dest, "msg");
	if (rc == KN_BBS_STORE_MAINTENANCE_OK)
		rc = copy_dir(path, dest, "index");
	if (rc == KN_BBS_STORE_MAINTENANCE_OK)
		rc = copy_dir(path, dest, "users");
	if (rc == KN_BBS_STORE_MAINTENANCE_OK)
		rc = copy_dir(path, dest, "read");
	if (rc == KN_BBS_STORE_MAINTENANCE_OK)
		rc = export_manifest(path, dest, &stats);
	kn_bbs_store_lock_release(&lock);
	return rc;
}

enum kn_bbs_store_maintenance_error
kn_bbs_store_purge_deleted(const char *path, size_t *purged_out)
{
	struct kn_bbs_store_lock lock;
	struct dirent *entry;
	DIR *dir;
	char msg_dir[KN_MESSAGE_STORE_PATH_MAX];
	char meta[KN_MESSAGE_STORE_PATH_MAX];
	char body[KN_MESSAGE_STORE_PATH_MAX];
	struct raw_message msg;
	uint64_t id;
	size_t purged;
	enum kn_bbs_store_maintenance_error rc;

	if (path_safe(path) == 0 || purged_out == NULL)
		return KN_BBS_STORE_MAINTENANCE_ERR_INVALID_ARGUMENT;
	kn_bbs_store_lock_init(&lock);
	if (kn_bbs_store_lock_exclusive(&lock, path) !=
	    KN_BBS_STORE_LOCK_OK)
		return KN_BBS_STORE_MAINTENANCE_ERR_LOCK;
	rc = join_path(path, "msg", msg_dir, sizeof(msg_dir));
	if (rc != KN_BBS_STORE_MAINTENANCE_OK)
		goto done;
	dir = opendir(msg_dir);
	if (dir == NULL) {
		rc = KN_BBS_STORE_MAINTENANCE_ERR_IO;
		goto done;
	}
	purged = 0;
	while ((entry = readdir(dir)) != NULL) {
		if (filename_id(entry->d_name, ".meta", &id) == 0)
			continue;
		rc = msg_path(path, id, "meta", meta, sizeof(meta));
		if (rc != KN_BBS_STORE_MAINTENANCE_OK)
			break;
		rc = parse_raw_meta_path(meta, &msg);
		if (rc != KN_BBS_STORE_MAINTENANCE_OK)
			continue;
		if (msg.deleted == 0)
			continue;
		rc = msg_path(path, id, "body", body, sizeof(body));
		if (rc != KN_BBS_STORE_MAINTENANCE_OK)
			break;
		(void)unlink(body);
		if (unlink(meta) == 0)
			purged++;
	}
	(void)closedir(dir);
done:
	kn_bbs_store_lock_release(&lock);
	if (rc != KN_BBS_STORE_MAINTENANCE_OK)
		return rc;
	rc = rebuild_indexes(path);
	if (rc != KN_BBS_STORE_MAINTENANCE_OK)
		return rc;
	*purged_out = purged;
	return KN_BBS_STORE_MAINTENANCE_OK;
}

enum kn_bbs_store_maintenance_error
kn_bbs_store_repair(const char *path, struct kn_bbs_store_finding *findings,
	size_t max_findings, size_t *count_out)
{
	struct kn_bbs_store_lock lock;
	const char *dirs[] = { "meta", "msg", "index", "users", "read" };
	char child[KN_MESSAGE_STORE_PATH_MAX];
	size_t count;
	size_t i;
	uint64_t max_id;
	uint64_t next_id;
	struct kn_bbs_store_stats stats;
	enum kn_bbs_store_maintenance_error rc;
	size_t scan_count;

	if (path_safe(path) == 0 || count_out == NULL)
		return KN_BBS_STORE_MAINTENANCE_ERR_INVALID_ARGUMENT;
	count = 0;
	rc = ensure_dir(path);
	if (rc != KN_BBS_STORE_MAINTENANCE_OK)
		return rc;
	kn_bbs_store_lock_init(&lock);
	if (kn_bbs_store_lock_exclusive(&lock, path) !=
	    KN_BBS_STORE_LOCK_OK)
		return KN_BBS_STORE_MAINTENANCE_ERR_LOCK;
	for (i = 0; i < sizeof(dirs) / sizeof(dirs[0]); i++) {
		rc = join_path(path, dirs[i], child, sizeof(child));
		if (rc != KN_BBS_STORE_MAINTENANCE_OK)
			goto done;
		if (dir_exists(child) == 0) {
			rc = ensure_dir(child);
			if (rc != KN_BBS_STORE_MAINTENANCE_OK)
				goto done;
			rc = add_finding(findings, max_findings, &count,
			    KN_BBS_STORE_FINDING_INFO, "dir-created", child,
			    "created missing directory", 0);
			if (rc != KN_BBS_STORE_MAINTENANCE_OK)
				goto done;
		}
	}
	memset(&stats, 0, sizeof(stats));
	scan_count = 0;
	rc = scan_messages(path, &stats, NULL, 0, &scan_count, &max_id);
	if (rc != KN_BBS_STORE_MAINTENANCE_OK)
		goto done;
	rc = next_id_read_path(path, &next_id);
	if (rc != KN_BBS_STORE_MAINTENANCE_OK || next_id <= max_id) {
		rc = next_id_write_path(path, max_id + 1);
		if (rc != KN_BBS_STORE_MAINTENANCE_OK)
			goto done;
		rc = add_finding(findings, max_findings, &count,
		    KN_BBS_STORE_FINDING_INFO, "next-id-repaired", path,
		    "repaired next-id", max_id + 1);
		if (rc != KN_BBS_STORE_MAINTENANCE_OK)
			goto done;
	}
done:
	kn_bbs_store_lock_release(&lock);
	if (rc != KN_BBS_STORE_MAINTENANCE_OK)
		return rc;
	rc = rebuild_indexes(path);
	if (rc != KN_BBS_STORE_MAINTENANCE_OK)
		return rc;
	rc = dedup_read_dir(path, findings, max_findings, &count);
	if (rc != KN_BBS_STORE_MAINTENANCE_OK)
		return rc;
	*count_out = count;
	return KN_BBS_STORE_MAINTENANCE_OK;
}

enum kn_bbs_store_maintenance_error
kn_bbs_store_stats(const char *path, struct kn_bbs_store_stats *stats)
{
	char read_dir[KN_MESSAGE_STORE_PATH_MAX];
	char index_dir[KN_MESSAGE_STORE_PATH_MAX];
	char area_prefix[] = "area-";
	struct kn_bbs_store_finding findings[1];
	struct dirent *entry;
	DIR *dir;
	size_t count;
	uint64_t max_id;
	enum kn_bbs_store_maintenance_error rc;

	if (path_safe(path) == 0 || stats == NULL)
		return KN_BBS_STORE_MAINTENANCE_ERR_INVALID_ARGUMENT;
	memset(stats, 0, sizeof(*stats));
	(void)next_id_read_path(path, &stats->next_id);
	count = 0;
	rc = scan_messages(path, stats, findings, 0, &count, &max_id);
	if (rc != KN_BBS_STORE_MAINTENANCE_OK)
		return rc;
	rc = scan_users(path, findings, 0, &count, stats);
	if (rc != KN_BBS_STORE_MAINTENANCE_OK)
		return rc;
	rc = join_path(path, "read", read_dir, sizeof(read_dir));
	if (rc == KN_BBS_STORE_MAINTENANCE_OK && (dir = opendir(read_dir)) !=
	    NULL) {
		while ((entry = readdir(dir)) != NULL) {
			if (strstr(entry->d_name, ".read") != NULL)
				stats->read_state_files++;
		}
		(void)closedir(dir);
	}
	rc = join_path(path, "index", index_dir, sizeof(index_dir));
	if (rc == KN_BBS_STORE_MAINTENANCE_OK && (dir = opendir(index_dir)) !=
	    NULL) {
		while ((entry = readdir(dir)) != NULL) {
			if (strncmp(entry->d_name, area_prefix,
			    strlen(area_prefix)) == 0)
				stats->bulletin_areas++;
		}
		(void)closedir(dir);
	}
	return KN_BBS_STORE_MAINTENANCE_OK;
}
