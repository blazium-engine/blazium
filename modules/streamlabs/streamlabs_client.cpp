/**************************************************************************/
/*  streamlabs_client.cpp                                                 */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             BLAZIUM ENGINE                             */
/*                          https://blazium.app                           */
/**************************************************************************/
/* Copyright (c) 2024-present Blazium Engine contributors.                */
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

#include "streamlabs_client.h"

#include "core/io/json.h"

StreamlabsClient *StreamlabsClient::singleton = nullptr;

StreamlabsClient *StreamlabsClient::get_singleton() {
	return singleton;
}

void StreamlabsClient::_bind_methods() {
	// Connection management
	ClassDB::bind_method(D_METHOD("connect_to_streamlabs", "socket_token"), &StreamlabsClient::connect_to_streamlabs);
	ClassDB::bind_method(D_METHOD("disconnect_from_streamlabs"), &StreamlabsClient::disconnect_from_streamlabs);
	ClassDB::bind_method(D_METHOD("poll"), &StreamlabsClient::poll);

	// Connection info
	ClassDB::bind_method(D_METHOD("is_streamlabs_connected"), &StreamlabsClient::is_streamlabs_connected);
	ClassDB::bind_method(D_METHOD("get_connection_state"), &StreamlabsClient::get_connection_state);

	// Enums
	BIND_ENUM_CONSTANT(STATE_DISCONNECTED);
	BIND_ENUM_CONSTANT(STATE_CONNECTING);
	BIND_ENUM_CONSTANT(STATE_CONNECTED);

	// Connection signals
	ADD_SIGNAL(MethodInfo("connected"));
	ADD_SIGNAL(MethodInfo("disconnected", PropertyInfo(Variant::STRING, "reason")));
	ADD_SIGNAL(MethodInfo("connection_error", PropertyInfo(Variant::DICTIONARY, "error")));

	// Platform-agnostic signals
	ADD_SIGNAL(MethodInfo("donation", PropertyInfo(Variant::DICTIONARY, "data")));

	// Twitch signals
	ADD_SIGNAL(MethodInfo("twitch_follow", PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("twitch_subscription", PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("twitch_bits", PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("twitch_raid", PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("twitch_host", PropertyInfo(Variant::DICTIONARY, "data")));

	// YouTube signals
	ADD_SIGNAL(MethodInfo("youtube_follow", PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("youtube_subscription", PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("youtube_superchat", PropertyInfo(Variant::DICTIONARY, "data")));

	// Mixer signals
	ADD_SIGNAL(MethodInfo("mixer_follow", PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("mixer_subscription", PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("mixer_host", PropertyInfo(Variant::DICTIONARY, "data")));
}

Error StreamlabsClient::connect_to_streamlabs(const String &p_socket_token) {
	ERR_FAIL_COND_V_MSG(connection_state != STATE_DISCONNECTED, ERR_ALREADY_IN_USE, "Already connected or connecting to Streamlabs");
	ERR_FAIL_COND_V_MSG(p_socket_token.is_empty(), ERR_INVALID_PARAMETER, "Socket token cannot be empty");

	socket_token = p_socket_token;

	// Get SocketIOClient singleton
	SocketIOClient *socketio = SocketIOClient::get_singleton();
	ERR_FAIL_NULL_V_MSG(socketio, ERR_UNAVAILABLE, "SocketIOClient singleton not available");

	// Build Streamlabs Socket API URL with token
	String url = "https://sockets.streamlabs.com?token=" + p_socket_token;

	// Connect to Streamlabs Socket API
	Error err = socketio->connect_to_url(url);
	if (err != OK) {
		return err;
	}

	connection_state = STATE_CONNECTING;

	// Get or create the default namespace
	socketio_namespace = socketio->get_namespace("/");
	ERR_FAIL_COND_V_MSG(socketio_namespace.is_null(), ERR_CANT_CREATE, "Failed to get Socket.IO namespace");

	// Connect signals from the namespace
	if (!socketio_namespace->is_connected("connected", callable_mp(this, &StreamlabsClient::_on_socketio_connected))) {
		socketio_namespace->connect("connected", callable_mp(this, &StreamlabsClient::_on_socketio_connected));
	}
	if (!socketio_namespace->is_connected("disconnected", callable_mp(this, &StreamlabsClient::_on_socketio_disconnected))) {
		socketio_namespace->connect("disconnected", callable_mp(this, &StreamlabsClient::_on_socketio_disconnected));
	}
	if (!socketio_namespace->is_connected("connect_error", callable_mp(this, &StreamlabsClient::_on_socketio_connect_error))) {
		socketio_namespace->connect("connect_error", callable_mp(this, &StreamlabsClient::_on_socketio_connect_error));
	}
	if (!socketio_namespace->is_connected("event", callable_mp(this, &StreamlabsClient::_on_event_received))) {
		socketio_namespace->connect("event", callable_mp(this, &StreamlabsClient::_on_event_received));
	}

	return OK;
}

void StreamlabsClient::disconnect_from_streamlabs() {
	if (connection_state == STATE_DISCONNECTED) {
		return;
	}

	SocketIOClient *socketio = SocketIOClient::get_singleton();
	if (socketio) {
		socketio->close();
	}

	if (socketio_namespace.is_valid()) {
		// Disconnect signals
		if (socketio_namespace->is_connected("connected", callable_mp(this, &StreamlabsClient::_on_socketio_connected))) {
			socketio_namespace->disconnect("connected", callable_mp(this, &StreamlabsClient::_on_socketio_connected));
		}
		if (socketio_namespace->is_connected("disconnected", callable_mp(this, &StreamlabsClient::_on_socketio_disconnected))) {
			socketio_namespace->disconnect("disconnected", callable_mp(this, &StreamlabsClient::_on_socketio_disconnected));
		}
		if (socketio_namespace->is_connected("connect_error", callable_mp(this, &StreamlabsClient::_on_socketio_connect_error))) {
			socketio_namespace->disconnect("connect_error", callable_mp(this, &StreamlabsClient::_on_socketio_connect_error));
		}
		if (socketio_namespace->is_connected("event", callable_mp(this, &StreamlabsClient::_on_event_received))) {
			socketio_namespace->disconnect("event", callable_mp(this, &StreamlabsClient::_on_event_received));
		}

		socketio_namespace.unref();
	}

	connection_state = STATE_DISCONNECTED;
}

void StreamlabsClient::poll() {
	SocketIOClient *socketio = SocketIOClient::get_singleton();
	if (socketio) {
		socketio->poll();
	}
}

bool StreamlabsClient::is_streamlabs_connected() const {
	return connection_state == STATE_CONNECTED;
}

StreamlabsClient::ConnectionState StreamlabsClient::get_connection_state() const {
	return connection_state;
}

void StreamlabsClient::_on_socketio_connected() {
	connection_state = STATE_CONNECTED;
	emit_signal("connected");
}

void StreamlabsClient::_on_socketio_disconnected(const String &p_reason) {
	connection_state = STATE_DISCONNECTED;
	emit_signal("disconnected", p_reason);
}

void StreamlabsClient::_on_socketio_connect_error(const Dictionary &p_error) {
	connection_state = STATE_DISCONNECTED;
	emit_signal("connection_error", p_error);
}

void StreamlabsClient::_on_event_received(const String &p_event_name, const Array &p_data) {
	// Streamlabs sends events on the "event" event name
	if (p_event_name != "event") {
		return;
	}

	// Parse the event data
	if (p_data.size() > 0) {
		Variant event_variant = p_data[0];
		if (event_variant.get_type() == Variant::DICTIONARY) {
			Dictionary event_data = event_variant;
			_handle_streamlabs_event(event_data);
		}
	}
}

void StreamlabsClient::_handle_streamlabs_event(const Dictionary &p_event_data) {
	// Extract event type and platform
	String event_type = "";
	String platform = "";

	if (p_event_data.has("type")) {
		event_type = p_event_data["type"];
	}

	if (p_event_data.has("for")) {
		platform = p_event_data["for"];
	}

	// Extract message array
	Array messages;
	if (p_event_data.has("message")) {
		Variant msg_variant = p_event_data["message"];
		if (msg_variant.get_type() == Variant::ARRAY) {
			messages = msg_variant;
		}
	}

	// Process each message in the array
	for (int i = 0; i < messages.size(); i++) {
		Variant msg_variant = messages[i];
		if (msg_variant.get_type() == Variant::DICTIONARY) {
			Dictionary message = msg_variant;
			_emit_platform_event(platform, event_type, message);
		}
	}
}

void StreamlabsClient::_emit_platform_event(const String &p_platform, const String &p_type, const Dictionary &p_message) {
	// Platform-agnostic events (no "for" field)
	if (p_platform.is_empty()) {
		if (p_type == "donation") {
			emit_signal("donation", p_message);
		}
		return;
	}

	// Twitch events
	if (p_platform == "twitch_account") {
		if (p_type == "follow") {
			emit_signal("twitch_follow", p_message);
		} else if (p_type == "subscription") {
			emit_signal("twitch_subscription", p_message);
		} else if (p_type == "bits") {
			emit_signal("twitch_bits", p_message);
		} else if (p_type == "raid") {
			emit_signal("twitch_raid", p_message);
		} else if (p_type == "host") {
			emit_signal("twitch_host", p_message);
		}
		return;
	}

	// YouTube events
	if (p_platform == "youtube_account") {
		if (p_type == "follow") {
			emit_signal("youtube_follow", p_message);
		} else if (p_type == "subscription") {
			emit_signal("youtube_subscription", p_message);
		} else if (p_type == "superchat") {
			emit_signal("youtube_superchat", p_message);
		}
		return;
	}

	// Mixer events
	if (p_platform == "mixer_account") {
		if (p_type == "follow") {
			emit_signal("mixer_follow", p_message);
		} else if (p_type == "subscription") {
			emit_signal("mixer_subscription", p_message);
		} else if (p_type == "host") {
			emit_signal("mixer_host", p_message);
		}
		return;
	}
}

StreamlabsClient::StreamlabsClient() {
	ERR_FAIL_COND_MSG(singleton != nullptr, "StreamlabsClient singleton already exists");
	singleton = this;
}

StreamlabsClient::~StreamlabsClient() {
	disconnect_from_streamlabs();
	singleton = nullptr;
}
