#include "gifmanager.h"
#include "image_frames.h"

#include "scene/resources/animated_texture.h"
#include "scene/resources/image_texture.h"
#include "scene/resources/sprite_frames.h"
#include <thirdparty/giflib/gif_lib.h>

GifManager *GifManager::singleton = nullptr;

GifManager::GifManager() {
	singleton = this;
}

GifManager::~GifManager() {
	singleton = nullptr;
}

GifManager *GifManager::get_singleton() {
	return singleton;
}

void GifManager::_bind_methods() {
	ClassDB::bind_method(D_METHOD("animated_texture_from_file", "path", "max_frames"), &GifManager::animated_texture_from_file, DEFVAL(0));
	ClassDB::bind_method(D_METHOD("animated_texture_from_buffer", "data", "max_frames"), &GifManager::animated_texture_from_buffer, DEFVAL(0));

	ClassDB::bind_method(D_METHOD("sprite_frames_from_file", "path", "max_frames", "fps"), &GifManager::sprite_frames_from_file, DEFVAL(0), DEFVAL(30));
	ClassDB::bind_method(D_METHOD("sprite_frames_from_buffer", "data", "max_frames", "fps"), &GifManager::sprite_frames_from_buffer, DEFVAL(0), DEFVAL(30));
}

Ref<AnimatedTexture> GifManager::animated_texture_from_file(const String &path, int max_frames = 0) {
	Ref<ImageFrames> frames = memnew(ImageFrames());
	Error err = frames->load(path, max_frames);
	if (err == OK) {
		return i_f_to_a_t(frames);
	}
	return nullptr;
}

Ref<AnimatedTexture> GifManager::animated_texture_from_buffer(const PackedByteArray &p_data, int max_frames = 0) {
	Ref<ImageFrames> frames = memnew(ImageFrames());
	Error err = frames->load_gif_from_buffer(p_data, max_frames);
	if (err == OK) {
		return i_f_to_a_t(frames);
	}
	return nullptr;
}

Ref<AnimatedTexture> GifManager::i_f_to_a_t(const Ref<ImageFrames> &frames) {
	Ref<AnimatedTexture> a;
	a.instantiate();
	int count = frames->get_frame_count();
	a->set_frames(count);
	for (int i = 0; i < count; i++) {
		a->set_frame_duration(i, frames->get_frame_delay(i));
		Ref<ImageTexture> tex = ImageTexture::create_from_image(frames->get_frame_image(i));
		a->set_frame_texture(i, tex);
	}
	return a;
}

Ref<SpriteFrames> GifManager::sprite_frames_from_file(const String &path, int max_frames = 0, double fps = 30) {
	Ref<ImageFrames> frames = memnew(ImageFrames());
	Error err = frames->load(path, max_frames);
	if (err == OK) {
		return i_f_to_s_f(frames, fps);
	}
	return nullptr;
}

Ref<SpriteFrames> GifManager::sprite_frames_from_buffer(const PackedByteArray &p_data, int max_frames = 0, double fps = 30) {
	Ref<ImageFrames> frames = memnew(ImageFrames());
	Error err = frames->load_gif_from_buffer(p_data, max_frames);
	if (err == OK) {
		return i_f_to_s_f(frames, fps);
	}
	return nullptr;
}

Ref<SpriteFrames> GifManager::i_f_to_s_f(const Ref<ImageFrames> &frames, double fps = 30) {
	Ref<SpriteFrames> sframes;
	sframes.instantiate();
	sframes->rename_animation("default", "gif");
	sframes->set_animation_loop("gif", true);
	int count = frames->get_frame_count();

	// Use the average time of all the frames.
	// real_t total_time = 0.0;
	for (int i = 0; i < count; i++) {
		//float duration = frames->get_frame_delay(i);
		// total_time += duration;
		Ref<ImageTexture> tex = ImageTexture::create_from_image(frames->get_frame_image(i));
		sframes->add_frame("gif", tex);
		// sframes->add_frame("gif", tex, duration); // with duration
	}

	// real_t average_time = total_time / count;
	// sframes->set_animation_speed("gif", average_time);
	sframes->set_animation_speed("gif", fps); // fps

	// Update durations
	for (int i = 0; i < count; i++) {
		float durationSeconds = frames->get_frame_delay(i);
		// float relativeDuration = durationSeconds * average_time;
		float relativeDuration = durationSeconds * fps;
		sframes->set_frame("gif", i, sframes->get_frame_texture("gif", i), relativeDuration);
	}

	return sframes;
}
