#ifndef GIFMANAGER_H
#define GIFMANAGER_H

#include "image_frames.h"

#include <thirdparty/giflib/gif_lib.h>

class AnimatedTexture;
class SpriteFrames;

class GifManager : public Object {
	GDCLASS(GifManager, Object);
	static GifManager *singleton;
	GifFileType *gif;

public:
	static GifManager *get_singleton();
	GifManager();
	~GifManager();

	Ref<AnimatedTexture> animated_texture_from_file(const String &path, int max_frames);
	Ref<AnimatedTexture> animated_texture_from_buffer(const PackedByteArray &p_data, int max_frames);
	Ref<SpriteFrames> sprite_frames_from_file(const String &path, int max_frames, double fps);
	Ref<SpriteFrames> sprite_frames_from_buffer(const PackedByteArray &p_data, int max_frames, double fps);

protected:
	static void _bind_methods();

private:
	Ref<AnimatedTexture> i_f_to_a_t(const Ref<ImageFrames> &frames);
	Ref<SpriteFrames> i_f_to_s_f(const Ref<ImageFrames> &frames, double fps);
};

#endif // GIFMANAGER_H
