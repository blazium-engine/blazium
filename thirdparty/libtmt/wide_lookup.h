#ifndef TMT_WIDE_LOOKUP_H
#define TMT_WIDE_LOOKUP_H

#include "tmt.h"
#include <stdbool.h>

enum tmt_mark {
	TMT_NOT_MARK,
	TMT_MARK,
	TMT_MARK_FULLWIDTH,
	TMT_FORMAT,
};

typedef enum tmt_mark tmt_mark_t;

bool is_wc_unicode_full_width(tmt_wchar_t wc, bool is_east_asia);
tmt_mark_t get_wc_unicode_mark_type(tmt_wchar_t wc);

#endif
