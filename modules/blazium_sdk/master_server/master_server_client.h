/**************************************************************************/
/*  master_server_client.h                                                */
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

#ifndef MASTER_SERVER_CLIENT_H
#define MASTER_SERVER_CLIENT_H

#include "../blazium_client.h"
#include "core/io/json.h"
#include "core/templates/vector.h"
#include "core/version.h"
#include "main/performance.h"
#include "scene/main/http_request.h"

class GameServerInfo : public Resource {
	GDCLASS(GameServerInfo, Resource);
	String id = "";
	String game_name = "";
	String ip_address = "";
	int port = 0;
	String description = "";
	int max_players = 0;
	int players = 0;
	String version = "";

protected:
	static void _bind_methods() {
		ClassDB::bind_method(D_METHOD("get_id"), &GameServerInfo::get_id);
		ClassDB::bind_method(D_METHOD("get_game_name"), &GameServerInfo::get_game_name);
		ClassDB::bind_method(D_METHOD("get_ip_address"), &GameServerInfo::get_ip_address);
		ClassDB::bind_method(D_METHOD("get_port"), &GameServerInfo::get_port);
		ClassDB::bind_method(D_METHOD("get_description"), &GameServerInfo::get_description);
		ClassDB::bind_method(D_METHOD("get_max_players"), &GameServerInfo::get_max_players);
		ClassDB::bind_method(D_METHOD("get_players"), &GameServerInfo::get_players);
		ClassDB::bind_method(D_METHOD("get_version"), &GameServerInfo::get_version);

		ClassDB::bind_method(D_METHOD("set_id", "id"), &GameServerInfo::set_id);
		ClassDB::bind_method(D_METHOD("set_game_name", "game_name"), &GameServerInfo::set_game_name);
		ClassDB::bind_method(D_METHOD("set_ip_address", "ip_address"), &GameServerInfo::set_ip_address);
		ClassDB::bind_method(D_METHOD("set_port", "port"), &GameServerInfo::set_port);
		ClassDB::bind_method(D_METHOD("set_description", "description"), &GameServerInfo::set_description);
		ClassDB::bind_method(D_METHOD("set_max_players", "max_players"), &GameServerInfo::set_max_players);
		ClassDB::bind_method(D_METHOD("set_players", "players"), &GameServerInfo::set_players);
		ClassDB::bind_method(D_METHOD("set_version", "version"), &GameServerInfo::set_version);

		ADD_PROPERTY(PropertyInfo(Variant::STRING, "id"), "set_id", "get_id");
		ADD_PROPERTY(PropertyInfo(Variant::STRING, "game_name"), "set_game_name", "get_game_name");
		ADD_PROPERTY(PropertyInfo(Variant::STRING, "ip_address"), "set_ip_address", "get_ip_address");
		ADD_PROPERTY(PropertyInfo(Variant::STRING, "port"), "set_port", "get_port");
		ADD_PROPERTY(PropertyInfo(Variant::STRING, "description"), "set_description", "get_description");
		ADD_PROPERTY(PropertyInfo(Variant::INT, "max_players"), "set_max_players", "get_max_players");
		ADD_PROPERTY(PropertyInfo(Variant::INT, "players"), "set_players", "get_players");
		ADD_PROPERTY(PropertyInfo(Variant::STRING, "version"), "set_version", "get_version");
	}

public:
	String get_id() const { return id; }
	String get_game_name() const { return game_name; }
	String get_ip_address() const { return ip_address; }
	int get_port() const { return port; }
	String get_description() const { return description; }
	int get_max_players() const { return max_players; }
	int get_players() const { return players; }
	String get_version() const { return version; }

	void set_id(String p_id) { this->id = p_id; }
	void set_game_name(String p_game_name) { this->game_name = p_game_name; }
	void set_ip_address(String p_ip_address) { this->ip_address = p_ip_address; }
	void set_port(int p_port) { this->port = p_port; }
	void set_description(String p_description) { this->description = p_description; }
	void set_max_players(int p_max_players) { this->max_players = p_max_players; }
	void set_players(int p_players) { this->players = p_players; }
	void set_version(String p_version) { this->version = p_version; }

	void set_dict(const Dictionary &p_dict) {
		if (p_dict.has("id") && p_dict.get("id", "") != "") {
			this->set_id(p_dict.get("id", ""));
		}
		this->set_game_name(p_dict.get("name", ""));
		this->set_ip_address(p_dict.get("ip_address", ""));
		this->set_port(p_dict.get("port", 0));
		this->set_description(p_dict.get("description", ""));
		this->set_max_players(p_dict.get("max_players", 0));
		this->set_players(p_dict.get("players", 0));
		this->set_version(p_dict.get("version", ""));
	}

	Dictionary get_dict() {
		Dictionary dict;
		dict["id"] = this->get_id();
		dict["name"] = this->get_game_name();
		dict["ip_address"] = this->get_ip_address();
		dict["port"] = this->get_port();
		dict["description"] = this->get_description();
		dict["max_players"] = this->get_max_players();
		dict["cur_players"] = this->get_players();
		dict["version"] = this->get_version();
		return dict;
	}
};

class MasterServerClient : public BlaziumClient {
	GDCLASS(MasterServerClient, BlaziumClient);

private:
	String server_url = "https://masterserver.blazium.app/api/v1";
	String game_id = "";
	Vector<String> get_headers() {
		Vector<String> headers;
		headers.append("BLAZIUM_GAMEID: " + game_id);
		return headers;
	}

protected:
	static void _bind_methods() {
		ClassDB::bind_method(D_METHOD("create_game", "game_server_info"), &MasterServerClient::create_game);
		ClassDB::bind_method(D_METHOD("update_game", "game_server_info"), &MasterServerClient::update_game);
		ClassDB::bind_method(D_METHOD("recent_games"), &MasterServerClient::recent_games);
		ClassDB::bind_method(D_METHOD("set_server_url", "server_url"), &MasterServerClient::set_server_url);
		ClassDB::bind_method(D_METHOD("get_server_url"), &MasterServerClient::get_server_url);
		ClassDB::bind_method(D_METHOD("set_game_id", "game_id"), &MasterServerClient::set_game_id);
		ClassDB::bind_method(D_METHOD("get_game_id"), &MasterServerClient::get_game_id);

		ADD_PROPERTY(PropertyInfo(Variant::STRING, "server_url", PROPERTY_HINT_NONE, ""), "set_server_url", "get_server_url");
		ADD_PROPERTY(PropertyInfo(Variant::STRING, "game_id", PROPERTY_HINT_NONE, ""), "set_game_id", "get_game_id");
	}

public:
	class MasterServerListResult : public RefCounted {
		GDCLASS(MasterServerListResult, RefCounted);
		String error = "";
		TypedArray<GameServerInfo> results = TypedArray<GameServerInfo>();

	protected:
		static void _bind_methods() {
			ClassDB::bind_method(D_METHOD("has_error"), &MasterServerListResult::has_error);
			ClassDB::bind_method(D_METHOD("get_error"), &MasterServerListResult::get_error);
			ClassDB::bind_method(D_METHOD("get_results"), &MasterServerListResult::get_result);
			ADD_PROPERTY(PropertyInfo(Variant::STRING, "error"), "", "get_error");
			ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "results", PROPERTY_HINT_RESOURCE_TYPE, "GameServerInfo"), "", "get_results");
		}

	public:
		void set_error(String p_error) { error = p_error; }
		void set_result(TypedArray<GameServerInfo> p_results) { results = p_results; }

		bool has_error() const { return error != ""; }
		String get_error() const { return error; }
		TypedArray<GameServerInfo> get_result() const { return results; }
	};
	class MasterServerListResponse : public RefCounted {
		GDCLASS(MasterServerListResponse, RefCounted);
		HTTPRequest *request;

	protected:
		static void _bind_methods() {
			ADD_SIGNAL(MethodInfo("finished", PropertyInfo(Variant::OBJECT, "result", PROPERTY_HINT_RESOURCE_TYPE, "MasterServerResult")));
		}

	public:
		void _on_request_completed(int p_status, int p_code, const PackedStringArray &p_headers, const PackedByteArray &p_data) {
			Ref<MasterServerListResult> result;
			result.instantiate();
			String result_str = String::utf8((const char *)p_data.ptr(), p_data.size());
			if (p_code != 200 || result_str == "") {
				result->set_error("Result code is not 200: code = " + String::num(p_code) + " " + result_str);
			} else {
				if (result_str != "") {
					Dictionary result_dict = JSON::parse_string(result_str);
					Dictionary data_result = result_dict.get("data", Dictionary());
					if (!result_dict.get("success", false)) {
						if (data_result.get("error", "") != "") {
							result->set_error(data_result.get("error", ""));
						} else {
							result->set_error("Request failed with code: " + String::num(p_code) + " " + result_str);
						}
					} else {
						Array servers_dict = data_result.get("servers", Array());
						TypedArray<GameServerInfo> servers_array;
						for (int i = 0; i < servers_dict.size(); i++) {
							Dictionary server_dict = servers_dict[i];
							Ref<GameServerInfo> game_info;
							game_info.instantiate();
							game_info->set_dict(server_dict);
							servers_array.push_back(game_info);
						}
						result->set_result(servers_array);
					}
				}
			}
			emit_signal(SNAME("finished"), result);
		}
		void call_request(String p_url, Vector<String> p_headers, MasterServerClient *p_client) {
			request = memnew(HTTPRequest);
			p_client->add_child(request);
			request->connect("request_completed", callable_mp(this, &MasterServerListResponse::_on_request_completed));
			request->request(p_url, p_headers, HTTPClient::METHOD_GET, String(""));
		}
	};

	class MasterServerResult : public RefCounted {
		GDCLASS(MasterServerResult, RefCounted);
		String error = "";
		Ref<GameServerInfo> result;

	protected:
		static void _bind_methods() {
			ClassDB::bind_method(D_METHOD("has_error"), &MasterServerResult::has_error);
			ClassDB::bind_method(D_METHOD("get_error"), &MasterServerResult::get_error);
			ClassDB::bind_method(D_METHOD("get_result"), &MasterServerResult::get_result);
			ADD_PROPERTY(PropertyInfo(Variant::STRING, "error"), "", "get_error");
			ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "result", PROPERTY_HINT_RESOURCE_TYPE, "GameServerInfo"), "", "get_result");
			ADD_PROPERTY_DEFAULT("result", Ref<GameServerInfo>());
		}

	public:
		void set_error(String p_error) { error = p_error; }
		void set_result(Ref<GameServerInfo> p_result) { result = p_result; }

		bool has_error() const { return error != ""; }
		String get_error() const { return error; }
		Ref<GameServerInfo> get_result() const { return result; }

		MasterServerResult() {
			result.instantiate();
		}
	};
	class MasterServerResponse : public RefCounted {
		GDCLASS(MasterServerResponse, RefCounted);
		HTTPRequest *request;
		Ref<GameServerInfo> game_info;

	protected:
		static void _bind_methods() {
			ADD_SIGNAL(MethodInfo("finished", PropertyInfo(Variant::OBJECT, "result", PROPERTY_HINT_RESOURCE_TYPE, "MasterServerResult")));
		}

	public:
		void _on_request_completed(int p_status, int p_code, const PackedStringArray &p_headers, const PackedByteArray &p_data) {
			Ref<MasterServerResult> result;
			result.instantiate();
			String result_str = String::utf8((const char *)p_data.ptr(), p_data.size());
			if (p_code != 200 || result_str == "") {
				result->set_error("code: " + String::num(p_code) + " " + result_str);
			} else {
				if (result_str != "") {
					Dictionary result_dict = JSON::parse_string(result_str);
					Dictionary data_result = result_dict.get("data", Dictionary());
					if (!result_dict.get("success", false)) {
						if (data_result.get("error", "") != "") {
							result->set_error(data_result.get("error", ""));
						} else {
							result->set_error("Request failed with code: " + String::num(p_code) + " " + result_str);
						}
					} else {
						// create call
						if (data_result.has("server")) {
							game_info->set_id(data_result.get("server", ""));
						}
						result->set_result(game_info);
					}
				}
			}
			emit_signal(SNAME("finished"), result);
		}
		void call_request(String p_url, Vector<String> p_headers, HTTPClient::Method p_method, Dictionary p_data, MasterServerClient *p_client, Ref<GameServerInfo> p_game_server_info) {
			game_info = p_game_server_info;
			request = memnew(HTTPRequest);
			p_client->add_child(request);
			request->connect("request_completed", callable_mp(this, &MasterServerResponse::_on_request_completed));
			request->request(p_url, p_headers, p_method, JSON::stringify(p_data));
		}

		MasterServerResponse() {
			game_info.instantiate();
		}
	};

	Ref<MasterServerResponse> create_game(Ref<GameServerInfo> p_game_server_info) {
		Ref<MasterServerResponse> response;
		response.instantiate();
		Dictionary dict_data = p_game_server_info->get_dict();
		response->call_request(server_url + "/server/create", get_headers(), HTTPClient::METHOD_POST, dict_data, this, p_game_server_info);
		return response;
	}

	Ref<MasterServerResponse> update_game(Ref<GameServerInfo> p_game_server_info) {
		Ref<MasterServerResponse> response;
		response.instantiate();
		Dictionary dict_data = p_game_server_info->get_dict();
		response->call_request(server_url + "/server/update/" + p_game_server_info->get_id(), get_headers(), HTTPClient::METHOD_PUT, dict_data, this, p_game_server_info);
		return response;
	}

	Ref<MasterServerListResponse> recent_games() {
		Ref<MasterServerListResponse> response;
		response.instantiate();
		response->call_request(server_url + "/servers/recent", get_headers(), this);
		return response;
	}

	void set_server_url(String p_server_url) { server_url = p_server_url; }
	String get_server_url() const { return server_url; }
	void set_game_id(String p_game_id) { game_id = p_game_id; }
	String get_game_id() const { return game_id; }
};

#endif // MASTER_SERVER_CLIENT_H
