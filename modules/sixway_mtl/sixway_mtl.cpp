
/*   Six-Way Volumetric Lighting for Godot v2.0     */
/*            Developed by Hamid.Memar              */
/*      License : Creative Commons (CC) BY 4.0      */

// Imports
#include "sixway_mtl.h"
#include "editor/editor_interface.h"
#include "editor/themes/editor_theme.h"

// Six-Way Lighting Material Implementation
void SixWayLightingMaterial::RegisterModule() {
	ClassDB::register_class<SixWayLightingMaterial>();
}
SixWayLightingMaterial::SixWayLightingMaterial() {
	_initialize();
}
void SixWayLightingMaterial::_initialize() {
	shader = memnew(Shader);
	shader->set_name("Six-Way Lighting Shader");
	shader->set_code(R"(
	shader_type spatial;
	render_mode blend_mix, cull_disabled, depth_draw_always;
	uniform sampler2D six_way_map_rtb : source_color;
	uniform sampler2D six_way_map_lbf : source_color;
	uniform sampler2D six_way_map_tea : source_color;
	uniform sampler2D normal_map : hint_normal;
	uniform sampler2D emission_ramp : source_color;
	uniform vec3 absorption : source_color = vec3(1.0, 1.0, 1.0);
	uniform float thickness : hint_range(-10, 10) = 0.15;
	uniform float density : hint_range(0, 1) = 1.0;
	uniform float scattering : hint_range(0, 1) = 0.0;
	uniform float normal_power : hint_range(0, 1) = 0.25;
	uniform float normal_blend : hint_range(0.0, 1.0) = 0.25;
	uniform float ao_power : hint_range(0.0, 1.0) = 0.5;
	uniform float emission_power : hint_range(0.0, 10.0) = 1.0;
	uniform bool billboard_mode = false;
	uniform bool billboard_y_axis_only = false;
	uniform bool billboard_keep_scale = true;
	uniform bool animate = false;
	uniform float animate_speed = 30.0;
	uniform float animate_offset = 0.0;
	uniform vec2 flipbook_dimensions = vec2(2, 2);
	vec2 animateUV(vec2 uv, vec2 size, float progress)
	{
		progress = floor(mod(progress, size.x * size.y));
		vec2 frame_size = vec2(1.0) / size;
		vec2 frame = fract(uv) * frame_size;
		frame.x += (mod(progress, size.x)) * frame_size.x;
		frame.y += floor(progress / size.x) * frame_size.y;
		return frame;
	}
	void vertex()
	{
		if (billboard_mode && billboard_y_axis_only)
		{
			MODELVIEW_MATRIX = VIEW_MATRIX * mat4(
				vec4(normalize(cross(vec3(0.0, 1.0, 0.0), MAIN_CAM_INV_VIEW_MATRIX[2].xyz)), 0.0),
				vec4(0.0, 1.0, 0.0, 0.0),
				vec4(normalize(cross(MAIN_CAM_INV_VIEW_MATRIX[0].xyz, vec3(0.0, 1.0, 0.0))), 0.0),
				MODEL_MATRIX[3]);
		}	
		if (billboard_mode && !billboard_y_axis_only)
		{
			MODELVIEW_MATRIX = VIEW_MATRIX * mat4(
				MAIN_CAM_INV_VIEW_MATRIX[0],
				MAIN_CAM_INV_VIEW_MATRIX[1],
				MAIN_CAM_INV_VIEW_MATRIX[2],
				MODEL_MATRIX[3]);
		}
		if (billboard_mode)
		{
			MODELVIEW_MATRIX = MODELVIEW_MATRIX * mat4(
				vec4(length(MODEL_MATRIX[0].xyz), 0.0, 0.0, 0.0),
				vec4(0.0, length(MODEL_MATRIX[1].xyz), 0.0, 0.0),
				vec4(0.0, 0.0, length(MODEL_MATRIX[2].xyz), 0.0),
				vec4(0.0, 0.0, 0.0, 1.0));
			MODELVIEW_NORMAL_MATRIX = mat3(MODELVIEW_MATRIX);
		}
	}
	void fragment()
	{
		vec2 uv = vec2(1.0);
		if (animate) uv = animateUV(UV, flipbook_dimensions, TIME * animate_speed + animate_offset);
		else uv = UV;
		vec3 ao = mix(vec3(1.0), texture(six_way_map_tea, uv).bbb, ao_power);
		ALBEDO = absorption * ao;
		ALPHA = texture(six_way_map_tea, uv).r * density;
		vec3 normal_map_sample = texture(normal_map, uv).rgb * 2.0 - 1.0;
		vec3 normals = TANGENT * normal_map_sample.x + BINORMAL * normal_map_sample.y + NORMAL * normal_map_sample.z;
		NORMAL_MAP = mix(vec3(.5,.5,1.0), normals, normal_power);
		float emission_strength = texture(six_way_map_tea, uv).g;
		vec3 emission_color = texture(emission_ramp, vec2(emission_strength, 0.0)).rgb * emission_strength * emission_power;
		EMISSION = emission_color;
	}
	void light()
	{
		vec2 uv = vec2(1.0);
		if (animate) uv = animateUV(UV, flipbook_dimensions, TIME * animate_speed + animate_offset);
		else uv = UV;
		vec3 light_dir = normalize(LIGHT);
		vec3 normal_dir = normalize(NORMAL);
		float weightXPos = max(light_dir.x, 0.0);
		float weightXNeg = max(-light_dir.x, 0.0);
		float weightYPos = max(light_dir.y, 0.0);
		float weightYNeg = max(-light_dir.y, 0.0);
		float weightZPos = max(light_dir.z, 0.0);
		float weightZNeg = max(-light_dir.z, 0.0);
		float total = weightXPos + weightXNeg + weightYPos + weightYNeg + weightZPos + weightZNeg;
		if (total > 0.0) {
			weightXPos /= total;
			weightXNeg /= total;
			weightYPos /= total;
			weightYNeg /= total;
			weightZPos /= total;
			weightZNeg /= total;
		}
		vec4 rtb = texture(six_way_map_rtb, uv);
		vec4 lbf = texture(six_way_map_lbf, uv);
		vec3 directional_color = weightXPos * vec3(rtb.r) + weightXNeg * vec3(lbf.r) +
			weightYPos * vec3(rtb.g) + weightYNeg * vec3(rtb.a) +
			weightZPos * vec3(lbf.b) + weightZNeg * vec3(rtb.b);
		float normal_strength = max(dot(normal_dir, light_dir), 0.0);
		vec3 blended_color = mix(directional_color, directional_color * normal_strength, normal_blend);
		vec3 final_color = blended_color * (1.0 - (scattering / 5.0)) + (scattering / 5.0) * vec3(1.0, 1.0, 1.0);
		vec3 transmittance = exp(-absorption * thickness);
		final_color = final_color * transmittance;
		DIFFUSE_LIGHT += (final_color * LIGHT_COLOR * ATTENUATION / 4.4);
	}
    )");
	RS::get_singleton()->material_set_shader(get_rid(), shader->get_rid());
}
RID SixWayLightingMaterial::get_shader_rid() const {
	return shader->get_rid();
}
Shader::Mode SixWayLightingMaterial::get_shader_mode() const {
	return Shader::MODE_SPATIAL;
}
bool SixWayLightingMaterial::_can_do_next_pass() const {
	return shader.is_valid() && shader->get_mode() == Shader::MODE_SPATIAL;
}
bool SixWayLightingMaterial::_can_use_render_priority() const {
	return shader.is_valid() && shader->get_mode() == Shader::MODE_SPATIAL;
}
void SixWayLightingMaterial::reset_state() {
}
void SixWayLightingMaterial::_set_shader_parameter(const StringName &p_param, const Variant &p_value) {
	if (p_value.get_type() == Variant::NIL) {
		param_cache.erase(p_param);
		RS::get_singleton()->material_set_param(_get_material(), p_param, Variant());
	} else {
		Variant *v = param_cache.getptr(p_param);
		if (!v) {
			remap_cache["sixway_configuration/" + p_param.operator String()] = p_param;
			param_cache.insert(p_param, p_value);
		} else {
			*v = p_value;
		}

		if (p_value.get_type() == Variant::OBJECT) {
			RID tex_rid = p_value;
			if (tex_rid == RID()) {
				param_cache.erase(p_param);
				RS::get_singleton()->material_set_param(_get_material(), p_param, Variant());
			} else {
				RS::get_singleton()->material_set_param(_get_material(), p_param, tex_rid);
			}
		} else {
			RS::get_singleton()->material_set_param(_get_material(), p_param, p_value);
		}
	}
}
Variant SixWayLightingMaterial::_get_shader_parameter(const StringName &p_param) const {
	if (param_cache.has(p_param)) {
		return param_cache[p_param];
	} else {
		return Variant();
	}
}
bool SixWayLightingMaterial::_set(const StringName &p_name, const Variant &p_value) {
	if (shader.is_valid()) {
		const StringName *sn = remap_cache.getptr(p_name);
		if (sn) {
			_set_shader_parameter(*sn, p_value);
			return true;
		}
		String s = p_name;
		if (s.begins_with("sixway_configuration/")) {
			String param = s.replace_first("sixway_configuration/", "");
			remap_cache[s] = param;
			_set_shader_parameter(param, p_value);
			return true;
		}
	}

	return false;
}
bool SixWayLightingMaterial::_get(const StringName &p_name, Variant &r_ret) const {
	if (shader.is_valid()) {
		const StringName *sn = remap_cache.getptr(p_name);
		if (sn) {
			r_ret = _get_shader_parameter(*sn);
			return true;
		}
	}

	return false;
}
void SixWayLightingMaterial::_get_property_list(List<PropertyInfo> *p_list) const {
	if (!shader.is_null()) {
		List<PropertyInfo> list;
		shader->get_shader_uniform_list(&list, true);

		HashMap<String, HashMap<String, List<PropertyInfo>>> groups;
		LocalVector<Pair<String, LocalVector<String>>> vgroups;
		{
			HashMap<String, List<PropertyInfo>> none_subgroup;
			none_subgroup.insert("<None>", List<PropertyInfo>());
			groups.insert("<None>", none_subgroup);
		}

		String last_group = "<None>";
		String last_subgroup = "<None>";

		bool is_none_group_undefined = true;
		bool is_none_group = true;

		for (List<PropertyInfo>::Element *E = list.front(); E; E = E->next()) {
			if (E->get().usage == PROPERTY_USAGE_GROUP) {
				if (!E->get().name.is_empty()) {
					Vector<String> vgroup = E->get().name.split("::");
					last_group = vgroup[0];
					if (vgroup.size() > 1) {
						last_subgroup = vgroup[1];
					} else {
						last_subgroup = "<None>";
					}
					is_none_group = false;

					if (!groups.has(last_group)) {
						PropertyInfo info;
						info.usage = PROPERTY_USAGE_GROUP;
						info.name = last_group.capitalize();
						info.hint_string = "sixway_configuration/";

						List<PropertyInfo> none_subgroup;
						none_subgroup.push_back(info);

						HashMap<String, List<PropertyInfo>> subgroup_map;
						subgroup_map.insert("<None>", none_subgroup);

						groups.insert(last_group, subgroup_map);
						vgroups.push_back(Pair<String, LocalVector<String>>(last_group, { "<None>" }));
					}

					if (!groups[last_group].has(last_subgroup)) {
						PropertyInfo info;
						info.usage = PROPERTY_USAGE_SUBGROUP;
						info.name = last_subgroup.capitalize();
						info.hint_string = "sixway_configuration/";

						List<PropertyInfo> subgroup;
						subgroup.push_back(info);

						groups[last_group].insert(last_subgroup, subgroup);
						for (Pair<String, LocalVector<String>> &group : vgroups) {
							if (group.first == last_group) {
								group.second.push_back(last_subgroup);
								break;
							}
						}
					}
				} else {
					last_group = "<None>";
					last_subgroup = "<None>";
					is_none_group = true;
				}
				continue;
			}

			if (is_none_group_undefined && is_none_group) {
				is_none_group_undefined = false;

				PropertyInfo info;
				info.usage = PROPERTY_USAGE_GROUP;
				info.name = "Shader Setup";
				info.hint_string = "sixway_configuration/";
				groups["<None>"]["<None>"].push_back(info);

				vgroups.push_back(Pair<String, LocalVector<String>>("<None>", { "<None>" }));
			}

			const bool is_uniform_cached = param_cache.has(E->get().name);
			bool is_uniform_type_compatible = true;

			if (is_uniform_cached) {
				const Variant &cached = param_cache.get(E->get().name);

				if (cached.is_array()) {
					is_uniform_type_compatible = Variant::can_convert(E->get().type, cached.get_type());
				} else {
					is_uniform_type_compatible = E->get().type == cached.get_type();
				}

				if (is_uniform_type_compatible && E->get().type == Variant::OBJECT && cached.get_type() == Variant::OBJECT) {
					Object *cached_obj = cached;
					if (!cached_obj->is_class(E->get().hint_string)) {
						is_uniform_type_compatible = false;
					}
				}
			}

			PropertyInfo info = E->get();
			info.name = "sixway_configuration/" + info.name;
			if (!is_uniform_cached || !is_uniform_type_compatible) {
				Variant default_value = RenderingServer::get_singleton()->shader_get_parameter_default(shader->get_rid(), E->get().name);
				param_cache.insert(E->get().name, default_value);
				remap_cache.insert(info.name, E->get().name);
			}
			groups[last_group][last_subgroup].push_back(info);
		}

		for (const Pair<String, LocalVector<String>> &group_pair : vgroups) {
			String group = group_pair.first;
			for (const String &subgroup : group_pair.second) {
				List<PropertyInfo> &prop_infos = groups[group][subgroup];
				for (List<PropertyInfo>::Element *item = prop_infos.front(); item; item = item->next()) {
					p_list->push_back(item->get());
				}
			}
		}
	}
}
bool SixWayLightingMaterial::_property_can_revert(const StringName &p_name) const {
	if (shader.is_valid()) {
		if (remap_cache.has(p_name)) {
			return true;
		}
		const String sname = p_name;
		return sname == "render_priority" || sname == "next_pass";
	}
	return false;
}
bool SixWayLightingMaterial::_property_get_revert(const StringName &p_name, Variant &r_property) const {
	if (shader.is_valid()) {
		const StringName *pr = remap_cache.getptr(p_name);
		if (pr) {
			r_property = RenderingServer::get_singleton()->shader_get_parameter_default(shader->get_rid(), *pr);
			return true;
		} else if (p_name == "render_priority") {
			r_property = 0;
			return true;
		} else if (p_name == "next_pass") {
			r_property = Variant();
			return true;
		}
	}
	return false;
}
bool SixWayLightingMaterial::_is_shader_property(const StringName &p_name) const {
	List<PropertyInfo> shaderParams;
	shader->get_shader_uniform_list(&shaderParams, true);
	for (const auto &param : shaderParams) {
		if (param.name == p_name) {
			return true;
		}
	}
	return false;
}
