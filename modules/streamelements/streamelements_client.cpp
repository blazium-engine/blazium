/**************************************************************************/
/*  streamelements_client.cpp                                             */
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

#include "streamelements_client.h"

#include "core/crypto/crypto_core.h"
#include "core/io/json.h"
#include "core/os/time.h"

StreamElementsClient *StreamElementsClient::singleton = nullptr;

StreamElementsClient *StreamElementsClient::get_singleton() {
	return singleton;
}

void StreamElementsClient::_bind_methods() {
	// Connection management
	ClassDB::bind_method(D_METHOD("connect_to_service", "token", "token_type"), &StreamElementsClient::connect_to_service, DEFVAL(TOKEN_JWT));
	ClassDB::bind_method(D_METHOD("close"), &StreamElementsClient::close);
	ClassDB::bind_method(D_METHOD("poll"), &StreamElementsClient::poll);

	// Subscription management
	ClassDB::bind_method(D_METHOD("subscribe", "topic", "room"), &StreamElementsClient::subscribe, DEFVAL(""));
	ClassDB::bind_method(D_METHOD("unsubscribe", "topic", "room"), &StreamElementsClient::unsubscribe, DEFVAL(""));
	ClassDB::bind_method(D_METHOD("get_subscribed_topics"), &StreamElementsClient::get_subscribed_topics);

	// Connection info
	ClassDB::bind_method(D_METHOD("is_service_connected"), &StreamElementsClient::is_service_connected);
	ClassDB::bind_method(D_METHOD("get_connection_state"), &StreamElementsClient::get_connection_state);

	// Reconnection settings
	ClassDB::bind_method(D_METHOD("set_auto_reconnect", "enabled"), &StreamElementsClient::set_auto_reconnect);
	ClassDB::bind_method(D_METHOD("get_auto_reconnect"), &StreamElementsClient::get_auto_reconnect);
	ClassDB::bind_method(D_METHOD("set_reconnect_delay_min", "seconds"), &StreamElementsClient::set_reconnect_delay_min);
	ClassDB::bind_method(D_METHOD("get_reconnect_delay_min"), &StreamElementsClient::get_reconnect_delay_min);
	ClassDB::bind_method(D_METHOD("set_reconnect_delay_max", "seconds"), &StreamElementsClient::set_reconnect_delay_max);
	ClassDB::bind_method(D_METHOD("get_reconnect_delay_max"), &StreamElementsClient::get_reconnect_delay_max);

	// Properties
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "auto_reconnect"), "set_auto_reconnect", "get_auto_reconnect");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "reconnect_delay_min"), "set_reconnect_delay_min", "get_reconnect_delay_min");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "reconnect_delay_max"), "set_reconnect_delay_max", "get_reconnect_delay_max");

	// Enums
	BIND_ENUM_CONSTANT(STATE_DISCONNECTED);
	BIND_ENUM_CONSTANT(STATE_CONNECTING);
	BIND_ENUM_CONSTANT(STATE_CONNECTED);
	BIND_ENUM_CONSTANT(STATE_RECONNECTING);

	BIND_ENUM_CONSTANT(TOKEN_JWT);
	BIND_ENUM_CONSTANT(TOKEN_APIKEY);
	BIND_ENUM_CONSTANT(TOKEN_OAUTH);

	// Signals - Generic
	ADD_SIGNAL(MethodInfo("message_received", PropertyInfo(Variant::STRING, "topic"), PropertyInfo(Variant::DICTIONARY, "data")));

	// Signals - Connection
	ADD_SIGNAL(MethodInfo("connected"));
	ADD_SIGNAL(MethodInfo("disconnected", PropertyInfo(Variant::STRING, "reason")));
	ADD_SIGNAL(MethodInfo("connection_error", PropertyInfo(Variant::STRING, "error")));

	// Signals - Subscription
	ADD_SIGNAL(MethodInfo("subscription_response", PropertyInfo(Variant::STRING, "topic"), PropertyInfo(Variant::BOOL, "success"), PropertyInfo(Variant::STRING, "message")));

	// Signals - Specific events
	ADD_SIGNAL(MethodInfo("activity_received", PropertyInfo(Variant::STRING, "activity_type"), PropertyInfo(Variant::STRING, "provider"), PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("chat_message_received", PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("tip_received", PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("follow_received", PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("subscriber_received", PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("stream_status_changed", PropertyInfo(Variant::STRING, "status")));
}

Error StreamElementsClient::connect_to_service(const String &p_token, TokenType p_token_type) {
	ERR_FAIL_COND_V_MSG(connection_state != STATE_DISCONNECTED, ERR_ALREADY_IN_USE, "Already connected or connecting");
	ERR_FAIL_COND_V_MSG(p_token.is_empty(), ERR_INVALID_PARAMETER, "Token cannot be empty");

	// Store authentication info
	auth_token = p_token;
	token_type = p_token_type;

	// Create WebSocket peer
	ws = Ref<WebSocketPeer>(WebSocketPeer::create());
	ERR_FAIL_COND_V_MSG(ws.is_null(), ERR_CANT_CREATE, "Failed to create WebSocket peer");

	// Configure TLS for secure connection
	Ref<TLSOptions> tls_options = TLSOptions::client();

	// Connect to StreamElements Astro Gateway
	Error err = ws->connect_to_url("wss://astro.streamelements.com", tls_options);
	if (err != OK) {
		ws.unref();
		emit_signal("connection_error", "Failed to initiate connection");
		return err;
	}

	connection_state = STATE_CONNECTING;
	return OK;
}

void StreamElementsClient::close() {
	if (ws.is_valid()) {
		ws->close(1000, "Client disconnect");
		ws.unref();
	}

	connection_state = STATE_DISCONNECTED;
	subscribed_topics.clear();
	pending_requests.clear();
	time_since_disconnect = 0.0;
}

void StreamElementsClient::poll() {
	if (ws.is_null()) {
		// Check if we should attempt reconnection
		if (connection_state == STATE_RECONNECTING && auto_reconnect) {
			_attempt_reconnect(0.016); // Approximate frame time
		}
		return;
	}

	ws->poll();
	WebSocketPeer::State state = ws->get_ready_state();

	switch (state) {
		case WebSocketPeer::STATE_CONNECTING:
			// Still connecting, keep polling
			break;

		case WebSocketPeer::STATE_OPEN:
			// Connection established
			if (connection_state == STATE_CONNECTING) {
				connection_state = STATE_CONNECTED;
				_reset_reconnect_delay();
				emit_signal("connected");

				// Resubscribe to topics if this is a reconnection
				if (subscribed_topics.size() > 0) {
					_resubscribe_all();
				}
			}

			// Process incoming messages
			_process_websocket_messages();
			break;

		case WebSocketPeer::STATE_CLOSING:
			// Closing handshake in progress
			break;

		case WebSocketPeer::STATE_CLOSED:
			// Connection closed
			int close_code = ws->get_close_code();
			String close_reason = ws->get_close_reason();

			ws.unref();

			if (connection_state == STATE_CONNECTED || connection_state == STATE_CONNECTING) {
				emit_signal("disconnected", close_reason.is_empty() ? "Connection closed" : close_reason);

				if (auto_reconnect && close_code != 1000) { // Don't auto-reconnect on normal closure
					connection_state = STATE_RECONNECTING;
					time_since_disconnect = 0.0;
				} else {
					connection_state = STATE_DISCONNECTED;
					subscribed_topics.clear();
				}
			}
			break;
	}
}

Error StreamElementsClient::subscribe(const String &p_topic, const String &p_room) {
	ERR_FAIL_COND_V_MSG(connection_state != STATE_CONNECTED, ERR_UNCONFIGURED, "Not connected to StreamElements");
	ERR_FAIL_COND_V_MSG(p_topic.is_empty(), ERR_INVALID_PARAMETER, "Topic cannot be empty");

	// Build subscription request
	Dictionary request_data;
	request_data["topic"] = p_topic;
	request_data["token"] = auth_token;

	// Set token type
	switch (token_type) {
		case TOKEN_JWT:
			request_data["token_type"] = "jwt";
			break;
		case TOKEN_APIKEY:
			request_data["token_type"] = "apikey";
			break;
		case TOKEN_OAUTH:
			request_data["token_type"] = "oauth";
			break;
	}

	// Add room if specified
	if (!p_room.is_empty()) {
		request_data["room"] = p_room;
	}

	// Send subscribe request
	Error err = _send_request("subscribe", request_data);
	if (err == OK) {
		subscribed_topics[p_topic] = true;
	}

	return err;
}

Error StreamElementsClient::unsubscribe(const String &p_topic, const String &p_room) {
	ERR_FAIL_COND_V_MSG(connection_state != STATE_CONNECTED, ERR_UNCONFIGURED, "Not connected to StreamElements");
	ERR_FAIL_COND_V_MSG(p_topic.is_empty(), ERR_INVALID_PARAMETER, "Topic cannot be empty");

	// Build unsubscribe request
	Dictionary request_data;
	request_data["topic"] = p_topic;
	request_data["token"] = auth_token;

	// Set token type
	switch (token_type) {
		case TOKEN_JWT:
			request_data["token_type"] = "jwt";
			break;
		case TOKEN_APIKEY:
			request_data["token_type"] = "apikey";
			break;
		case TOKEN_OAUTH:
			request_data["token_type"] = "oauth";
			break;
	}

	// Add room if specified
	if (!p_room.is_empty()) {
		request_data["room"] = p_room;
	}

	// Send unsubscribe request
	Error err = _send_request("unsubscribe", request_data);
	if (err == OK) {
		subscribed_topics.erase(p_topic);
	}

	return err;
}

Array StreamElementsClient::get_subscribed_topics() const {
	Array topics;
	for (const KeyValue<String, bool> &E : subscribed_topics) {
		if (E.value) {
			topics.push_back(E.key);
		}
	}
	return topics;
}

bool StreamElementsClient::is_service_connected() const {
	return connection_state == STATE_CONNECTED;
}

StreamElementsClient::ConnectionState StreamElementsClient::get_connection_state() const {
	return connection_state;
}

void StreamElementsClient::set_auto_reconnect(bool p_enabled) {
	auto_reconnect = p_enabled;
}

bool StreamElementsClient::get_auto_reconnect() const {
	return auto_reconnect;
}

void StreamElementsClient::set_reconnect_delay_min(float p_seconds) {
	reconnect_delay_min = MAX(0.1, p_seconds);
	reconnect_delay_current = reconnect_delay_min;
}

float StreamElementsClient::get_reconnect_delay_min() const {
	return reconnect_delay_min;
}

void StreamElementsClient::set_reconnect_delay_max(float p_seconds) {
	reconnect_delay_max = MAX(reconnect_delay_min, p_seconds);
}

float StreamElementsClient::get_reconnect_delay_max() const {
	return reconnect_delay_max;
}

// Private methods

void StreamElementsClient::_process_websocket_messages() {
	while (ws->get_available_packet_count() > 0) {
		const uint8_t *buffer;
		int buffer_size;

		Error err = ws->get_packet(&buffer, buffer_size);
		if (err != OK) {
			ERR_PRINT("Failed to get WebSocket packet");
			continue;
		}

		if (!ws->was_string_packet()) {
			WARN_PRINT("Received binary packet from StreamElements (expected text)");
			continue;
		}

		// Parse JSON message
		String message_str;
		message_str.parse_utf8((const char *)buffer, buffer_size);

		Ref<JSON> json;
		json.instantiate();
		err = json->parse(message_str);
		if (err != OK) {
			ERR_PRINT(vformat("Failed to parse JSON from StreamElements: %s", message_str));
			continue;
		}

		Dictionary message = json->get_data();
		_handle_message(message);
	}
}

void StreamElementsClient::_handle_message(const Dictionary &p_message) {
	if (!p_message.has("type")) {
		ERR_PRINT("StreamElements message missing 'type' field");
		return;
	}

	String type = p_message["type"];

	if (type == "response") {
		_handle_response(p_message);
	} else if (type == "message") {
		_handle_notification(p_message);
	} else {
		WARN_PRINT(vformat("Unknown StreamElements message type: %s", type));
	}
}

void StreamElementsClient::_handle_response(const Dictionary &p_response) {
	String nonce = p_response.get("nonce", "");
	String error = p_response.get("error", "");
	Dictionary data = p_response.get("data", Dictionary());
	String message = data.get("message", "");
	String topic = data.get("topic", "");

	// Check for errors
	if (!error.is_empty()) {
		ERR_PRINT(vformat("StreamElements error (%s): %s", error, message));
		emit_signal("subscription_response", topic, false, message);
		return;
	}

	// Success response
	if (!topic.is_empty()) {
		emit_signal("subscription_response", topic, true, message);
	}
}

void StreamElementsClient::_handle_notification(const Dictionary &p_notification) {
	String topic = p_notification.get("topic", "");
	Dictionary data = p_notification.get("data", Dictionary());

	if (topic.is_empty()) {
		WARN_PRINT("StreamElements notification missing topic");
		return;
	}

	// Emit generic signal
	emit_signal("message_received", topic, data);

	// Emit specific signals based on topic
	_emit_specific_signals(topic, data);
}

void StreamElementsClient::_emit_specific_signals(const String &p_topic, const Dictionary &p_data) {
	if (p_topic == "channel.activities") {
		String activity_type = p_data.get("type", "");
		String provider = p_data.get("provider", "");
		Dictionary activity_data = p_data.get("data", Dictionary());

		// Emit generic activity signal
		emit_signal("activity_received", activity_type, provider, p_data);

		// Emit specific activity signals
		if (activity_type == "follow") {
			emit_signal("follow_received", p_data);
		} else if (activity_type == "subscriber") {
			emit_signal("subscriber_received", p_data);
		} else if (activity_type == "tip") {
			emit_signal("tip_received", p_data);
		}
	} else if (p_topic == "channel.chat.message") {
		emit_signal("chat_message_received", p_data);
	} else if (p_topic == "channel.stream.status") {
		String status = p_data.get("status", "");
		emit_signal("stream_status_changed", status);
	}
	// Add more topic-specific signal handling as needed
}

String StreamElementsClient::_generate_nonce() {
	// Generate a UUID v4
	PackedByteArray random_bytes;
	random_bytes.resize(16);

	CryptoCore::RandomGenerator rng;
	Error err = rng.init();
	ERR_FAIL_COND_V(err != OK, "");

	err = rng.get_random_bytes(random_bytes.ptrw(), 16);
	ERR_FAIL_COND_V(err != OK, "");

	// Format as UUID
	String uuid = vformat("%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
			random_bytes[0], random_bytes[1], random_bytes[2], random_bytes[3],
			random_bytes[4], random_bytes[5],
			random_bytes[6], random_bytes[7],
			random_bytes[8], random_bytes[9],
			random_bytes[10], random_bytes[11], random_bytes[12], random_bytes[13],
			random_bytes[14], random_bytes[15]);

	return uuid;
}

Error StreamElementsClient::_send_request(const String &p_type, const Dictionary &p_data) {
	ERR_FAIL_COND_V(ws.is_null(), ERR_UNCONFIGURED);
	ERR_FAIL_COND_V(ws->get_ready_state() != WebSocketPeer::STATE_OPEN, ERR_CONNECTION_ERROR);

	// Build request
	Dictionary request;
	request["type"] = p_type;
	request["nonce"] = _generate_nonce();
	request["data"] = p_data;

	// Store pending request
	pending_requests[request["nonce"]] = request;

	// Serialize to JSON
	String json_str = JSON::stringify(request);

	// Send as text message
	Error err = ws->send_text(json_str);
	if (err != OK) {
		ERR_PRINT(vformat("Failed to send request to StreamElements: %d", err));
		pending_requests.erase(request["nonce"]);
	}

	return err;
}

void StreamElementsClient::_attempt_reconnect(float p_delta) {
	time_since_disconnect += p_delta;

	if (time_since_disconnect >= reconnect_delay_current) {
		time_since_disconnect = 0.0;

		// Attempt to reconnect
		Error err = connect_to_service(auth_token, token_type);
		if (err != OK) {
			// Increase delay with exponential backoff
			reconnect_delay_current = MIN(reconnect_delay_current * 2.0, reconnect_delay_max);
		}
	}
}

void StreamElementsClient::_reset_reconnect_delay() {
	reconnect_delay_current = reconnect_delay_min;
	time_since_disconnect = 0.0;
}

void StreamElementsClient::_resubscribe_all() {
	// Resubscribe to all previously subscribed topics
	Array topics_to_resubscribe;
	for (const KeyValue<String, bool> &E : subscribed_topics) {
		if (E.value) {
			topics_to_resubscribe.push_back(E.key);
		}
	}

	// Clear current subscriptions (will be repopulated by subscribe calls)
	subscribed_topics.clear();

	// Resubscribe
	for (int i = 0; i < topics_to_resubscribe.size(); i++) {
		subscribe(topics_to_resubscribe[i]);
	}
}

StreamElementsClient::StreamElementsClient() {
	singleton = this;
}

StreamElementsClient::~StreamElementsClient() {
	close();
	singleton = nullptr;
}

