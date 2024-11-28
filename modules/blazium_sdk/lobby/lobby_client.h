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

class LobbyClient : public BlaziumClient {
	GDCLASS(LobbyClient, BlaziumClient);
	enum CommandType {
		CREATE_LOBBY = 0,
		SIMPLE_REQUEST,
		LOBBY_VIEW,
		LOBBY_LIST
	};
	String server_url = "wss://lobby.blazium.app/connect";
	String lobby_id;

public:
	class CreateLobbyResponse : public RefCounted {
		GDCLASS(CreateLobbyResponse, RefCounted);

	protected:
		static void _bind_methods() {
			ADD_SIGNAL(MethodInfo("finished", PropertyInfo(Variant::OBJECT, "result", PROPERTY_HINT_RESOURCE_TYPE, "CreateLobbyResult")));
		}

	public:
		class CreateLobbyResult : public RefCounted {
			GDCLASS(CreateLobbyResult, RefCounted);
			String error;
			String lobby_id;
			String lobby_name;

		protected:
			static void _bind_methods() {
				ClassDB::bind_method(D_METHOD("has_error"), &CreateLobbyResult::has_error);
				ClassDB::bind_method(D_METHOD("get_error"), &CreateLobbyResult::get_error);
				ClassDB::bind_method(D_METHOD("get_lobby_id"), &CreateLobbyResult::get_lobby_id);
				ClassDB::bind_method(D_METHOD("get_lobby_name"), &CreateLobbyResult::get_lobby_name);
				ADD_PROPERTY(PropertyInfo(Variant::STRING, "error"), "", "get_error");
				ADD_PROPERTY(PropertyInfo(Variant::STRING, "lobby_id"), "", "get_lobby_id");
				ADD_PROPERTY(PropertyInfo(Variant::STRING, "lobby_name"), "", "get_lobby_name");
			}

		public:
			void set_error(const String &p_error) { this->error = p_error; }
			void set_lobby_id(const String &p_lobby_id) { this->lobby_id = p_lobby_id; }
			void set_lobby_name(const String &p_lobby_name) { this->lobby_name = p_lobby_name; }

			bool has_error() const { return !error.is_empty(); }
			String get_error() const { return error; }
			String get_lobby_id() const { return lobby_id; }
			String get_lobby_name() const { return lobby_name; }
			CreateLobbyResult(const CreateLobbyResult &p_other) :
					error(p_other.error), lobby_id(p_other.lobby_id) {}
			CreateLobbyResult() {}
		};
		CreateLobbyResponse(const CreateLobbyResponse &other) {}
		CreateLobbyResponse() {}
	};

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

	class LobbyInfo : public RefCounted {
		GDCLASS(LobbyInfo, RefCounted);
		String id;
		String name;
		String host;
		String host_name;
		int max_players = 0;
		int players = 0;
		bool sealed = false;

	protected:
		static void _bind_methods() {
			ClassDB::bind_method(D_METHOD("get_host"), &LobbyInfo::get_host);
			ClassDB::bind_method(D_METHOD("get_max_players"), &LobbyInfo::get_max_players);
			ClassDB::bind_method(D_METHOD("is_sealed"), &LobbyInfo::is_sealed);
			ClassDB::bind_method(D_METHOD("get_id"), &LobbyInfo::get_id);
			ClassDB::bind_method(D_METHOD("get_name"), &LobbyInfo::get_name);
			ClassDB::bind_method(D_METHOD("get_host_name"), &LobbyInfo::get_host_name);
			ClassDB::bind_method(D_METHOD("get_players"), &LobbyInfo::get_players);

			ADD_PROPERTY(PropertyInfo(Variant::STRING, "id"), "", "get_id");
			ADD_PROPERTY(PropertyInfo(Variant::STRING, "name"), "", "get_name");
			ADD_PROPERTY(PropertyInfo(Variant::STRING, "host_name"), "", "get_host_name");
			ADD_PROPERTY(PropertyInfo(Variant::INT, "players"), "", "get_players");
			ADD_PROPERTY(PropertyInfo(Variant::STRING, "host"), "", "get_host");
			ADD_PROPERTY(PropertyInfo(Variant::INT, "max_players"), "", "get_max_players");
			ADD_PROPERTY(PropertyInfo(Variant::BOOL, "sealed"), "", "is_sealed");
		}

	public:
		void set_id(const String &p_id) { this->id = p_id; }
		void set_name(const String &p_name) { this->name = p_name; }
		void set_host(const String &p_host) { this->host = p_host; }
		void set_host_name(const String &p_host_name) { this->host_name = p_host_name; }
		void set_max_players(int p_max_players) { this->max_players = p_max_players; }
		void set_players(int p_players) { this->players = p_players; }
		void set_sealed(bool p_sealed) { this->sealed = p_sealed; }

		String get_id() const { return id; }
		String get_name() const { return name; }
		String get_host() const { return host; }
		String get_host_name() const { return host_name; }
		int get_max_players() const { return max_players; }
		int get_players() const { return players; }
		bool is_sealed() const { return sealed; }
		LobbyInfo(const LobbyInfo &p_other) :
				host(p_other.host), max_players(p_other.max_players), sealed(p_other.sealed) {}
		LobbyInfo() {}
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
				ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "lobbies"), "", "get_lobbies");
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

	class LobbyPeer : public RefCounted {
		GDCLASS(LobbyPeer, RefCounted);
		String id;
		String name;
		bool ready = false;

	protected:
		static void _bind_methods() {
			ClassDB::bind_method(D_METHOD("get_id"), &LobbyPeer::get_id);
			ClassDB::bind_method(D_METHOD("get_name"), &LobbyPeer::get_name);
			ClassDB::bind_method(D_METHOD("is_ready"), &LobbyPeer::is_ready);
			ADD_PROPERTY(PropertyInfo(Variant::STRING, "id"), "", "get_id");
			ADD_PROPERTY(PropertyInfo(Variant::STRING, "name"), "", "get_name");
			ADD_PROPERTY(PropertyInfo(Variant::BOOL, "ready"), "", "is_ready");
		}

	public:
		void set_id(const String &p_id) { this->id = p_id; }
		void set_name(const String &p_name) { this->name = p_name; }
		void set_ready(bool p_ready) { this->ready = p_ready; }

		String get_id() const { return id; }
		String get_name() const { return name; }
		bool is_ready() const { return ready; }
		LobbyPeer(const LobbyPeer &p_other) :
				id(p_other.id), name(p_other.name), ready(p_other.ready) {}
		LobbyPeer() {}
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
				ClassDB::bind_method(D_METHOD("get_lobby_info"), &ViewLobbyResult::get_lobby_info);
				ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "peers"), "", "get_peers");
				ADD_PROPERTY(PropertyInfo(Variant::STRING, "error"), "", "get_error");
			}

		public:
			void set_error(const String &p_error) { this->error = p_error; }
			void set_peers(const TypedArray<LobbyPeer> &p_peers) { this->peers = p_peers; }
			void set_lobby_info(const Ref<LobbyInfo> &p_lobby_info) { this->lobby_info = p_lobby_info; }

			bool has_error() const { return !error.is_empty(); }
			String get_error() const { return error; }
			TypedArray<LobbyPeer> get_peers() const { return peers; }
			Ref<LobbyInfo> get_lobby_info() const { return lobby_info; }
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

	String _get_data_from_dict(const Dictionary &p_dict, const String &p_key);
	void _receive_data(const Dictionary &p_data);
	void _send_data(const Dictionary &p_data);
	void _wait_ready();
	String _increment_counter();

protected:
	void _notification(int p_notification);
	static void _bind_methods();

public:
	bool connect_to_lobby(const String &p_game_id);
	void set_server_url(const String &p_server_url) { this->server_url = p_server_url; }
	String get_server_url() { return server_url; }
	bool get_connected() { return connected; }
	String get_lobby_id() { return lobby_id; }
	Ref<CreateLobbyResponse> create_lobby(const String &p_lobby_name, int p_max_players, const String &p_password);
	Ref<LobbyResponse> join_lobby(const String &p_lobby_id, const String &p_password);
	Ref<LobbyResponse> leave_lobby();
	Ref<ListLobbyResponse> list_lobby(int p_start, int p_count);
	Ref<ViewLobbyResponse> view_lobby(const String &p_lobby_id, const String &p_password);
	Ref<LobbyResponse> kick_peer(const String &p_peer_id);
	Ref<LobbyResponse> lobby_ready();
	Ref<LobbyResponse> lobby_unready();
	Ref<LobbyResponse> set_peer_name(const String &p_peer_name);
	Ref<LobbyResponse> seal_lobby();
	Ref<LobbyResponse> unseal_lobby();
	Ref<LobbyResponse> lobby_data(const String &p_peer_data);
	Ref<LobbyResponse> lobby_data_to(const String &p_peer_data, const String &p_target_peer);

	LobbyClient();
	~LobbyClient();
};

#endif // LOBBY_CLIENT_H
