def can_build(env, platform):
    return True


def configure(env):
    pass


def get_doc_classes():
    return [
        "ImageFrames",
        "GifManager",
        "GifToSpriteFramesImportPlugin",
        "GifToSpriteFramesPlugin",
        "GifToAnimatedTextureImportPlugin",
        "GifToAnimatedTexturePlugin",
    ]


def get_doc_path():
    return "doc_classes"
