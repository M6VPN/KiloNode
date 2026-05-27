/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/compat/compat_risk.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "kilonode/compat_risk.h"

static enum kn_compat_risk_error add_risk(
	struct kn_compat_risk_register *, const char *, const char *,
	enum kn_compat_risk_severity, const char *, const char *,
	enum kn_compat_risk_status);

void
kn_compat_risk_register_clear(struct kn_compat_risk_register *reg)
{
	if (reg == NULL)
		return;
	memset(reg, 0, sizeof(*reg));
}

const char *
kn_compat_risk_error_name(enum kn_compat_risk_error error)
{
	switch (error) {
	case KN_COMPAT_RISK_OK:
		return "ok";
	case KN_COMPAT_RISK_ERR_INVALID_ARGUMENT:
		return "invalid-argument";
	case KN_COMPAT_RISK_ERR_BUFFER:
		return "buffer";
	case KN_COMPAT_RISK_ERR_INVALID_VALUE:
		return "invalid-value";
	}
	return "unknown";
}

const char *
kn_compat_risk_severity_name(enum kn_compat_risk_severity severity)
{
	switch (severity) {
	case KN_COMPAT_RISK_LOW:
		return "low";
	case KN_COMPAT_RISK_MEDIUM:
		return "medium";
	case KN_COMPAT_RISK_HIGH:
		return "high";
	case KN_COMPAT_RISK_CRITICAL:
		return "critical";
	}
	return "unknown";
}

const char *
kn_compat_risk_status_name(enum kn_compat_risk_status status)
{
	switch (status) {
	case KN_COMPAT_RISK_OPEN:
		return "open";
	case KN_COMPAT_RISK_MITIGATED:
		return "mitigated";
	case KN_COMPAT_RISK_ACCEPTED:
		return "accepted";
	case KN_COMPAT_RISK_DEFERRED:
		return "deferred";
	}
	return "unknown";
}

enum kn_compat_risk_error
kn_compat_risk_register_default(struct kn_compat_risk_register *reg)
{
	enum kn_compat_risk_error rc;

	if (reg == NULL)
		return KN_COMPAT_RISK_ERR_INVALID_ARGUMENT;
	kn_compat_risk_register_clear(reg);
#define RISK(id, title, sev, area, mitigation, status) do { \
	rc = add_risk(reg, (id), (title), (sev), (area), (mitigation), \
	    (status)); \
	if (rc != KN_COMPAT_RISK_OK) return rc; \
} while (0)
	RISK("R001", "GPL contamination risk", KN_COMPAT_RISK_CRITICAL,
	    "clean-room", "use black-box observations only",
	    KN_COMPAT_RISK_OPEN);
	RISK("R002", "prompt or output copying risk", KN_COMPAT_RISK_HIGH,
	    "clean-room", "store outputs as observations, not code",
	    KN_COMPAT_RISK_OPEN);
	RISK("R003", "command ambiguity risk", KN_COMPAT_RISK_MEDIUM,
	    "node", "require manual black-box coverage",
	    KN_COMPAT_RISK_OPEN);
	RISK("R004", "connected-mode AX.25 prerequisite",
	    KN_COMPAT_RISK_CRITICAL, "transport",
	    "block CONNECT planning until state machine exists",
	    KN_COMPAT_RISK_DEFERRED);
	RISK("R005", "NET/ROM prerequisite", KN_COMPAT_RISK_HIGH,
	    "routing", "keep NODES and ROUTES blocked", KN_COMPAT_RISK_DEFERRED);
	RISK("R006", "BBS mailbox format unknown", KN_COMPAT_RISK_HIGH,
	    "bbs", "observe mailbox behaviour externally", KN_COMPAT_RISK_OPEN);
	RISK("R007", "forwarding protocol unknown", KN_COMPAT_RISK_HIGH,
	    "forwarding", "capture protocol boundary only",
	    KN_COMPAT_RISK_DEFERRED);
	RISK("R008", "unsafe transmit risk", KN_COMPAT_RISK_CRITICAL,
	    "tx", "keep TX behind existing gates", KN_COMPAT_RISK_OPEN);
	RISK("R009", "user and sysop authentication risk", KN_COMPAT_RISK_HIGH,
	    "auth", "defer sysop commands until auth model exists",
	    KN_COMPAT_RISK_DEFERRED);
	RISK("R010", "KiloNode-native shell regression",
	    KN_COMPAT_RISK_MEDIUM, "node", "separate compatibility mode",
	    KN_COMPAT_RISK_OPEN);
#undef RISK
	return KN_COMPAT_RISK_OK;
}

enum kn_compat_risk_error
kn_compat_risk_register_report(const struct kn_compat_risk_register *reg,
	char *buf, size_t bufsiz)
{
	size_t severity[4];
	size_t status[4];
	size_t off;
	size_t i;
	int needed;

	if (reg == NULL || buf == NULL || bufsiz == 0)
		return KN_COMPAT_RISK_ERR_INVALID_ARGUMENT;
	memset(severity, 0, sizeof(severity));
	memset(status, 0, sizeof(status));
	for (i = 0; i < reg->risk_count; i++) {
		severity[reg->risks[i].severity]++;
		status[reg->risks[i].status]++;
	}
	needed = snprintf(buf, bufsiz,
	    "RISK-REGISTER count=%llu critical=%llu high=%llu open=%llu "
	    "deferred=%llu\n",
	    (unsigned long long)reg->risk_count,
	    (unsigned long long)severity[KN_COMPAT_RISK_CRITICAL],
	    (unsigned long long)severity[KN_COMPAT_RISK_HIGH],
	    (unsigned long long)status[KN_COMPAT_RISK_OPEN],
	    (unsigned long long)status[KN_COMPAT_RISK_DEFERRED]);
	if (needed < 0 || (size_t)needed >= bufsiz)
		return KN_COMPAT_RISK_ERR_BUFFER;
	off = (size_t)needed;
	for (i = 0; i < reg->risk_count; i++) {
		needed = snprintf(buf + off, bufsiz - off,
		    "RISK id=%s severity=%s status=%s area=%s title=\"%s\" "
		    "mitigation=\"%s\"\n",
		    reg->risks[i].id,
		    kn_compat_risk_severity_name(reg->risks[i].severity),
		    kn_compat_risk_status_name(reg->risks[i].status),
		    reg->risks[i].area, reg->risks[i].title,
		    reg->risks[i].mitigation);
		if (needed < 0 || (size_t)needed >= bufsiz - off)
			return KN_COMPAT_RISK_ERR_BUFFER;
		off += (size_t)needed;
	}
	return KN_COMPAT_RISK_OK;
}

static enum kn_compat_risk_error
add_risk(struct kn_compat_risk_register *reg, const char *id,
	const char *title, enum kn_compat_risk_severity severity,
	const char *area, const char *mitigation,
	enum kn_compat_risk_status status)
{
	struct kn_compat_risk *risk;

	if (reg->risk_count >= KN_COMPAT_RISK_MAX)
		return KN_COMPAT_RISK_ERR_INVALID_VALUE;
	risk = &reg->risks[reg->risk_count++];
	(void)snprintf(risk->id, sizeof(risk->id), "%s", id);
	(void)snprintf(risk->title, sizeof(risk->title), "%s", title);
	risk->severity = severity;
	(void)snprintf(risk->area, sizeof(risk->area), "%s", area);
	(void)snprintf(risk->mitigation, sizeof(risk->mitigation), "%s",
	    mitigation);
	risk->status = status;
	return KN_COMPAT_RISK_OK;
}
