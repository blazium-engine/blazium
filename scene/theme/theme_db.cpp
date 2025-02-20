/**************************************************************************/
/*  theme_db.cpp                                                          */
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

#include "theme_db.h"

#include "core/config/project_settings.h"
#include "core/io/resource_loader.h"
#include "scene/gui/control.h"
#include "scene/main/node.h"
#include "scene/main/window.h"
#include "scene/resources/font.h"
#include "scene/resources/image_texture.h"
#include "scene/resources/style_box.h"
#include "scene/resources/texture.h"
#include "servers/text_server.h"

#ifdef USE_LEGACY_THEME
#include "scene/theme/default_theme.h"
#else
#include "scene/theme/blazium_default_theme.h"

// Default engine theme creation and configuration.

Color ThemeDB::_get_font_color() const {
	Color fcolor = Color(0.875, 0.875, 0.875);
	FontColorOverride fcolor_override = (FontColorOverride)(int)GLOBAL_GET("gui/theme/font_color_override");
	bool is_dark_theme = ((Color)GLOBAL_GET("gui/theme/base_color")).get_luminance() <= 0.5;
	if (fcolor_override == FONT_COLOR_OVERRIDE_DARK || (fcolor_override == FONT_COLOR_OVERRIDE_AUTO && !is_dark_theme)) {
		fcolor = Color(0.05, 0.05, 0.05);
	} else if (fcolor_override == FONT_COLOR_OVERRIDE_CUSTOM) {
		fcolor = GLOBAL_GET("gui/theme/custom_font_color");
	}
	return fcolor;
}
#endif // USE_LEGACY_THEME

void ThemeDB::initialize_theme() {
	// Default theme-related project settings.

#ifdef USE_LEGACY_THEME
	// Allow creating the default theme at a different scale to suit higher/lower base resolutions.
	float default_theme_scale = GLOBAL_DEF(PropertyInfo(Variant::FLOAT, "gui/theme/default_theme_scale", PROPERTY_HINT_RANGE, "0.5,8,0.01", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_RESTART_IF_CHANGED), 1.0);

	String project_font_path = GLOBAL_DEF_RST_BASIC(PropertyInfo(Variant::STRING, "gui/theme/custom_font", PROPERTY_HINT_FILE, "*.tres,*.res,*.otf,*.ttc,*.ttf,*.woff,*.woff2,*.fnt,*.font,*.pfb,*.pfm", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_RESTART_IF_CHANGED), "");

	TextServer::FontAntialiasing font_antialiasing = (TextServer::FontAntialiasing)(int)GLOBAL_DEF_RST(PropertyInfo(Variant::INT, "gui/theme/default_font_antialiasing", PROPERTY_HINT_ENUM, "None,Grayscale,LCD Subpixel", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_RESTART_IF_CHANGED), 1);
	TextServer::Hinting font_hinting = (TextServer::Hinting)(int)GLOBAL_DEF_RST(PropertyInfo(Variant::INT, "gui/theme/default_font_hinting", PROPERTY_HINT_ENUM, "None,Light,Normal", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_RESTART_IF_CHANGED), TextServer::HINTING_LIGHT);
	TextServer::SubpixelPositioning font_subpixel_positioning = (TextServer::SubpixelPositioning)(int)GLOBAL_DEF_RST(PropertyInfo(Variant::INT, "gui/theme/default_font_subpixel_positioning", PROPERTY_HINT_ENUM, "Disabled,Auto,One Half of a Pixel,One Quarter of a Pixel", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_RESTART_IF_CHANGED), TextServer::SUBPIXEL_POSITIONING_AUTO);
	TextServer::FontLCDSubpixelLayout lcd_subpixel_layout = (TextServer::FontLCDSubpixelLayout)(int)GLOBAL_DEF_RST(PropertyInfo(Variant::INT, "gui/theme/lcd_subpixel_layout", PROPERTY_HINT_ENUM, "Disabled,Horizontal RGB,Horizontal BGR,Vertical RGB,Vertical BGR"), 1);

	const bool font_msdf = GLOBAL_DEF_RST("gui/theme/default_font_multichannel_signed_distance_field", false);
	const bool font_generate_mipmaps = GLOBAL_DEF_RST("gui/theme/default_font_generate_mipmaps", false);
#endif // USE_LEGACY_THEME

	String project_theme_path = GLOBAL_DEF_RST_BASIC(PropertyInfo(Variant::STRING, "gui/theme/custom", PROPERTY_HINT_FILE, "*.tres,*.res,*.theme", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_RESTART_IF_CHANGED), "");

	// Attempt to load custom project theme and font.

	if (!project_theme_path.is_empty()) {
		Ref<Theme> theme = ResourceLoader::load(project_theme_path);
		if (theme.is_valid()) {
			set_project_theme(theme);
		} else {
			ERR_PRINT("Error loading custom project theme '" + project_theme_path + "'");
		}
	}

	// Always generate the default theme to serve as a fallback for all required theme definitions.

#ifdef USE_LEGACY_THEME
	String custom_font = project_font_path;
#endif // USE_LEGACY_THEME

	if (RenderingServer::get_singleton()) {
		Ref<Font> project_font;
		if (!custom_font.is_empty() && FileAccess::exists(custom_font)) {
			project_font = ResourceLoader::load(custom_font);
			if (project_font.is_null()) {
				ERR_PRINT("Error loading custom project font '" + custom_font + "'");
			}
#ifdef USE_LEGACY_THEME
			else {
				set_fallback_font(project_font);
			}
#endif // USE_LEGACY_THEME
		}

#ifdef USE_LEGACY_THEME
		make_default_theme(default_theme_scale, project_font, font_subpixel_positioning, font_hinting, font_antialiasing, lcd_subpixel_layout, font_msdf, font_generate_mipmaps);
#else
		make_default_theme(project_font, scale, font_subpixel_positioning, font_hinting, font_antialiasing, font_lcd_subpixel_layout, font_msdf, font_generate_mipmaps, base_color, accent_color, _get_font_color(), font_outline_color, contrast, normal_contrast, hover_contrast, pressed_contrast, bg_contrast, margin, padding, border_width, corner_radius, font_size, font_outline_size, font_embolden, font_spacing_glyph, font_spacing_space, font_spacing_top, font_spacing_bottom);
#endif // USE_LEGACY_THEME
	}

	_init_default_theme_context();
}

void ThemeDB::initialize_theme_noproject() {
	if (RenderingServer::get_singleton()) {
#ifdef USE_LEGACY_THEME
		make_default_theme(1.0, Ref<Font>());
#else
		make_default_theme(Ref<Font>());
#endif // USE_LEGACY_THEME
	}

	_init_default_theme_context();
}

void ThemeDB::finalize_theme() {
	if (!RenderingServer::get_singleton()) {
		WARN_PRINT("Finalizing theme when there is no RenderingServer is an error; check the order of operations.");
	}

	_finalize_theme_contexts();
#ifndef USE_LEGACY_THEME
	finalize_default_theme();
#endif // USE_LEGACY_THEME
	default_theme.unref();

	fallback_font.unref();
	fallback_icon.unref();
	fallback_stylebox.unref();
}

// Global Theme resources.

void ThemeDB::set_default_theme(const Ref<Theme> &p_default) {
	default_theme = p_default;
}

Ref<Theme> ThemeDB::get_default_theme() {
	return default_theme;
}

void ThemeDB::set_project_theme(const Ref<Theme> &p_project_default) {
	project_theme = p_project_default;
}

Ref<Theme> ThemeDB::get_project_theme() {
	return project_theme;
}

// Universal fallback values for theme item types.

void ThemeDB::set_fallback_base_scale(float p_base_scale) {
	if (fallback_base_scale == p_base_scale) {
		return;
	}

	fallback_base_scale = p_base_scale;
	emit_signal(SNAME("fallback_changed"));
}

float ThemeDB::get_fallback_base_scale() {
	return fallback_base_scale;
}

void ThemeDB::set_fallback_font(const Ref<Font> &p_font) {
	if (fallback_font == p_font) {
		return;
	}

	fallback_font = p_font;
	emit_signal(SNAME("fallback_changed"));
}

Ref<Font> ThemeDB::get_fallback_font() {
	return fallback_font;
}

void ThemeDB::set_fallback_font_size(int p_font_size) {
	if (fallback_font_size == p_font_size) {
		return;
	}

	fallback_font_size = p_font_size;
	emit_signal(SNAME("fallback_changed"));
}

int ThemeDB::get_fallback_font_size() {
	return fallback_font_size;
}

void ThemeDB::set_fallback_icon(const Ref<Texture2D> &p_icon) {
	if (fallback_icon == p_icon) {
		return;
	}

	fallback_icon = p_icon;
	emit_signal(SNAME("fallback_changed"));
}

Ref<Texture2D> ThemeDB::get_fallback_icon() {
	return fallback_icon;
}

void ThemeDB::set_fallback_stylebox(const Ref<StyleBox> &p_stylebox) {
	if (fallback_stylebox == p_stylebox) {
		return;
	}

	fallback_stylebox = p_stylebox;
	emit_signal(SNAME("fallback_changed"));
}

Ref<StyleBox> ThemeDB::get_fallback_stylebox() {
	return fallback_stylebox;
}

void ThemeDB::get_native_type_dependencies(const StringName &p_base_type, Vector<StringName> &r_result) {
	if (p_base_type == StringName()) {
		return;
	}

	// TODO: It may make sense to stop at Control/Window, because their parent classes cannot be used in
	// a meaningful way.
	if (!ClassDB::get_inheritance_chain_nocheck(p_base_type, r_result)) {
		r_result.push_back(p_base_type);
	}
}

// Global theme contexts.

ThemeContext *ThemeDB::create_theme_context(Node *p_node, Vector<Ref<Theme>> &p_themes) {
	ERR_FAIL_COND_V(!p_node->is_inside_tree(), nullptr);
	ERR_FAIL_COND_V(theme_contexts.has(p_node), nullptr);
	ERR_FAIL_COND_V(p_themes.is_empty(), nullptr);

	ThemeContext *context = memnew(ThemeContext);
	context->node = p_node;
	context->parent = get_nearest_theme_context(p_node);
	context->set_themes(p_themes);

	theme_contexts[p_node] = context;
	_propagate_theme_context(p_node, context);

	p_node->connect(SceneStringName(tree_exited), callable_mp(this, &ThemeDB::destroy_theme_context).bind(p_node));

	return context;
}

void ThemeDB::destroy_theme_context(Node *p_node) {
	ERR_FAIL_COND(!theme_contexts.has(p_node));

	p_node->disconnect(SceneStringName(tree_exited), callable_mp(this, &ThemeDB::destroy_theme_context));

	ThemeContext *context = theme_contexts[p_node];

	theme_contexts.erase(p_node);
	_propagate_theme_context(p_node, context->parent);

	memdelete(context);
}

void ThemeDB::_propagate_theme_context(Node *p_from_node, ThemeContext *p_context) {
	Control *from_control = Object::cast_to<Control>(p_from_node);
	Window *from_window = from_control ? nullptr : Object::cast_to<Window>(p_from_node);

	if (from_control) {
		from_control->set_theme_context(p_context);
	} else if (from_window) {
		from_window->set_theme_context(p_context);
	}

	for (int i = 0; i < p_from_node->get_child_count(); i++) {
		Node *child_node = p_from_node->get_child(i);

		// If the child is the root of another global context, stop the propagation
		// in this branch.
		if (theme_contexts.has(child_node)) {
			theme_contexts[child_node]->parent = p_context;
			continue;
		}

		_propagate_theme_context(child_node, p_context);
	}
}

void ThemeDB::_init_default_theme_context() {
	default_theme_context = memnew(ThemeContext);

	Vector<Ref<Theme>> themes;

	// Only add the project theme to the default context when running projects.

#ifdef TOOLS_ENABLED
	if (!Engine::get_singleton()->is_editor_hint()) {
		themes.push_back(project_theme);
	}
#else
	themes.push_back(project_theme);
#endif

	themes.push_back(default_theme);
	default_theme_context->set_themes(themes);
}

void ThemeDB::_finalize_theme_contexts() {
	if (default_theme_context) {
		memdelete(default_theme_context);
		default_theme_context = nullptr;
	}
	while (theme_contexts.size()) {
		HashMap<Node *, ThemeContext *>::Iterator E = theme_contexts.begin();
		memdelete(E->value);
		theme_contexts.remove(E);
	}
}

ThemeContext *ThemeDB::get_theme_context(Node *p_node) const {
	if (!theme_contexts.has(p_node)) {
		return nullptr;
	}

	return theme_contexts[p_node];
}

ThemeContext *ThemeDB::get_default_theme_context() const {
	return default_theme_context;
}

ThemeContext *ThemeDB::get_nearest_theme_context(Node *p_for_node) const {
	ERR_FAIL_COND_V(!p_for_node->is_inside_tree(), nullptr);

	Node *parent_node = p_for_node->get_parent();
	while (parent_node) {
		if (theme_contexts.has(parent_node)) {
			return theme_contexts[parent_node];
		}

		parent_node = parent_node->get_parent();
	}

	return nullptr;
}

// Theme item binding.

void ThemeDB::bind_class_item(Theme::DataType p_data_type, const StringName &p_class_name, const StringName &p_prop_name, const StringName &p_item_name, ThemeItemSetter p_setter) {
	ERR_FAIL_COND_MSG(theme_item_binds[p_class_name].has(p_prop_name), vformat("Failed to bind theme item '%s' in class '%s': already bound", p_prop_name, p_class_name));

	ThemeItemBind bind;
	bind.data_type = p_data_type;
	bind.class_name = p_class_name;
	bind.item_name = p_item_name;
	bind.setter = p_setter;

	theme_item_binds[p_class_name][p_prop_name] = bind;
	theme_item_binds_list[p_class_name].push_back(bind);
}

void ThemeDB::bind_class_external_item(Theme::DataType p_data_type, const StringName &p_class_name, const StringName &p_prop_name, const StringName &p_item_name, const StringName &p_type_name, ThemeItemSetter p_setter) {
	ERR_FAIL_COND_MSG(theme_item_binds[p_class_name].has(p_prop_name), vformat("Failed to bind theme item '%s' in class '%s': already bound", p_prop_name, p_class_name));

	ThemeItemBind bind;
	bind.data_type = p_data_type;
	bind.class_name = p_class_name;
	bind.item_name = p_item_name;
	bind.type_name = p_type_name;
	bind.external = true;
	bind.setter = p_setter;

	theme_item_binds[p_class_name][p_prop_name] = bind;
	theme_item_binds_list[p_class_name].push_back(bind);
}

void ThemeDB::update_class_instance_items(Node *p_instance) {
	ERR_FAIL_NULL(p_instance);

	// Use the hierarchy to initialize all inherited theme caches. Setters carry the necessary
	// context and will set the values appropriately.
	StringName class_name = p_instance->get_class();
	while (class_name != StringName()) {
		HashMap<StringName, HashMap<StringName, ThemeItemBind>>::Iterator E = theme_item_binds.find(class_name);
		if (E) {
			for (const KeyValue<StringName, ThemeItemBind> &F : E->value) {
				F.value.setter(p_instance, F.value.item_name, F.value.type_name);
			}
		}

		class_name = ClassDB::get_parent_class_nocheck(class_name);
	}
}

void ThemeDB::get_class_items(const StringName &p_class_name, List<ThemeItemBind> *r_list, bool p_include_inherited, Theme::DataType p_filter_type) {
	List<StringName> class_hierarchy;
	StringName class_name = p_class_name;
	while (class_name != StringName()) {
		class_hierarchy.push_front(class_name); // Put parent classes in front.
		class_name = ClassDB::get_parent_class_nocheck(class_name);
	}

	HashSet<StringName> inherited_props;
	for (const StringName &theme_type : class_hierarchy) {
		HashMap<StringName, List<ThemeItemBind>>::Iterator E = theme_item_binds_list.find(theme_type);
		if (E) {
			for (const ThemeItemBind &F : E->value) {
				if (p_filter_type != Theme::DATA_TYPE_MAX && F.data_type != p_filter_type) {
					continue;
				}
				if (inherited_props.has(F.item_name)) {
					continue; // Skip inherited properties.
				}
				if (F.external || F.class_name != p_class_name) {
					inherited_props.insert(F.item_name);

					if (!p_include_inherited) {
						continue; // Track properties defined in parent classes, and skip them.
					}
				}

				r_list->push_back(F);
			}
		}
	}
}

void ThemeDB::_sort_theme_items() {
	for (KeyValue<StringName, List<ThemeDB::ThemeItemBind>> &E : theme_item_binds_list) {
		E.value.sort_custom<ThemeItemBind::SortByType>();
	}
}

#ifndef USE_LEGACY_THEME
void ThemeDB::freeze_default_theme() {
	default_theme->freeze_change_propagation();
}

void ThemeDB::unfreeze_default_theme() {
	default_theme->unfreeze_and_propagate_changes();
}

bool ThemeDB::is_default_theme_frozen() const {
	return default_theme->is_frozen();
}

void ThemeDB::_update_default_theme() {
	bool update_icons = false;
	bool scale_changed = false;
	bool border_padding_changed = false;
	bool update_colors = false;

	float _scale = GLOBAL_GET("gui/theme/default_theme_scale");
	if (default_theme->get_default_base_scale() != _scale) {
		default_theme->set_default_base_scale(_scale);
		scale_changed = true;
	}

	Color _font_color = _get_font_color();
	if (font_color != _font_color) {
		if ((font_color.get_luminance() > 0.5) != (_font_color.get_luminance() > 0.5)) {
			update_colors = true;
		}
		font_color = _font_color;
		freeze_default_theme();
		update_font_color(default_theme, font_color);
		emit_signal(SNAME("font_color_changed"));
		update_icons = true;
	}

	Color _accent_color = GLOBAL_GET("gui/theme/accent_color");
	if (accent_color != _accent_color) {
		accent_color = _accent_color;
		update_icons = true;
		update_colors = true;
	}

	if (update_icons || scale_changed) {
		freeze_default_theme();
		update_theme_icons(default_theme, font_color, accent_color);
		emit_signal(SNAME("icons_changed"));
	}

	Color _font_outline_color = GLOBAL_GET("gui/theme/font_outline_color");
	if (font_outline_color != _font_outline_color) {
		font_outline_color = _font_outline_color;
		freeze_default_theme();
		update_font_outline_color(default_theme, font_outline_color);
		emit_signal(SNAME("font_outline_color_changed"));
	}

	int _margin = GLOBAL_GET("gui/theme/margin");
	if (margin != _margin || scale_changed) {
		margin = _margin;
		freeze_default_theme();
		update_theme_margins(default_theme, margin);
		emit_signal(SNAME("margin_changed"));
	}

	int _padding = GLOBAL_GET("gui/theme/padding");
	if (padding != _padding || scale_changed) {
		padding = _padding;
		freeze_default_theme();
		update_theme_padding(default_theme, padding);
		emit_signal(SNAME("padding_changed"));
		border_padding_changed = true;
	}

	int _corner_radius = GLOBAL_GET("gui/theme/corner_radius");
	if (corner_radius != _corner_radius || scale_changed) {
		corner_radius = _corner_radius;
		freeze_default_theme();
		update_theme_corner_radius(default_theme, corner_radius);
		emit_signal(SNAME("corner_radius_changed"));
	}

	int _border_width = GLOBAL_GET("gui/theme/border_width");
	if (border_width != _border_width || scale_changed) {
		border_width = _border_width;
		freeze_default_theme();
		update_theme_border_width(default_theme, border_width);
		emit_signal(SNAME("border_width_changed"));
		border_padding_changed = true;
	}

	int _font_outline_size = GLOBAL_GET("gui/theme/font_outline_size");
	if (font_outline_size != _font_outline_size || scale_changed) {
		font_outline_size = _font_outline_size;
		freeze_default_theme();
		update_font_outline_size(default_theme, font_outline_size);
		emit_signal(SNAME("font_outline_size_changed"));
	}

	int _font_size = GLOBAL_GET("gui/theme/font_size");
	if (font_size != _font_size) {
		font_size = _font_size;
		freeze_default_theme();
		update_font_size(default_theme, font_size);
		emit_signal(SNAME("font_size_changed"));
	}

	if (border_padding_changed || scale_changed) {
		freeze_default_theme();
		update_theme_border_padding(default_theme, border_width + padding);
		emit_signal(SNAME("border_padding_changed"));
	}

	Color _base_color = GLOBAL_GET("gui/theme/base_color");
	if (base_color != _base_color) {
		base_color = _base_color;
		update_colors = true;
	}

	float _contrast = GLOBAL_GET("gui/theme/contrast");
	if (contrast != _contrast) {
		contrast = _contrast;
		update_colors = true;
	}

	float _normal_contrast = GLOBAL_GET("gui/theme/normal_contrast");
	if (normal_contrast != _normal_contrast) {
		normal_contrast = _normal_contrast;
		update_colors = true;
	}

	float _hover_contrast = GLOBAL_GET("gui/theme/hover_contrast");
	if (hover_contrast != _hover_contrast) {
		hover_contrast = _hover_contrast;
		update_colors = true;
	}

	float _pressed_contrast = GLOBAL_GET("gui/theme/pressed_contrast");
	if (pressed_contrast != _pressed_contrast) {
		pressed_contrast = _pressed_contrast;
		update_colors = true;
	}

	float _bg_contrast = GLOBAL_GET("gui/theme/bg_contrast");
	if (bg_contrast != _bg_contrast) {
		bg_contrast = _bg_contrast;
		update_colors = true;
	}

	if (update_colors) {
		freeze_default_theme();
		update_theme_colors(default_theme, base_color, accent_color, contrast, normal_contrast, hover_contrast, pressed_contrast, bg_contrast);
		emit_signal(SNAME("colors_changed"));
	}

	float _font_embolden = GLOBAL_GET("gui/theme/font_embolden");
	if (font_embolden != _font_embolden) {
		font_embolden = _font_embolden;
		freeze_default_theme();
		update_font_embolden(font_embolden);
	}

	int _font_spacing_glyph = GLOBAL_GET("gui/theme/font_spacing_glyph");
	if (font_spacing_glyph != _font_spacing_glyph || scale_changed) {
		font_spacing_glyph = _font_spacing_glyph;
		freeze_default_theme();
		update_font_spacing_glyph(Math::round(font_spacing_glyph * _scale));
	}

	int _font_spacing_space = GLOBAL_GET("gui/theme/font_spacing_space");
	if (font_spacing_space != _font_spacing_space || scale_changed) {
		font_spacing_space = _font_spacing_space;
		freeze_default_theme();
		update_font_spacing_space(Math::round(font_spacing_space * _scale));
	}

	int _font_spacing_top = GLOBAL_GET("gui/theme/font_spacing_top");
	if (font_spacing_top != _font_spacing_top || scale_changed) {
		font_spacing_top = _font_spacing_top;
		freeze_default_theme();
		update_font_spacing_top(Math::round(font_spacing_top * _scale));
	}

	int _font_spacing_bottom = GLOBAL_GET("gui/theme/font_spacing_bottom");
	if (font_spacing_bottom != _font_spacing_bottom || scale_changed) {
		font_spacing_bottom = _font_spacing_bottom;
		freeze_default_theme();
		update_font_spacing_bottom(Math::round(font_spacing_bottom * _scale));
	}

	String _custom_font = GLOBAL_GET("gui/theme/custom_font");
	if (custom_font != _custom_font) {
		custom_font = _custom_font;
		Ref<Font> project_font;
		if (!custom_font.is_empty() && FileAccess::exists(custom_font)) {
			project_font = ResourceLoader::load(custom_font);
		} else {
			project_font = Ref<Font>();
		}
		freeze_default_theme();
		update_theme_font(default_theme, project_font);
		emit_signal(SNAME("font_changed"));
	}

	TextServer::SubpixelPositioning _font_subpixel_positioning = (TextServer::SubpixelPositioning)(int)GLOBAL_GET("gui/theme/default_font_subpixel_positioning");
	if (font_subpixel_positioning != _font_subpixel_positioning) {
		font_subpixel_positioning = _font_subpixel_positioning;
		freeze_default_theme();
		update_font_subpixel_positioning(font_subpixel_positioning);
	}

	TextServer::FontAntialiasing _font_antialiasing = (TextServer::FontAntialiasing)(int)GLOBAL_GET("gui/theme/default_font_antialiasing");
	if (font_antialiasing != _font_antialiasing) {
		font_antialiasing = _font_antialiasing;
		freeze_default_theme();
		update_font_antialiasing(font_antialiasing);
	}

	TextServer::FontLCDSubpixelLayout _font_lcd_subpixel_layout = (TextServer::FontLCDSubpixelLayout)(int)GLOBAL_GET("gui/theme/lcd_subpixel_layout");
	if (font_lcd_subpixel_layout != _font_lcd_subpixel_layout) {
		font_lcd_subpixel_layout = _font_lcd_subpixel_layout;
		freeze_default_theme();
		update_font_lcd_subpixel_layout(font_lcd_subpixel_layout);
	}

	TextServer::Hinting _font_hinting = (TextServer::Hinting)(int)GLOBAL_GET("gui/theme/default_font_hinting");
	if (font_hinting != _font_hinting) {
		font_hinting = _font_hinting;
		freeze_default_theme();
		update_font_hinting(font_hinting);
	}

	bool _font_msdf = GLOBAL_GET("gui/theme/default_font_multichannel_signed_distance_field");
	if (font_msdf != _font_msdf) {
		font_msdf = _font_msdf;
		freeze_default_theme();
		update_font_msdf(font_msdf);
	}

	bool _font_generate_mipmaps = GLOBAL_GET("gui/theme/default_font_generate_mipmaps");
	if (font_generate_mipmaps != _font_generate_mipmaps) {
		font_generate_mipmaps = _font_generate_mipmaps;
		freeze_default_theme();
		update_font_generate_mipmaps(font_generate_mipmaps);
	}

	if (scale_changed) {
		freeze_default_theme();
		update_theme_scale(default_theme);
		emit_signal(SNAME("scale_changed"));
	}

	if (is_default_theme_frozen()) {
		callable_mp(this, &ThemeDB::unfreeze_default_theme).call_deferred();
		emit_signal(SNAME("theme_changed"));
	}
}

Error ThemeDB::theme_add_user_icon(const String &p_icon_name, const String &p_icon_source) {
	return add_user_icon(p_icon_name, p_icon_source, scale, font_color, accent_color);
}

Error ThemeDB::theme_remove_user_icon(const String &p_icon_name) {
	return remove_user_icon(p_icon_name);
}

bool ThemeDB::theme_has_user_icon(const String &p_icon_name) {
	return has_user_icon(p_icon_name);
}

Ref<ImageTexture> ThemeDB::theme_get_user_icon(const String &p_icon_name) {
	return get_user_icon(p_icon_name);
}

PackedStringArray ThemeDB::theme_get_user_icons_list() {
	return get_user_icons_list();
}

bool ThemeDB::theme_has_icon(const String &p_icon_name) {
	return has_icon(p_icon_name);
}

Ref<ImageTexture> ThemeDB::theme_get_icon(const String &p_icon_name) {
	return get_icon(p_icon_name);
}

PackedStringArray ThemeDB::theme_get_icons_list() {
	return get_icons_list();
}
#endif // !USE_LEGACY_THEME

// Object methods.

void ThemeDB::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_default_theme"), &ThemeDB::get_default_theme);
	ClassDB::bind_method(D_METHOD("get_project_theme"), &ThemeDB::get_project_theme);

	ClassDB::bind_method(D_METHOD("set_fallback_base_scale", "base_scale"), &ThemeDB::set_fallback_base_scale);
	ClassDB::bind_method(D_METHOD("get_fallback_base_scale"), &ThemeDB::get_fallback_base_scale);
	ClassDB::bind_method(D_METHOD("set_fallback_font", "font"), &ThemeDB::set_fallback_font);
	ClassDB::bind_method(D_METHOD("get_fallback_font"), &ThemeDB::get_fallback_font);
	ClassDB::bind_method(D_METHOD("set_fallback_font_size", "font_size"), &ThemeDB::set_fallback_font_size);
	ClassDB::bind_method(D_METHOD("get_fallback_font_size"), &ThemeDB::get_fallback_font_size);
	ClassDB::bind_method(D_METHOD("set_fallback_icon", "icon"), &ThemeDB::set_fallback_icon);
	ClassDB::bind_method(D_METHOD("get_fallback_icon"), &ThemeDB::get_fallback_icon);
	ClassDB::bind_method(D_METHOD("set_fallback_stylebox", "stylebox"), &ThemeDB::set_fallback_stylebox);
	ClassDB::bind_method(D_METHOD("get_fallback_stylebox"), &ThemeDB::get_fallback_stylebox);

#ifndef USE_LEGACY_THEME
	ClassDB::bind_method(D_METHOD("freeze_default_theme"), &ThemeDB::freeze_default_theme);
	ClassDB::bind_method(D_METHOD("unfreeze_default_theme"), &ThemeDB::unfreeze_default_theme);
	ClassDB::bind_method(D_METHOD("is_default_theme_frozen"), &ThemeDB::is_default_theme_frozen);
	ClassDB::bind_method(D_METHOD("add_user_icon", "icon_name", "icon_source"), &ThemeDB::theme_add_user_icon);
	ClassDB::bind_method(D_METHOD("remove_user_icon", "icon_name"), &ThemeDB::theme_remove_user_icon);
	ClassDB::bind_method(D_METHOD("has_user_icon", "icon_name"), &ThemeDB::theme_has_user_icon);
	ClassDB::bind_method(D_METHOD("get_user_icon", "icon_name"), &ThemeDB::theme_get_user_icon);
	ClassDB::bind_method(D_METHOD("get_user_icon_list"), &ThemeDB::theme_get_user_icons_list);
	ClassDB::bind_method(D_METHOD("has_icon", "icon_name"), &ThemeDB::theme_has_icon);
	ClassDB::bind_method(D_METHOD("get_icon", "icon_name"), &ThemeDB::theme_get_icon);
	ClassDB::bind_method(D_METHOD("get_icon_list"), &ThemeDB::theme_get_icons_list);
#endif // !USE_LEGACY_THEME

	ADD_GROUP("Fallback values", "fallback_");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "fallback_base_scale", PROPERTY_HINT_RANGE, "0.0,2.0,0.01,or_greater"), "set_fallback_base_scale", "get_fallback_base_scale");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "fallback_font", PROPERTY_HINT_RESOURCE_TYPE, "Font", PROPERTY_USAGE_NONE), "set_fallback_font", "get_fallback_font");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "fallback_font_size", PROPERTY_HINT_RANGE, "0,256,1,or_greater,suffix:px"), "set_fallback_font_size", "get_fallback_font_size");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "fallback_icon", PROPERTY_HINT_RESOURCE_TYPE, "Texture2D", PROPERTY_USAGE_NONE), "set_fallback_icon", "get_fallback_icon");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "fallback_stylebox", PROPERTY_HINT_RESOURCE_TYPE, "StyleBox", PROPERTY_USAGE_NONE), "set_fallback_stylebox", "get_fallback_stylebox");

	ADD_SIGNAL(MethodInfo("fallback_changed"));
#ifndef USE_LEGACY_THEME
	ADD_SIGNAL(MethodInfo("theme_changed"));
	ADD_SIGNAL(MethodInfo("icons_changed"));
	ADD_SIGNAL(MethodInfo("colors_changed"));
	ADD_SIGNAL(MethodInfo("scale_changed"));
	ADD_SIGNAL(MethodInfo("margin_changed"));
	ADD_SIGNAL(MethodInfo("padding_changed"));
	ADD_SIGNAL(MethodInfo("corner_radius_changed"));
	ADD_SIGNAL(MethodInfo("border_width_changed"));
	ADD_SIGNAL(MethodInfo("border_padding_changed"));
	ADD_SIGNAL(MethodInfo("font_color_changed"));
	ADD_SIGNAL(MethodInfo("font_outline_color_changed"));
	ADD_SIGNAL(MethodInfo("font_outline_size_changed"));
	ADD_SIGNAL(MethodInfo("font_size_changed"));
	ADD_SIGNAL(MethodInfo("font_changed"));
#endif // !USE_LEGACY_THEME
}

// Memory management, reference, and initialization.

ThemeDB *ThemeDB::singleton = nullptr;

ThemeDB *ThemeDB::get_singleton() {
	return singleton;
}

ThemeDB::ThemeDB() {
	singleton = this;
#ifndef USE_LEGACY_THEME
	base_color = GLOBAL_DEF_BASIC(PropertyInfo(Variant::COLOR, "gui/theme/base_color", PROPERTY_HINT_COLOR_NO_ALPHA), Color(0.188, 0.188, 0.188));
	accent_color = GLOBAL_DEF_BASIC(PropertyInfo(Variant::COLOR, "gui/theme/accent_color", PROPERTY_HINT_COLOR_NO_ALPHA), Color(0.226, 0.478, 0.921));
	GLOBAL_DEF_BASIC(PropertyInfo(Variant::INT, "gui/theme/font_color_override", PROPERTY_HINT_ENUM, "Auto,Light,Dark,Custom"), 0);
	GLOBAL_DEF_BASIC(PropertyInfo(Variant::COLOR, "gui/theme/custom_font_color", PROPERTY_HINT_COLOR_NO_ALPHA), Color(0.875, 0.875, 0.875));
	font_color = _get_font_color();
	font_outline_color = GLOBAL_DEF_BASIC(PropertyInfo(Variant::COLOR, "gui/theme/font_outline_color"), Color(0, 0, 0, 1));
	contrast = GLOBAL_DEF_BASIC(PropertyInfo(Variant::FLOAT, "gui/theme/contrast", PROPERTY_HINT_RANGE, "-1.0, 1.0, 0.01"), -0.6);
	normal_contrast = GLOBAL_DEF_BASIC(PropertyInfo(Variant::FLOAT, "gui/theme/normal_contrast", PROPERTY_HINT_RANGE, "-1.0, 1.0, 0.01"), 0.4);
	hover_contrast = GLOBAL_DEF_BASIC(PropertyInfo(Variant::FLOAT, "gui/theme/hover_contrast", PROPERTY_HINT_RANGE, "-1.0, 1.0, 0.01"), -0.2);
	pressed_contrast = GLOBAL_DEF_BASIC(PropertyInfo(Variant::FLOAT, "gui/theme/pressed_contrast", PROPERTY_HINT_RANGE, "-1.0, 1.0, 0.01"), 0.6);
	bg_contrast = GLOBAL_DEF_BASIC(PropertyInfo(Variant::FLOAT, "gui/theme/bg_contrast", PROPERTY_HINT_RANGE, "-1.0, 1.0, 0.01"), 0.2);
	margin = GLOBAL_DEF_BASIC(PropertyInfo(Variant::INT, "gui/theme/margin", PROPERTY_HINT_RANGE, "0, 32, 1"), 4);
	padding = GLOBAL_DEF_BASIC(PropertyInfo(Variant::INT, "gui/theme/padding", PROPERTY_HINT_RANGE, "0, 32, 1"), 4);
	border_width = GLOBAL_DEF_BASIC(PropertyInfo(Variant::INT, "gui/theme/border_width", PROPERTY_HINT_RANGE, "0, 32, 1"), 2);
	corner_radius = GLOBAL_DEF_BASIC(PropertyInfo(Variant::INT, "gui/theme/corner_radius", PROPERTY_HINT_RANGE, "0, 32, 1"), 6);
	font_size = GLOBAL_DEF_BASIC(PropertyInfo(Variant::INT, "gui/theme/font_size", PROPERTY_HINT_RANGE, "0, 64, 1"), 16);
	font_outline_size = GLOBAL_DEF_BASIC(PropertyInfo(Variant::INT, "gui/theme/font_outline_size", PROPERTY_HINT_RANGE, "0, 64, 1"), 0);
	font_embolden = GLOBAL_DEF_BASIC(PropertyInfo(Variant::FLOAT, "gui/theme/font_embolden", PROPERTY_HINT_RANGE, "-2.0, 2.0, 0.01"), 0.0);
	font_spacing_glyph = GLOBAL_DEF_BASIC(PropertyInfo(Variant::INT, "gui/theme/font_spacing_glyph", PROPERTY_HINT_RANGE, "-64, 64, 1"), 0);
	font_spacing_space = GLOBAL_DEF_BASIC(PropertyInfo(Variant::INT, "gui/theme/font_spacing_space", PROPERTY_HINT_RANGE, "-64, 64, 1"), 0);
	font_spacing_top = GLOBAL_DEF_BASIC(PropertyInfo(Variant::INT, "gui/theme/font_spacing_top", PROPERTY_HINT_RANGE, "-64, 64, 1"), 0);
	font_spacing_bottom = GLOBAL_DEF_BASIC(PropertyInfo(Variant::INT, "gui/theme/font_spacing_bottom", PROPERTY_HINT_RANGE, "-64, 64, 1"), 0);
	scale = GLOBAL_DEF_BASIC(PropertyInfo(Variant::FLOAT, "gui/theme/default_theme_scale", PROPERTY_HINT_RANGE, "0.5,8,0.01"), 1.0);
	custom_font = GLOBAL_DEF_BASIC(PropertyInfo(Variant::STRING, "gui/theme/custom_font", PROPERTY_HINT_FILE, "*.tres,*.res,*.otf,*.ttf,*.ttc,*.woff,*.woff2,*.fnt,*.font,*.pfb,*.pfm"), "");
	font_subpixel_positioning = (TextServer::SubpixelPositioning)(int)GLOBAL_DEF_BASIC(PropertyInfo(Variant::INT, "gui/theme/default_font_subpixel_positioning", PROPERTY_HINT_ENUM, "Disabled,Auto,One Half of a Pixel,One Quarter of a Pixel"), TextServer::SUBPIXEL_POSITIONING_AUTO);
	font_antialiasing = (TextServer::FontAntialiasing)(int)GLOBAL_DEF_BASIC(PropertyInfo(Variant::INT, "gui/theme/default_font_antialiasing", PROPERTY_HINT_ENUM, "None,Grayscale,LCD Subpixel"), 1);
	font_lcd_subpixel_layout = (TextServer::FontLCDSubpixelLayout)(int)GLOBAL_DEF_BASIC(PropertyInfo(Variant::INT, "gui/theme/lcd_subpixel_layout", PROPERTY_HINT_ENUM, "Disabled,Horizontal RGB,Horizontal BGR,Vertical RGB,Vertical BGR"), 1);
	font_hinting = (TextServer::Hinting)(int)GLOBAL_DEF_BASIC(PropertyInfo(Variant::INT, "gui/theme/default_font_hinting", PROPERTY_HINT_ENUM, "None,Light,Normal"), TextServer::HINTING_LIGHT);
	font_msdf = GLOBAL_DEF_BASIC("gui/theme/default_font_multichannel_signed_distance_field", false);
	font_generate_mipmaps = GLOBAL_DEF_BASIC("gui/theme/default_font_generate_mipmaps", false);
#endif // !USE_LEGACY_THEME
	if (MessageQueue::get_singleton()) { // May not exist in tests etc.
		callable_mp(this, &ThemeDB::_sort_theme_items).call_deferred();
#ifndef USE_LEGACY_THEME
		ProjectSettings::get_singleton()->connect("settings_changed", callable_mp(this, &ThemeDB::_update_default_theme));
#endif // !USE_LEGACY_THEME
	}
}

ThemeDB::~ThemeDB() {
	// For technical reasons unit tests recreate and destroy the default
	// theme over and over again. Make sure that finalize_theme() also
	// frees any objects that can be recreated by initialize_theme*().

	_finalize_theme_contexts();
#ifndef USE_LEGACY_THEME
	finalize_default_theme();
#endif // !USE_LEGACY_THEME

	default_theme.unref();
	project_theme.unref();

	fallback_font.unref();
	fallback_icon.unref();
	fallback_stylebox.unref();

	singleton = nullptr;
}

void ThemeContext::_emit_changed() {
	emit_signal(CoreStringName(changed));
}

void ThemeContext::set_themes(Vector<Ref<Theme>> &p_themes) {
	for (const Ref<Theme> &theme : themes) {
		theme->disconnect_changed(callable_mp(this, &ThemeContext::_emit_changed));
	}

	themes.clear();

	for (const Ref<Theme> &theme : p_themes) {
		if (theme.is_null()) {
			continue;
		}

		themes.push_back(theme);
		theme->connect_changed(callable_mp(this, &ThemeContext::_emit_changed));
	}

	_emit_changed();
}

const Vector<Ref<Theme>> ThemeContext::get_themes() const {
	return themes;
}

Ref<Theme> ThemeContext::get_fallback_theme() const {
	// We expect all contexts to be valid and non-empty, but just in case...
	if (themes.size() == 0) {
		return ThemeDB::get_singleton()->get_default_theme();
	}

	return themes[themes.size() - 1];
}

void ThemeContext::_bind_methods() {
	ADD_SIGNAL(MethodInfo("changed"));
}
