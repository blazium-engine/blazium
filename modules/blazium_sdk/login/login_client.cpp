#include "login_client.h"

void LoginClient::_bind_methods() {
	ClassDB::bind_method(D_METHOD("request_login_info", "login_type"), &LoginClient::request_login_info);
	ClassDB::bind_method(D_METHOD("set_server_url", "server_url"), &LoginClient::set_server_url);
	ClassDB::bind_method(D_METHOD("get_server_url"), &LoginClient::get_server_url);
	ClassDB::bind_method(D_METHOD("set_game_id", "game_id"), &LoginClient::set_game_id);
	ClassDB::bind_method(D_METHOD("get_game_id"), &LoginClient::get_game_id);
	ClassDB::bind_method(D_METHOD("get_connected"), &LoginClient::get_connected);

	ClassDB::bind_method(D_METHOD("connect_to_server"), &LoginClient::connect_to_server);
	ClassDB::bind_method(D_METHOD("disconnect_from_server"), &LoginClient::disconnect_from_server);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "server_url", PROPERTY_HINT_NONE, ""), "set_server_url", "get_server_url");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "game_id", PROPERTY_HINT_NONE, ""), "set_game_id", "get_game_id");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "connected"), "", "get_connected");

	ADD_SIGNAL(MethodInfo("log_updated", PropertyInfo(Variant::STRING, "command"), PropertyInfo(Variant::STRING, "logs")));
	ADD_SIGNAL(MethodInfo("disconnected_from_server", PropertyInfo(Variant::STRING, "reason")));
	ADD_SIGNAL(MethodInfo("connected_to_server"));
	ADD_SIGNAL(MethodInfo("received_jwt", PropertyInfo(Variant::STRING, "jwt"), PropertyInfo(Variant::STRING, "type")));
}

bool LoginClient::connect_to_server() {
	if (connected) {
		return true;
	}
	String lobby_url = get_server_url();
	String url = lobby_url;
	PackedStringArray protocols;
	protocols.push_back("blazium");
	protocols.push_back(game_id);
	_socket->set_supported_protocols(protocols);
	Error err = _socket->connect_to_url(url);
	if (err != OK) {
		set_process_internal(false);
		emit_signal("log_updated", "error", "Unable to connect to lobby server at: " + url);
		connected = false;
		return false;
	}
	set_process_internal(true);
	emit_signal("log_updated", "connect_to_lobby", "Connecting to: " + url);
	return true;
}

void LoginClient::disconnect_from_server() {
	if (connected) {
		_socket->close(1000, "Normal Closure");
		connected = false;
		set_process_internal(false);
		emit_signal("disconnected_from_server", _socket->get_close_reason());
		emit_signal("log_updated", "disconnected_from_server", "Disconnected from: " + get_server_url());
	}
}
