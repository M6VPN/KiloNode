/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/compat/compat_capture.c */

#include <sys/types.h>

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "kilonode/compat_capture.h"

static enum kn_compat_capture_error append_format(char *, size_t, size_t *,
	const char *, ...);
static void sanitize_text(const char *, char *, size_t);
static uint8_t unsafe_output_path(const char *);

const char *
kn_compat_capture_error_name(enum kn_compat_capture_error error)
{
	switch (error) {
	case KN_COMPAT_CAPTURE_OK:
		return "ok";
	case KN_COMPAT_CAPTURE_ERR_INVALID_ARGUMENT:
		return "invalid-argument";
	case KN_COMPAT_CAPTURE_ERR_BUFFER:
		return "buffer";
	case KN_COMPAT_CAPTURE_ERR_IO:
		return "io";
	case KN_COMPAT_CAPTURE_ERR_UNSAFE_PATH:
		return "unsafe-path";
	}

	return "unknown";
}

enum kn_compat_capture_error
kn_compat_capture_compare(const struct kn_compat_observation *left,
	const struct kn_compat_observation *right, char *buf, size_t bufsiz)
{
	char left_preview[256];
	char right_preview[256];
	int same;
	int needed;

	if (left == NULL || right == NULL || buf == NULL || bufsiz == 0)
		return KN_COMPAT_CAPTURE_ERR_INVALID_ARGUMENT;

	sanitize_text(left->observed, left_preview, sizeof(left_preview));
	sanitize_text(right->observed, right_preview, sizeof(right_preview));
	same = strcmp(left->observed, right->observed) == 0 ? 1 : 0;
	needed = snprintf(buf, bufsiz,
	    "OBSERVATION-COMPARE left=%s right=%s same=%s "
	    "left_len=%llu right_len=%llu left_preview=\"%s\" "
	    "right_preview=\"%s\"",
	    left->name, right->name, same != 0 ? "true" : "false",
	    (unsigned long long)left->observed_len,
	    (unsigned long long)right->observed_len, left_preview,
	    right_preview);

	return needed >= 0 && (size_t)needed < bufsiz ?
	    KN_COMPAT_CAPTURE_OK : KN_COMPAT_CAPTURE_ERR_BUFFER;
}

enum kn_compat_capture_error
kn_compat_capture_transcript_candidate(
	const struct kn_compat_observation *observation, char *buf,
	size_t bufsiz)
{
	char preview[512];
	size_t offset;
	enum kn_compat_capture_error rc;

	if (observation == NULL || buf == NULL || bufsiz == 0)
		return KN_COMPAT_CAPTURE_ERR_INVALID_ARGUMENT;

	sanitize_text(observation->observed, preview, sizeof(preview));
	offset = 0;
	rc = append_format(buf, bufsiz, &offset,
	    "# KiloNode compatibility transcript candidate v1\n"
	    "# source=black-box-observation\n"
	    "# implementation-use=false\n"
	    "name %s-candidate\n"
	    "source black-box-observation\n"
	    "observation %s\n"
	    "mode %s\n"
	    "input %s\n"
	    "expect-observed contains=%s\n",
	    observation->name, observation->name,
	    kn_compat_observe_mode_name(observation->mode),
	    observation->input, preview);
	if (rc != KN_COMPAT_CAPTURE_OK)
		return rc;

	return KN_COMPAT_CAPTURE_OK;
}

enum kn_compat_capture_error
kn_compat_capture_write_observation(
	const struct kn_compat_observation *observation, const char *path)
{
	FILE *fp;

	if (observation == NULL || path == NULL || path[0] == '\0')
		return KN_COMPAT_CAPTURE_ERR_INVALID_ARGUMENT;
	if (unsafe_output_path(path) != 0)
		return KN_COMPAT_CAPTURE_ERR_UNSAFE_PATH;

	fp = fopen(path, "w");
	if (fp == NULL)
		return KN_COMPAT_CAPTURE_ERR_IO;
	(void)fprintf(fp,
	    "# KiloNode black-box observation v1\n"
	    "name %s\n"
	    "subject %s\n"
	    "method %s\n"
	    "date %s\n"
	    "observer %s\n",
	    observation->name, observation->subject,
	    kn_compat_observe_method_name(observation->method),
	    observation->date,
	    observation->observer[0] == '\0' ? "unknown" :
	    observation->observer);
	if (observation->binary_path[0] != '\0')
		(void)fprintf(fp, "binary %s\n", observation->binary_path);
	if (observation->config_path[0] != '\0')
		(void)fprintf(fp, "config %s\n", observation->config_path);
	if (observation->connect_target[0] != '\0')
		(void)fprintf(fp, "connect %s\n", observation->connect_target);
	(void)fprintf(fp,
	    "mode %s\n"
	    "input %s\n"
	    "observed-begin\n"
	    "%s"
	    "observed-end\n",
	    kn_compat_observe_mode_name(observation->mode),
	    observation->input, observation->observed);
	if (observation->notes[0] != '\0')
		(void)fprintf(fp, "notes %s\n", observation->notes);
	if (ferror(fp)) {
		(void)fclose(fp);
		return KN_COMPAT_CAPTURE_ERR_IO;
	}
	(void)fclose(fp);

	return KN_COMPAT_CAPTURE_OK;
}

enum kn_compat_capture_error
kn_compat_capture_write_text(const char *text, const char *path)
{
	FILE *fp;

	if (text == NULL || path == NULL || path[0] == '\0')
		return KN_COMPAT_CAPTURE_ERR_INVALID_ARGUMENT;
	if (unsafe_output_path(path) != 0)
		return KN_COMPAT_CAPTURE_ERR_UNSAFE_PATH;

	fp = fopen(path, "w");
	if (fp == NULL)
		return KN_COMPAT_CAPTURE_ERR_IO;
	if (fputs(text, fp) == EOF) {
		(void)fclose(fp);
		return KN_COMPAT_CAPTURE_ERR_IO;
	}
	(void)fclose(fp);
	return KN_COMPAT_CAPTURE_OK;
}

static enum kn_compat_capture_error
append_format(char *buf, size_t bufsiz, size_t *offset, const char *fmt, ...)
{
	va_list args;
	int needed;

	if (*offset >= bufsiz)
		return KN_COMPAT_CAPTURE_ERR_BUFFER;
	va_start(args, fmt);
	needed = vsnprintf(buf + *offset, bufsiz - *offset, fmt, args);
	va_end(args);
	if (needed < 0 || (size_t)needed >= bufsiz - *offset)
		return KN_COMPAT_CAPTURE_ERR_BUFFER;
	*offset += (size_t)needed;

	return KN_COMPAT_CAPTURE_OK;
}

static void
sanitize_text(const char *src, char *dst, size_t dst_len)
{
	size_t i;
	size_t off;
	int needed;
	unsigned char ch;

	if (dst == NULL || dst_len == 0)
		return;
	dst[0] = '\0';
	if (src == NULL)
		return;

	off = 0;
	for (i = 0; src[i] != '\0'; i++) {
		ch = (unsigned char)src[i];
		if (ch == '\n') {
			needed = snprintf(dst + off, dst_len - off, "\\n");
		} else if (ch == '"' || ch == '\\') {
			needed = snprintf(dst + off, dst_len - off, "\\%c",
			    (int)ch);
		} else if (ch >= 0x20 && ch <= 0x7e) {
			needed = snprintf(dst + off, dst_len - off, "%c",
			    (int)ch);
		} else {
			needed = snprintf(dst + off, dst_len - off, "\\x%02x",
			    (unsigned int)ch);
		}
		if (needed < 0 || (size_t)needed >= dst_len - off)
			return;
		off += (size_t)needed;
	}
}

static uint8_t
unsafe_output_path(const char *path)
{
	if (path == NULL || path[0] == '\0')
		return 1;
	if (strstr(path, "../") != NULL || strcmp(path, "/") == 0)
		return 1;

	return 0;
}
