/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/proto/ax25_frame_builder.c */

#include <sys/types.h>

#include <string.h>

#include "kilonode/ax25_frame_builder.h"
#include "kilonode/buffer.h"

static enum kn_ax25_frame_builder_error append_addresses(
	const struct kn_ax25_frame_builder_request *, struct kn_buffer *);
static enum kn_ax25_frame_builder_error append_control(
	const struct kn_ax25_frame_builder_request *, struct kn_buffer *);
static enum kn_ax25_frame_builder_error ax25_rc(enum kn_ax25_error);
static enum kn_ax25_frame_builder_error control_for_type(
	const struct kn_ax25_frame_builder_request *, uint8_t *);
static enum kn_ax25_frame_builder_error plan_type_to_s(
	enum kn_ax25_frame_plan_type, enum kn_ax25_s_subtype *);
static enum kn_ax25_frame_builder_error plan_type_to_u(
	enum kn_ax25_frame_plan_type, enum kn_ax25_u_subtype *);

static enum kn_ax25_frame_builder_error
append_addresses(const struct kn_ax25_frame_builder_request *request,
	struct kn_buffer *out)
{
	struct kn_ax25_addr addr;
	size_t i;
	enum kn_ax25_error rc;

	memset(&addr, 0, sizeof(addr));
	addr.callsign = request->destination;
	rc = kn_ax25_address_encode(&addr, 0, out);
	if (rc != KN_AX25_OK)
		return ax25_rc(rc);

	memset(&addr, 0, sizeof(addr));
	addr.callsign = request->source;
	rc = kn_ax25_address_encode(&addr,
	    (uint8_t)(request->digipeater_count == 0), out);
	if (rc != KN_AX25_OK)
		return ax25_rc(rc);

	for (i = 0; i < request->digipeater_count; i++) {
		memset(&addr, 0, sizeof(addr));
		addr.callsign = request->digipeaters[i];
		rc = kn_ax25_address_encode(&addr,
		    (uint8_t)(i + 1 == request->digipeater_count), out);
		if (rc != KN_AX25_OK)
			return ax25_rc(rc);
	}

	return KN_AX25_FRAME_BUILDER_OK;
}

static enum kn_ax25_frame_builder_error
append_control(const struct kn_ax25_frame_builder_request *request,
	struct kn_buffer *out)
{
	uint8_t control;

	if (control_for_type(request, &control) !=
	    KN_AX25_FRAME_BUILDER_OK)
		return KN_AX25_FRAME_BUILDER_ERR_INVALID_VALUE;
	if (kn_buffer_append_byte(out, control) != 0)
		return KN_AX25_FRAME_BUILDER_ERR_BUFFER;

	if (request->type != KN_AX25_FRAME_PLAN_UI)
		return KN_AX25_FRAME_BUILDER_OK;

	if (kn_buffer_append_byte(out, request->pid) != 0)
		return KN_AX25_FRAME_BUILDER_ERR_BUFFER;
	if (kn_buffer_append(out, request->payload,
	    request->payload_len) != 0)
		return KN_AX25_FRAME_BUILDER_ERR_BUFFER;

	return KN_AX25_FRAME_BUILDER_OK;
}

static enum kn_ax25_frame_builder_error
ax25_rc(enum kn_ax25_error rc)
{
	switch (rc) {
	case KN_AX25_OK:
		return KN_AX25_FRAME_BUILDER_OK;
	case KN_AX25_ERR_TOO_MANY_DIGIS:
		return KN_AX25_FRAME_BUILDER_ERR_TOO_MANY_DIGIS;
	case KN_AX25_ERR_BUFFER:
		return KN_AX25_FRAME_BUILDER_ERR_BUFFER;
	case KN_AX25_ERR_INVALID_ARGUMENT:
	case KN_AX25_ERR_SHORT_FRAME:
	case KN_AX25_ERR_UNTERMINATED_ADDRESS:
	case KN_AX25_ERR_MALFORMED_ADDRESS:
	case KN_AX25_ERR_INVALID_SSID:
	case KN_AX25_ERR_MISSING_PID:
		break;
	}

	return KN_AX25_FRAME_BUILDER_ERR_INVALID_VALUE;
}

static enum kn_ax25_frame_builder_error
control_for_type(const struct kn_ax25_frame_builder_request *request,
	uint8_t *control)
{
	enum kn_ax25_s_subtype s_subtype;
	enum kn_ax25_u_subtype u_subtype;

	if (request->type == KN_AX25_FRAME_PLAN_RR ||
	    request->type == KN_AX25_FRAME_PLAN_RNR ||
	    request->type == KN_AX25_FRAME_PLAN_REJ) {
		if (plan_type_to_s(request->type, &s_subtype) !=
		    KN_AX25_FRAME_BUILDER_OK)
			return KN_AX25_FRAME_BUILDER_ERR_INVALID_VALUE;
		if (kn_ax25_control_encode_s(s_subtype, request->nr,
		    request->poll_final, control) != KN_AX25_CONTROL_OK)
			return KN_AX25_FRAME_BUILDER_ERR_INVALID_VALUE;
		return KN_AX25_FRAME_BUILDER_OK;
	}

	if (plan_type_to_u(request->type, &u_subtype) !=
	    KN_AX25_FRAME_BUILDER_OK)
		return KN_AX25_FRAME_BUILDER_ERR_INVALID_VALUE;
	if (kn_ax25_control_encode_u(u_subtype, request->poll_final,
	    control) != KN_AX25_CONTROL_OK)
		return KN_AX25_FRAME_BUILDER_ERR_INVALID_VALUE;

	return KN_AX25_FRAME_BUILDER_OK;
}

static enum kn_ax25_frame_builder_error
plan_type_to_s(enum kn_ax25_frame_plan_type type,
	enum kn_ax25_s_subtype *subtype)
{
	if (subtype == NULL)
		return KN_AX25_FRAME_BUILDER_ERR_INVALID_ARGUMENT;

	switch (type) {
	case KN_AX25_FRAME_PLAN_RR:
		*subtype = KN_AX25_S_SUBTYPE_RR;
		return KN_AX25_FRAME_BUILDER_OK;
	case KN_AX25_FRAME_PLAN_RNR:
		*subtype = KN_AX25_S_SUBTYPE_RNR;
		return KN_AX25_FRAME_BUILDER_OK;
	case KN_AX25_FRAME_PLAN_REJ:
		*subtype = KN_AX25_S_SUBTYPE_REJ;
		return KN_AX25_FRAME_BUILDER_OK;
	default:
		break;
	}

	return KN_AX25_FRAME_BUILDER_ERR_INVALID_VALUE;
}

static enum kn_ax25_frame_builder_error
plan_type_to_u(enum kn_ax25_frame_plan_type type,
	enum kn_ax25_u_subtype *subtype)
{
	if (subtype == NULL)
		return KN_AX25_FRAME_BUILDER_ERR_INVALID_ARGUMENT;

	switch (type) {
	case KN_AX25_FRAME_PLAN_SABM:
		*subtype = KN_AX25_U_SUBTYPE_SABM;
		return KN_AX25_FRAME_BUILDER_OK;
	case KN_AX25_FRAME_PLAN_SABME:
		*subtype = KN_AX25_U_SUBTYPE_SABME;
		return KN_AX25_FRAME_BUILDER_OK;
	case KN_AX25_FRAME_PLAN_UA:
		*subtype = KN_AX25_U_SUBTYPE_UA;
		return KN_AX25_FRAME_BUILDER_OK;
	case KN_AX25_FRAME_PLAN_DM:
		*subtype = KN_AX25_U_SUBTYPE_DM;
		return KN_AX25_FRAME_BUILDER_OK;
	case KN_AX25_FRAME_PLAN_DISC:
		*subtype = KN_AX25_U_SUBTYPE_DISC;
		return KN_AX25_FRAME_BUILDER_OK;
	case KN_AX25_FRAME_PLAN_FRMR:
		*subtype = KN_AX25_U_SUBTYPE_FRMR;
		return KN_AX25_FRAME_BUILDER_OK;
	case KN_AX25_FRAME_PLAN_UI:
		*subtype = KN_AX25_U_SUBTYPE_UI;
		return KN_AX25_FRAME_BUILDER_OK;
	default:
		break;
	}

	return KN_AX25_FRAME_BUILDER_ERR_INVALID_VALUE;
}

enum kn_ax25_frame_builder_error
kn_ax25_frame_builder_build(
	const struct kn_ax25_frame_builder_request *request, uint8_t *out,
	size_t out_len, size_t *written)
{
	struct kn_buffer frame;
	enum kn_ax25_frame_builder_error rc;

	if (request == NULL || out == NULL || written == NULL)
		return KN_AX25_FRAME_BUILDER_ERR_INVALID_ARGUMENT;
	if (request->digipeater_count > KN_AX25_MAX_DIGIS)
		return KN_AX25_FRAME_BUILDER_ERR_TOO_MANY_DIGIS;
	if (request->poll_final > 1 || request->nr > 7)
		return KN_AX25_FRAME_BUILDER_ERR_INVALID_VALUE;
	if (request->payload == NULL && request->payload_len > 0)
		return KN_AX25_FRAME_BUILDER_ERR_INVALID_ARGUMENT;

	*written = 0;
	if (kn_buffer_init(&frame, 0) != 0)
		return KN_AX25_FRAME_BUILDER_ERR_BUFFER;

	rc = append_addresses(request, &frame);
	if (rc != KN_AX25_FRAME_BUILDER_OK)
		goto out;
	rc = append_control(request, &frame);
	if (rc != KN_AX25_FRAME_BUILDER_OK)
		goto out;
	if (frame.len > out_len) {
		rc = KN_AX25_FRAME_BUILDER_ERR_TOO_LARGE;
		goto out;
	}

	memcpy(out, frame.data, frame.len);
	*written = frame.len;
	rc = KN_AX25_FRAME_BUILDER_OK;

out:
	kn_buffer_free(&frame);
	return rc;
}

enum kn_ax25_frame_builder_error
kn_ax25_frame_builder_build_plan(const struct kn_ax25_frame_plan *plan,
	uint8_t *out, size_t out_len, size_t *written)
{
	struct kn_ax25_frame_builder_request request;
	enum kn_ax25_frame_builder_error rc;

	rc = kn_ax25_frame_builder_request_from_plan(plan, &request);
	if (rc != KN_AX25_FRAME_BUILDER_OK)
		return rc;

	return kn_ax25_frame_builder_build(&request, out, out_len, written);
}

void
kn_ax25_frame_builder_request_clear(
	struct kn_ax25_frame_builder_request *request)
{
	if (request == NULL)
		return;

	memset(request, 0, sizeof(*request));
}

enum kn_ax25_frame_builder_error
kn_ax25_frame_builder_request_from_plan(
	const struct kn_ax25_frame_plan *plan,
	struct kn_ax25_frame_builder_request *request)
{
	size_t i;

	if (plan == NULL || request == NULL)
		return KN_AX25_FRAME_BUILDER_ERR_INVALID_ARGUMENT;
	if (kn_ax25_frame_plan_validate(plan) != KN_AX25_FRAME_PLAN_OK)
		return KN_AX25_FRAME_BUILDER_ERR_INVALID_VALUE;

	kn_ax25_frame_builder_request_clear(request);
	request->source = plan->source;
	request->destination = plan->destination;
	request->digipeater_count = plan->digipeater_count;
	for (i = 0; i < plan->digipeater_count; i++)
		request->digipeaters[i] = plan->digipeaters[i];
	request->type = plan->type;
	request->poll_final = plan->poll_final;
	request->nr = plan->nr;
	request->pid = plan->pid;
	request->payload = plan->payload;
	request->payload_len = plan->payload_len;

	return KN_AX25_FRAME_BUILDER_OK;
}
