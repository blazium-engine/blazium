
/*   Six-Way Volumetric Lighting for Godot v2.0     */
/*            Developed by Hamid.Memar              */
/*      License : Creative Commons (CC) BY 4.0      */

#ifndef HM_SIX_WAY_LIGHTING_SHADER_H
#define HM_SIX_WAY_LIGHTING_SHADER_H

// Imports
#include "scene/resources/material.h"

// Six-Way Lighting Material Definition
class SixWayLightingMaterial : public Material {
	GDCLASS(SixWayLightingMaterial, Material);

// Constructor
public:
	SixWayLightingMaterial();

// Base Classes Methods
public:
	virtual RID get_shader_rid() const override;
	virtual Shader::Mode get_shader_mode() const override;
	virtual bool _can_do_next_pass() const override;
	virtual bool _can_use_render_priority() const override;
	virtual void reset_state();

// Internal Methods
private:
	void _initialize();
	void _set_shader_parameter(const StringName &p_param, const Variant &p_value);
	Variant _get_shader_parameter(const StringName &p_param) const;
	bool _set(const StringName &p_name, const Variant &p_value);
	bool _get(const StringName &p_name, Variant &r_ret) const;
	void _get_property_list(List<PropertyInfo> *p_list) const;
	bool _property_can_revert(const StringName &p_name) const;
	bool _property_get_revert(const StringName &p_name, Variant &r_property) const;
	bool _is_shader_property(const StringName &p_name) const;

// Static Methods
public:
	static void RegisterModule();

// Internal Objects
private:
	Ref<Shader> shader;
	mutable HashMap<StringName, StringName> remap_cache;
	mutable HashMap<StringName, Variant> param_cache;
};

#endif // HM_SIX_WAY_LIGHTING_SHADER_H
