#ifndef U8MBTOWC_H
#define U8MBTOWC_H

#include "tmt.h"

struct utf8_state {
   	unsigned char data[4];
	int pos;
};

#define UTF8_INCOMPLETE -2
#define UTF8_INVALID    -1

int utf8_to_wc(tmt_wchar_t * dest, const char * src, size_t src_len, struct utf8_state * state);
int wc_to_utf8(char * dest, size_t dest_len, tmt_wchar_t src);

#endif
