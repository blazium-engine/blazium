#ifndef TERMINAL_PLUGIN_H
#define TERMINAL_PLUGIN_H

#include "editor/plugins/editor_plugin.h"
#include "gdterm.h"

class MarginContainer;
class HBoxContainer;
class VScrollBar;

class TerminalPlugin : public EditorPlugin {
	GDCLASS(TerminalPlugin, EditorPlugin);

	MarginContainer *container;
	HBoxContainer *hbox;
	VScrollBar *scrollbar;
	GDTerm *terminal;
	bool scrollbar_changing = false;

	void _on_scrollback_changed();
	void _on_scrollbar_value_changed(double p_value);
	bool _handle_gui_input(const Ref<InputEvent> &p_event);

protected:
	void _notification(int p_what);
	static void _bind_methods();

public:
	virtual String get_plugin_name() const override { return "Terminal"; }
	bool has_main_screen() const override { return false; }
	virtual void make_visible(bool p_visible) override;

	TerminalPlugin();
	~TerminalPlugin();
};

#endif // TERMINAL_PLUGIN_H
