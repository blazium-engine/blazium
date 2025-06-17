#include "import_gif_to_sprite_frames.h"
#include "../gifmanager.h"
#include "../image_frames.h"
#include "core/core_bind.h"
#include "core/io/resource_saver.h"
#include "scene/resources/sprite_frames.h"

#include <thirdparty/giflib/gif_lib.h>

String GifToSpriteFramesImportPlugin::get_importer_name() const {
	return "import_gif_to_sprite_frames";
}
String GifToSpriteFramesImportPlugin::get_visible_name() const {
	return "GIF to SpriteFrames";
}

int32_t GifToSpriteFramesImportPlugin::get_preset_count() const {
	return COUNT;
}

String GifToSpriteFramesImportPlugin::get_preset_name(int32_t preset_index = 0) const {
	switch (preset_index) {
		case Presets::DEFAULT:
		default:
			return "Default";
	}
}

void GifToSpriteFramesImportPlugin::get_recognized_extensions(List<String> *p_extensions) const {
	p_extensions->push_back("gif");
}

void GifToSpriteFramesImportPlugin::get_import_options(const String &p_path, List<ImportOption> *r_options, int p_preset) const {
	r_options->push_back(ResourceImporter::ImportOption(PropertyInfo(Variant::INT, "frames_per_second", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_UPDATE_ALL_IF_MODIFIED), true));
}

String GifToSpriteFramesImportPlugin::get_save_extension() const {
	return "tres";
}

String GifToSpriteFramesImportPlugin::get_resource_type() const {
	return "SpriteFrames";
}

float GifToSpriteFramesImportPlugin::get_priority() const {
	return 1.0;
}

int32_t GifToSpriteFramesImportPlugin::get_import_order() const {
	return IMPORT_ORDER_DEFAULT;
}

bool GifToSpriteFramesImportPlugin::get_option_visibility(const String &p_path, const String &p_option, const HashMap<StringName, Variant> &p_options) const {
	return true;
}

Error GifToSpriteFramesImportPlugin::import(ResourceUID::ID p_source_id, const String &p_source_file, const String &p_save_path, const HashMap<StringName, Variant> &p_options, List<String> *r_platform_variants, List<String> *r_gen_files, Variant *r_metadata) {
	double fps = 30;
	if (p_options.has("frames_per_second")) {
		fps = p_options["frames_per_second"];
	}
	Ref<SpriteFrames> sf = GifManager::get_singleton()->sprite_frames_from_file(p_source_file, 0, fps);
	return core_bind::ResourceSaver::get_singleton()->save(sf, p_save_path + "." + get_save_extension(), 0);
}
