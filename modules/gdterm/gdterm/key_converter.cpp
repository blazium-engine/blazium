#include "key_converter.h"

extern "C" {
#include "thirdparty/libtmt/tmt.h"
#include "thirdparty/libtmt/u8mbtowc.h"
}
#include <string.h>
#include <cassert>

const char *numbers[] = {
	"0",
	"1",
	"2",
	"3",
	"4",
	"5",
	"6",
	"7",
	"8",
	"9",
};

const char *symbols1[] = {
	" ",
	"!",
	"\"",
	"#",
	"$",
	"%",
	"&",
	"'",
	"(",
	")",
	"*",
	"+",
	",",
	"-",
	".",
	"/",
};

const char *symbols2[] = {
	":",
	";",
	"<",
	"=",
	">",
	"?",
};

const char *symbols3[] = {
	"[",
	"\\",
	"]",
	"_",
	"`",
};

const char *symbols4[] = {
	"{",
	"|",
	"}",
	"~",
};

void fill_term_string(char *buffer, int buf_len, wchar_t unicode, Key key, bool send_alt_meta_as_esc) {
	assert(buf_len > 5); // 4 for maximum unicode + 1 for prefixing with escape
	buffer[0] = '\0';
	Key code = key & KeyModifierMask::CODE_MASK;
	Key ctrl = key & KeyModifierMask::CTRL;
	Key alt = key & KeyModifierMask::ALT;
	Key meta = key & KeyModifierMask::META;
	switch (code) {
		case Key::NONE:
		case Key::SHIFT:
		case Key::CTRL:
		case Key::META:
		case Key::ALT:
		case Key::CAPSLOCK:
		case Key::NUMLOCK:
		case Key::SCROLLLOCK:
		case Key::PAUSE:
		case Key::PRINT:
		case Key::SYSREQ:
		case Key::CLEAR:
		case Key::SPECIAL:
		case Key::MENU:
		case Key::HYPER:
		case Key::HELP:
		case Key::BACK:
		case Key::FORWARD:
		case Key::STOP:
		case Key::REFRESH:
		case Key::VOLUMEDOWN:
		case Key::VOLUMEMUTE:
		case Key::VOLUMEUP:
		case Key::MEDIAPLAY:
		case Key::MEDIASTOP:
		case Key::MEDIAPREVIOUS:
		case Key::MEDIANEXT:
		case Key::MEDIARECORD:
		case Key::HOMEPAGE:
		case Key::FAVORITES:
		case Key::SEARCH:
		case Key::STANDBY:
		case Key::OPENURL:
		case Key::LAUNCHMAIL:
		case Key::LAUNCHMEDIA:
		case Key::LAUNCH0:
		case Key::LAUNCH1:
		case Key::LAUNCH2:
		case Key::LAUNCH3:
		case Key::LAUNCH4:
		case Key::LAUNCH5:
		case Key::LAUNCH6:
		case Key::LAUNCH7:
		case Key::LAUNCH8:
		case Key::LAUNCH9:
		case Key::LAUNCHA:
		case Key::LAUNCHB:
		case Key::LAUNCHC:
		case Key::LAUNCHD:
		case Key::LAUNCHE:
		case Key::LAUNCHF:
		case Key::GLOBE:
		case Key::KEYBOARD:
		case Key::JIS_EISU:
		case Key::JIS_KANA:
		case Key::UNKNOWN:
		case Key::F11:
		case Key::F12:
		case Key::F13:
		case Key::F14:
		case Key::F15:
		case Key::F16:
		case Key::F17:
		case Key::F18:
		case Key::F19:
		case Key::F20:
		case Key::F21:
		case Key::F22:
		case Key::F23:
		case Key::F24:
		case Key::F25:
		case Key::F26:
		case Key::F27:
		case Key::F28:
		case Key::F29:
		case Key::F30:
		case Key::F31:
		case Key::F32:
		case Key::F33:
		case Key::F34:
		case Key::F35:
			break;
		case Key::ESCAPE:
			strcpy(buffer, "\x1b");
			break;
		case Key::TAB:
			strcpy(buffer, "\t");
			break;
		case Key::BACKSPACE:
			if (ctrl != Key::NONE) {
				strcpy(buffer, TMT_KEY_BACKSPACE);
			} else {
				strcpy(buffer, "\x7f");
			}
			break;
		case Key::KEY_DELETE:
			strcpy(buffer, "\x1b[3~");
			break;
		case Key::KP_ENTER:
			strcpy(buffer, "\r");
			break;
		case Key::ENTER:
			strcpy(buffer, "\r");
			break;
		case Key::INSERT:
			strcpy(buffer, TMT_KEY_INSERT);
			break;
		case Key::HOME:
			strcpy(buffer, TMT_KEY_HOME);
			break;
		case Key::END:
			strcpy(buffer, TMT_KEY_END);
			break;
		case Key::LEFT:
			strcpy(buffer, TMT_KEY_LEFT);
			break;
		case Key::UP:
			strcpy(buffer, TMT_KEY_UP);
			break;
		case Key::RIGHT:
			strcpy(buffer, TMT_KEY_RIGHT);
			break;
		case Key::DOWN:
			strcpy(buffer, TMT_KEY_DOWN);
			break;
		case Key::PAGEUP:
			strcpy(buffer, TMT_KEY_PAGE_UP);
			break;
		case Key::PAGEDOWN:
			strcpy(buffer, TMT_KEY_PAGE_DOWN);
			break;
		case Key::F1:
			strcpy(buffer, TMT_KEY_F1);
			break;
		case Key::F2:
			strcpy(buffer, TMT_KEY_F2);
			break;
		case Key::F3:
			strcpy(buffer, TMT_KEY_F3);
			break;
		case Key::F4:
			strcpy(buffer, TMT_KEY_F4);
			break;
		case Key::F5:
			strcpy(buffer, TMT_KEY_F5);
			break;
		case Key::F6:
			strcpy(buffer, TMT_KEY_F6);
			break;
		case Key::F7:
			strcpy(buffer, TMT_KEY_F7);
			break;
		case Key::F8:
			strcpy(buffer, TMT_KEY_F8);
			break;
		case Key::F9:
			strcpy(buffer, TMT_KEY_F9);
			break;
		case Key::F10:
			strcpy(buffer, TMT_KEY_F10);
			break;
		default: {
			int pos = 0;
			if ((meta != Key::NONE || alt != Key::NONE) && send_alt_meta_as_esc) {
				buffer[pos++] = '\x1b';
			}
			int len = 1;
			if (ctrl != Key::NONE && (code >= Key::AT) && (code <= Key::BRACKETRIGHT)) {
				buffer[pos] = (char)(code - Key::AT);
				buffer[pos + 1] = '\0';
			} else if (ctrl != Key::NONE && (code == Key::ASCIITILDE)) {
				buffer[pos] = '\036';
				buffer[pos + 1] = '\0';
			} else if (ctrl != Key::NONE && (code == Key::SLASH)) {
				buffer[pos] = '\037';
				buffer[pos + 1] = '\0';
			} else {
				len = wc_to_utf8(&buffer[pos], buf_len, unicode);
				if (len < buf_len) {
					buffer[pos + len] = '\0';
				} else {
					buffer[pos + buf_len - 1] = '\0';
				}
			}
		}
	}
}
