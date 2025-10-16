def can_build(env, platform):
    """
    Determines if the Kick API module can be built on the given platform.

    Args:
        env: SCons environment
        platform: Target platform (e.g., "windows", "linux", "web", "macos", "android", "ios")

    Returns:
        bool: True if module can be built, False otherwise
    """
    return True  # Allow building on all platforms


def configure(env):
    """
    Configure the module's build environment.

    Args:
        env: SCons environment to configure
    """
    pass  # No special configuration needed


def get_doc_classes():
    """
    Returns a list of classes that should have documentation generated.

    Returns:
        list: Class names that will appear in the engine documentation
    """
    return [
        "KickAPI",
        "KickHTTPClient",
        "KickRequestBase",
        "KickCategoriesRequests",
        "KickChannelsRequests",
        "KickChatRequests",
        "KickEventsRequests",
        "KickLivestreamsRequests",
        "KickModerationRequests",
        "KickOAuthRequests",
        "KickUsersRequests",
    ]


def get_doc_path():
    """
    Returns the path to the documentation directory.

    Returns:
        str: Path to doc_classes directory
    """
    return "doc_classes"
