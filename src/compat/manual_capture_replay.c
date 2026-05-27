/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/compat/manual_capture_replay.c */

#include <sys/types.h>
#include <sys/stat.h>

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "kilonode/compat_packet_capture.h"
#include "kilonode/manual_capture_replay.h"

static enum kn_manual_capture_error copy_file(const char *, const char *);
static enum kn_manual_capture_error destination_path(const char *,
	const char *, char *, char *, size_t);
static int has_capture_suffix(const char *);
static int planned_placeholder(const char *);
static void sanitize_notes(const char *, char *, size_t);
static enum kn_manual_capture_status validate_capture_path(const char *,
	char *, size_t);

enum kn_manual_capture_error
kn_manual_capture_import(const struct kn_manual_capture_import_request *request,
	struct kn_manual_capture_import_result *result,
	struct kn_manual_capture_error_info *info)
{
	struct kn_manual_capture_workspace workspace;
	struct kn_manual_capture_index index;
	struct kn_manual_capture_entry entry;
	struct stat st;
	char rel[KN_MANUAL_CAPTURE_FIELD_MAX];
	char dst[KN_MANUAL_CAPTURE_PATH_MAX];
	char method[KN_MANUAL_CAPTURE_FIELD_MAX];

	if (request == NULL || request->source_path == NULL ||
	    request->workspace_root == NULL)
		return KN_MANUAL_CAPTURE_ERR_INVALID_ARGUMENT;
	if (has_capture_suffix(request->source_path) == 0)
		return KN_MANUAL_CAPTURE_ERR_INVALID_VALUE;
	if (kn_manual_capture_workspace_check(request->workspace_root,
	    &workspace, info) != KN_MANUAL_CAPTURE_OK)
		return KN_MANUAL_CAPTURE_ERR_UNSAFE_PATH;
	if (stat(request->source_path, &st) != 0 || !S_ISREG(st.st_mode))
		return KN_MANUAL_CAPTURE_ERR_IO;
	if ((uint64_t)st.st_size > KN_MANUAL_CAPTURE_FILE_MAX)
		return KN_MANUAL_CAPTURE_ERR_INVALID_VALUE;
	if (destination_path(request->workspace_root, request->source_path, rel,
	    dst, sizeof(dst)) != KN_MANUAL_CAPTURE_OK)
		return KN_MANUAL_CAPTURE_ERR_BUFFER;
	if (copy_file(request->source_path, dst) != KN_MANUAL_CAPTURE_OK)
		return KN_MANUAL_CAPTURE_ERR_IO;
	if (kn_manual_capture_index_load(request->workspace_root, &index,
	    info) != KN_MANUAL_CAPTURE_OK)
		return KN_MANUAL_CAPTURE_ERR_IO;
	memset(&entry, 0, sizeof(entry));
	(void)snprintf(entry.file, sizeof(entry.file), "%s", rel);
	entry.status = validate_capture_path(dst, method, sizeof(method));
	entry.replay = KN_MANUAL_CAPTURE_REPLAY_NOT_RUN;
	(void)snprintf(entry.method, sizeof(entry.method), "%s", method);
	entry.source = request->source;
	entry.added = (uint64_t)time(NULL);
	sanitize_notes(request->notes, entry.notes, sizeof(entry.notes));
	if (kn_manual_capture_index_add(&index, &entry) !=
	    KN_MANUAL_CAPTURE_OK)
		return KN_MANUAL_CAPTURE_ERR_INVALID_VALUE;
	if (kn_manual_capture_index_save(request->workspace_root, &index,
	    info) != KN_MANUAL_CAPTURE_OK)
		return KN_MANUAL_CAPTURE_ERR_IO;
	if (result != NULL) {
		result->id = index.entries[index.entry_count - 1].id;
		(void)snprintf(result->file, sizeof(result->file), "%s", rel);
		result->status = entry.status;
	}

	return KN_MANUAL_CAPTURE_OK;
}

enum kn_manual_capture_error
kn_manual_capture_validate_all(const char *root,
	struct kn_manual_capture_index *out,
	struct kn_manual_capture_error_info *info)
{
	struct kn_manual_capture_index index;
	char path[KN_MANUAL_CAPTURE_PATH_MAX];
	char method[KN_MANUAL_CAPTURE_FIELD_MAX];
	size_t i;

	if (root == NULL)
		return KN_MANUAL_CAPTURE_ERR_INVALID_ARGUMENT;
	if (kn_manual_capture_index_load(root, &index, info) !=
	    KN_MANUAL_CAPTURE_OK)
		return KN_MANUAL_CAPTURE_ERR_IO;
	for (i = 0; i < index.entry_count; i++) {
		if (kn_manual_capture_workspace_join(root, index.entries[i].file,
		    path, sizeof(path)) != KN_MANUAL_CAPTURE_OK)
			return KN_MANUAL_CAPTURE_ERR_UNSAFE_PATH;
		index.entries[i].status = validate_capture_path(path, method,
		    sizeof(method));
		(void)snprintf(index.entries[i].method,
		    sizeof(index.entries[i].method), "%s", method);
	}
	if (kn_manual_capture_index_save(root, &index, info) !=
	    KN_MANUAL_CAPTURE_OK)
		return KN_MANUAL_CAPTURE_ERR_IO;
	if (out != NULL)
		*out = index;
	return KN_MANUAL_CAPTURE_OK;
}

enum kn_manual_capture_error
kn_manual_capture_replay_one(const char *root, uint32_t id,
	struct kn_manual_capture_replay_result *result,
	struct kn_manual_capture_error_info *info)
{
	struct kn_manual_capture_index index;
	struct kn_manual_capture_entry *entry;
	char path[KN_MANUAL_CAPTURE_PATH_MAX];
	enum kn_bench_diag_replay_error rc;

	if (root == NULL || id == 0 || result == NULL)
		return KN_MANUAL_CAPTURE_ERR_INVALID_ARGUMENT;
	if (kn_manual_capture_index_load(root, &index, info) !=
	    KN_MANUAL_CAPTURE_OK)
		return KN_MANUAL_CAPTURE_ERR_IO;
	entry = kn_manual_capture_index_find(&index, id);
	if (entry == NULL)
		return KN_MANUAL_CAPTURE_ERR_INVALID_VALUE;
	if (kn_manual_capture_workspace_join(root, entry->file, path,
	    sizeof(path)) != KN_MANUAL_CAPTURE_OK)
		return KN_MANUAL_CAPTURE_ERR_UNSAFE_PATH;
	memset(result, 0, sizeof(*result));
	result->id = id;
	rc = kn_bench_diag_replay_capture(path, &result->diag);
	if (rc != KN_BENCH_DIAG_REPLAY_OK || result->diag.pass == 0) {
		result->replay = KN_MANUAL_CAPTURE_REPLAY_FAIL;
		entry->replay = result->replay;
		(void)kn_manual_capture_index_save(root, &index, info);
		return KN_MANUAL_CAPTURE_ERR_REPLAY;
	}
	result->replay = result->diag.unsupported != 0 ?
	    KN_MANUAL_CAPTURE_REPLAY_UNSUPPORTED : KN_MANUAL_CAPTURE_REPLAY_PASS;
	entry->replay = result->replay;
	if (entry->status == KN_MANUAL_CAPTURE_STATUS_UNCHECKED)
		entry->status = result->diag.unsupported != 0 ?
		    KN_MANUAL_CAPTURE_STATUS_UNSUPPORTED :
		    KN_MANUAL_CAPTURE_STATUS_VALID;
	if (kn_manual_capture_index_save(root, &index, info) !=
	    KN_MANUAL_CAPTURE_OK)
		return KN_MANUAL_CAPTURE_ERR_IO;

	return result->diag.tx_writes_attempted == 0 &&
	    result->diag.prepared_tx_writes_attempted == 0 ?
	    KN_MANUAL_CAPTURE_OK : KN_MANUAL_CAPTURE_ERR_REPLAY;
}

enum kn_manual_capture_error
kn_manual_capture_replay_all(const char *root,
	struct kn_manual_capture_replay_all_result *result,
	struct kn_manual_capture_error_info *info)
{
	struct kn_manual_capture_index index;
	struct kn_manual_capture_replay_result one;
	size_t i;

	if (root == NULL || result == NULL)
		return KN_MANUAL_CAPTURE_ERR_INVALID_ARGUMENT;
	memset(result, 0, sizeof(*result));
	if (kn_manual_capture_index_load(root, &index, info) !=
	    KN_MANUAL_CAPTURE_OK)
		return KN_MANUAL_CAPTURE_ERR_IO;
	for (i = 0; i < index.entry_count; i++) {
		if (index.entries[i].status == KN_MANUAL_CAPTURE_STATUS_INVALID)
			continue;
		if (kn_manual_capture_replay_one(root, index.entries[i].id,
		    &one, info) != KN_MANUAL_CAPTURE_OK) {
			result->fail_count++;
			continue;
		}
		result->count++;
		result->tx_writes += one.diag.tx_writes_attempted;
		result->tx_writes += one.diag.prepared_tx_writes_attempted;
		if (one.replay == KN_MANUAL_CAPTURE_REPLAY_UNSUPPORTED)
			result->unsupported_count++;
		else if (one.replay == KN_MANUAL_CAPTURE_REPLAY_PASS)
			result->pass_count++;
		else
			result->fail_count++;
	}

	return result->tx_writes == 0 ? KN_MANUAL_CAPTURE_OK :
	    KN_MANUAL_CAPTURE_ERR_REPLAY;
}

static enum kn_manual_capture_error
copy_file(const char *src, const char *dst)
{
	FILE *in;
	FILE *out;
	char buf[1024];
	size_t n;

	in = fopen(src, "rb");
	if (in == NULL)
		return KN_MANUAL_CAPTURE_ERR_IO;
	out = fopen(dst, "wb");
	if (out == NULL) {
		(void)fclose(in);
		return KN_MANUAL_CAPTURE_ERR_IO;
	}
	while ((n = fread(buf, 1, sizeof(buf), in)) > 0) {
		if (fwrite(buf, 1, n, out) != n) {
			(void)fclose(in);
			(void)fclose(out);
			return KN_MANUAL_CAPTURE_ERR_IO;
		}
	}
	(void)fclose(in);
	(void)fclose(out);
	return KN_MANUAL_CAPTURE_OK;
}

static enum kn_manual_capture_error
destination_path(const char *root, const char *src, char *rel, char *dst,
	size_t dstsiz)
{
	const char *base;
	const char *dot;
	char candidate[KN_MANUAL_CAPTURE_FIELD_MAX];
	char stem[181];
	FILE *fp;
	unsigned int suffix;
	size_t stem_len;

	base = strrchr(src, '/');
	base = base == NULL ? src : base + 1;
	if (base[0] == '\0' || strstr(base, "..") != NULL ||
	    strchr(base, '\\') != NULL)
		return KN_MANUAL_CAPTURE_ERR_UNSAFE_PATH;
	dot = strstr(base, ".capture");
	if (dot == NULL)
		return KN_MANUAL_CAPTURE_ERR_INVALID_VALUE;
	stem_len = (size_t)(dot - base);
	if (stem_len == 0 || stem_len >= sizeof(stem) || stem_len > 180)
		return KN_MANUAL_CAPTURE_ERR_INVALID_VALUE;
	memcpy(stem, base, stem_len);
	stem[stem_len] = '\0';
	for (suffix = 0; suffix < 1000; suffix++) {
		if (suffix == 0)
			(void)snprintf(candidate, sizeof(candidate),
			    "imported/%s.capture", stem);
		else
			(void)snprintf(candidate, sizeof(candidate),
			    "imported/%s-%u.capture", stem, suffix + 1);
		if (kn_manual_capture_workspace_join(root, candidate, dst,
		    dstsiz) != KN_MANUAL_CAPTURE_OK)
			return KN_MANUAL_CAPTURE_ERR_BUFFER;
		fp = fopen(dst, "r");
		if (fp == NULL) {
			(void)snprintf(rel, KN_MANUAL_CAPTURE_FIELD_MAX, "%s",
			    candidate);
			return KN_MANUAL_CAPTURE_OK;
		}
		(void)fclose(fp);
	}
	return KN_MANUAL_CAPTURE_ERR_TOO_MANY;
}

static int
has_capture_suffix(const char *path)
{
	size_t len;

	if (path == NULL)
		return 0;
	len = strlen(path);
	return len > 8 && strcmp(path + len - 8, ".capture") == 0;
}

static int
planned_placeholder(const char *path)
{
	return path != NULL && strstr(path, "fx25") != NULL;
}

static void
sanitize_notes(const char *src, char *dst, size_t dstsiz)
{
	size_t i;

	if (dst == NULL || dstsiz == 0)
		return;
	if (src == NULL || src[0] == '\0') {
		(void)snprintf(dst, dstsiz, "-");
		return;
	}
	for (i = 0; i + 1 < dstsiz && src[i] != '\0'; i++) {
		if (src[i] <= ' ' || src[i] == '"' || src[i] == '\\')
			dst[i] = '_';
		else
			dst[i] = src[i];
	}
	dst[i] = '\0';
}

static enum kn_manual_capture_status
validate_capture_path(const char *path, char *method, size_t methodsiz)
{
	struct kn_compat_packet_capture capture;

	if (method != NULL && methodsiz > 0)
		method[0] = '\0';
	if (kn_compat_packet_capture_parse_file(path, &capture, NULL) !=
	    KN_COMPAT_PACKET_CAPTURE_OK) {
		if (method != NULL && methodsiz > 0)
			(void)snprintf(method, methodsiz, "unknown");
		return KN_MANUAL_CAPTURE_STATUS_INVALID;
	}
	if (method != NULL && methodsiz > 0)
		(void)snprintf(method, methodsiz, "%s",
		    kn_compat_packet_method_name(capture.method));
	if (planned_placeholder(path) != 0 || planned_placeholder(capture.name))
		return KN_MANUAL_CAPTURE_STATUS_UNSUPPORTED;

	return KN_MANUAL_CAPTURE_STATUS_VALID;
}
