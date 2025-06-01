
#include "u8mbtowc.h"
#include <stdio.h>

int
wc_to_utf8(char * dest, size_t dest_len, tmt_wchar_t src) {
	if (src <= 0x7f) {
		if (dest_len > 0) {
			dest[0] = src;
			return 1;
		}
	} else if (src <= 0x7ff) {
		if (dest_len > 1) {
			dest[0] = 0xc0 | ((src >> 6) & 0x1f);
			dest[1] = 0x80 | (src & 0x3f);
			return 2;
		}
	} else if (src <= 0xffff) {
		if (dest_len > 2) {
			dest[0] = 0xe0 | ((src >> 12) & 0x0f);
			dest[1] = 0x80 | ((src >> 6) & 0x3f);
			dest[2] = 0x80 | (src & 0x3f);
			return 3;
		}
	} else if (src <= 0x10ffff) {
		if (dest_len > 3) {
			dest[0] = 0xf0 | ((src >> 18) & 0x07);
			dest[1] = 0x80 | ((src >> 12) & 0x3f);
			dest[2] = 0x80 | ((src >> 6) & 0x3f);
			dest[3] = 0x80 | (src & 0x3f);
			return 4;
		}
	}
	return -1;
}

int
utf8_to_wc(tmt_wchar_t * dest, const char * src, size_t src_len, struct utf8_state * state) {
	int src_idx = 0;
	while (src_idx < src_len) {
		unsigned char cur = src[src_idx++];
		switch (state->pos) {
			case 0:
				if (cur < 0x80) {
					if (dest != NULL) {
						*dest = cur;
					}
					state->pos = 0;
					return src_idx;
				}
				if (cur < 0xc2) {
					state->pos = 0;
					return UTF8_INVALID;
				}
				state->data[0] = cur;
				state->pos += 1;
				break;
			case 1:
				if ((state->data[0] >= 0xc2) && (state->data[0] < 0xe0)) {
					if (cur < 0x80) {
						state->pos = 0;
						return UTF8_INVALID;
					}
					if (dest != NULL) {
						*dest = ((state->data[0] & 0x1f) << 6) | (cur & 0x3f);
					}
					state->pos = 0;
					return src_idx;
				}
				state->data[1] = cur;
				state->pos += 1;
				break;
			case 2:
				if ((state->data[0] >= 0xe0) && (state->data[0] < 0xf0)) {
					if ((state->data[1] < 0x80) || (cur < 0x80)) {
						state->pos = 0;
						return UTF8_INVALID;
					}
					if (dest != NULL) {
						*dest = ((state->data[0] & 0x0f) << 12) | ((state->data[1] & 0x3f) << 6) | (cur & 0x3f);
					}
					state->pos = 0;
					return src_idx;
				}
				state->data[2] = cur;
				state->pos += 1;
				break;
			case 3:
				if (state->data[0] >= 0xf0) {
					if ((state->data[1] < 0x80) || (state->data[2] < 0x80) || (cur < 0x80)) {
						state->pos = 0;
						return UTF8_INVALID;
					}
					if (dest != NULL) {
						*dest = ((state->data[0] & 0x07) << 18) | ((state->data[1] & 0x3f) << 12) | ((state->data[2] & 0x3f) << 6) | (cur & 0x3f);
					}
					state->pos = 0;
					return src_idx;
				}
				state->pos = 0;
				return UTF8_INVALID;
		}
	}

	return UTF8_INCOMPLETE;
}
