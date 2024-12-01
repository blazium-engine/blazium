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
	int max_players = 0;
	int players = 0;
	bool sealed = false;
	bool password_protected;

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

		ADD_PROPERTY(PropertyInfo(Variant::STRING, "id"), "", "get_id");
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

	void set_dict(const Dictionary &p_dict) {
		this->set_host(p_dict.get("host", ""));
		this->set_max_players(p_dict.get("max_players", 0));
		this->set_sealed(p_dict.get("sealed", false));
		this->set_players(p_dict.get("players", 0));
		this->set_id(p_dict.get("id", ""));
		this->set_lobby_name(p_dict.get("name", ""));
		this->set_host_name(p_dict.get("host_name", ""));
		this->set_password_protected(p_dict.get("has_password", false));
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
		return dict;
	}

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

protected:
	static void _bind_methods() {
		ClassDB::bind_method(D_METHOD("get_id"), &LobbyPeer::get_id);
		ClassDB::bind_method(D_METHOD("get_peer_name"), &LobbyPeer::get_peer_name);
		ClassDB::bind_method(D_METHOD("is_ready"), &LobbyPeer::is_ready);
		ADD_PROPERTY(PropertyInfo(Variant::STRING, "id"), "", "get_id");
		ADD_PROPERTY(PropertyInfo(Variant::STRING, "peer_name"), "", "get_peer_name");
		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "ready"), "", "is_ready");
	}

public:
	void set_id(const String &p_id) { this->id = p_id; }
	void set_peer_name(const String &p_peer_name) { this->peer_name = p_peer_name; }
	void set_ready(bool p_ready) { this->ready = p_ready; }
	void set_dict(const Dictionary &p_dict) {
		this->set_id(p_dict.get("id", ""));
		this->set_peer_name(p_dict.get("name", ""));
		this->set_ready(p_dict.get("ready", ""));
	}
	Dictionary get_dict() const {
		Dictionary dict;
		dict["id"] = this->get_id();
		dict["name"] = this->get_peer_name();
		dict["ready"] = this->is_ready();
		return dict;
	}

	String get_id() const { return id; }
	String get_peer_name() const { return peer_name; }
	bool is_ready() const { return ready; }
	LobbyPeer() {}
};

class LobbyClient : public BlaziumClient {
	GDCLASS(LobbyClient, BlaziumClient);
	enum CommandType {
		LOBBY_REQUEST = 0,
		LOBBY_VIEW,
		LOBBY_LIST
	};
	String server_url = "wss://lobby.blazium.app/connect";
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
			LobbyResult(const LobbyResult &p_other) :
					error(p_other.error) {}
			LobbyResult() {}
		};
		LobbyResponse(const LobbyResponse &p_other) {}
		LobbyResponse() {}
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
			ListLobbyResult(const ListLobbyResult &p_other) :
					error(p_other.error), lobbies(p_other.lobbies) {}
			ListLobbyResult() {}
		};
		ListLobbyResponse(const ListLobbyResponse &p_other) {}
		ListLobbyResponse() {}
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
			TypedArray<LobbyPeer> peers;
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
			void set_peers(const TypedArray<LobbyPeer> &p_peers) { this->peers = p_peers; }
			void set_lobby(const Ref<LobbyInfo> &p_lobby_info) { this->lobby_info = p_lobby_info; }

			bool has_error() const { return !error.is_empty(); }
			String get_error() const { return error; }
			TypedArray<LobbyPeer> get_peers() const { return peers; }
			Ref<LobbyInfo> get_lobby() const { return lobby_info; }
			ViewLobbyResult() {
				lobby_info.instantiate();
			}
			~ViewLobbyResult() {
			}
		};
		ViewLobbyResponse(const ViewLobbyResponse &p_other) {}
		ViewLobbyResponse() {}
	};

private:
	Ref<WebSocketPeer> _socket;
	int _counter = 0;
	bool connected = false;
	Dictionary _commands;

	void _receive_data(const Dictionary &p_data);
	void _send_data(const Dictionary &p_data);
	String _increment_counter();

protected:
	void _notification(int p_notification);
	static void _bind_methods();

public:
	void set_server_url(const String &p_server_url) { this->server_url = p_server_url; }
	String get_server_url() { return server_url; }
	bool is_host() { return lobby->get_host() == peer->get_id(); }
	bool get_connected() { return connected; }
	void set_lobby(const Ref<LobbyInfo> &p_lobby) { this->lobby = p_lobby; }
	Ref<LobbyInfo> get_lobby() { return lobby; }
	void set_peer(const Ref<LobbyPeer> &p_peer) { this->peer = p_peer; }
	Ref<LobbyPeer> get_peer() { return peer; }
	TypedArray<LobbyPeer> get_peers() { return peers; }

	bool connect_to_lobby(const String &p_game_id);
	Ref<ViewLobbyResponse> create_lobby(const String &p_lobby_name, int p_max_players, const String &p_password);
	Ref<ViewLobbyResponse> join_lobby(const String &p_lobby_id, const String &p_password);
	Ref<LobbyResponse> leave_lobby();
	Ref<ListLobbyResponse> list_lobby(int p_start, int p_count);
	Ref<ViewLobbyResponse> view_lobby(const String &p_lobby_id, const String &p_password);
	Ref<LobbyResponse> kick_peer(const String &p_peer_id);
	Ref<LobbyResponse> lobby_ready(bool p_ready);
	Ref<LobbyResponse> set_peer_name(const String &p_peer_name);
	Ref<LobbyResponse> seal_lobby(bool seal);
	Ref<LobbyResponse> lobby_data(const Variant &p_peer_data);
	Ref<LobbyResponse> lobby_data_to(const Variant &p_peer_data, const String &p_target_peer);

	LobbyClient();
	~LobbyClient();
};

#endif // LOBBY_CLIENT_H
