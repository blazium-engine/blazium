#include "./blazium_lobby.h"
#include "scene/main/node.h"
LobbyClient::LobbyClient() {
    _socket = Ref<WebSocketPeer>(WebSocketPeer::create());
}

LobbyClient::~LobbyClient() {
    _socket->close(1000, "Disconnected");
    set_process_internal(false);
}

void LobbyClient::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_server_url", "server_url"), &LobbyClient::set_server_url);
    ClassDB::bind_method(D_METHOD("get_server_url"), &LobbyClient::get_server_url);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "server_url", PROPERTY_HINT_NONE, ""), "set_server_url", "get_server_url");
    // Register methods
    ClassDB::bind_method(D_METHOD("connect_to_lobby", "game_id"), &LobbyClient::connect_to_lobby);
    ClassDB::bind_method(D_METHOD("create_lobby", "max_players", "password"), &LobbyClient::create_lobby, DEFVAL(4), DEFVAL(false));
    ClassDB::bind_method(D_METHOD("join_lobby", "lobby_name", "password"), &LobbyClient::join_lobby, DEFVAL(""));
    ClassDB::bind_method(D_METHOD("leave_lobby"), &LobbyClient::leave_lobby);
    ClassDB::bind_method(D_METHOD("list_lobby", "start", "count"), &LobbyClient::list_lobby, DEFVAL(0), DEFVAL(10));
    ClassDB::bind_method(D_METHOD("view_lobby", "lobby_name", "password"), &LobbyClient::view_lobby, DEFVAL(""));
    ClassDB::bind_method(D_METHOD("kick_peer", "peer_id"), &LobbyClient::kick_peer);
    ClassDB::bind_method(D_METHOD("lobby_ready"), &LobbyClient::lobby_ready);
    ClassDB::bind_method(D_METHOD("lobby_unready"), &LobbyClient::lobby_unready);
    ClassDB::bind_method(D_METHOD("seal_lobby"), &LobbyClient::seal_lobby);
    ClassDB::bind_method(D_METHOD("unseal_lobby"), &LobbyClient::unseal_lobby);

    // Register signals
    ADD_SIGNAL(MethodInfo("peer_named", PropertyInfo(Variant::STRING, "peer"), PropertyInfo(Variant::STRING, "name")));
    ADD_SIGNAL(MethodInfo("received_data", PropertyInfo(Variant::STRING, "data")));
    ADD_SIGNAL(MethodInfo("received_data_to", PropertyInfo(Variant::STRING, "data")));
    ADD_SIGNAL(MethodInfo("lobby_created", PropertyInfo(Variant::STRING, "lobby")));
    ADD_SIGNAL(MethodInfo("lobby_joined", PropertyInfo(Variant::STRING, "lobby")));
    ADD_SIGNAL(MethodInfo("lobby_left"));
    ADD_SIGNAL(MethodInfo("lobby_sealed"));
    ADD_SIGNAL(MethodInfo("lobby_unsealed"));
    ADD_SIGNAL(MethodInfo("peer_joined", PropertyInfo(Variant::STRING, "peer")));
    ADD_SIGNAL(MethodInfo("peer_left", PropertyInfo(Variant::STRING, "peer")));
    ADD_SIGNAL(MethodInfo("peer_ready", PropertyInfo(Variant::STRING, "peer")));
    ADD_SIGNAL(MethodInfo("peer_unready", PropertyInfo(Variant::STRING, "peer")));
    ADD_SIGNAL(MethodInfo("lobby_view", PropertyInfo(Variant::STRING, "host"), PropertyInfo(Variant::BOOL, "sealed"), PropertyInfo(Variant::ARRAY, "peer_ids"), PropertyInfo(Variant::ARRAY, "peer_names"), PropertyInfo(Variant::ARRAY, "peer_readys")));
    ADD_SIGNAL(MethodInfo("lobby_list", PropertyInfo(Variant::ARRAY, "lobbies")));
    ADD_SIGNAL(MethodInfo("append_log", PropertyInfo(Variant::STRING, "command"), PropertyInfo(Variant::STRING, "command"), PropertyInfo(Variant::STRING, "logs")));
}

void LobbyClient::connect_to_lobby(const String &game_id) {
    String lobby_url = get_server_url();
    String url = lobby_url + "?gameID=" + game_id;
    Error err = _socket->connect_to_url(url);
    if (err != OK) {
        emit_signal("append_log", "error", "Unable to connect to lobby server at: " + url);
        return;
    }
    set_process_internal(true);
    emit_signal("append_log", "connect_to_lobby","Connected to: " + url);
}

String LobbyClient::_increment_counter() {
    return String::num(_counter++);
}

Ref<LobbyClient::CreateLobbyResponse> LobbyClient::create_lobby(int max_players, const String &password) {
    String id = _increment_counter();
    Dictionary command;
    command["command"] = "create_lobby";
    Dictionary data;
    command["data"] = data;
    data["max_players"] = max_players;
    data["password"] = password;
    data["id"] = id;
    Array command_array;
    Ref<CreateLobbyResponse> response;
    response.instantiate();
    command_array.push_back(CREATE_LOBBY);
    command_array.push_back(response);
    _commands[id] = command_array;
    _send_data(command);
    return response;
}

Ref<LobbyClient::LobbyResponse> LobbyClient::join_lobby(const String &lobby_name, const String &password) {
    String id = _increment_counter();
    Dictionary command;
    command["command"] = "join_lobby";
    Dictionary data;
    command["data"] = data;
    data["lobby_name"] = lobby_name;
    data["password"] = password;
    data["id"] = id;
    Array command_array;
    Ref<LobbyResponse> response;
    response.instantiate();
    command_array.push_back(SIMPLE_REQUEST);
    command_array.push_back(response);
    _commands[id] = command_array;
    _send_data(command);
    return response;
}

Ref<LobbyClient::LobbyResponse> LobbyClient::leave_lobby() {
    String id = _increment_counter();
    Dictionary command;
    command["command"] = "leave_lobby";
    Dictionary data;
    command["data"] = data;
    data["id"] = id;
    Array command_array;
    Ref<LobbyResponse> response;
    response.instantiate();
    command_array.push_back(SIMPLE_REQUEST);
    command_array.push_back(response);
    _commands[id] = command_array;
    _send_data(command);
    return response;
}

Ref<LobbyClient::ListLobbyResponse> LobbyClient::list_lobby(int start, int count) {
    String id = _increment_counter();
    Dictionary command;
    command["command"] = "list_lobby";
    Dictionary data;
    command["data"] = data;
    data["id"] = id;
    Array command_array;
    Ref<ListLobbyResponse> response;
    response.instantiate();
    command_array.push_back(LOBBY_LIST);
    command_array.push_back(response);
    _commands[id] = command_array;
    _send_data(command);
    return response;
}

Ref<LobbyClient::ViewLobbyResponse> LobbyClient::view_lobby(const String &lobby_name, const String &password) {
    String id = _increment_counter();
    Dictionary command;
    command["command"] = "view_lobby";
    Dictionary data;
    command["data"] = data;
    data["id"] = id;
    Array command_array;
    Ref<ViewLobbyResponse> response;
    response.instantiate();
    command_array.push_back(LOBBY_VIEW);
    command_array.push_back(response);
    _commands[id] = command_array;
    _send_data(command);
    return response;
}

Ref<LobbyClient::LobbyResponse> LobbyClient::kick_peer(const String &peer_id) {
    String id = _increment_counter();
    Dictionary command;
    command["command"] = "kick_peer";
    Dictionary data;
    command["data"] = data;
    data["id"] = id;
    Array command_array;
    Ref<LobbyResponse> response;
    response.instantiate();
    command_array.push_back(SIMPLE_REQUEST);
    command_array.push_back(response);
    _commands[id] = command_array;
    _send_data(command);
    return response;
}

Ref<LobbyClient::LobbyResponse> LobbyClient::lobby_ready() {
    String id = _increment_counter();
    Dictionary command;
    command["command"] = "lobby_ready";
    Dictionary data;
    command["data"] = data;
    data["id"] = id;
    Array command_array;
    Ref<LobbyResponse> response;
    response.instantiate();
    command_array.push_back(SIMPLE_REQUEST);
    command_array.push_back(response);
    _commands[id] = command_array;
    _send_data(command);
    return response;
}

Ref<LobbyClient::LobbyResponse> LobbyClient::lobby_unready() {
    String id = _increment_counter();
    Dictionary command;
    command["command"] = "lobby_unready";
    Dictionary data;
    command["data"] = data;
    data["id"] = id;
    Array command_array;
    Ref<LobbyResponse> response;
    response.instantiate();
    command_array.push_back(SIMPLE_REQUEST);
    command_array.push_back(response);
    _commands[id] = command_array;
    _send_data(command);
    return response;
}

Ref<LobbyClient::LobbyResponse> LobbyClient::seal_lobby() {
    String id = _increment_counter();
    Dictionary command;
    command["command"] = "seal_lobby";
    Dictionary data;
    command["data"] = data;
    data["id"] = id;
    Array command_array;
    Ref<LobbyResponse> response;
    response.instantiate();
    command_array.push_back(SIMPLE_REQUEST);
    command_array.push_back(response);
    _commands[id] = command_array;
    _send_data(command);
    return response;
}

Ref<LobbyClient::LobbyResponse> LobbyClient::unseal_lobby() {
    String id = _increment_counter();
    Dictionary command;
    command["command"] = "unseal_lobby";
    Dictionary data;
    command["data"] = data;
    data["id"] = id;
    Array command_array;
    Ref<LobbyResponse> response;
    response.instantiate();
    command_array.push_back(SIMPLE_REQUEST);
    command_array.push_back(response);
    _commands[id] = command_array;
    _send_data(command);
    return response;
}

void LobbyClient::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_INTERNAL_PROCESS: {

    _socket->poll();

    WebSocketPeer::State state = _socket->get_ready_state();
    if (state == WebSocketPeer::STATE_OPEN) {
        while (_socket->get_available_packet_count() > 0) {
            Vector<uint8_t> packet_buffer;
            Error err = _socket->get_packet_buffer(packet_buffer);
            if (err != OK) {
                emit_signal("append_log", "error", "Unable to get packet.");
                return;
            }
            String packet_string = String::utf8((const char *)packet_buffer.ptr(), packet_buffer.size());
            _receive_data(JSON::parse_string(packet_string));
        }
    } else if (state == WebSocketPeer::STATE_CLOSED) {
        emit_signal("append_log", "error", "WebSocket closed unexpectedly.");
    }
		} break;
	}
}

void LobbyClient::_send_data(const Dictionary &data) {
    if (_socket->get_ready_state() != WebSocketPeer::STATE_OPEN) {
        emit_signal("append_log", "error", "Socket is not ready.");
        return;
    }
    _socket->send_text(JSON::stringify(data));
}

void LobbyClient::_receive_data(const Dictionary &dict) {
    String command = "error";
    if (dict.has("command")) {
        command = dict["command"];
    }
    emit_signal("append_log", command, dict["message"]);
    if (command == "lobby_created") {
        emit_signal("lobby_created", dict["data"]);
    } else if (command == "joined_lobby") {
        emit_signal("lobby_joined", dict["data"]);
    } else if (command == "lobby_left") {
        emit_signal("lobby_left");
    } else if (command == "lobby_sealed") {
        emit_signal("lobby_sealed");
    } else if (command == "lobby_unsealed") {
        emit_signal("lobby_unsealed");
    } else if (command == "lobby_list") {
        emit_signal("lobby_list", dict["data"]);
    } else if (command == "lobby_view") {
        Dictionary data_dict = dict["data"];
        Dictionary lobby_dict = data_dict["lobby"];
        
        Array peer_ids;
        Array peer_names;
        Array peer_readys;

        // Iterate through peers and populate arrays
        Array peers = data_dict["peers"];
        for (int i = 0; i < peers.size(); ++i) {
            Dictionary peer_json = peers[i];
            peer_ids.push_back(peer_json["peer_id"]);
            peer_names.push_back(peer_json["peer_name"]);
            peer_readys.push_back(peer_json["peer_ready"]);
        }

        // Emit the signal
        emit_signal("lobby_view", lobby_dict["host"], lobby_dict["sealed"], peer_ids, peer_names, peer_readys);
    } else if (command == "peer_ready") {
        emit_signal("peer_ready", dict["data"]);
    } else if (command == "peer_unready") {
        emit_signal("peer_unready", dict["data"]);
    } else if (command == "peer_joined") {
        emit_signal("peer_joined", dict["data"]);
    } else if (command == "peer_left") {
        emit_signal("peer_left", dict["data"]);
    } else if (command == "error") {
        emit_signal("append_log", "error", dict["message"]);
    } else{
        emit_signal("append_log", "error", "Unknown command received.");
    }
}
