/**************************************************************************/
/*  split_container.cpp                                                   */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#include "split_container.h"

#include "scene/main/viewport.h"
#include "scene/theme/theme_db.h"

void SplitContainerDragger::gui_input(const Ref<InputEvent> &p_event) {
	ERR_FAIL_COND(p_event.is_null());

	if (!sc->dragging_enabled) {
		return;
	}

	Ref<InputEventMouseButton> mb = p_event;

	if (mb.is_valid() && mb->get_button_index() == MouseButton::LEFT) {
		if (mb->is_pressed()) {
			sc->_compute_middle_sep(true);
			dragging = true;
			sc->emit_signal(SNAME("drag_started"));
			drag_ofs = sc->split_offset;
			if (sc->vertical) {
				drag_from = get_transform().xform(mb->get_position()).y;
			} else {
				drag_from = get_transform().xform(mb->get_position()).x;
			}
		} else {
			dragging = false;
			sc->emit_signal(SNAME("drag_ended"));
			get_viewport()->update_mouse_cursor_state();
		}
		queue_redraw();
	}

	Ref<InputEventMouseMotion> mm = p_event;
	if (mm.is_valid() && dragging) {
		Control *first = sc->_get_child(0);
		Control *second = sc->_get_child(1);
		if (!first || !second) {
			return;
		}

		bool first_visible = first->is_visible();
		bool second_visible = second && second->is_visible();
		Vector2i in_parent_pos = get_transform().xform(mm->get_position());
		bool vertical = sc->vertical;
		int axis = vertical ? 1 : 0;
		bool rtl = !vertical && is_layout_rtl();

		if (sc->collapse_mode != SplitContainer::COLLAPSE_NONE) {
			Control *target = nullptr;
			bool target_visible = false;

			if (sc->collapse_mode != SplitContainer::COLLAPSE_SECOND) {
				int limit = rtl ? sc->get_size()[axis] - first->get_combined_minimum_size()[axis] * 0.5 : first->get_combined_minimum_size()[axis] * 0.5;
				bool error = rtl ? in_parent_pos[axis] <= limit : in_parent_pos[axis] >= limit;
				if (error) {
					if (!first_visible) {
						target = first;
						target_visible = true;
					}
				} else if (first_visible) {
					target = first;
				}
			}

			if (sc->collapse_mode != SplitContainer::COLLAPSE_FIRST) {
				int limit = rtl ? second->get_combined_minimum_size()[axis] * 0.5 : sc->get_size()[axis] - second->get_combined_minimum_size()[axis] * 0.5;
				bool error = rtl ? in_parent_pos[axis] >= limit : in_parent_pos[axis] <= limit;
				if (error) {
					if (!second_visible) {
						target = second;
						target_visible = true;
					}
				} else if (second_visible) {
					target = second;
				}
			}

			if (target) {
				target->set_visible(target_visible);
				sc->child_collapsed = !target_visible;
				int ms = target->get_combined_minimum_size()[axis];
				drag_ofs += target_visible ? (target == second ? ms : -ms) : (target == second ? -ms : ms);
			}
		}

		if (!sc->child_collapsed) {
			if (rtl) {
				sc->split_offset = drag_ofs - (in_parent_pos.x - drag_from);
			} else {
				sc->split_offset = drag_ofs + (vertical ? in_parent_pos.y : in_parent_pos.x) - drag_from;
			}
		}
		sc->_compute_middle_sep(true);
		sc->_resort_children();
		sc->emit_signal(SNAME("dragged"), sc->get_split_offset());
	}
}

Control::CursorShape SplitContainerDragger::get_cursor_shape(const Point2 &p_pos) const {
	if (!sc->collapsed && sc->dragging_enabled) {
		return (sc->vertical ? CURSOR_VSPLIT : CURSOR_HSPLIT);
	}
	return Control::get_cursor_shape(p_pos);
}

void SplitContainerDragger::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_MOUSE_ENTER: {
			mouse_inside = true;
			queue_redraw();
		} break;

		case NOTIFICATION_MOUSE_EXIT: {
			mouse_inside = false;
			queue_redraw();
		} break;

		case NOTIFICATION_DRAW: {
			bool skip_style_bg = true;
			bool skip_grabber_icon = true;

			if (sc->dragger_visibility == SplitContainer::DRAGGER_VISIBLE) {
				bool can_draw = dragging || mouse_inside;
				skip_style_bg = !sc->theme_cache.draw_split_bar || (!can_draw && (!mouse_inside && sc->theme_cache.autohide_split_bar));
				skip_grabber_icon = !sc->theme_cache.draw_grabber_icon || (!can_draw && !sc->child_collapsed && (!mouse_inside && sc->theme_cache.autohide));
			}

			if (!skip_style_bg) {
				int sep = sc->_get_separation();
				Size2 size = get_size();
				bool vertical = sc->vertical;
				Point2 ofs = vertical ? Point2(0, Math::round((size.height - sep) * 0.5)) : Point2(Math::round((size.width - sep) * 0.5), 0);
				draw_style_box(sc->_get_split_bar_background(), Rect2(ofs, vertical ? Size2(size.width, sep) : Size2(sep, size.height)));
			}

			if (!skip_grabber_icon) {
				Ref<Texture2D> tex = sc->_get_grabber_icon();
				Color grabber_color = dragging ? sc->theme_cache.grabber_icon_pressed : sc->theme_cache.grabber_icon_normal;
				draw_texture(tex, ((get_size() - tex->get_size()) * 0.5).round(), grabber_color);
			}

#ifdef TOOLS_ENABLED
			if (sc->show_drag_area && Engine::get_singleton()->is_editor_hint()) {
				draw_rect(Rect2(Vector2(0, 0), get_size()), sc->dragging_enabled ? Color(1, 1, 0, 0.3) : Color(1, 0, 0, 0.3));
			}
#endif // TOOLS_ENABLED
		} break;
	}
}

SplitContainerDragger::SplitContainerDragger(SplitContainer *p_sc) {
	sc = p_sc;

	set_mouse_filter(sc->dragging_enabled ? MOUSE_FILTER_STOP : MOUSE_FILTER_IGNORE);
}

Control *SplitContainer::_get_child(int p_idx) const {
	if (p_idx >= get_child_count(false)) {
		return nullptr;
	}

	int idx = 0;
	for (int i = 0; i < get_child_count(false); i++) {
		Control *c = Object::cast_to<Control>(get_child(i, false));
		if (!c) {
			continue;
		}

		if (collapse_mode == COLLAPSE_NONE && !c->is_visible()) {
			continue;
		}

		if (idx == p_idx) {
			return c;
		}

		idx++;
	}
	return nullptr;
}

Ref<Texture2D> SplitContainer::_get_grabber_icon() const {
	if (is_fixed) {
		return theme_cache.grabber_icon;
	} else {
		if (vertical) {
			return theme_cache.grabber_icon_v;
		} else {
			return theme_cache.grabber_icon_h;
		}
	}
}

Ref<StyleBox> SplitContainer::_get_split_bar_background() const {
	if (is_fixed) {
		return theme_cache.split_bar_background;
	} else {
		if (vertical) {
			return theme_cache.split_bar_background_v;
		} else {
			return theme_cache.split_bar_background_h;
		}
	}
}

int SplitContainer::_get_separation() const {
	if (dragger_visibility == DRAGGER_HIDDEN_COLLAPSED) {
		return 0;
	}

	int sep = MAX(theme_cache.separation, 0);
	if (theme_cache.draw_grabber_icon) {
		sep = MAX(sep, vertical ? _get_grabber_icon()->get_height() : _get_grabber_icon()->get_width());
	}
	return sep;
}

Size2 SplitContainer::get_minimum_size() const {
	Control *first = _get_child(0);
	if (!first) {
		return Size2();
	}

	Size2 ms;
	Control *second = _get_child(1);
	int axis = vertical ? 1 : 0;
	bool dragger_visible = dragger_visibility != DRAGGER_HIDDEN_COLLAPSED;

	if (first->is_visible() || (collapse_mode == COLLAPSE_FIRST || collapse_mode == COLLAPSE_ALL)) {
		ms = first->get_combined_minimum_size();
	} else {
		dragger_visible = false;
	}

	if (second && (second->is_visible() || (collapse_mode == COLLAPSE_SECOND || collapse_mode == COLLAPSE_ALL))) {
		Size2 ms2 = second->get_combined_minimum_size();
		int cross_axis = vertical ? 0 : 1;
		ms[axis] += ms2[axis];
		ms[cross_axis] = MAX(ms[cross_axis], ms2[cross_axis]);
	} else {
		dragger_visible = false;
	}

	if (dragger_visible) {
		ms[axis] += _get_separation();
	}

	return ms;
}

void SplitContainer::_validate_property(PropertyInfo &p_property) const {
	if (is_fixed && p_property.name == "vertical") {
		p_property.usage = PROPERTY_USAGE_NONE;
	}
}

void SplitContainer::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_LAYOUT_DIRECTION_CHANGED:
		case NOTIFICATION_TRANSLATION_CHANGED: {
			queue_sort();
		} break;

		case NOTIFICATION_SORT_CHILDREN: {
			_resort_children();
		} break;

		case NOTIFICATION_THEME_CHANGED: {
			dragger_control->queue_redraw();
			update_minimum_size();
		} break;
	}
}

void SplitContainer::_resort_children() {
	if (!is_inside_tree()) {
		return;
	}
	child_collapsed = false;

	Control *first = _get_child(0);
	if (!first) {
		dragger_control->hide();
		return;
	}
	bool first_visible = first && first->is_visible();

	Control *second = _get_child(1);
	if (!second) {
		if (first_visible) {
			first->set_rect(Rect2(Point2(), get_size()));
		}
		dragger_control->hide();
		return;
	}
	bool second_visible = second && second->is_visible();

	if (!first_visible && !second_visible) {
		dragger_control->hide();
		return;
	}

	bool dragger_visible = true;
	Size2 size = get_size();
	bool rtl = is_layout_rtl();

	// One child is hidden.
	if ((first_visible && !second_visible) || (second_visible && !first_visible)) {
		bool first_collapsed = !first_visible && (collapse_mode == COLLAPSE_FIRST || collapse_mode == COLLAPSE_ALL);
		bool second_collapsed = !second_visible && (collapse_mode == COLLAPSE_SECOND || collapse_mode == COLLAPSE_ALL);
		Control *target = first_visible ? first : second;

		child_collapsed = first_collapsed || second_collapsed;
		dragger_visible = child_collapsed;
		dragger_control->set_visible(dragger_visible && !collapsed);

		if (!dragger_visible) {
			target->set_rect(Rect2(Point2(), size));
		} else {
			_compute_middle_sep(false);
			int sep = _get_separation();
			int dragger_thickness = MAX(sep, theme_cache.minimum_grab_thickness);
			int dragger_ofs = Math::round((dragger_thickness - sep) * 0.5);
			Point2 dragger_begin;
			Point2 target_begin;
			if (first_visible) {
				if (vertical) {
					dragger_begin = Point2(0, size.height - sep - dragger_ofs);
				} else {
					dragger_begin = rtl ? Point2(0, -dragger_ofs) : Point2(size.width - sep - dragger_ofs, 0);
					target_begin = rtl ? Point2(sep, 0) : Point2();
				}
			} else {
				if (vertical) {
					dragger_begin = Point2(0, -dragger_ofs);
					target_begin = Point2(0, sep);
				} else {
					dragger_begin = rtl ? Point2(size.width - sep - dragger_ofs, 0) : Point2(-dragger_ofs, 0);
					target_begin = rtl ? Point2() : Point2(sep, 0);
				}
			}
			dragger_control->set_rect(Rect2(dragger_begin, vertical ? Size2(size.width, dragger_thickness) : Size2(dragger_thickness, size.height)));
			target->set_rect(Rect2(target_begin, vertical ? Point2(size.width, size.height - sep) : Point2(size.width - sep, size.height)));
		}
		dragger_control->queue_redraw();
		return;
	}

	dragger_control->set_visible(dragger_visible && !collapsed);

	_compute_middle_sep(false);

	int sep = _get_separation();
	int dragger_thickness = MAX(sep, theme_cache.minimum_grab_thickness);
	int dragger_ofs = Math::round((dragger_thickness - sep) * 0.5);
	int dragger_pos = middle_sep;

	if (vertical) {
		first->set_rect(Rect2(Point2(), Size2(size.width, dragger_pos)));
		second->set_rect(Rect2(Point2(0, dragger_pos + sep), Size2(size.width, size.height - dragger_pos - sep)));
		dragger_control->set_rect(Rect2(Point2(0, dragger_pos - dragger_ofs), Size2(size.width, dragger_thickness)));
	} else {
		if (rtl) {
			dragger_pos = size.width - dragger_pos - sep;
			second->set_rect(Rect2(Point2(), Size2(dragger_pos, size.height)));
			first->set_rect(Rect2(Point2(dragger_pos + sep, 0), Size2(size.width - dragger_pos - sep, size.height)));
		} else {
			first->set_rect(Rect2(Point2(), Size2(dragger_pos, size.height)));
			second->set_rect(Rect2(Point2(dragger_pos + sep, 0), Size2(size.width - dragger_pos - sep, size.height)));
		}
		dragger_control->set_rect(Rect2(Point2(dragger_pos - dragger_ofs, 0), Size2(dragger_thickness, size.height)));
	}
	dragger_control->queue_redraw();
}

void SplitContainer::_compute_middle_sep(bool p_clamp) {
	Control *first = _get_child(0);
	Control *second = _get_child(1);
	bool first_visible = first && first->is_visible();
	bool second_visible = second && second->is_visible();

	if (!first_visible && !second_visible) {
		return;
	}

	bool first_expanded = (vertical ? first->get_v_size_flags() : first->get_h_size_flags()) & SIZE_EXPAND;
	bool second_expanded = (vertical ? second->get_v_size_flags() : second->get_h_size_flags()) & SIZE_EXPAND;

	// A hack to fix split offset after resizing the container when a child is collapsed.
	if (p_clamp && child_collapsed) {
		split_offset = first_visible ? 1e6 : -1e6;
	}

	int wished_size = 0;
	int split_offset_with_collapse = 0;
	if (!collapsed) {
		split_offset_with_collapse = split_offset;
	}
	int axis = vertical ? 1 : 0;
	int first_ms = first->get_combined_minimum_size()[axis];
	int second_ms = second->get_combined_minimum_size()[axis];
	int sep = _get_separation();
	int size = get_size()[axis];

	if (first_expanded && second_expanded) {
		float ratio = first->get_stretch_ratio() / (first->get_stretch_ratio() + second->get_stretch_ratio());
		wished_size = size * ratio - sep * 0.5 + split_offset_with_collapse;
	} else if (first_expanded) {
		wished_size = size - sep + split_offset_with_collapse;
	} else {
		wished_size = split_offset_with_collapse;
	}
	middle_sep = CLAMP(wished_size, first_ms, size - sep - second_ms);

	if (p_clamp) {
		split_offset -= wished_size - middle_sep;
	}
}

void SplitContainer::set_split_offset(int p_offset) {
	if (child_collapsed || split_offset == p_offset) {
		return;
	}

	split_offset = p_offset;
	_resort_children();
}

int SplitContainer::get_split_offset() const {
	return split_offset;
}

void SplitContainer::clamp_split_offset() {
	if (!_get_child(0) || !_get_child(1)) {
		return;
	}

	_compute_middle_sep(true);
	queue_sort();
}

void SplitContainer::set_collapsed(bool p_collapsed) {
	if (collapsed == p_collapsed) {
		return;
	}

	collapsed = p_collapsed;
	_resort_children();
}

bool SplitContainer::is_collapsed() const {
	return collapsed;
}

void SplitContainer::set_dragging_enabled(bool p_enabled) {
	if (dragging_enabled == p_enabled) {
		return;
	}
	dragging_enabled = p_enabled;

	if (!dragging_enabled && dragger_control->dragging) {
		dragger_control->dragging = false;
		emit_signal(SNAME("drag_ended"));
	}

	dragger_control->set_mouse_filter(dragging_enabled ? MOUSE_FILTER_STOP : MOUSE_FILTER_IGNORE);
	dragger_control->queue_redraw();

	if (get_viewport()) {
		get_viewport()->update_mouse_cursor_state();
	}
}

bool SplitContainer::is_dragging_enabled() const {
	return dragging_enabled;
}

void SplitContainer::set_dragger_visibility(DraggerVisibility p_visibility) {
	if (dragger_visibility == p_visibility) {
		return;
	}

	dragger_visibility = p_visibility;

	dragger_control->queue_redraw();
	_resort_children();
}

SplitContainer::DraggerVisibility SplitContainer::get_dragger_visibility() const {
	return dragger_visibility;
}

void SplitContainer::set_vertical(bool p_vertical) {
	ERR_FAIL_COND_MSG(is_fixed, "Can't change orientation of " + get_class() + ".");
	if (vertical == p_vertical) {
		return;
	}

	vertical = p_vertical;
	update_minimum_size();
	_resort_children();
}

bool SplitContainer::is_vertical() const {
	return vertical;
}

void SplitContainer::set_collapse_mode(CollapseMode p_mode) {
	ERR_FAIL_INDEX(int(p_mode), 4);
	if (collapse_mode == p_mode) {
		return;
	}

	collapse_mode = p_mode;
	_resort_children();
}

SplitContainer::CollapseMode SplitContainer::get_collapse_mode() const {
	return collapse_mode;
}

void SplitContainer::set_show_drag_area(bool p_enabled) {
#ifdef TOOLS_ENABLED
	if (show_drag_area == p_enabled) {
		return;
	}

	show_drag_area = p_enabled;

	dragger_control->queue_redraw();
#endif // TOOLS_ENABLED
}

bool SplitContainer::is_showing_drag_area() const {
	return show_drag_area;
}

Vector<int> SplitContainer::get_allowed_size_flags_horizontal() const {
	Vector<int> flags;
	flags.append(SIZE_FILL);
	if (!vertical) {
		flags.append(SIZE_EXPAND);
	}
	flags.append(SIZE_SHRINK_BEGIN);
	flags.append(SIZE_SHRINK_CENTER);
	flags.append(SIZE_SHRINK_END);
	return flags;
}

Vector<int> SplitContainer::get_allowed_size_flags_vertical() const {
	Vector<int> flags;
	flags.append(SIZE_FILL);
	if (vertical) {
		flags.append(SIZE_EXPAND);
	}
	flags.append(SIZE_SHRINK_BEGIN);
	flags.append(SIZE_SHRINK_CENTER);
	flags.append(SIZE_SHRINK_END);
	return flags;
}

Control *SplitContainer::get_dragger_control() const {
	return dragger_control;
}

void SplitContainer::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_split_offset", "offset"), &SplitContainer::set_split_offset);
	ClassDB::bind_method(D_METHOD("get_split_offset"), &SplitContainer::get_split_offset);

	ClassDB::bind_method(D_METHOD("set_collapsed", "collapsed"), &SplitContainer::set_collapsed);
	ClassDB::bind_method(D_METHOD("is_collapsed"), &SplitContainer::is_collapsed);

	ClassDB::bind_method(D_METHOD("set_dragger_visibility", "mode"), &SplitContainer::set_dragger_visibility);
	ClassDB::bind_method(D_METHOD("get_dragger_visibility"), &SplitContainer::get_dragger_visibility);

	ClassDB::bind_method(D_METHOD("set_vertical", "vertical"), &SplitContainer::set_vertical);
	ClassDB::bind_method(D_METHOD("is_vertical"), &SplitContainer::is_vertical);

	ClassDB::bind_method(D_METHOD("set_collapse_mode", "mode"), &SplitContainer::set_collapse_mode);
	ClassDB::bind_method(D_METHOD("get_collapse_mode"), &SplitContainer::get_collapse_mode);

	ClassDB::bind_method(D_METHOD("set_drag_area_highlight_in_editor", "drag_area_highlight_in_editor"), &SplitContainer::set_show_drag_area);
	ClassDB::bind_method(D_METHOD("is_drag_area_highlight_in_editor_enabled"), &SplitContainer::is_showing_drag_area);

	ClassDB::bind_method(D_METHOD("set_dragging_enabled", "dragging_enabled"), &SplitContainer::set_dragging_enabled);
	ClassDB::bind_method(D_METHOD("is_dragging_enabled"), &SplitContainer::is_dragging_enabled);

	ClassDB::bind_method(D_METHOD("get_drag_area_control"), &SplitContainer::get_dragger_control);

#ifndef DISABLE_DEPRECATED
	ClassDB::bind_method(D_METHOD("clamp_split_offset"), &SplitContainer::clamp_split_offset);
#endif // !DISABLE_DEPRECATED

	ADD_SIGNAL(MethodInfo("dragged", PropertyInfo(Variant::INT, "offset")));
	ADD_SIGNAL(MethodInfo("drag_started"));
	ADD_SIGNAL(MethodInfo("drag_ended"));

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "dragging_enabled"), "set_dragging_enabled", "is_dragging_enabled");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "split_offset", PROPERTY_HINT_NONE, "suffix:px"), "set_split_offset", "get_split_offset");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "collapsed"), "set_collapsed", "is_collapsed");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "dragger_visibility", PROPERTY_HINT_ENUM, "Visible,Hidden,Hidden and Collapsed"), "set_dragger_visibility", "get_dragger_visibility");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "vertical"), "set_vertical", "is_vertical");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "collapse_mode", PROPERTY_HINT_ENUM, "None,First,Second,All"), "set_collapse_mode", "get_collapse_mode");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "drag_area_highlight_in_editor"), "set_drag_area_highlight_in_editor", "is_drag_area_highlight_in_editor_enabled");

	BIND_ENUM_CONSTANT(DRAGGER_VISIBLE);
	BIND_ENUM_CONSTANT(DRAGGER_HIDDEN);
	BIND_ENUM_CONSTANT(DRAGGER_HIDDEN_COLLAPSED);

	BIND_ENUM_CONSTANT(COLLAPSE_NONE);
	BIND_ENUM_CONSTANT(COLLAPSE_FIRST);
	BIND_ENUM_CONSTANT(COLLAPSE_SECOND);
	BIND_ENUM_CONSTANT(COLLAPSE_ALL);

	BIND_THEME_ITEM(Theme::DATA_TYPE_CONSTANT, SplitContainer, separation);
	BIND_THEME_ITEM(Theme::DATA_TYPE_CONSTANT, SplitContainer, minimum_grab_thickness);
	BIND_THEME_ITEM(Theme::DATA_TYPE_CONSTANT, SplitContainer, draw_grabber_icon);
	BIND_THEME_ITEM(Theme::DATA_TYPE_CONSTANT, SplitContainer, draw_split_bar);
	BIND_THEME_ITEM(Theme::DATA_TYPE_CONSTANT, SplitContainer, autohide);
	BIND_THEME_ITEM(Theme::DATA_TYPE_CONSTANT, SplitContainer, autohide_split_bar);

	BIND_THEME_ITEM(Theme::DATA_TYPE_COLOR, SplitContainer, grabber_icon_normal);
	BIND_THEME_ITEM(Theme::DATA_TYPE_COLOR, SplitContainer, grabber_icon_pressed);

	BIND_THEME_ITEM(Theme::DATA_TYPE_STYLEBOX, SplitContainer, split_bar_background);
	BIND_THEME_ITEM_CUSTOM(Theme::DATA_TYPE_STYLEBOX, SplitContainer, split_bar_background_h, "h_split_bar_background");
	BIND_THEME_ITEM_CUSTOM(Theme::DATA_TYPE_STYLEBOX, SplitContainer, split_bar_background_v, "v_split_bar_background");

	BIND_THEME_ITEM_CUSTOM(Theme::DATA_TYPE_ICON, SplitContainer, grabber_icon, "grabber");
	BIND_THEME_ITEM_CUSTOM(Theme::DATA_TYPE_ICON, SplitContainer, grabber_icon_h, "h_grabber");
	BIND_THEME_ITEM_CUSTOM(Theme::DATA_TYPE_ICON, SplitContainer, grabber_icon_v, "v_grabber");
}

SplitContainer::SplitContainer(bool p_vertical) {
	vertical = p_vertical;

	dragger_control = memnew(SplitContainerDragger(this));
	add_child(dragger_control, false, Node::INTERNAL_MODE_BACK);
}
