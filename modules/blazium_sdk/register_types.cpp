#include "register_types.h"
#include "lobby/lobby_client.h"
#include "pogr/pogr_client.h"
#include "blazium_client.h"

void initialize_blazium_sdk_module(ModuleInitializationLevel p_level) {
    if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
		GDREGISTER_ABSTRACT_CLASS(BlaziumClient);
		GDREGISTER_CLASS(LobbyClient);
		GDREGISTER_CLASS(LobbyClient::LobbyInfo);
		GDREGISTER_CLASS(LobbyClient::LobbyPeer);
		GDREGISTER_CLASS(LobbyClient::CreateLobbyResponse::CreateLobbyResult);
		GDREGISTER_CLASS(LobbyClient::CreateLobbyResponse);
		GDREGISTER_CLASS(LobbyClient::LobbyResponse::LobbyResult);
		GDREGISTER_CLASS(LobbyClient::LobbyResponse);
		GDREGISTER_CLASS(LobbyClient::ListLobbyResponse::ListLobbyResult);
		GDREGISTER_CLASS(LobbyClient::ListLobbyResponse);
		GDREGISTER_CLASS(LobbyClient::ViewLobbyResponse::ViewLobbyResult);
		GDREGISTER_CLASS(LobbyClient::ViewLobbyResponse);
		GDREGISTER_CLASS(POGRClient);
		GDREGISTER_CLASS(POGRClient::POGRResponse);
	}
}

void uninitialize_blazium_sdk_module(ModuleInitializationLevel p_level) {
}
