/**************************************************************************/
/*  color_picker.h                                                        */
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

#ifndef COLOR_PICKER_H
#define COLOR_PICKER_H

#include "scene/gui/box_container.h"
#include "scene/gui/button.h"
#include "scene/gui/popup.h"

class AspectRatioContainer;
class ColorButton;
class ColorMode;
class ColorModeRGB;
class ColorModeHSV;
class ColorModeRAW;
class ColorModeOKHSL;
class GridContainer;
class HSlider;
class Label;
class LineEdit;
class MenuButton;
class OptionButton;
class Panel;
class PanelContainer;
class PopupMenu;
class SpinBox;
class StyleBoxFlat;
class TextureRect;
class FoldableContainer;
class FileDialog;

class ColorPicker : public VBoxContainer {
	GDCLASS(ColorPicker, VBoxContainer);

	// These classes poke into theme items for their internal logic.
	friend class ColorModeRGB;
	friend class ColorModeHSV;
	friend class ColorModeRAW;
	friend class ColorModeOKHSL;

public:
	enum ColorModeType {
		MODE_RGB,
		MODE_HSV,
		MODE_RAW,
		MODE_OKHSL,

		MODE_MAX
	};

	enum PickerShapeType {
		SHAPE_HSV_RECTANGLE,
		SHAPE_HSV_WHEEL,
		SHAPE_VHS_CIRCLE,
		SHAPE_OKHSL_CIRCLE,
		SHAPE_NONE,

		SHAPE_MAX
	};

	static const int SLIDER_COUNT = 3;

private:
	enum class MenuOption {
		MENU_SAVE,
		MENU_SAVE_AS,
		MENU_LOAD,
		MENU_QUICKLOAD,
		MENU_CLEAR,
	};

	static Ref<Shader> wheel_shader;
	static Ref<Shader> circle_shader;
	static Ref<Shader> circle_ok_color_shader;
	static List<Color> preset_cache;
	static List<Color> recent_preset_cache;

#ifdef TOOLS_ENABLED
	Object *editor_settings = nullptr;
#endif

	int current_slider_count = SLIDER_COUNT;
	const float WHEEL_RADIUS = 0.42;

	bool slider_theme_modified = true;

	Vector<ColorMode *> modes;

	Popup *picker_window = nullptr;
	TextureRect *picker_texture_zoom = nullptr;
	Panel *picker_preview = nullptr;
	Panel *picker_preview_color = nullptr;
	Ref<StyleBoxFlat> picker_preview_style_box;
	Ref<StyleBoxFlat> picker_preview_style_box_color;

	// Legacy color picking.
	TextureRect *picker_texture_rect = nullptr;
	Color picker_color;
	FileDialog *file_dialog = nullptr;
	MenuButton *menu_btn = nullptr;
	PopupMenu *options_menu = nullptr;

	MarginContainer *internal_margin = nullptr;
	VBoxContainer *real_vbox = nullptr;
	HBoxContainer *hb_edit = nullptr;
	Control *uv_edit = nullptr;
	Control *w_edit = nullptr;
	AspectRatioContainer *wheel_edit = nullptr;
	MarginContainer *wheel_margin = nullptr;
	Ref<ShaderMaterial> wheel_mat;
	Ref<ShaderMaterial> circle_mat;
	Control *wheel = nullptr;
	Control *wheel_uv = nullptr;
	Control *sample = nullptr;
	FoldableContainer *preset_foldable = nullptr;
	Button *add_preset_button = nullptr;
	FoldableContainer *recent_preset_foldable = nullptr;
	HBoxContainer *preset_hbc = nullptr;
	HBoxContainer *recent_preset_hbc = nullptr;
	Button *btn_pick = nullptr;
	Label *palette_name = nullptr;
	String palette_path;
	PopupMenu *shape_popup = nullptr;
	MenuButton *btn_shape = nullptr;
	HBoxContainer *mode_hbc = nullptr;
	HBoxContainer *sample_hbc = nullptr;
	PanelContainer *sliders_panel = nullptr;
	VBoxContainer *slider_vbc = nullptr;
	GridContainer *slider_gc = nullptr;
	HBoxContainer *hex_hbc = nullptr;
	Button *mode_btns[MODE_MAX];
	Ref<ButtonGroup> mode_group = nullptr;
	ColorButton *selected_recent_preset = nullptr;
	Ref<ButtonGroup> preset_group;
	Ref<ButtonGroup> recent_preset_group;
#ifdef TOOLS_ENABLED
	Callable quick_open_callback;
	Callable palette_saved_callback;
#endif // TOOLS_ENABLED

	OptionButton *mode_option_button = nullptr;

	HSlider *sliders[SLIDER_COUNT];
	SpinBox *values[SLIDER_COUNT];
	Label *labels[SLIDER_COUNT];
	Button *text_type = nullptr;
	LineEdit *c_text = nullptr;

	HSlider *alpha_slider = nullptr;
	SpinBox *alpha_value = nullptr;
	Label *alpha_label = nullptr;

	bool edit_alpha = true;
	Size2i ms;
	bool text_is_constructor = false;
	PickerShapeType current_shape = SHAPE_HSV_RECTANGLE;
	ColorModeType current_mode = MODE_RGB;

	const int PRESET_COLUMN_COUNT = 9;
	int prev_preset_size = 0;
	int prev_rencet_preset_size = 0;
	List<Color> presets;
	List<Color> recent_presets;

	Color color;
	Color old_color;
	Color pre_picking_color;
	bool is_picking_color = false;

	bool display_old_color = false;
	bool deferred_mode_enabled = false;
	bool updating = true;
	bool changing_color = false;
	bool spinning = false;
	bool can_add_swatches = true;
	bool presets_visible = true;
	bool color_modes_visible = true;
	bool sampler_visible = true;
	bool sliders_visible = true;
	bool hex_visible = true;
	bool line_edit_mouse_release = false;
	bool text_changed = false;
	bool currently_dragging = false;

	float h = 0.0;
	float s = 0.0;
	float v = 0.0;

	float ok_hsl_h = 0.0;
	float ok_hsl_s = 0.0;
	float ok_hsl_l = 0.0;

	Color last_color;

	struct ThemeCache {
		float base_scale = 1.0;

		int content_margin = 0;
		int label_width = 0;
		int preset_size = 0;

		int sample_height = 0;
		int sv_height = 0;
		int sv_width = 0;
		int h_width = 0;

		bool center_slider_grabbers = true;
		bool colorize_sliders = true;

		Ref<Texture2D> menu_option;
		Ref<Texture2D> screen_picker;
		Ref<Texture2D> expanded_arrow;
		Ref<Texture2D> folded_arrow;
		Ref<Texture2D> folded_arrow_mirrored;
		Ref<Texture2D> add_preset;

		Ref<Texture2D> shape_rect;
		Ref<Texture2D> shape_rect_wheel;
		Ref<Texture2D> shape_circle;

		Ref<Texture2D> bar_arrow;
		Ref<Texture2D> sample_bg;
		Ref<Texture2D> sample_revert;
		Ref<Texture2D> overbright_indicator;
		Ref<Texture2D> picker_cursor;
		Ref<Texture2D> picker_cursor_bg;
		Ref<Texture2D> color_hue;

		Ref<Texture2D> hex_icon;
		Ref<Texture2D> hex_code_icon;

		/* Mode buttons */
		Ref<StyleBox> mode_button_normal;
		Ref<StyleBox> mode_button_pressed;
		Ref<StyleBox> mode_button_hover;
		Ref<StyleBox> sliders_panel;
	} theme_cache;

	void _copy_color_to_hsv();
	void _copy_hsv_to_color();

	void create_slider(GridContainer *gc, int idx);
	void _html_submitted(const String &p_html);
	void _slider_drag_started();
	void _slider_value_changed();
	void _slider_drag_ended();
	void _update_controls();
	void _update_color(bool p_update_sliders = true);
	void _update_text_value();
	void _text_type_toggled();
	void _sample_input(const Ref<InputEvent> &p_event);
	void _sample_draw();
	void _hsv_draw(int p_which, Control *c);
	void _slider_draw(int p_which);

	void _uv_input(const Ref<InputEvent> &p_event, Control *c);
	void _w_input(const Ref<InputEvent> &p_event);
	void _slider_or_spin_input(const Ref<InputEvent> &p_event);
	void _line_edit_input(const Ref<InputEvent> &p_event);
	void _preset_foldable_button_pressed(int p_idx);
	void _preset_input(const Ref<InputEvent> &p_event, const Color &p_color);
	void _recent_preset_pressed(const bool p_pressed, ColorButton *p_preset);
	void _preset_pressed(const bool p_pressed, ColorButton *p_preset);
	void _text_changed(const String &p_new_text);
	void _add_preset_pressed();
	void _html_focus_exit();
	void _pick_button_pressed();
	void _target_gui_input(const Ref<InputEvent> &p_event);
	void _pick_finished();
	void _update_menu_items();
	void _options_menu_cbk(int p_which);

	// Legacy color picking.
	void _pick_button_pressed_legacy();
	void _picker_texture_input(const Ref<InputEvent> &p_event);

	void _add_preset_button(const Color &p_color);
	void _add_recent_preset_button(const Color &p_color);

	void _save_palette(bool p_is_save_as);
	void _load_palette();

	Variant _get_drag_data_fw(const Point2 &p_point, Control *p_from_control);
	bool _can_drop_data_fw(const Point2 &p_point, const Variant &p_data, Control *p_from_control) const;
	void _drop_data_fw(const Point2 &p_point, const Variant &p_data, Control *p_from_control);

protected:
	void _validate_property(PropertyInfo &p_property) const;
	virtual void _update_theme_item_cache() override;

	void _notification(int);
	static void _bind_methods();

public:
#ifdef TOOLS_ENABLED
	void set_editor_settings(Object *p_editor_settings);
	void set_quick_open_callback(const Callable &p_file_selected);
	void set_palette_saved_callback(const Callable &p_palette_saved);
#endif
	HSlider *get_slider(int idx);
	Vector<float> get_active_slider_values();

	static void init_shaders();
	static void finish_shaders();

	void add_mode(ColorMode *p_mode);

	void set_edit_alpha(bool p_show);
	bool is_editing_alpha() const;

	void _set_pick_color(const Color &p_color, bool p_update_sliders);
	void set_pick_color(const Color &p_color);
	Color get_pick_color() const;
	void set_old_color(const Color &p_color);
	Color get_old_color() const;
	void _quick_open_palette_file_selected(const String &p_path);
	void _palette_file_selected(const String &p_path);

	void set_display_old_color(bool p_enabled);
	bool is_displaying_old_color() const;

	void set_picker_shape(PickerShapeType p_shape);
	PickerShapeType get_picker_shape() const;

	void add_preset(const Color &p_color);
	void add_recent_preset(const Color &p_color);
	void erase_preset(const Color &p_color);
	void erase_recent_preset(const Color &p_color);
	PackedColorArray get_presets() const;
	PackedColorArray get_recent_presets() const;

#ifdef TOOLS_ENABLED
	void _update_presets();
	void _update_recent_presets();
#endif // TOOLS_ENABLED

	void _select_from_preset_container(const Color &p_color);
	bool _select_from_recent_preset_hbc(const Color &p_color);

	void set_color_mode(ColorModeType p_mode);
	ColorModeType get_color_mode() const;

	void set_deferred_mode(bool p_enabled);
	bool is_deferred_mode() const;

	void set_can_add_swatches(bool p_enabled);
	bool are_swatches_enabled() const;

	void set_presets_visible(bool p_visible);
	bool are_presets_visible() const;

	void set_modes_visible(bool p_visible);
	bool are_modes_visible() const;

	void set_sampler_visible(bool p_visible);
	bool is_sampler_visible() const;

	void set_sliders_visible(bool p_visible);
	bool are_sliders_visible() const;

	void set_hex_visible(bool p_visible);
	bool is_hex_visible() const;

	void set_focus_on_line_edit();

	ColorPicker();
	~ColorPicker();
};

class ColorPickerPopupPanel : public PopupPanel {
	virtual void _input_from_window(const Ref<InputEvent> &p_event) override;
};

class ColorPickerButton : public Button {
	GDCLASS(ColorPickerButton, Button);

	// Initialization is now done deferred,
	// this improves performance in the inspector as the color picker
	// can be expensive to initialize.

	PopupPanel *popup = nullptr;
	ColorPicker *picker = nullptr;
	Color color;
	bool edit_alpha = true;

	struct ThemeCache {
		Ref<StyleBox> normal_style;
		Ref<Texture2D> background_icon;

		Ref<Texture2D> overbright_indicator;
	} theme_cache;

	void _about_to_popup();
	void _color_changed(const Color &p_color);
	void _modal_closed();

	virtual void pressed() override;

	void _update_picker();

protected:
	void _validate_property(PropertyInfo &p_property) const;
	void _notification(int);
	static void _bind_methods();

public:
	void set_pick_color(const Color &p_color);
	Color get_pick_color() const;

	void set_edit_alpha(bool p_show);
	bool is_editing_alpha() const;

	ColorPicker *get_picker();
	PopupPanel *get_popup();

	ColorPickerButton(const String &p_text = String());
};

VARIANT_ENUM_CAST(ColorPicker::PickerShapeType);
VARIANT_ENUM_CAST(ColorPicker::ColorModeType);

#endif // COLOR_PICKER_H
