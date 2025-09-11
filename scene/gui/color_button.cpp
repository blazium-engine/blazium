/**************************************************************************/
/*  color_button.cpp                                                      */
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

#include "color_button.h"

#include "scene/theme/theme_db.h"

Size2 ColorButton::get_minimum_size() const {
	return _get_final_minimum_size(_get_current_style()->get_minimum_size() + theme_cache.overbright_indicator->get_size());
}

void ColorButton::set_color_no_signal(const Color &p_color) {
	if (color != p_color) {
		color = p_color;
		queue_redraw();
		queue_accessibility_update();
	}
}

void ColorButton::set_color(const Color &p_color) {
	if (color != p_color) {
		color = p_color;
		queue_redraw();
		queue_accessibility_update();
		emit_signal(SNAME("color_changed"), color);
	}
}

Color ColorButton::get_color() const {
	return color;
}

void ColorButton::set_flat(bool p_enabled) {
	if (flat != p_enabled) {
		flat = p_enabled;
		queue_redraw();
	}
}

bool ColorButton::is_flat() const {
	return flat;
}

void ColorButton::set_edit_alpha(bool p_enabled) {
	if (edit_alpha == p_enabled) {
		return;
	}

	edit_alpha = p_enabled;
	notify_property_list_changed();
}

bool ColorButton::is_editing_alpha() const {
	return edit_alpha;
}

Ref<StyleBox> ColorButton::_get_current_style() const {
	BaseButton::DrawMode mode = get_draw_mode();
	if (mode == DRAW_NORMAL) {
		return theme_cache.normal;
	} else if (mode == DRAW_HOVER_PRESSED) {
		return has_theme_stylebox("hover_pressed") ? theme_cache.hover_pressed : theme_cache.pressed;
	} else if (mode == DRAW_PRESSED) {
		return theme_cache.pressed;
	} else if (mode == DRAW_HOVER) {
		return theme_cache.hover;
	} else {
		return theme_cache.disabled;
	}
}

void ColorButton::_validate_property(PropertyInfo &p_property) const {
	if (p_property.name == "color") {
		p_property.hint = edit_alpha ? PROPERTY_HINT_NONE : PROPERTY_HINT_COLOR_NO_ALPHA;
	}
}

void ColorButton::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ACCESSIBILITY_UPDATE: {
			RID ae = get_accessibility_element();
			ERR_FAIL_COND(ae.is_null());

			DisplayServer::get_singleton()->accessibility_update_set_role(ae, DisplayServer::AccessibilityRole::ROLE_COLOR_PICKER);
			DisplayServer::get_singleton()->accessibility_update_set_color_value(ae, color);
		} break;

		case NOTIFICATION_DRAW: {
			RID ci = get_canvas_item();
			Size2 size = get_size();

			if (!flat) {
				Ref<StyleBox> stylebox = _get_current_style();
				stylebox->draw(ci, Rect2(Point2(), size));
			}

			const Rect2 r = Rect2(theme_cache.normal->get_offset(), size - theme_cache.normal->get_minimum_size());
			theme_cache.background_icon->draw_rect(ci, r, true);
			RenderingServer::get_singleton()->canvas_item_add_rect(ci, r, color);

			if (color.r > 1 || color.g > 1 || color.b > 1) {
				// Draw an indicator to denote that the color is "overbright" and can't be displayed accurately in the preview
				draw_texture(theme_cache.overbright_indicator, theme_cache.normal->get_offset());
			}

			if (has_focus()) {
				theme_cache.focus->draw(ci, Rect2(Point2(), size));
			}
		} break;

		case NOTIFICATION_THEME_CHANGED: {
			update_minimum_size();
			queue_redraw();
		}
	}
}

void ColorButton::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_color_no_signal", "color"), &ColorButton::set_color_no_signal);
	ClassDB::bind_method(D_METHOD("set_color", "color"), &ColorButton::set_color);
	ClassDB::bind_method(D_METHOD("get_color"), &ColorButton::get_color);
	ClassDB::bind_method(D_METHOD("set_flat", "enabled"), &ColorButton::set_flat);
	ClassDB::bind_method(D_METHOD("is_flat"), &ColorButton::is_flat);
	ClassDB::bind_method(D_METHOD("set_edit_alpha", "enabled"), &ColorButton::set_edit_alpha);
	ClassDB::bind_method(D_METHOD("is_editing_alpha"), &ColorButton::is_editing_alpha);

	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "color"), "set_color_no_signal", "get_color");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "edit_alpha"), "set_edit_alpha", "is_editing_alpha");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "flat"), "set_flat", "is_flat");

	ADD_SIGNAL(MethodInfo("color_changed", PropertyInfo(Variant::COLOR, "color")));

	BIND_THEME_ITEM(Theme::DATA_TYPE_STYLEBOX, ColorButton, normal);
	BIND_THEME_ITEM(Theme::DATA_TYPE_STYLEBOX, ColorButton, hover);
	BIND_THEME_ITEM(Theme::DATA_TYPE_STYLEBOX, ColorButton, pressed);
	BIND_THEME_ITEM(Theme::DATA_TYPE_STYLEBOX, ColorButton, hover_pressed);
	BIND_THEME_ITEM(Theme::DATA_TYPE_STYLEBOX, ColorButton, disabled);
	BIND_THEME_ITEM(Theme::DATA_TYPE_STYLEBOX, ColorButton, focus);

	BIND_THEME_ITEM_CUSTOM(Theme::DATA_TYPE_ICON, ColorButton, background_icon, "bg");
	BIND_THEME_ITEM(Theme::DATA_TYPE_ICON, ColorButton, overbright_indicator);
}

ColorButton::ColorButton(const Color &p_color) {
	color = p_color;
}
