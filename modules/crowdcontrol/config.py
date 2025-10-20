def can_build(env, platform):
    """
    Determines if the module can be built on the given platform.
    Crowd Control requires WebSocket support, which is available on all platforms.
    """
    return True


def configure(env):
    """
    Configure the module's build environment.
    """
    pass


def get_doc_classes():
    """
    Returns a list of classes that should have documentation generated.
    """
    return [
        "CrowdControl",
        "CrowdControlEffectParameter",
        "CrowdControlEffect",
        "CrowdControlGamePackMeta",
        "CrowdControlGamePack",
    ]


def get_doc_path():
    """
    Returns the path to the documentation directory.
    """
    return "doc_classes"
