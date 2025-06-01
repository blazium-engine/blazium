#ifndef KEY_CONVERTER_H
#define KEY_CONVERTER_H

#include "core/os/keyboard.h"

void fill_term_string(char *buffer, int buf_len, wchar_t unicode, Key key, bool send_alt_meta_as_esc);

#endif // KEY_CONVERTER_H
