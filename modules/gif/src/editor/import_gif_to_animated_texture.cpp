#include "import_gif_to_animated_texture.h"
#include "../gifmanager.h"
#include "../image_frames.h"
#include "core/core_bind.h"
#include "core/io/resource_importer.h"
#include "core/io/resource_saver.h"
#include "scene/resources/animated_texture.h"

#include <thirdparty/giflib/gif_lib.h>

String GifToAnimatedTextureImportPlugin::get_importer_name() const {
	return "import_gif_to_animated_texture";
}
String GifToAnimatedTextureImportPlugin::get_visible_name() const {
	return "GIF to AnimatedTexure";
}

int32_t GifToAnimatedTextureImportPlugin::get_preset_count() const {
	return COUNT;
}

String GifToAnimatedTextureImportPlugin::get_preset_name(int32_t preset_index) const {
	switch (preset_index) {
		case Presets::DEFAULT:
		default:
			return "Default";
	}
}

void GifToAnimatedTextureImportPlugin::get_recognized_extensions(List<String> *p_extensions) const {
	p_extensions->push_back("gif");
}

void GifToAnimatedTextureImportPlugin::get_import_options(const String &p_path, List<ImportOption> *r_options, int p_preset) const {
}

String GifToAnimatedTextureImportPlugin::get_save_extension() const {
	return "tres";
}

String GifToAnimatedTextureImportPlugin::get_resource_type() const {
	return "AnimatedTexture";
}

float GifToAnimatedTextureImportPlugin::get_priority() const {
	return 1.1;
}

int32_t GifToAnimatedTextureImportPlugin::get_import_order() const {
	return ResourceImporter::ImportOrder::IMPORT_ORDER_DEFAULT;
}

bool GifToAnimatedTextureImportPlugin::get_option_visibility(const String &p_path, const String &p_option, const HashMap<StringName, Variant> &p_options) const {
	return true;
}

Error GifToAnimatedTextureImportPlugin::import(ResourceUID::ID p_source_id, const String &p_source_file, const String &p_save_path, const HashMap<StringName, Variant> &p_options, List<String> *r_platform_variants, List<String> *r_gen_files, Variant *r_metadata) {
	Ref<AnimatedTexture> at = GifManager::get_singleton()->animated_texture_from_file(p_source_file, 0);
	return core_bind::ResourceSaver::get_singleton()->save(at, p_save_path + "." + get_save_extension(), 0);
}
