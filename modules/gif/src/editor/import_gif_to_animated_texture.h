#ifndef IMPORT_GIF_TO_ANIMATED_TEXTURE_H
#define IMPORT_GIF_TO_ANIMATED_TEXTURE_H

#include "../image_frames.h"

#include "core/io/resource_importer.h"
#include "core/object/object.h"
#include "scene/resources/texture.h"

#include <thirdparty/giflib/gif_lib.h>

class GifToAnimatedTextureImportPlugin : public ResourceImporter {
	GDCLASS(GifToAnimatedTextureImportPlugin, ResourceImporter);

public:
	enum Presets {
		DEFAULT,
		COUNT
	};

	virtual String get_importer_name() const override;
	virtual String get_visible_name() const override;
	virtual int32_t get_preset_count() const override;
	virtual String get_preset_name(int32_t preset_index) const override;
	virtual void get_recognized_extensions(List<String> *p_extensions) const override;
	virtual void get_import_options(const String &p_path, List<ImportOption> *r_options, int p_preset) const override;
	virtual String get_save_extension() const override;
	virtual String get_resource_type() const override;
	virtual float get_priority() const override;
	virtual int32_t get_import_order() const override;
	virtual bool get_option_visibility(const String &p_path, const String &p_option, const HashMap<StringName, Variant> &p_options) const override;
	virtual Error import(ResourceUID::ID p_source_id, const String &p_source_file, const String &p_save_path, const HashMap<StringName, Variant> &p_options, List<String> *r_platform_variants, List<String> *r_gen_files, Variant *r_metadata) override;

private:
};

VARIANT_ENUM_CAST(GifToAnimatedTextureImportPlugin::Presets);

#endif // IMPORT_GIF_TO_ANIMATED_TEXTURE_H
