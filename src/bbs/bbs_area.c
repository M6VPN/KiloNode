/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/bbs/bbs_area.c */

#include <sys/types.h>

#include <ctype.h>
#include <stdint.h>
#include <string.h>

#include "kilonode/bbs_area.h"

enum kn_bbs_area_error
kn_bbs_area_normalize(const char *input, char *out, size_t out_len)
{
	size_t i;

	if (input == NULL || out == NULL || out_len == 0)
		return KN_BBS_AREA_ERR_INVALID_ARGUMENT;
	if (input[0] == '\0')
		return KN_BBS_AREA_ERR_INVALID_NAME;

	for (i = 0; input[i] != '\0'; i++) {
		if (i + 1 >= out_len || i >= KN_MESSAGE_AREA_MAX)
			return KN_BBS_AREA_ERR_BUFFER;
		if (!(isalnum((unsigned char)input[i]) || input[i] == '_' ||
		    input[i] == '-'))
			return KN_BBS_AREA_ERR_INVALID_NAME;
		out[i] = (char)toupper((unsigned char)input[i]);
	}
	out[i] = '\0';
	return KN_BBS_AREA_OK;
}

uint8_t
kn_bbs_area_valid(const char *area)
{
	char normalized[KN_MESSAGE_AREA_MAX + 1];

	return kn_bbs_area_normalize(area, normalized, sizeof(normalized)) ==
	    KN_BBS_AREA_OK ? 1 : 0;
}
