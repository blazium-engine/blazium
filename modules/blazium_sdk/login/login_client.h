/**************************************************************************/
/*  login_client.h                                                        */
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

#ifndef LOGIN_CLIENT_H
#define LOGIN_CLIENT_H

#include "../blazium_client.h"
#include "core/io/json.h"
#include "modules/websocket/websocket_peer.h"

class LoginClient : public BlaziumClient {
	GDCLASS(LoginClient, BlaziumClient);

protected:
	String server_url = "wss://login.blazium.app/connect";
	String game_id = "";
	bool connected = false;

public:
	class LoginResponse : public RefCounted {
		GDCLASS(LoginResponse, RefCounted);

	protected:
		static void _bind_methods() {
			ADD_SIGNAL(MethodInfo("finished", PropertyInfo(Variant::OBJECT, "result", PROPERTY_HINT_RESOURCE_TYPE, "LobbyResult")));
		}

	public:
		class LoginResult : public RefCounted {
			GDCLASS(LoginResult, RefCounted);

			String error = "";
			String login_url = "";
			String login_type = "";

		protected:
			static void _bind_methods() {
				ClassDB::bind_method(D_METHOD("get_login_url"), &LoginResult::get_login_url);
				ClassDB::bind_method(D_METHOD("get_login_type"), &LoginResult::get_login_type);
				ClassDB::bind_method(D_METHOD("has_error"), &LoginResult::has_error);
				ClassDB::bind_method(D_METHOD("get_error"), &LoginResult::get_error);
				ADD_PROPERTY(PropertyInfo(Variant::STRING, "error"), "", "get_error");
				ADD_PROPERTY(PropertyInfo(Variant::STRING, "login_url"), "", "get_login_url");
				ADD_PROPERTY(PropertyInfo(Variant::STRING, "login_type"), "", "get_login_type");
			}

		public:
			void set_login_type(String p_type) { this->login_type = p_type; }
			void set_login_url(String p_url) { this->login_url = p_url; }
			void set_error(String p_error) { this->error = p_error; }

			bool has_error() const { return !error.is_empty(); }
			String get_error() const { return error; }
			String get_login_url() const { return login_url; }
			String get_login_type() const { return login_type; }
		};
	};

protected:
	Ref<WebSocketPeer> _socket;
	Ref<LoginResponse> login_response;

	void _receive_data(const Dictionary &p_data) {
		String action = p_data.get("action", "");
		if (action == "login_url") {
			String url = p_data.get("url", "");
			String type = p_data.get("type", "");
			Ref<LoginResponse::LoginResult> login_result;
			login_result.instantiate();
			login_result->set_login_url(url);
			login_result->set_login_type(type);
			login_response->emit_signal("finished", login_result);
		}
		if (action == "error") {
			String error = p_data.get("error", "");
			Ref<LoginResponse::LoginResult> login_result;
			login_result.instantiate();
			login_result->set_error(error);
			login_response->emit_signal("finished", login_result);
		}
		if (action == "jwt") {
			String jwt = p_data.get("url", "");
			String type = p_data.get("type", "");
			emit_signal("received_jwt", jwt, type);
		}
	}

	void _notification(int p_notification) {
		switch (p_notification) {
			case NOTIFICATION_INTERNAL_PROCESS: {
				_socket->poll();

				WebSocketPeer::State state = _socket->get_ready_state();
				if (state == WebSocketPeer::STATE_OPEN) {
					if (!connected) {
						emit_signal("connected_to_server");
					}
					connected = true;
					while (_socket->get_available_packet_count() > 0) {
						Vector<uint8_t> packet_buffer;
						Error err = _socket->get_packet_buffer(packet_buffer);
						if (err != OK) {
							return;
						}
						String packet_string = String::utf8((const char *)packet_buffer.ptr(), packet_buffer.size());
						_receive_data(JSON::parse_string(packet_string));
					}
				} else if (state == WebSocketPeer::STATE_CLOSED) {
					emit_signal("disconnected_from_server", _socket->get_close_reason());
					set_process_internal(false);
					connected = false;
				}
			} break;
		}
	}
	static void _bind_methods();

	void _send_data(const Dictionary &p_data_dict) {
		if (_socket->get_ready_state() != WebSocketPeer::STATE_OPEN) {
			emit_signal("log_updated", "error", "Socket is not ready.");
			return;
		}
		Error err = _socket->send_text(JSON::stringify(p_data_dict));
		if (err != OK) {
			emit_signal("log_updated", "error", "No longer connected.");
			_socket->close(1000, "Disconnected");
		}
	}

public:
	void set_server_url(const String &p_server_url) { this->server_url = p_server_url; }
	String get_server_url() { return server_url; }
	void set_game_id(const String &p_game_id) { this->game_id = p_game_id; }
	String get_game_id() { return game_id; }
	bool get_connected() { return connected; }

	bool connect_to_server();
	void disconnect_from_server();

	Ref<LoginResponse> request_login_info(String p_type) {
		Dictionary command;
		command["action"] = "getLogin";
		command["type"] = p_type;
		login_response = Ref<LoginResponse>();
		login_response.instantiate();
		_send_data(command);
		return login_response;
	}

	LoginClient() {
		_socket = Ref<WebSocketPeer>(WebSocketPeer::create());
		set_process_internal(false);
	}

	~LoginClient() {
		_socket->close();
		set_process_internal(false);
	}
};

#endif // LOGIN_CLIENT_H
