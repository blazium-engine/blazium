#include "register_types.h"

#include "core/core_bind.cpp"
#include "editor/plugins/editor_plugin.h"
#include "src/editor/import_gif_to_animated_texture.h"
#include "src/editor/import_gif_to_sprite_frames.h"
#include "src/gifmanager.h"
#include "src/image_frames.h"

static GifManager *GifMngrPtr;

void initialize_gif_module(ModuleInitializationLevel p_level) {
	if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
		ClassDB::register_class<ImageFrames>();
		ClassDB::register_class<GifManager>();
		GifMngrPtr = memnew(GifManager);
		core_bind::Engine::get_singleton()->register_singleton("GifManager", GifManager::get_singleton());
	}

	if (p_level == MODULE_INITIALIZATION_LEVEL_EDITOR) {
		ClassDB::register_class<GifToSpriteFramesImportPlugin>();
		ClassDB::register_class<GifToSpriteFramesPlugin>();
		EditorPlugins::add_by_type<GifToSpriteFramesPlugin>();

		ClassDB::register_class<GifToAnimatedTextureImportPlugin>();
		ClassDB::register_class<GifToAnimatedTexturePlugin>();
		EditorPlugins::add_by_type<GifToAnimatedTexturePlugin>();
	}
}

void uninitialize_gif_module(ModuleInitializationLevel p_level) {
	if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
		core_bind::Engine::get_singleton()->unregister_singleton("GifManager");
		memdelete(GifMngrPtr);
	}

	if (p_level == MODULE_INITIALIZATION_LEVEL_EDITOR) {
		//EditorPlugins::remove_by_type<GifToSpriteFramesPlugin>();
		//EditorPlugins::remove_by_type<GifToAnimatedTexturePlugin>();
	}
}
