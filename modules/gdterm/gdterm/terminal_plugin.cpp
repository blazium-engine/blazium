#include "terminal_plugin.h"
#include "core/input/shortcut.h"
#include "editor/editor_node.h"
#include "editor/gui/editor_bottom_panel.h"
#include "scene/gui/box_container.h"
#include "scene/gui/margin_container.h"
#include "scene/gui/scroll_bar.h"

void TerminalPlugin::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			if (terminal) {
				if (!terminal->is_active()) {
					terminal->start();
				}
			}

			if (scrollbar) {
				// Initialize scrollbar values based on GDTerm's scroll state.
				scrollbar->set_min(0);
				scrollbar->set_page(terminal->get_num_screen_lines());
				scrollbar->set_max(terminal->get_num_scrollback_lines() + terminal->get_num_screen_lines());
				scrollbar->set_value(terminal->get_num_scrollback_lines());
			}
		} break;
	}
}

void TerminalPlugin::_on_scrollback_changed() {
	if (scrollbar && terminal) {
		scrollbar_changing = true;
		scrollbar->set_max(terminal->get_num_scrollback_lines() + terminal->get_num_screen_lines());
		scrollbar->set_page(terminal->get_num_screen_lines());
		scrollbar->set_value(terminal->get_scroll_pos());
		scrollbar_changing = false;
	}
}

void TerminalPlugin::_on_scrollbar_value_changed(double p_value) {
	if (!scrollbar_changing && terminal) {
		int row = round(p_value);
		if (row != terminal->get_scroll_pos()) {
			terminal->set_scroll_pos(row);
		}
	}
}

bool TerminalPlugin::_handle_gui_input(const Ref<InputEvent> &p_event) {
	Ref<InputEventMouseButton> mb = p_event;
	if (mb.is_valid() && mb->is_pressed()) {
		if (mb->get_button_index() == MouseButton::WHEEL_UP) {
			if (scrollbar->get_value() > scrollbar->get_min()) {
				scrollbar->set_value(scrollbar->get_value() - 1);
			}
			return true;
		} else if (mb->get_button_index() == MouseButton::WHEEL_DOWN) {
			if (scrollbar->get_value() < terminal->get_num_scrollback_lines()) {
				scrollbar->set_value(scrollbar->get_value() + 1);
			}
			return true;
		}
	}
	return false;
}

void TerminalPlugin::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_on_scrollback_changed"), &TerminalPlugin::_on_scrollback_changed);
	ClassDB::bind_method(D_METHOD("_on_scrollbar_value_changed", "value"), &TerminalPlugin::_on_scrollbar_value_changed);
	ClassDB::bind_method(D_METHOD("_handle_gui_input", "event"), &TerminalPlugin::_handle_gui_input);
}

void TerminalPlugin::make_visible(bool p_visible) {
	if (p_visible) {
		EditorNode::get_bottom_panel()->make_item_visible(container);
	}
}

TerminalPlugin::TerminalPlugin() {
	// Create and add the terminal UI as a MarginContainer.
	container = memnew(MarginContainer);
	container->set_h_size_flags(Control::SizeFlags::SIZE_EXPAND_FILL);
	container->set_v_size_flags(Control::SizeFlags::SIZE_EXPAND_FILL);

	// Use HBoxContainer to layout GDTerm and VScrollBar side by side.
	hbox = memnew(HBoxContainer);
	hbox->set_h_size_flags(Control::SizeFlags::SIZE_EXPAND_FILL);
	hbox->set_v_size_flags(Control::SizeFlags::SIZE_EXPAND_FILL);

	terminal = memnew(GDTerm);
	terminal->set_h_size_flags(Control::SizeFlags::SIZE_EXPAND_FILL);
	terminal->set_v_size_flags(Control::SizeFlags::SIZE_EXPAND_FILL);
	terminal->connect("scrollback_changed", callable_mp(this, &TerminalPlugin::_on_scrollback_changed));
	terminal->connect("gui_input", callable_mp(this, &TerminalPlugin::_handle_gui_input));

	scrollbar = memnew(VScrollBar);
	scrollbar->set_h_size_flags(Control::SizeFlags::SIZE_SHRINK_END);
	scrollbar->set_v_size_flags(Control::SizeFlags::SIZE_EXPAND_FILL);
	scrollbar->connect("value_changed", callable_mp(this, &TerminalPlugin::_on_scrollbar_value_changed));

	// Add terminal and scrollbar to HBoxContainer, and HBoxContainer to MarginContainer.
	hbox->add_child(terminal);
	hbox->add_child(scrollbar);
	container->add_child(hbox);
	EditorNode::get_bottom_panel()->add_item(TTR("Terminal"), container);
}

TerminalPlugin::~TerminalPlugin() {
	if (container) {
		EditorNode::get_bottom_panel()->remove_item(container);
		memdelete(container);
	}
}
