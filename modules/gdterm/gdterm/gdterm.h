#ifndef GDTERM_H
#define GDTERM_H

#include "core/input/input_event.h"
#include "pty/pty_proxy.h"
#include "scene/gui/control.h"
#include "scene/main/timer.h"
#include "scene/resources/font.h"

#include <fstream>
#include <string>
#include <vector>

static const int DIRECTIVE_UNKNOWN = 0;
static const int DIRECTIVE_SET_STATE = 1;
static const int DIRECTIVE_CLEAR_STATE = 2;
static const int DIRECTIVE_WRITE_GLYPH = 3;

struct GDTermLineDirectiveData {
	LineTag tag;
	std::string text;
};

struct GDTermLineDirective {
	int kind;
	GDTermLineDirectiveData data;
};

struct GDTermLine {
	int glyph_length;
	int selectable_length;
	std::vector<GDTermLineDirective> dirs;
};

class GDTerm : public Control, TermRenderer {
	GDCLASS(GDTerm, Control)

	Ref<Font> font;
	Ref<Font> dim_font;
	Ref<Font> bold_font;
	int font_size;
	Color black;
	Color red;
	Color green;
	Color yellow;
	Color blue;
	Color magenta;
	Color cyan;
	Color white;
	Color bright_black;
	Color bright_red;
	Color bright_green;
	Color bright_yellow;
	Color bright_blue;
	Color bright_magenta;
	Color bright_cyan;
	Color bright_white;
	Color foreground;
	Color background;
	String vt_handler_log_path;
	bool send_alt_meta_as_escape;

	PtyProxy *_proxy;

	bool _active;
	Vector2 _font_space_size;

	// Renderable data
	std::vector<GDTermLine> _scrollback;
	std::vector<GDTermLine> _screen_lines;
	int _scrollback_pos;

	// Updated on PtyProxy thread, read on scene thread
	std::mutex _pending_mutex;
	bool _pending_dirty;
	std::vector<GDTermLine> _pending_saved_scrollback;
	std::vector<GDTermLine> _pending_scrollback; // Empty if no new scrollback to be added
	std::vector<GDTermLine> _pending_screen_lines; // Always contains all screen lines
	int _pending_state;
	int _pending_target;
	int _pending_screen_row;
	bool _pending_cursor_dirty;
	int _pending_cursor_row;
	int _pending_cursor_col;
	int _pending_cursor_displayed;
	bool _drawing_active;
	int _pending_resize_row;
	int _pending_resize_col;

	// Cursor settings
	Timer *_cursor_timer;
	bool _cursor_displayed;
	bool _cursor_showing;
	int _cursor_blink_count;
	int _cursor_row;
	int _cursor_col;

	// Blinking settings
	Timer *_blink_timer;
	bool _blink_displayed;
	bool _blink_showing;

	// Size settings
	Vector2 _min_size;
	int _rows;
	int _cols;

	// Selection settings
	bool _selecting;
	bool _selection_active;
	int _select_start_row;
	int _select_start_col;
	int _select_end_row;
	int _select_end_col;
	uint64_t _select_tick;
	int _select_mode;

	// Send Input
	String _send_input_buffer;

	// Logging
	std::fstream *_vt_handler_input_log;

	// Terminal Settings
	bool _send_alt_meta_as_escape;

protected:
	static void _bind_methods();

public:
	GDTerm();
	~GDTerm();

	void set_font(const Ref<Font> &p_font);
	Ref<Font> get_font() const;

	void set_dim_font(const Ref<Font> &p_dim_font);
	Ref<Font> get_dim_font() const;

	void set_bold_font(const Ref<Font> &p_bold_font);
	Ref<Font> get_bold_font() const;

	void set_font_size(int p_font_size);
	int get_font_size() const;

	void set_black(Color c);
	Color get_black() const;

	void set_red(Color c);
	Color get_red() const;

	void set_green(Color c);
	Color get_green() const;

	void set_yellow(Color c);
	Color get_yellow() const;

	void set_blue(Color c);
	Color get_blue() const;

	void set_magenta(Color c);
	Color get_magenta() const;

	void set_cyan(Color c);
	Color get_cyan() const;

	void set_white(Color c);
	Color get_white() const;

	void set_bright_black(Color c);
	Color get_bright_black() const;

	void set_bright_red(Color c);
	Color get_bright_red() const;

	void set_bright_green(Color c);
	Color get_bright_green() const;

	void set_bright_yellow(Color c);
	Color get_bright_yellow() const;

	void set_bright_blue(Color c);
	Color get_bright_blue() const;

	void set_bright_magenta(Color c);
	Color get_bright_magenta() const;

	void set_bright_cyan(Color c);
	Color get_bright_cyan() const;

	void set_bright_white(Color c);
	Color get_bright_white() const;

	void set_foreground(Color c);
	Color get_foreground() const;

	void set_background(Color c);
	Color get_background() const;

	void set_vt_handler_log_path(String c);
	String get_vt_handler_log_path() const;

	void set_send_alt_meta_as_escape(bool f);
	bool get_send_alt_meta_as_escape() const;

	void clear();
	void start();
	void stop();
	String get_selected_text() const;
	void send_input(String text);
	bool is_active();

	int get_num_scrollback_lines() const;
	int get_num_screen_lines() const;
	int get_scroll_pos() const;
	void set_scroll_pos(int pos);

	void _process(double p_delta);
	void _ready();
	void _draw();

	void _notification(int p_what);
	virtual void gui_input(const Ref<InputEvent> &p_event) override;
	virtual Vector2 get_minimum_size() const override;

	virtual void update_cursor(int row, int col) override;
	virtual void screen_begin() override;
	virtual void screen_set_row(int row) override;
	virtual void screen_add_tag(LineTag tag) override;
	virtual void screen_remove_tag(LineTag tag) override;
	virtual void screen_add_glyph(const char *c, int len) override;
	virtual void screen_done() override;
	virtual void scroll_begin(int size) override;
	virtual void scroll_done() override;
	virtual void play_bell() override;
	virtual void show_cursor(bool flag) override;
	virtual void resize_complete() override;
	virtual void exited() override;
	virtual void log_pty_input(const char *data) override;
	virtual void log_vt_handler_input(unsigned char *data, int data_len) override;

private:
	void _make_pending_active();
	bool _clear_drawing();
	void _restart_cursor();
	bool _is_cursor_pos(int row, int col);
	bool _is_in_selection(int max_sel_col, int row, int col);
	bool _is_copy_request(Key code);
	bool _is_paste_request(Key code);
	bool _is_control_tab(Key code);
	bool _is_shift_control_tab(Key code);
	bool _is_control_c(Key code);
	void _do_resize();
	void _resize_pty();
	void _notify_scrollback();
	void _draw_term_line(Vector2 &pos, const GDTermLine &line, int cursor_row, int actual_row);
	bool _blink_on_line(GDTermLine &line) const;
	void _update_select_for_start_col(int row, int col);
	void _update_select_for_end_col(int row, int col);
	const GDTermLine &_get_term_line(int row) const;
	void _calc_select_word_col(const GDTermLine &line, int &start_col, int &end_col, int mouse_col) const;
	int _calc_line_size(const GDTermLine &line) const;
	void _send_input_chunk(int max_send);
	bool _is_screen_dirty();
	void _clear_pending();
	bool _resize_screen_lines();

	void _on_cursor_timeout();
	void _on_blink_timeout();
	void _on_bell_request();
	void _on_focus_entered();
	void _on_focus_exited();
	void _on_inactive();
	void _on_resized();
};

#endif // GDTERM_H
