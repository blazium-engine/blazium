def can_build(env, platform):
    """
    IRC client module can be built on all platforms.
    SSL/TLS support via mbedtls is optional but recommended.
    """
    # Mark mbedtls as optional dependency
    env.module_add_dependencies("ircclient", ["mbedtls"], True)
    return True


def configure(env):
    """
    Configure the IRC client module build environment.
    """
    pass


def get_doc_classes():
    """
    Returns the list of classes to include in documentation.
    """
    return [
        "IRCClient",
        "IRCClientNode",
        "IRCMessage",
        "IRCChannel",
        "IRCUser",
        "IRCDCCTransfer",
        "TwitchIRCClient",
        "TwitchIRCClientNode",
        "TwitchMessage",
    ]


def get_doc_path():
    """
    Returns the path to the documentation directory.
    """
    return "doc_classes"
