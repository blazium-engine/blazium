def can_build(env, platform):
    return platform == "macos" or platform == "windows" or platform == "linuxbsd"


def configure(env):
    pass


def get_doc_classes():
    return [
        "Steam",
    ]


def get_doc_path():
    return "doc_classes"
