/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/kiss.h */

#ifndef KILONODE_KISS_H
#define KILONODE_KISS_H

#include <sys/types.h>

#include <stdint.h>

#include "kilonode/buffer.h"

#define KN_KISS_FEND  0xc0
#define KN_KISS_FESC  0xdb
#define KN_KISS_TFEND 0xdc
#define KN_KISS_TFESC 0xdd

int kn_kiss_escape(const uint8_t *, size_t, struct kn_buffer *);
int kn_kiss_unescape(const uint8_t *, size_t, struct kn_buffer *);

#endif
