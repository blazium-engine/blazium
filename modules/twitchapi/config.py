def can_build(env, platform):
    """
    Determines if the module can be built on the given platform.

    The Twitch API module can be built on all platforms.
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
        "TwitchAPI",
        "TwitchHTTPClient",
        "TwitchRequestBase",
        "TwitchAdsRequests",
        "TwitchAnalyticsRequests",
        "TwitchBitsRequests",
        "TwitchChannelPointsRequests",
        "TwitchChannelsRequests",
        "TwitchChatRequests",
        "TwitchClipsRequests",
        "TwitchGamesRequests",
        "TwitchModerationRequests",
        "TwitchStreamsRequests",
        "TwitchUsersRequests",
    ]


def get_doc_path():
    """
    Returns the path to the documentation directory.
    """
    return "doc_classes"
