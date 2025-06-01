#ifndef PTY_PROXY_H
#define PTY_PROXY_H

enum LineTagCode {
	UNDEFINED,
	BOLD,
	DIM,
	UNDERLINE,
	BLINK,
	REVERSE,
	INVISIBLE,
	FULLWIDTH,
	FG_COLOR_BLACK,
	FG_COLOR_RED,
	FG_COLOR_GREEN,
	FG_COLOR_YELLOW,
	FG_COLOR_BLUE,
	FG_COLOR_MAGENTA,
	FG_COLOR_CYAN,
	FG_COLOR_WHITE,
	FG_COLOR_RGB,
	FG_COLOR_BRIGHT_BLACK,
	FG_COLOR_BRIGHT_RED,
	FG_COLOR_BRIGHT_GREEN,
	FG_COLOR_BRIGHT_YELLOW,
	FG_COLOR_BRIGHT_BLUE,
	FG_COLOR_BRIGHT_MAGENTA,
	FG_COLOR_BRIGHT_CYAN,
	FG_COLOR_BRIGHT_WHITE,
	BG_COLOR_BLACK,
	BG_COLOR_RED,
	BG_COLOR_GREEN,
	BG_COLOR_YELLOW,
	BG_COLOR_BLUE,
	BG_COLOR_MAGENTA,
	BG_COLOR_CYAN,
	BG_COLOR_WHITE,
	BG_COLOR_RGB,
	BG_COLOR_BRIGHT_BLACK,
	BG_COLOR_BRIGHT_RED,
	BG_COLOR_BRIGHT_GREEN,
	BG_COLOR_BRIGHT_YELLOW,
	BG_COLOR_BRIGHT_BLUE,
	BG_COLOR_BRIGHT_MAGENTA,
	BG_COLOR_BRIGHT_CYAN,
	BG_COLOR_BRIGHT_WHITE,
};

struct LineTag {
	LineTagCode code;
	unsigned char red;
	unsigned char green;
	unsigned char blue;
};

class TermRenderer {
public:
	virtual void update_cursor(int row, int col) = 0;
	virtual void screen_begin() = 0;
	virtual void screen_set_row(int row) = 0;
	virtual void screen_add_tag(LineTag tag) = 0;
	virtual void screen_remove_tag(LineTag tag) = 0;
	virtual void screen_add_glyph(const char *c, int len) = 0;
	virtual void screen_done() = 0;
	virtual void scroll_begin(int size) = 0;
	virtual void scroll_done() = 0;
	virtual void play_bell() = 0;
	virtual void show_cursor(bool flag) = 0;
	virtual void resize_complete() = 0;
	virtual void exited() = 0;
	virtual void log_pty_input(const char *data) = 0;
	virtual void log_vt_handler_input(unsigned char *data, int data_len) = 0;
	virtual ~TermRenderer() {}
};

class PtyProxy {
public:
	PtyProxy();
	virtual ~PtyProxy();

	void set_renderer(TermRenderer *renderer) {
		_renderer = renderer;
	}

	virtual int send_string(const char *data) = 0;
	virtual int available_to_send() = 0;
	virtual void resize_screen(int nrows, int ncols) = 0;

	// Callback methods
	void _screen_begin();
	void _screen_set_row(int idx);
	void _screen_add_glyph(char *buffer, int numchars);
	void _screen_add_tag(LineTag tag);
	void _screen_remove_tag(LineTag tag);
	void _screen_done();
	void _scroll_begin(int size);
	void _scroll_done();
	void _update_cursor(int row, int col);
	void _play_bell();
	void _show_cursor(bool flag);
	void _log_pty_input(const char *msg);

protected:
	TermRenderer *_renderer;

	struct TMT *_tmt;
	int _num_rows;
	int _num_cols;

	void _handle_from_pty(unsigned char *buffer, int len);
	void _handle_pty_exited();
	void _apply_resize();
};

#endif // PTY_PROXY_H
