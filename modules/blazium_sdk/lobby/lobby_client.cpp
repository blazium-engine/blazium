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
	lobby.instantiate();
	peer.instantiate();
	_socket = Ref<WebSocketPeer>(WebSocketPeer::create());
	set_process_internal(false);
}

LobbyClient::~LobbyClient() {
	_socket->close();
	set_process_internal(false);
}

void LobbyClient::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_server_url", "server_url"), &LobbyClient::set_server_url);
	ClassDB::bind_method(D_METHOD("get_server_url"), &LobbyClient::get_server_url);
	ClassDB::bind_method(D_METHOD("is_host"), &LobbyClient::is_host);
	ClassDB::bind_method(D_METHOD("get_connected"), &LobbyClient::get_connected);
	ClassDB::bind_method(D_METHOD("get_lobby"), &LobbyClient::get_lobby);
	ClassDB::bind_method(D_METHOD("get_peer"), &LobbyClient::get_peer);
	ClassDB::bind_method(D_METHOD("get_peers"), &LobbyClient::get_peers);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "server_url", PROPERTY_HINT_NONE, ""), "set_server_url", "get_server_url");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "connected"), "", "get_connected");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "lobby", PROPERTY_HINT_RESOURCE_TYPE, "LobbyInfo"), "", "get_lobby");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "peer", PROPERTY_HINT_RESOURCE_TYPE, "LobbyPeer"), "", "get_peer");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "peers", PROPERTY_HINT_ARRAY_TYPE, "LobbyPeer"), "", "get_peers");
	ADD_PROPERTY_DEFAULT("peers", TypedArray<LobbyPeer>());
	ADD_PROPERTY_DEFAULT("peer", Ref<LobbyPeer>());
	ADD_PROPERTY_DEFAULT("lobby", Ref<LobbyInfo>());
	// Register methods
	ClassDB::bind_method(D_METHOD("connect_to_lobby", "game_id"), &LobbyClient::connect_to_lobby);
	ClassDB::bind_method(D_METHOD("set_peer_name", "peer_name"), &LobbyClient::set_peer_name);
	ClassDB::bind_method(D_METHOD("create_lobby", "title", "max_players", "password"), &LobbyClient::create_lobby, DEFVAL(4), DEFVAL(""));
	ClassDB::bind_method(D_METHOD("join_lobby", "lobby_id", "password"), &LobbyClient::join_lobby, DEFVAL(""));
	ClassDB::bind_method(D_METHOD("leave_lobby"), &LobbyClient::leave_lobby);
	ClassDB::bind_method(D_METHOD("list_lobby", "start", "count"), &LobbyClient::list_lobby, DEFVAL(0), DEFVAL(10));
	ClassDB::bind_method(D_METHOD("view_lobby", "lobby_id", "password"), &LobbyClient::view_lobby, DEFVAL(""), DEFVAL(""));
	ClassDB::bind_method(D_METHOD("kick_peer", "peer_id"), &LobbyClient::kick_peer);
	ClassDB::bind_method(D_METHOD("lobby_ready", "ready"), &LobbyClient::lobby_ready);
	ClassDB::bind_method(D_METHOD("seal_lobby", "seal"), &LobbyClient::seal_lobby);
	ClassDB::bind_method(D_METHOD("lobby_data", "data"), &LobbyClient::lobby_data);
	ClassDB::bind_method(D_METHOD("lobby_data_to", "data", "target_peer"), &LobbyClient::lobby_data_to);

	// Register signals
	ADD_SIGNAL(MethodInfo("disconnected_from_lobby"));
	ADD_SIGNAL(MethodInfo("peer_named", PropertyInfo(Variant::OBJECT, "peer", PROPERTY_HINT_RESOURCE_TYPE, "LobbyPeer")));
	ADD_SIGNAL(MethodInfo("received_data", PropertyInfo(Variant::OBJECT, "data"), PropertyInfo(Variant::OBJECT, "from_peer", PROPERTY_HINT_RESOURCE_TYPE, "LobbyPeer")));
	ADD_SIGNAL(MethodInfo("received_data_to", PropertyInfo(Variant::OBJECT, "data"), PropertyInfo(Variant::OBJECT, "from_peer", PROPERTY_HINT_RESOURCE_TYPE, "LobbyPeer")));
	ADD_SIGNAL(MethodInfo("lobby_created", PropertyInfo(Variant::OBJECT, "lobby", PROPERTY_HINT_RESOURCE_TYPE, "LobbyInfo"), PropertyInfo(Variant::ARRAY, "peers", PROPERTY_HINT_ARRAY_TYPE, "LobbyPeer")));
	ADD_SIGNAL(MethodInfo("lobby_joined", PropertyInfo(Variant::OBJECT, "lobby", PROPERTY_HINT_RESOURCE_TYPE, "LobbyInfo"), PropertyInfo(Variant::ARRAY, "peers", PROPERTY_HINT_ARRAY_TYPE, "LobbyPeer")));
	ADD_SIGNAL(MethodInfo("lobby_left"));
	ADD_SIGNAL(MethodInfo("lobby_sealed", PropertyInfo(Variant::BOOL, "sealed")));
	ADD_SIGNAL(MethodInfo("peer_joined", PropertyInfo(Variant::OBJECT, "peer", PROPERTY_HINT_RESOURCE_TYPE, "LobbyPeer")));
	ADD_SIGNAL(MethodInfo("peer_left", PropertyInfo(Variant::OBJECT, "peer", PROPERTY_HINT_RESOURCE_TYPE, "LobbyPeer"), PropertyInfo(Variant::BOOL, "kicked")));
	ADD_SIGNAL(MethodInfo("peer_ready", PropertyInfo(Variant::OBJECT, "peer", PROPERTY_HINT_RESOURCE_TYPE, "LobbyPeer"), PropertyInfo(Variant::BOOL, "ready")));
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

Ref<LobbyClient::ViewLobbyResponse> LobbyClient::create_lobby(const String &p_lobby_name, int p_max_players, const String &p_password) {
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
	Ref<ViewLobbyResponse> response;
	response.instantiate();
	command_array.push_back(LOBBY_VIEW);
	command_array.push_back(response);
	_commands[id] = command_array;
	_send_data(command);
	return response;
}

Ref<LobbyClient::ViewLobbyResponse> LobbyClient::join_lobby(const String &p_lobby_id, const String &p_password) {
	String id = _increment_counter();
	Dictionary command;
	command["command"] = "join_lobby";
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
	command_array.push_back(LOBBY_REQUEST);
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
	if (p_lobby_id.is_empty()) {
		data_dict["lobby_id"] = lobby->get_id();
	} else {
		data_dict["lobby_id"] = p_lobby_id;
	}
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
	command_array.push_back(LOBBY_REQUEST);
	command_array.push_back(response);
	_commands[id] = command_array;
	_send_data(command);
	return response;
}

Ref<LobbyClient::LobbyResponse> LobbyClient::lobby_ready(bool p_ready) {
	String id = _increment_counter();
	Dictionary command;
	if (p_ready) {
		command["command"] = "lobby_ready";
	} else {
		command["command"] = "lobby_unready";
	}
	Dictionary data_dict;
	command["data"] = data_dict;
	data_dict["id"] = id;
	Array command_array;
	Ref<LobbyResponse> response;
	response.instantiate();
	command_array.push_back(LOBBY_REQUEST);
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
	data_dict["name"] = p_peer_name;
	data_dict["id"] = id;
	command["data"] = data_dict;
	Array command_array;
	Ref<LobbyResponse> response;
	response.instantiate();
	command_array.push_back(LOBBY_REQUEST);
	command_array.push_back(response);
	_commands[id] = command_array;
	_send_data(command);
	return response;
}

Ref<LobbyClient::LobbyResponse> LobbyClient::seal_lobby(bool seal) {
	String id = _increment_counter();
	Dictionary command;
	if (seal) {
		command["command"] = "seal_lobby";
	} else {
		command["command"] = "unseal_lobby";
	}
	Dictionary data_dict;
	command["data"] = data_dict;
	data_dict["id"] = id;
	Array command_array;
	Ref<LobbyResponse> response;
	response.instantiate();
	command_array.push_back(LOBBY_REQUEST);
	command_array.push_back(response);
	_commands[id] = command_array;
	_send_data(command);
	return response;
}

Ref<LobbyClient::LobbyResponse> LobbyClient::lobby_data(const Variant &p_peer_data) {
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
	command_array.push_back(LOBBY_REQUEST);
	command_array.push_back(response);
	_commands[id] = command_array;
	_send_data(command);
	return response;
}

Ref<LobbyClient::LobbyResponse> LobbyClient::lobby_data_to(const Variant &p_peer_data, const String &p_target_peer) {
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
	command_array.push_back(LOBBY_REQUEST);
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
	Error err = _socket->send_text(JSON::stringify(p_data_dict));
	if (err != OK) {
		emit_signal("append_log", "error", "No longer connected.");
		_socket->close(1000, "Disconnected");
	}
}

void update_peers(Dictionary p_data_dict, TypedArray<LobbyPeer> &peers) {
	Array peers_array = p_data_dict.get("peers", Array());
	TypedArray<LobbyPeer> peers_info;
	peers.clear();
	for (int i = 0; i < peers_array.size(); ++i) {
		Ref<LobbyPeer> peer = Ref<LobbyPeer>(memnew(LobbyPeer));
		peer->set_dict(peers_array[i]);
		peers.push_back(peer);
	}
}

// TODO optimize this
void sort_peers_by_id(TypedArray<LobbyPeer> &peers) {
	for (int i = 0; i < peers.size(); ++i) {
		for (int j = i + 1; j < peers.size(); ++j) {
			Ref<LobbyPeer> peer_i = peers[i];
			Ref<LobbyPeer> peer_j = peers[j];
			if (peer_i->get_id().casecmp_to(peer_j->get_id()) > 0) {
				Ref<LobbyPeer> temp = peers[i];
				peers[i] = peers[j];
				peers[j] = temp;
			}
		}
	}
}

void LobbyClient::_receive_data(const Dictionary &p_dict) {
	String command = p_dict.get("command", "error");
	String message = p_dict.get("message", command);
	Dictionary data_dict = p_dict.get("data", Dictionary());
	String message_id = data_dict.get("id", "");
	Array command_array = _commands.get(message_id, Array());
	_commands.erase(message_id);
	emit_signal("append_log", command, message);
	if (command_array.size() == 2 && command != "error") {
		int command_type = command_array[0];
		switch (command_type) {
			case LOBBY_REQUEST: {
				Ref<LobbyResponse> response = command_array[1];
				if (response.is_valid()) {
					Ref<LobbyResponse::LobbyResult> result = Ref<LobbyResponse::LobbyResult>(memnew(LobbyResponse::LobbyResult));
					response->emit_signal("finished", result);
				}
			} break;
			case LOBBY_VIEW: {
				Dictionary lobby_dict = data_dict.get("lobby", Dictionary());

				// Iterate through peers and populate arrays
				TypedArray<LobbyPeer> peers_info;
				update_peers(data_dict, peers_info);
				sort_peers_by_id(peers_info);
				Ref<LobbyInfo> lobby_info = Ref<LobbyInfo>(memnew(LobbyInfo));
				lobby_info->set_dict(lobby_dict);
				if (lobby_info->get_id() == lobby->get_id()) {
					// Update lobby info because we viewed our own lobby
					lobby->set_dict(lobby_info->get_dict());
					peers = peers_info;
				}
				if (command_array.size() == 2) {
					Ref<ViewLobbyResponse> response = command_array[1];
					if (response.is_valid()) {
						Ref<ViewLobbyResponse::ViewLobbyResult> result;
						result.instantiate();
						result->set_peers(peers_info);
						result->set_lobby(lobby_info);
						response->emit_signal("finished", result);
					}
				}
			} break;
		}
	}
	if (command == "peer_state") {
		Dictionary peer_dict = data_dict.get("peer", Dictionary());
		peer->set_id(peer_dict.get("id", ""));
	} else if (command == "lobby_created") {
		lobby->set_dict(data_dict.get("lobby", Dictionary()));
		update_peers(data_dict, peers);
		sort_peers_by_id(peers);
		emit_signal("lobby_created", lobby, peers);
	} else if (command == "joined_lobby") {
		lobby->set_dict(data_dict.get("lobby", Dictionary()));
		update_peers(data_dict, peers);
		sort_peers_by_id(peers);
		emit_signal("lobby_joined", lobby, peers);
	} else if (command == "lobby_left") {
		lobby->set_dict(Dictionary());
		peers.clear();
		emit_signal("lobby_left");
	} else if (command == "lobby_sealed") {
		Dictionary lobby_dict = data_dict.get("lobby", Dictionary());
		lobby->set_sealed(true);
		emit_signal("lobby_sealed", true);
	} else if (command == "lobby_unsealed") {
		lobby->set_sealed(false);
		emit_signal("lobby_sealed", false);
	} else if (command == "lobby_list") {
		Array arr = data_dict.get("lobbies", Array());
		TypedArray<Dictionary> lobbies_input = arr;
		TypedArray<LobbyInfo> lobbies_output;
		for (int i = 0; i < lobbies_input.size(); ++i) {
			Dictionary lobby_dict = lobbies_input[i];
			Ref<LobbyInfo> lobby_info;
			lobby_info.instantiate();
			lobby_info->set_dict(lobby_dict);

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
		// nothing for now
	} else if (command == "peer_name") {
		String peer_id = data_dict.get("peer_id", "");
		String peer_name = data_dict.get("name", "");
		if (peer->get_id() == peer_id) {
			peer->set_peer_name(peer_name);
		}
		for (int i = 0; i < peers.size(); ++i) {
			Ref<LobbyPeer> updated_peer = peers[i];
			if (updated_peer->get_id() == peer_id) {
				updated_peer->set_peer_name(peer_name);
				// if the named peer is the host, update the host name
				if (updated_peer->get_id() == lobby->get_host()) {
					lobby->set_host_name(peer_name);
				}
				emit_signal("peer_named", updated_peer);
				break;
			}
		}
	} else if (command == "peer_ready") {
		String peer_id = data_dict.get("peer_id", "");
		if (peer->get_id() == peer_id) {
			peer->set_ready(true);
		}
		for (int i = 0; i < peers.size(); ++i) {
			Ref<LobbyPeer> updated_peer = peers[i];
			if (updated_peer->get_id() == String(peer_id)) {
				updated_peer->set_ready(true);
				emit_signal("peer_ready", updated_peer, true);
				break;
			}
		}
	} else if (command == "peer_unready") {
		String peer_id = data_dict.get("peer_id", "");
		if (peer->get_id() == peer_id) {
			peer->set_ready(false);
		}
		for (int i = 0; i < peers.size(); ++i) {
			Ref<LobbyPeer> updated_peer = peers[i];
			if (updated_peer->get_id() == String(data_dict.get("peer_id", ""))) {
				updated_peer->set_ready(false);
				emit_signal("peer_ready", updated_peer, false);
				break;
			}
		}
	} else if (command == "peer_joined") {
		Ref<LobbyPeer> joining_peer = Ref<LobbyPeer>(memnew(LobbyPeer));
		Dictionary peer_dict = data_dict.get("peer", Dictionary());
		joining_peer->set_id(peer_dict.get("id", ""));
		joining_peer->set_peer_name(peer_dict.get("name", ""));
		peers.append(joining_peer);
		sort_peers_by_id(peers);
		lobby->set_players(peers.size());
		emit_signal("peer_joined", joining_peer);
	} else if (command == "peer_left") {
		for (int i = 0; i < peers.size(); ++i) {
			Ref<LobbyPeer> leaving_peer = peers[i];
			if (leaving_peer->get_id() == String(data_dict.get("peer_id", ""))) {
				peers.remove_at(i);
				lobby->set_players(peers.size());
				emit_signal("peer_left", leaving_peer, data_dict.get("kicked", false));
				break;
			}
		}
		sort_peers_by_id(peers);
	} else if (command == "lobby_data") {
		String from_peer_id = data_dict.get("from_peer", "");
		for (int i = 0; i < peers.size(); ++i) {
			Ref<LobbyPeer> from_peer = peers[i];
			if (from_peer->get_id() == from_peer_id) {
				emit_signal("received_data", data_dict.get("peer_data", Variant()), from_peer);
				break;
			}
		}
	} else if (command == "data_to") {
		String from_peer_id = data_dict.get("from_peer", "");
		for (int i = 0; i < peers.size(); ++i) {
			Ref<LobbyPeer> from_peer = peers[i];
			if (from_peer->get_id() == from_peer_id) {
				emit_signal("received_data_to", data_dict.get("peer_data", Variant()), from_peer);
				break;
			}
		}
	} else if (command == "lobby_data_sent") {
		// nothing for now
	} else if (command == "data_to_sent") {
		// nothing for now
	} else if (command == "error") {
		if (command_array.size() == 2) {
			int command_type = command_array[0];
			switch (command_type) {
				case LOBBY_REQUEST: {
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
