/**************************************************************************/
/*  lobby_client.h                                                        */
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

#ifndef LOBBY_CLIENT_H
#define LOBBY_CLIENT_H

#include "../blazium_client.h"
#include "core/io/json.h"
#include "modules/websocket/websocket_peer.h"

class LobbyInfo : public Resource {
	GDCLASS(LobbyInfo, Resource);
	String id;
	String lobby_name;
	String host;
	String host_name;
	Dictionary tags;
	Dictionary data;
	int max_players = 0;
	int players = 0;
	bool sealed = false;
	bool password_protected = false;

protected:
	static void _bind_methods() {
		ClassDB::bind_method(D_METHOD("get_host"), &LobbyInfo::get_host);
		ClassDB::bind_method(D_METHOD("get_max_players"), &LobbyInfo::get_max_players);
		ClassDB::bind_method(D_METHOD("is_sealed"), &LobbyInfo::is_sealed);
		ClassDB::bind_method(D_METHOD("is_password_protected"), &LobbyInfo::is_password_protected);
		ClassDB::bind_method(D_METHOD("get_id"), &LobbyInfo::get_id);
		ClassDB::bind_method(D_METHOD("get_lobby_name"), &LobbyInfo::get_lobby_name);
		ClassDB::bind_method(D_METHOD("get_host_name"), &LobbyInfo::get_host_name);
		ClassDB::bind_method(D_METHOD("get_players"), &LobbyInfo::get_players);
		ClassDB::bind_method(D_METHOD("get_tags"), &LobbyInfo::get_tags);
		ClassDB::bind_method(D_METHOD("get_data"), &LobbyInfo::get_data);

		ADD_PROPERTY(PropertyInfo(Variant::STRING, "id"), "", "get_id");
		ADD_PROPERTY(PropertyInfo(Variant::STRING, "tags"), "", "get_tags");
		ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "data"), "", "get_data");
		ADD_PROPERTY(PropertyInfo(Variant::STRING, "lobby_name"), "", "get_lobby_name");
		ADD_PROPERTY(PropertyInfo(Variant::STRING, "host_name"), "", "get_host_name");
		ADD_PROPERTY(PropertyInfo(Variant::INT, "players"), "", "get_players");
		ADD_PROPERTY(PropertyInfo(Variant::STRING, "host"), "", "get_host");
		ADD_PROPERTY(PropertyInfo(Variant::INT, "max_players"), "", "get_max_players");
		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "sealed"), "", "is_sealed");
		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "password_protected"), "", "is_password_protected");
	}

public:
	void set_id(const String &p_id) { this->id = p_id; }
	void set_lobby_name(const String &p_lobby_name) { this->lobby_name = p_lobby_name; }
	void set_host(const String &p_host) { this->host = p_host; }
	void set_host_name(const String &p_host_name) { this->host_name = p_host_name; }
	void set_max_players(int p_max_players) { this->max_players = p_max_players; }
	void set_players(int p_players) { this->players = p_players; }
	void set_sealed(bool p_sealed) { this->sealed = p_sealed; }
	void set_password_protected(bool p_password_protected) { this->password_protected = p_password_protected; }
	void set_tags(const Dictionary &p_tags) { this->tags = p_tags; }
	void set_data(const Dictionary &p_data) { this->data = p_data; }

	void set_dict(const Dictionary &p_dict) {
		this->set_host(p_dict.get("host", ""));
		this->set_max_players(p_dict.get("max_players", 0));
		this->set_sealed(p_dict.get("sealed", false));
		this->set_players(p_dict.get("players", 0));
		this->set_id(p_dict.get("id", ""));
		this->set_lobby_name(p_dict.get("name", ""));
		this->set_host_name(p_dict.get("host_name", ""));
		this->set_password_protected(p_dict.get("has_password", false));
		this->set_tags(p_dict.get("tags", Dictionary()));
		this->set_data(p_dict.get("public_data", Dictionary()));
	}
	Dictionary get_dict() const {
		Dictionary dict;
		dict["host"] = this->get_host();
		dict["max_players"] = this->get_max_players();
		dict["sealed"] = this->is_sealed();
		dict["players"] = this->get_players();
		dict["id"] = this->get_id();
		dict["name"] = this->get_lobby_name();
		dict["host_name"] = this->get_host_name();
		dict["has_password"] = this->is_password_protected();
		dict["tags"] = this->get_tags();
		dict["public_data"] = this->get_data();
		return dict;
	}

	Dictionary get_data() const { return data; }
	Dictionary get_tags() const { return tags; }
	String get_id() const { return id; }
	String get_lobby_name() const { return lobby_name; }
	String get_host() const { return host; }
	String get_host_name() const { return host_name; }
	int get_max_players() const { return max_players; }
	int get_players() const { return players; }
	bool is_sealed() const { return sealed; }
	bool is_password_protected() const { return password_protected; }
	LobbyInfo() {}
};

class LobbyPeer : public Resource {
	GDCLASS(LobbyPeer, Resource);
	String id;
	String peer_name;
	bool ready = false;
	Dictionary data;

protected:
	static void _bind_methods() {
		ClassDB::bind_method(D_METHOD("get_id"), &LobbyPeer::get_id);
		ClassDB::bind_method(D_METHOD("get_peer_name"), &LobbyPeer::get_peer_name);
		ClassDB::bind_method(D_METHOD("is_ready"), &LobbyPeer::is_ready);
		ClassDB::bind_method(D_METHOD("get_data"), &LobbyPeer::get_data);
		ADD_PROPERTY(PropertyInfo(Variant::STRING, "id"), "", "get_id");
		ADD_PROPERTY(PropertyInfo(Variant::STRING, "peer_name"), "", "get_peer_name");
		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "ready"), "", "is_ready");
		ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "data"), "", "get_data");
	}

public:
	void set_id(const String &p_id) { this->id = p_id; }
	void set_peer_name(const String &p_peer_name) { this->peer_name = p_peer_name; }
	void set_ready(bool p_ready) { this->ready = p_ready; }
	void set_data(const Dictionary &p_data) { this->data = p_data; }
	void set_dict(const Dictionary &p_dict) {
		this->set_id(p_dict.get("id", ""));
		this->set_peer_name(p_dict.get("name", ""));
		this->set_ready(p_dict.get("ready", ""));
		this->set_data(p_dict.get("public_data", Dictionary()));
	}
	Dictionary get_dict() const {
		Dictionary dict;
		dict["id"] = this->get_id();
		dict["name"] = this->get_peer_name();
		dict["ready"] = this->is_ready();
		dict["public_data"] = this->get_data();
		return dict;
	}

	Dictionary get_data() const { return data; }
	String get_id() const { return id; }
	String get_peer_name() const { return peer_name; }
	bool is_ready() const { return ready; }
	LobbyPeer() {}
};

class LobbyClient : public BlaziumClient {
	GDCLASS(LobbyClient, BlaziumClient);

protected:
	String server_url;
	String reconnection_token;
	String game_id = "";
	Dictionary host_data;
	Dictionary peer_data;
	Ref<LobbyInfo> lobby;
	Ref<LobbyPeer> peer;
	TypedArray<LobbyPeer> peers;

public:
	class LobbyResponse : public RefCounted {
		GDCLASS(LobbyResponse, RefCounted);

	protected:
		static void _bind_methods() {
			ADD_SIGNAL(MethodInfo("finished", PropertyInfo(Variant::OBJECT, "result", PROPERTY_HINT_RESOURCE_TYPE, "LobbyResult")));
		}

	public:
		class LobbyResult : public RefCounted {
			GDCLASS(LobbyResult, RefCounted);

			String error;

		protected:
			static void _bind_methods() {
				ClassDB::bind_method(D_METHOD("has_error"), &LobbyResult::has_error);
				ClassDB::bind_method(D_METHOD("get_error"), &LobbyResult::get_error);
				ADD_PROPERTY(PropertyInfo(Variant::STRING, "error"), "", "get_error");
			}

		public:
			void set_error(String p_error) { this->error = p_error; }

			bool has_error() const { return !error.is_empty(); }
			String get_error() const { return error; }
		};
	};

	class ListLobbyResponse : public RefCounted {
		GDCLASS(ListLobbyResponse, RefCounted);

	protected:
		static void _bind_methods() {
			ADD_SIGNAL(MethodInfo("finished", PropertyInfo(Variant::OBJECT, "result", PROPERTY_HINT_RESOURCE_TYPE, "ListLobbyResult")));
		}

	public:
		class ListLobbyResult : public RefCounted {
			GDCLASS(ListLobbyResult, RefCounted);

			String error;
			TypedArray<LobbyInfo> lobbies;

		protected:
			static void _bind_methods() {
				ClassDB::bind_method(D_METHOD("has_error"), &ListLobbyResult::has_error);
				ClassDB::bind_method(D_METHOD("get_error"), &ListLobbyResult::get_error);
				ClassDB::bind_method(D_METHOD("get_lobbies"), &ListLobbyResult::get_lobbies);
				ADD_PROPERTY(PropertyInfo(Variant::STRING, "error"), "", "get_error");
				ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "lobbies", PROPERTY_HINT_ARRAY_TYPE, "LobbyInfo"), "", "get_lobbies");
				ADD_PROPERTY_DEFAULT("lobbies", TypedArray<LobbyInfo>());
			}

		public:
			void set_error(const String &p_error) { this->error = p_error; }
			void set_lobbies(const TypedArray<LobbyInfo> &p_lobbies) { this->lobbies = p_lobbies; }

			bool has_error() const { return !error.is_empty(); }
			String get_error() const { return error; }
			TypedArray<LobbyInfo> get_lobbies() const { return lobbies; }
		};
	};

	class ViewLobbyResponse : public RefCounted {
		GDCLASS(ViewLobbyResponse, RefCounted);

	protected:
		static void _bind_methods() {
			ADD_SIGNAL(MethodInfo("finished", PropertyInfo(Variant::OBJECT, "result", PROPERTY_HINT_RESOURCE_TYPE, "ViewLobbyResult")));
		}

	public:
		class ViewLobbyResult : public RefCounted {
			GDCLASS(ViewLobbyResult, RefCounted);
			String error;
			TypedArray<LobbyPeer> peers_info;
			Ref<LobbyInfo> lobby_info;

		protected:
			static void _bind_methods() {
				ClassDB::bind_method(D_METHOD("has_error"), &ViewLobbyResult::has_error);
				ClassDB::bind_method(D_METHOD("get_error"), &ViewLobbyResult::get_error);
				ClassDB::bind_method(D_METHOD("get_peers"), &ViewLobbyResult::get_peers);
				ClassDB::bind_method(D_METHOD("get_lobby"), &ViewLobbyResult::get_lobby);
				ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "peers", PROPERTY_HINT_ARRAY_TYPE, "LobbyPeer"), "", "get_peers");
				ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "lobby", PROPERTY_HINT_RESOURCE_TYPE, "LobbyInfo"), "", "get_lobby");
				ADD_PROPERTY_DEFAULT("lobby", Ref<LobbyInfo>());
				ADD_PROPERTY_DEFAULT("peers", TypedArray<LobbyPeer>());
				ADD_PROPERTY(PropertyInfo(Variant::STRING, "error"), "", "get_error");
			}

		public:
			void set_error(const String &p_error) { this->error = p_error; }
			void set_peers(const TypedArray<LobbyPeer> &p_peers) { this->peers_info = p_peers; }
			void set_lobby(const Ref<LobbyInfo> &p_lobby_info) { this->lobby_info = p_lobby_info; }

			bool has_error() const { return !error.is_empty(); }
			String get_error() const { return error; }
			TypedArray<LobbyPeer> get_peers() const { return peers_info; }
			Ref<LobbyInfo> get_lobby() const { return lobby_info; }
			ViewLobbyResult() {
				lobby_info.instantiate();
			}
			~ViewLobbyResult() {
			}
		};
	};

protected:
	Ref<WebSocketPeer> _socket;
	int _counter = 0;
	bool connected = false;
	Dictionary _commands;

	void _clear_lobby();
	void _receive_data(const Dictionary &p_data);
	void _send_data(const Dictionary &p_data);
	void _update_peers(Dictionary p_data_dict, TypedArray<LobbyPeer> &peers);
	String _increment_counter();

	enum CommandType {
		LOBBY_REQUEST = 0,
		LOBBY_VIEW,
		LOBBY_LIST,
		LOBBY_CALL
	};

	void _notification(int p_notification);
	static void _bind_methods();

public:
	void set_server_url(const String &p_server_url) { this->server_url = p_server_url; }
	String get_server_url() { return server_url; }
	void set_reconnection_token(const String &p_reconnection_token) { this->reconnection_token = p_reconnection_token; }
	String get_reconnection_token() { return reconnection_token; }
	void set_game_id(const String &p_game_id) { this->game_id = p_game_id; }
	String get_game_id() { return game_id; }
	bool is_host() { return lobby->get_host() == peer->get_id(); }
	bool get_connected() { return connected; }
	Dictionary get_host_data() { return host_data; }
	Dictionary get_peer_data() { return peer_data; }
	void set_lobby(const Ref<LobbyInfo> &p_lobby) { this->lobby = p_lobby; }
	Ref<LobbyInfo> get_lobby() { return lobby; }
	void set_peer(const Ref<LobbyPeer> &p_peer) { this->peer = p_peer; }
	Ref<LobbyPeer> get_peer() { return peer; }
	TypedArray<LobbyPeer> get_peers() { return peers; }

	bool connect_to_lobby();
	void disconnect_from_lobby();
	Ref<ViewLobbyResponse> create_lobby(const String &p_lobby_name, const Dictionary &p_tags, int p_max_players, const String &p_password);
	Ref<ViewLobbyResponse> join_lobby(const String &p_lobby_id, const String &p_password);
	Ref<LobbyResponse> leave_lobby();
	Ref<ListLobbyResponse> list_lobby(const Dictionary &p_tags, int p_start, int p_count);
	Ref<ViewLobbyResponse> view_lobby(const String &p_lobby_id, const String &p_password);
	Ref<LobbyResponse> kick_peer(const String &p_peer_id);
	Ref<LobbyResponse> set_lobby_tags(const Dictionary &p_tags);
	Ref<LobbyResponse> del_lobby_tags(const TypedArray<String> &p_keys);
	Ref<LobbyResponse> lobby_chat(const String &chat_message);
	Ref<LobbyResponse> lobby_ready(bool p_ready);
	Ref<LobbyResponse> set_peer_name(const String &p_peer_name);
	Ref<LobbyResponse> seal_lobby(bool seal);
	Ref<LobbyResponse> lobby_call(const String &p_method, const Array &p_args);
	Ref<LobbyResponse> lobby_notify(const Variant &p_peer_data);
	Ref<LobbyResponse> peer_notify(const Variant &p_peer_data, const String &p_target_peer);

	Ref<LobbyResponse> lobby_data(const Dictionary &p_lobby_data, bool p_is_private);
	Ref<LobbyResponse> del_lobby_data(const TypedArray<String> &p_keys, bool p_is_private);

	Ref<LobbyResponse> set_peer_data(const Dictionary &p_peer_data, const String &p_target_peer, bool p_is_private);
	Ref<LobbyResponse> del_peer_data(const TypedArray<String> &p_keys, const String &p_target_peer, bool p_is_private);

	Ref<LobbyResponse> set_peers_data(const Dictionary &p_peer_data, bool p_is_private);
	Ref<LobbyResponse> del_peers_data(const TypedArray<String> &p_keys, bool p_is_private);

	LobbyClient();
	~LobbyClient();
};

#endif // LOBBY_CLIENT_H
