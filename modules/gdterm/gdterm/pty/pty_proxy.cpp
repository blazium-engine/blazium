#include "pty_proxy.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>

extern "C" {
#include "thirdparty/libtmt/tmt.h"
#include "thirdparty/libtmt/u8mbtowc.h"

int fill_chars(char *buf, size_t buf_size, TMTCHAR *chars, int start, int end) {
	char *pos = buf;
	int total = 0;
	for (int i = start; i < end; i++) {
		int len = wc_to_utf8(pos, buf_size - total, chars[i].c);
		if (len > 0) {
			pos += len;
			total += len;
		}
		for (size_t mark = 0; mark < chars[i].num_marks; mark++) {
			len = wc_to_utf8(pos, buf_size - total, chars[i].marks[mark]);
			if (len > 0) {
				pos += len;
				total += len;
			}
		}
	}
	return total;
}

LineTag get_fg_color_tag(tmt_color_t color) {
	struct LineTag tag;
	tag.code = LineTagCode::UNDEFINED;
	switch (color.code) {
		case TMT_COLOR_BLACK:
			tag.code = LineTagCode::FG_COLOR_BLACK;
			break;
		case TMT_COLOR_RED:
			tag.code = LineTagCode::FG_COLOR_RED;
			break;
		case TMT_COLOR_GREEN:
			tag.code = LineTagCode::FG_COLOR_GREEN;
			break;
		case TMT_COLOR_YELLOW:
			tag.code = LineTagCode::FG_COLOR_YELLOW;
			break;
		case TMT_COLOR_BLUE:
			tag.code = LineTagCode::FG_COLOR_BLUE;
			break;
		case TMT_COLOR_MAGENTA:
			tag.code = LineTagCode::FG_COLOR_MAGENTA;
			break;
		case TMT_COLOR_CYAN:
			tag.code = LineTagCode::FG_COLOR_CYAN;
			break;
		case TMT_COLOR_WHITE:
			tag.code = LineTagCode::FG_COLOR_WHITE;
			break;
		case TMT_COLOR_BRIGHT_BLACK:
			tag.code = LineTagCode::FG_COLOR_BRIGHT_BLACK;
			break;
		case TMT_COLOR_BRIGHT_RED:
			tag.code = LineTagCode::FG_COLOR_BRIGHT_RED;
			break;
		case TMT_COLOR_BRIGHT_GREEN:
			tag.code = LineTagCode::FG_COLOR_BRIGHT_GREEN;
			break;
		case TMT_COLOR_BRIGHT_YELLOW:
			tag.code = LineTagCode::FG_COLOR_BRIGHT_YELLOW;
			break;
		case TMT_COLOR_BRIGHT_BLUE:
			tag.code = LineTagCode::FG_COLOR_BRIGHT_BLUE;
			break;
		case TMT_COLOR_BRIGHT_MAGENTA:
			tag.code = LineTagCode::FG_COLOR_BRIGHT_MAGENTA;
			break;
		case TMT_COLOR_BRIGHT_CYAN:
			tag.code = LineTagCode::FG_COLOR_BRIGHT_CYAN;
			break;
		case TMT_COLOR_BRIGHT_WHITE:
			tag.code = LineTagCode::FG_COLOR_BRIGHT_WHITE;
			break;
		case TMT_COLOR_RGB:
			tag.code = LineTagCode::FG_COLOR_RGB;
			tag.red = color.red;
			tag.green = color.green;
			tag.blue = color.blue;
			break;
		case TMT_COLOR_DEFAULT:
		case TMT_COLOR_MAX:
			break;
	}
	return tag;
}

LineTag get_bg_color_tag(tmt_color_t color) {
	LineTag tag;
	tag.code = LineTagCode::UNDEFINED;
	switch (color.code) {
		case TMT_COLOR_BLACK:
			tag.code = LineTagCode::BG_COLOR_BLACK;
			break;
		case TMT_COLOR_RED:
			tag.code = LineTagCode::BG_COLOR_RED;
			break;
		case TMT_COLOR_GREEN:
			tag.code = LineTagCode::BG_COLOR_GREEN;
			break;
		case TMT_COLOR_YELLOW:
			tag.code = LineTagCode::BG_COLOR_YELLOW;
			break;
		case TMT_COLOR_BLUE:
			tag.code = LineTagCode::BG_COLOR_BLUE;
			break;
		case TMT_COLOR_MAGENTA:
			tag.code = LineTagCode::BG_COLOR_MAGENTA;
			break;
		case TMT_COLOR_CYAN:
			tag.code = LineTagCode::BG_COLOR_CYAN;
			break;
		case TMT_COLOR_WHITE:
			tag.code = LineTagCode::BG_COLOR_WHITE;
			break;
		case TMT_COLOR_BRIGHT_BLACK:
			tag.code = LineTagCode::BG_COLOR_BRIGHT_BLACK;
			break;
		case TMT_COLOR_BRIGHT_RED:
			tag.code = LineTagCode::BG_COLOR_BRIGHT_RED;
			break;
		case TMT_COLOR_BRIGHT_GREEN:
			tag.code = LineTagCode::BG_COLOR_BRIGHT_GREEN;
			break;
		case TMT_COLOR_BRIGHT_YELLOW:
			tag.code = LineTagCode::BG_COLOR_BRIGHT_YELLOW;
			break;
		case TMT_COLOR_BRIGHT_BLUE:
			tag.code = LineTagCode::BG_COLOR_BRIGHT_BLUE;
			break;
		case TMT_COLOR_BRIGHT_MAGENTA:
			tag.code = LineTagCode::BG_COLOR_BRIGHT_MAGENTA;
			break;
		case TMT_COLOR_BRIGHT_CYAN:
			tag.code = LineTagCode::BG_COLOR_BRIGHT_CYAN;
			break;
		case TMT_COLOR_BRIGHT_WHITE:
			tag.code = LineTagCode::BG_COLOR_BRIGHT_WHITE;
			break;
		case TMT_COLOR_RGB:
			tag.code = LineTagCode::BG_COLOR_RGB;
			tag.red = color.red;
			tag.green = color.green;
			tag.blue = color.blue;
			break;
		case TMT_COLOR_DEFAULT:
		case TMT_COLOR_MAX:
			break;
	}
	return tag;
}

int get_num_codes(int char_start_idx, int cidx, TMTLINE *line) {
	int num_codes = 0;
	for (int cpos = char_start_idx; cpos < cidx; cpos++) {
		num_codes += 1 + line->chars[cpos].num_marks;
	}
	return num_codes;
}

int add_glyph_chars(int char_start_idx, int cidx, TMTLINE *line, PtyProxy *proxy) {
	int maxchar = sizeof(tmt_wchar_t) * get_num_codes(char_start_idx, cidx, line);
	if (maxchar <= 0) {
		return cidx;
	}
	char *buffer = (char *)malloc(maxchar);
	int numchars = fill_chars(buffer, maxchar, line->chars, char_start_idx, cidx);
	if (numchars > 0) {
		proxy->_screen_add_glyph(buffer, numchars);
	}
	free(buffer);
	return cidx;
}

void handle_screen(PtyProxy *proxy, TMTSCREEN *screen) {
	bool bold = false;
	bool dim = false;
	bool underline = false;
	bool blink = false;
	bool reverse = false;
	bool invisible = false;
	for (size_t lidx = 0; lidx < screen->nline; lidx++) {
		bool fullwidth = false;
		tmt_color_t fg = tmt_color_t{ TMT_COLOR_DEFAULT, 0, 0, 0 };
		tmt_color_t bg = tmt_color_t{ TMT_COLOR_DEFAULT, 0, 0, 0 };
		TMTLINE *line = screen->lines[lidx];
		if (line->dirty) {
			proxy->_screen_set_row(lidx);
			int char_start_idx = 0;
			for (size_t cidx = 0; cidx < screen->ncol; cidx++) {
				if (line->chars[char_start_idx].char_type == TMT_IGNORED) {
					char_start_idx = cidx;
				} else {
					char_start_idx = add_glyph_chars(char_start_idx, cidx, line, proxy);
				}
				if (line->chars[cidx].char_type == TMT_FULLWIDTH) {
					if (!fullwidth) {
						fullwidth = true;
						proxy->_screen_add_tag(LineTag{ LineTagCode::FULLWIDTH, 0, 0, 0 });
					}
				}
				if (line->chars[cidx].char_type == TMT_HALFWIDTH) {
					if (fullwidth) {
						fullwidth = false;
						proxy->_screen_remove_tag(LineTag{ LineTagCode::FULLWIDTH, 0, 0, 0 });
					}
				}
				if (line->chars[cidx].char_type == TMT_FORMATTER) {
					if (fullwidth) {
						fullwidth = false;
						proxy->_screen_remove_tag(LineTag{ LineTagCode::FULLWIDTH, 0, 0, 0 });
					}
				}
				if (line->chars[cidx].a.bold != bold) {
					bold = line->chars[cidx].a.bold;
					if (bold) {
						proxy->_screen_add_tag(LineTag{ LineTagCode::BOLD, 0, 0, 0 });
					} else {
						proxy->_screen_remove_tag(LineTag{ LineTagCode::BOLD, 0, 0, 0 });
					}
				}
				if (line->chars[cidx].a.dim != dim) {
					dim = line->chars[cidx].a.dim;
					if (dim) {
						proxy->_screen_add_tag(LineTag{ LineTagCode::DIM, 0, 0, 0 });
					} else {
						proxy->_screen_remove_tag(LineTag{ LineTagCode::DIM, 0, 0, 0 });
					}
				}
				if (line->chars[cidx].a.underline != underline) {
					underline = line->chars[cidx].a.underline;
					if (underline) {
						proxy->_screen_add_tag(LineTag{ LineTagCode::UNDERLINE, 0, 0, 0 });
					} else {
						proxy->_screen_remove_tag(LineTag{ LineTagCode::UNDERLINE, 0, 0, 0 });
					}
				}
				if (line->chars[cidx].a.blink != blink) {
					blink = line->chars[cidx].a.blink;
					if (blink) {
						proxy->_screen_add_tag(LineTag{ LineTagCode::BLINK, 0, 0, 0 });
					} else {
						proxy->_screen_remove_tag(LineTag{ LineTagCode::BLINK, 0, 0, 0 });
					}
				}
				if (line->chars[cidx].a.reverse != reverse) {
					reverse = line->chars[cidx].a.reverse;
					if (reverse) {
						proxy->_screen_add_tag(LineTag{ LineTagCode::REVERSE, 0, 0, 0 });
					} else {
						proxy->_screen_remove_tag(LineTag{ LineTagCode::REVERSE, 0, 0, 0 });
					}
				}
				if (line->chars[cidx].a.invisible != invisible) {
					invisible = line->chars[cidx].a.invisible;
					if (invisible) {
						proxy->_screen_add_tag(LineTag{ LineTagCode::INVISIBLE, 0, 0, 0 });
					} else {
						proxy->_screen_remove_tag(LineTag{ LineTagCode::INVISIBLE, 0, 0, 0 });
					}
				}
				if ((line->chars[cidx].a.fg.code != fg.code) || (line->chars[cidx].a.fg.code == TMT_COLOR_RGB)) {
					if (fg.code >= TMT_COLOR_BLACK) {
						proxy->_screen_remove_tag(get_fg_color_tag(fg));
					}
					fg = line->chars[cidx].a.fg;
					if (fg.code >= TMT_COLOR_BLACK) {
						proxy->_screen_add_tag(get_fg_color_tag(fg));
					}
				}
				if ((line->chars[cidx].a.bg.code != bg.code) || (line->chars[cidx].a.bg.code == TMT_COLOR_RGB)) {
					if (bg.code >= TMT_COLOR_BLACK) {
						proxy->_screen_remove_tag(get_bg_color_tag(bg));
					}
					bg = line->chars[cidx].a.bg;
					if (bg.code >= TMT_COLOR_BLACK) {
						proxy->_screen_add_tag(get_bg_color_tag(bg));
					}
				}
			}
			char_start_idx = add_glyph_chars(char_start_idx, screen->ncol, line, proxy);
		}
	}
}

void tmt_callback(tmt_msg_t m, struct TMT *v, const void *r, void *p) {
	PtyProxy *proxy = (PtyProxy *)p;
	switch (m) {
		case TMT_MSG_MOVED: {
			TMTPOINT *point = (TMTPOINT *)r;
			proxy->_update_cursor(point->r, point->c);
			break;
		}
		case TMT_MSG_UPDATE: {
			proxy->_screen_begin();
			TMTSCREEN *screen = (TMTSCREEN *)r;
			handle_screen(proxy, screen);
			tmt_clean(v);
			proxy->_screen_done();
			break;
		}
		case TMT_MSG_SCROLL: {
			TMTSCREEN *screen = (TMTSCREEN *)r;
			int scroll_size = 0;
			for (size_t i = 0; i < screen->nline; i++) {
				if (!screen->lines[i]->dirty) {
					break;
				}
				scroll_size += 1;
			}
			proxy->_scroll_begin(scroll_size);
			handle_screen(proxy, screen);
			tmt_clean_scroll(v);
			proxy->_scroll_done();
			break;
		}
		case TMT_MSG_ANSWER: {
			const char *msg = (const char *)r;
			proxy->_log_pty_input(msg);
			proxy->send_string(msg);
			break;
		}
		case TMT_MSG_BELL: {
			proxy->_play_bell();
			break;
		}
		case TMT_MSG_CURSOR: {
			const char *condition = (const char *)r;
			if (strcmp(condition, "t") == 0) {
				proxy->_show_cursor(true);
			} else {
				proxy->_show_cursor(false);
			}
			break;
		}
	}
}
}

PtyProxy::PtyProxy() {
	_renderer = nullptr;
	_num_rows = 24;
	_num_cols = 80;
	_tmt = tmt_open(_num_rows, _num_cols, tmt_callback, this, NULL);
}

PtyProxy::~PtyProxy() {
	tmt_close(_tmt);
}

void PtyProxy::_handle_from_pty(unsigned char *data, int data_len) {
	if (_renderer != nullptr) {
		_renderer->log_vt_handler_input(data, data_len);
	}
	tmt_write(_tmt, (const char *)data, data_len);
}

void PtyProxy::_handle_pty_exited() {
	if (_renderer != nullptr) {
		_renderer->exited();
	}
}

void PtyProxy::_apply_resize() {
	tmt_resize(_tmt, _num_rows, _num_cols);
	_renderer->resize_complete();
}

void PtyProxy::_screen_begin() {
	if (_renderer != nullptr) {
		_renderer->screen_begin();
	}
}

void PtyProxy::_screen_set_row(int row) {
	if (_renderer != nullptr) {
		_renderer->screen_set_row(row);
	}
}

void PtyProxy::_screen_add_glyph(char *buf, int len) {
	if (_renderer != nullptr) {
		_renderer->screen_add_glyph(buf, len);
	}
}

void PtyProxy::_screen_add_tag(LineTag tag) {
	if (_renderer != nullptr) {
		_renderer->screen_add_tag(tag);
	}
}

void PtyProxy::_screen_remove_tag(LineTag tag) {
	if (_renderer != nullptr) {
		_renderer->screen_remove_tag(tag);
	}
}

void PtyProxy::_screen_done() {
	if (_renderer != nullptr) {
		_renderer->screen_done();
	}
}

void PtyProxy::_update_cursor(int row, int col) {
	if (_renderer != nullptr) {
		_renderer->update_cursor(row, col);
	}
}

void PtyProxy::_play_bell() {
	if (_renderer != nullptr) {
		_renderer->play_bell();
	}
}

void PtyProxy::_show_cursor(bool flag) {
	if (_renderer != nullptr) {
		_renderer->show_cursor(flag);
	}
}

void PtyProxy::_log_pty_input(const char *msg) {
	if (_renderer != nullptr) {
		_renderer->log_pty_input(msg);
	}
}

void PtyProxy::_scroll_begin(int size) {
	if (_renderer != nullptr) {
		_renderer->scroll_begin(size);
	}
}

void PtyProxy::_scroll_done() {
	if (_renderer != nullptr) {
		_renderer->scroll_done();
	}
}
