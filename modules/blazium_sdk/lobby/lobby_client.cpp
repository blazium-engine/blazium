/**************************************************************************/
/*  lobby_client.cpp                                                      */
/**************************************************************************/
/*                         This file is part of:                          */
/*                            BLAZIUM ENGINE                              */
/*                        https://blazium.app                             */
/**************************************************************************/
/* Copyright (c) 2024-present Blazium Engine contributors.                */
/* Copyright (c) 2024 Dragos Daian, Randolph William Aarseth II.          */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#include "./lobby_client.h"
#include "scene/main/node.h"
LobbyClient::LobbyClient() {
	_socket = Ref<WebSocketPeer>(WebSocketPeer::create());
	set_process_internal(false);
}

LobbyClient::~LobbyClient() {
	_socket->close(1000, "Disconnected");
	set_process_internal(false);
}

void LobbyClient::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_server_url", "server_url"), &LobbyClient::set_server_url);
	ClassDB::bind_method(D_METHOD("get_server_url"), &LobbyClient::get_server_url);
	ClassDB::bind_method(D_METHOD("get_connected"), &LobbyClient::get_connected);
	ClassDB::bind_method(D_METHOD("get_lobby_id"), &LobbyClient::get_lobby_id);
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "server_url", PROPERTY_HINT_NONE, ""), "set_server_url", "get_server_url");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "connected"), "", "get_connected");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "lobby_id"), "", "get_lobby_id");
	// Register methods
	ClassDB::bind_method(D_METHOD("connect_to_lobby", "game_id"), &LobbyClient::connect_to_lobby);
	ClassDB::bind_method(D_METHOD("create_lobby", "title", "max_players", "password"), &LobbyClient::create_lobby, DEFVAL(4), DEFVAL(""));
	ClassDB::bind_method(D_METHOD("join_lobby", "lobby_id", "password"), &LobbyClient::join_lobby, DEFVAL(""));
	ClassDB::bind_method(D_METHOD("leave_lobby"), &LobbyClient::leave_lobby);
	ClassDB::bind_method(D_METHOD("list_lobby", "start", "count"), &LobbyClient::list_lobby, DEFVAL(0), DEFVAL(10));
	ClassDB::bind_method(D_METHOD("view_lobby", "lobby_id", "password"), &LobbyClient::view_lobby, DEFVAL(""));
	ClassDB::bind_method(D_METHOD("kick_peer", "peer_id"), &LobbyClient::kick_peer);
	ClassDB::bind_method(D_METHOD("lobby_ready"), &LobbyClient::lobby_ready);
	ClassDB::bind_method(D_METHOD("lobby_unready"), &LobbyClient::lobby_unready);
	ClassDB::bind_method(D_METHOD("seal_lobby"), &LobbyClient::seal_lobby);
	ClassDB::bind_method(D_METHOD("unseal_lobby"), &LobbyClient::unseal_lobby);
	ClassDB::bind_method(D_METHOD("lobby_data", "data"), &LobbyClient::lobby_data);
	ClassDB::bind_method(D_METHOD("lobby_data_to", "data", "target_peer"), &LobbyClient::lobby_data_to);

	// Register signals
	ADD_SIGNAL(MethodInfo("disconnected_from_lobby"));
	ADD_SIGNAL(MethodInfo("peer_named", PropertyInfo(Variant::STRING, "peer"), PropertyInfo(Variant::STRING, "name")));
	ADD_SIGNAL(MethodInfo("received_data", PropertyInfo(Variant::STRING, "data")));
	ADD_SIGNAL(MethodInfo("received_data_to", PropertyInfo(Variant::STRING, "data")));
	ADD_SIGNAL(MethodInfo("lobby_created", PropertyInfo(Variant::STRING, "lobby")));
	ADD_SIGNAL(MethodInfo("lobby_joined", PropertyInfo(Variant::STRING, "lobby")));
	ADD_SIGNAL(MethodInfo("lobby_left"));
	ADD_SIGNAL(MethodInfo("lobby_sealed"));
	ADD_SIGNAL(MethodInfo("lobby_unsealed"));
	ADD_SIGNAL(MethodInfo("peer_joined", PropertyInfo(Variant::STRING, "peer")));
	ADD_SIGNAL(MethodInfo("peer_left", PropertyInfo(Variant::STRING, "peer"), PropertyInfo(Variant::BOOL, "kicked")));
	ADD_SIGNAL(MethodInfo("peer_ready", PropertyInfo(Variant::STRING, "peer")));
	ADD_SIGNAL(MethodInfo("peer_unready", PropertyInfo(Variant::STRING, "peer")));
	ADD_SIGNAL(MethodInfo("append_log", PropertyInfo(Variant::STRING, "command"), PropertyInfo(Variant::STRING, "info"), PropertyInfo(Variant::STRING, "logs")));
}

bool LobbyClient::connect_to_lobby(const String &p_game_id) {
	if (connected) {
		return true;
	}
	String lobby_url = get_server_url();
	String url = lobby_url + "?gameID=" + p_game_id;
	Error err = _socket->connect_to_url(url);
	if (err != OK) {
		set_process_internal(false);
		emit_signal("append_log", "error", "Unable to connect to lobby server at: " + url);
		connected = false;
		return false;
	}
	connected = true;
	set_process_internal(true);
	emit_signal("append_log", "connect_to_lobby", "Connected to: " + url);
	return true;
}

String LobbyClient::_increment_counter() {
	return String::num(_counter++);
}

Ref<LobbyClient::CreateLobbyResponse> LobbyClient::create_lobby(const String &p_lobby_name, int p_max_players, const String &p_password) {
	String id = _increment_counter();
	Dictionary command;
	command["command"] = "create_lobby";
	Dictionary data_dict;
	command["data"] = data_dict;
	data_dict["name"] = p_lobby_name;
	data_dict["max_players"] = p_max_players;
	data_dict["password"] = p_password;
	data_dict["id"] = id;
	Array command_array;
	Ref<CreateLobbyResponse> response;
	response.instantiate();
	command_array.push_back(CREATE_LOBBY);
	command_array.push_back(response);
	_commands[id] = command_array;
	_send_data(command);
	return response;
}

Ref<LobbyClient::LobbyResponse> LobbyClient::join_lobby(const String &p_lobby_id, const String &p_password) {
	String id = _increment_counter();
	Dictionary command;
	command["command"] = "join_lobby";
	Dictionary data_dict;
	command["data"] = data_dict;
	data_dict["lobby_id"] = p_lobby_id;
	data_dict["password"] = p_password;
	data_dict["id"] = id;
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
	Dictionary data_dict;
	command["data"] = data_dict;
	data_dict["id"] = id;
	Array command_array;
	Ref<LobbyResponse> response;
	response.instantiate();
	command_array.push_back(SIMPLE_REQUEST);
	command_array.push_back(response);
	_commands[id] = command_array;
	_send_data(command);
	return response;
}

Ref<LobbyClient::ListLobbyResponse> LobbyClient::list_lobby(int p_start, int p_count) {
	String id = _increment_counter();
	Dictionary command;
	command["command"] = "list_lobby";
	Dictionary data_dict;
	data_dict["id"] = id;
	data_dict["start"] = p_start;
	data_dict["count"] = p_count;
	command["data"] = data_dict;
	Array command_array;
	Ref<ListLobbyResponse> response;
	response.instantiate();
	command_array.push_back(LOBBY_LIST);
	command_array.push_back(response);
	_commands[id] = command_array;
	_send_data(command);
	return response;
}

Ref<LobbyClient::ViewLobbyResponse> LobbyClient::view_lobby(const String &p_lobby_id, const String &p_password) {
	String id = _increment_counter();
	Dictionary command;
	command["command"] = "view_lobby";
	Dictionary data_dict;
	command["data"] = data_dict;
	data_dict["lobby_id"] = p_lobby_id;
	data_dict["password"] = p_password;
	data_dict["id"] = id;
	Array command_array;
	Ref<ViewLobbyResponse> response;
	response.instantiate();
	command_array.push_back(LOBBY_VIEW);
	command_array.push_back(response);
	_commands[id] = command_array;
	_send_data(command);
	return response;
}

Ref<LobbyClient::LobbyResponse> LobbyClient::kick_peer(const String &p_peer_id) {
	String id = _increment_counter();
	Dictionary command;
	command["command"] = "kick_peer";
	Dictionary data_dict;
	command["data"] = data_dict;
	data_dict["peer_id"] = p_peer_id;
	data_dict["id"] = id;
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
	Dictionary data_dict;
	command["data"] = data_dict;
	data_dict["id"] = id;
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
	Dictionary data_dict;
	command["data"] = data_dict;
	data_dict["id"] = id;
	Array command_array;
	Ref<LobbyResponse> response;
	response.instantiate();
	command_array.push_back(SIMPLE_REQUEST);
	command_array.push_back(response);
	_commands[id] = command_array;
	_send_data(command);
	return response;
}

Ref<LobbyClient::LobbyResponse> LobbyClient::set_peer_name(const String &p_peer_name) {
	String id = _increment_counter();
	Dictionary command;
	command["command"] = "set_name";
	Dictionary data_dict;
	data_dict["peer_name"] = p_peer_name;
	data_dict["id"] = id;
	command["data"] = data_dict;
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
	Dictionary data_dict;
	command["data"] = data_dict;
	data_dict["id"] = id;
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
	Dictionary data_dict;
	command["data"] = data_dict;
	data_dict["id"] = id;
	Array command_array;
	Ref<LobbyResponse> response;
	response.instantiate();
	command_array.push_back(SIMPLE_REQUEST);
	command_array.push_back(response);
	_commands[id] = command_array;
	_send_data(command);
	return response;
}

Ref<LobbyClient::LobbyResponse> LobbyClient::lobby_data(const String &p_peer_data) {
	String id = _increment_counter();
	Dictionary command;
	command["command"] = "lobby_data";
	Dictionary data_dict;
	data_dict["peer_data"] = p_peer_data;
	data_dict["id"] = id;
	command["data"] = data_dict;
	Array command_array;
	Ref<LobbyResponse> response;
	response.instantiate();
	command_array.push_back(SIMPLE_REQUEST);
	command_array.push_back(response);
	_commands[id] = command_array;
	_send_data(command);
	return response;
}

Ref<LobbyClient::LobbyResponse> LobbyClient::lobby_data_to(const String &p_peer_data, const String &p_target_peer) {
	String id = _increment_counter();
	Dictionary command;
	command["command"] = "data_to";
	Dictionary data_dict;
	data_dict["peer_data"] = p_peer_data;
	data_dict["target_peer"] = p_target_peer;
	data_dict["id"] = id;
	command["data"] = data_dict;
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
				emit_signal("disconnected_from_lobby");
				set_process_internal(false);
				connected = false;
			}
		} break;
	}
}

void LobbyClient::_send_data(const Dictionary &p_data_dict) {
	if (_socket->get_ready_state() != WebSocketPeer::STATE_OPEN) {
		emit_signal("append_log", "error", "Socket is not ready.");
		return;
	}
	_socket->send_text(JSON::stringify(p_data_dict));
}

void LobbyClient::_receive_data(const Dictionary &p_dict) {
	String command = p_dict.get("command", "error");
	String message = p_dict.get("message", command);
	Dictionary data_dict = p_dict.get("data", Dictionary());
	String message_id = data_dict.get("id", "");
	Array command_array = _commands.get(message_id, Array());
	_commands.erase(message_id);
	emit_signal("append_log", command, message);
	if (command == "lobby_created") {
		lobby_id = data_dict.get("lobby_id", "");
		if (command_array.size() == 2) {
			Ref<CreateLobbyResponse> response = command_array[1];
			if (response.is_valid()) {
				Ref<CreateLobbyResponse::CreateLobbyResult> result;
				result.instantiate();
				result->set_lobby_id(lobby_id);
				result->set_lobby_name(data_dict.get("lobby_name", ""));
				response->emit_signal("finished", result);
			}
		}
		emit_signal("lobby_created", lobby_id);
	} else if (command == "joined_lobby") {
		lobby_id = data_dict.get("lobby_id", "");
		if (command_array.size() == 2) {
			Ref<LobbyResponse> response = command_array[1];
			if (response.is_valid()) {
				Ref<LobbyResponse::LobbyResult> result;
				result.instantiate();
				response->emit_signal("finished", result);
			}
		}
		emit_signal("lobby_joined", lobby_id);
	} else if (command == "lobby_left") {
		// Either if you leave a lobby, or if you get kicked
		if (command_array.size() == 2) {
			Ref<LobbyResponse> response = command_array[1];
			if (response.is_valid()) {
				Ref<LobbyResponse::LobbyResult> result;
				result.instantiate();
				response->emit_signal("finished", result);
			}
		}
		lobby_id = "";
		emit_signal("lobby_left");
	} else if (command == "lobby_sealed") {
		// Either if you seal a lobby, or if host seals
		if (command_array.size() == 2) {
			Ref<LobbyResponse> response = command_array[1];
			if (response.is_valid()) {
				Ref<LobbyResponse::LobbyResult> result;
				result.instantiate();
				response->emit_signal("finished", result);
			}
		}
		emit_signal("lobby_sealed");
	} else if (command == "lobby_unsealed") {
		// Either if you unseal a lobby, or if host unseals
		if (command_array.size() == 2) {
			Ref<LobbyResponse> response = command_array[1];
			if (response.is_valid()) {
				Ref<LobbyResponse::LobbyResult> result;
				result.instantiate();
				response->emit_signal("finished", result);
			}
		}
		emit_signal("lobby_unsealed");
	} else if (command == "lobby_list") {
		Array arr = data_dict.get("lobbies", Array());
		TypedArray<Dictionary> lobbies_input = arr;
		TypedArray<Dictionary> lobbies_output;
		for (int i = 0; i < lobbies_input.size(); ++i) {
			Dictionary lobby_dict = lobbies_input[i];
			String lobby_id = lobby_dict.get("lobby_id", "");
			String lobby_name = lobby_dict.get("lobby_name", "");
			String host = lobby_dict.get("host", "");
			String host_name = lobby_dict.get("host_name", "");
			bool sealed = lobby_dict.get("sealed", false);
			int max_players = lobby_dict.get("max_players", 0);
			int num_players = lobby_dict.get("players", 0);
			lobby_dict["max_players"] = max_players;
			Ref<LobbyInfo> lobby_info;
			lobby_info.instantiate();
			lobby_info->set_host(host);
			lobby_info->set_max_players(max_players);
			lobby_info->set_sealed(sealed);
			lobby_info->set_players(num_players);
			lobby_info->set_id(lobby_id);
			lobby_info->set_name(lobby_name);
			lobby_info->set_host_name(host_name);
			
			lobbies_output.push_back(lobby_info);
		}
		if (command_array.size() == 2) {
			Ref<ListLobbyResponse> response = command_array[1];
			if (response.is_valid()) {
				Ref<ListLobbyResponse::ListLobbyResult> result;
				result.instantiate();
				result->set_lobbies(lobbies_output);
				response->emit_signal("finished", result);
			}
		}
	} else if (command == "lobby_view") {
		Dictionary lobby_dict = data_dict.get("lobby", Dictionary());
		String host_name = lobby_dict.get("host_name", "");
		String host = lobby_dict.get("host", "");
		String name = lobby_dict.get("name", "");
		String id = lobby_dict.get("id", "");
		bool sealed = lobby_dict.get("sealed", false);
		int max_players = lobby_dict.get("max_players", 0);
		int players = lobby_dict.get("players", 0);

		// Iterate through peers and populate arrays
		Array arr = data_dict["peers"];
		TypedArray<LobbyPeer> peers;
		for (int i = 0; i < arr.size(); ++i) {
			Ref<LobbyPeer> peer;
			peer.instantiate();
			String peer_id = arr[i].get("id");
			String peer_name = arr[i].get("name");
			bool peer_ready = arr[i].get("ready");
			peer->set_id(peer_id);
			peer->set_name(peer_name);
			peer->set_ready(peer_ready);
			peers.push_back(peer);
		}
		Ref<LobbyInfo> lobby_info;
		lobby_info.instantiate();
		lobby_info->set_host(host);
		lobby_info->set_sealed(sealed);
		lobby_info->set_max_players(max_players);
		lobby_info->set_players(players);
		lobby_info->set_id(id);
		lobby_info->set_name(name);
		lobby_info->set_host_name(host_name);
		if (command_array.size() == 2) {
			Ref<ViewLobbyResponse> response = command_array[1];
			if (response.is_valid()) {
				Ref<ViewLobbyResponse::ViewLobbyResult> result;
				result.instantiate();
				result->set_peers(peers);
				result->set_lobby_info(lobby_info);
				response->emit_signal("finished", result);
			}
		}
	} else if (command == "peer_name") {
		// Either if you ready a lobby, or if someone else readies
		String peer_id = data_dict.get("peer_id", "");
		String peer_name = data_dict.get("name", "");
		if (command_array.size() == 2) {
			Ref<LobbyResponse> response = command_array[1];
			if (response.is_valid()) {
				Ref<LobbyResponse::LobbyResult> result;
				result.instantiate();
				response->emit_signal("finished", result);
			}
		}
		emit_signal("peer_named", peer_id, peer_name);
	} else if (command == "peer_ready") {
		// Either if you ready a lobby, or if someone else readies
		String peer_id = data_dict.get("peer_id", "");
		if (command_array.size() == 2) {
			Ref<LobbyResponse> response = command_array[1];
			if (response.is_valid()) {
				Ref<LobbyResponse::LobbyResult> result;
				result.instantiate();
				response->emit_signal("finished", result);
			}
		}
		emit_signal("peer_ready", peer_id);
	} else if (command == "peer_unready") {
		// Either if you unready a lobby, or if someone else unreadies
		String peer_id = data_dict.get("peer_id", "");
		if (command_array.size() == 2) {
			Ref<LobbyResponse> response = command_array[1];
			if (response.is_valid()) {
				Ref<LobbyResponse::LobbyResult> result;
				result.instantiate();
				response->emit_signal("finished", result);
			}
		}
		emit_signal("peer_unready", peer_id);
	} else if (command == "peer_joined") {
		String peer_id = data_dict.get("peer_id", "");
		String peer_name = data_dict.get("peer_name", "");
		emit_signal("peer_joined", peer_id, peer_name);
	} else if (command == "peer_left") {
		// Either if you kick a peer, or a peer leaves
		String peer_id = data_dict.get("peer_id", "");
		bool kicked = data_dict.get("kicked", false);
		if (command_array.size() == 2) {
			Ref<LobbyResponse> response = command_array[1];
			if (response.is_valid()) {
				Ref<LobbyResponse::LobbyResult> result;
				result.instantiate();
				response->emit_signal("finished", result);
			}
		}
		emit_signal("peer_left", peer_id, kicked);
	} else if (command == "lobby_data") {
		String peer_data = data_dict.get("peer_data", "");
		if (command_array.size() == 2) {
			Ref<LobbyResponse> response = command_array[1];
			if (response.is_valid()) {
				Ref<LobbyResponse::LobbyResult> result;
				result.instantiate();
				response->emit_signal("finished", result);
			}
		}
		emit_signal("received_data", peer_data);
	} else if (command == "data_to") {
		String peer_data = data_dict.get("peer_data", "");
		if (command_array.size() == 2) {
			Ref<LobbyResponse> response = command_array[1];
			if (response.is_valid()) {
				Ref<LobbyResponse::LobbyResult> result;
				result.instantiate();
				response->emit_signal("finished", result);
			}
		}
		emit_signal("received_data_to", peer_data);
	} else if (command == "lobby_data_sent") {
		if (command_array.size() == 2) {
			Ref<LobbyResponse> response = command_array[1];
			if (response.is_valid()) {
				Ref<LobbyResponse::LobbyResult> result;
				result.instantiate();
				response->emit_signal("finished", result);
			}
		}
	} else if (command == "data_to_sent") {
		if (command_array.size() == 2) {
			Ref<LobbyResponse> response = command_array[1];
			if (response.is_valid()) {
				Ref<LobbyResponse::LobbyResult> result;
				result.instantiate();
				response->emit_signal("finished", result);
			}
		}
	} else if (command == "error") {
		if (command_array.size() == 2) {
			int command_type = command_array[0];
			switch (command_type) {
				case SIMPLE_REQUEST: {
					Ref<LobbyResponse> lobby_response = command_array[1];
					if (lobby_response.is_valid()) {
						Ref<LobbyResponse::LobbyResult> result;
						result.instantiate();
						result->set_error(message);
						lobby_response->emit_signal("finished", result);
					}
				} break;
				case LOBBY_LIST: {
					Ref<ListLobbyResponse> list_response = command_array[1];
					if (list_response.is_valid()) {
						Ref<ListLobbyResponse::ListLobbyResult> result;
						result.instantiate();
						result->set_error(message);
						list_response->emit_signal("finished", result);
					}
				} break;
				case CREATE_LOBBY: {
					Ref<CreateLobbyResponse> create_response = command_array[1];
					if (create_response.is_valid()) {
						Ref<CreateLobbyResponse::CreateLobbyResult> result;
						result.instantiate();
						result->set_error(message);
						create_response->emit_signal("finished", result);
					}
				} break;
				case LOBBY_VIEW: {
					Ref<ViewLobbyResponse> view_response = command_array[1];
					if (view_response.is_valid()) {
						Ref<ViewLobbyResponse::ViewLobbyResult> result;
						result.instantiate();
						result->set_error(message);
						view_response->emit_signal("finished", result);
					}
				} break;
				default: {
					emit_signal("append_log", "error", p_dict["message"]);
				} break;
			}
		}
	} else {
		emit_signal("append_log", "error", "Unknown command received.");
	}
}
