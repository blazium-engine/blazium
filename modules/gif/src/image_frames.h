#ifndef IMAGE_FRAMES_H
#define IMAGE_FRAMES_H

#include "core/io/image.h"
#include "core/math/rect2.h"
#include "core/object/ref_counted.h"

#include <thirdparty/giflib/gif_lib.h>

class ImageFrames;

typedef Error (*LoadImageFramesFunction)(Ref<ImageFrames> &r_image_frames, const Variant &source, int max_frames);

class ImageFrames : public RefCounted {
	GDCLASS(ImageFrames, RefCounted);

	struct Frame {
		Ref<Image> image;
		float delay;
	};
	Vector<Frame> frames;

protected:
	static void _bind_methods();

public:
	Error load(const String &p_path, int max_frames = 0);
	Error load_gif_from_buffer(const PackedByteArray &p_data, int max_frames = 0);

	Error save_gif(const String &p_path, int p_color_count = 256);

	void add_frame(const Ref<Image> &p_image, float p_delay);
	void remove_frame(int p_idx);

	void set_frame_image(int p_idx, const Ref<Image> &p_image);
	Ref<Image> get_frame_image(int p_idx) const;

	void set_frame_delay(int p_idx, float p_delay);
	float get_frame_delay(int p_idx) const;

	Rect2 get_bounding_rect() const;
	int get_frame_count() const;

	void clear();
};

class GifLoader {
	GifFileType *gif;

	enum SourceType {
		k_FILE,
		k_BUFFER
	};
	Error parse_error(Error parse_error, const String &message);
	Error gif_error(int gif_error);

	Error _open(void *source, SourceType source_type);
	Error _load_frames(Ref<ImageFrames> &r_image_frames, int max_frames = 0);
	Error _close();

public:
	Error load_from_file_access(Ref<ImageFrames> &r_image_frames, Ref<FileAccess> f, int max_frames = 0);
	Error load_from_buffer(Ref<ImageFrames> &r_image_frames, const PackedByteArray &p_data, int max_frames = 0);

private:
	static int readFromFile(GifFileType *gif, GifByteType *data, int length);
	static int readFromBuffer(GifFileType *gif, GifByteType *data, int length);
};

struct GIFBuffer { // Used to read the GIF data from a buffer.
	const uint8_t *data;
	int size;
	int index;

	GIFBuffer(const PackedByteArray &p_data) {
		data = p_data.ptr();
		size = p_data.size();
		index = 0;
	}
	GIFBuffer(const uint8_t *p_data, int p_size) {
		data = p_data;
		size = p_size;
		index = 0;
	}
};

enum SourceType {
	k_FILE,
	k_BUFFER
};

#endif // IMAGE_FRAMES_H
