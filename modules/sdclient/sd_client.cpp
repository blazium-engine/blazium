/**************************************************************************/
/*  sd_client.cpp                                                         */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
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

#include "sd_client.h"

#include "core/io/json.h"
#include "core/os/os.h"
#include "core/string/ustring.h"
#include "core/variant/variant.h"

SDClient *SDClient::singleton = nullptr;

SDClient *SDClient::get_singleton() {
	return singleton;
}

void SDClient::_bind_methods() {
	// Connection management
	ClassDB::bind_method(D_METHOD("parse_command_line_args"), &SDClient::parse_command_line_args);
	ClassDB::bind_method(D_METHOD("connect_from_args"), &SDClient::connect_from_args);
	ClassDB::bind_method(D_METHOD("connect_manual", "port", "uuid", "register_event"), &SDClient::connect_manual);
	ClassDB::bind_method(D_METHOD("disconnect_from_stream_deck"), &SDClient::disconnect_from_stream_deck);
	ClassDB::bind_method(D_METHOD("poll"), &SDClient::poll);
	ClassDB::bind_method(D_METHOD("get_connection_state"), &SDClient::get_connection_state);
	ClassDB::bind_method(D_METHOD("is_stream_deck_connected"), &SDClient::is_stream_deck_connected);

	// Registration info access
	ClassDB::bind_method(D_METHOD("get_registration_info"), &SDClient::get_registration_info);
	ClassDB::bind_method(D_METHOD("get_devices"), &SDClient::get_devices);
	ClassDB::bind_method(D_METHOD("get_application_info"), &SDClient::get_application_info);
	ClassDB::bind_method(D_METHOD("get_colors"), &SDClient::get_colors);

	// Event subscription
	ClassDB::bind_method(D_METHOD("subscribe_to_event", "event_name", "callback"), &SDClient::subscribe_to_event);
	ClassDB::bind_method(D_METHOD("unsubscribe_from_event", "event_name", "callback"), &SDClient::unsubscribe_from_event);

	// Settings commands
	ClassDB::bind_method(D_METHOD("get_settings", "context"), &SDClient::get_settings);
	ClassDB::bind_method(D_METHOD("set_settings", "context", "settings"), &SDClient::set_settings);
	ClassDB::bind_method(D_METHOD("get_global_settings", "context"), &SDClient::get_global_settings);
	ClassDB::bind_method(D_METHOD("set_global_settings", "context", "settings"), &SDClient::set_global_settings);
	ClassDB::bind_method(D_METHOD("get_secrets", "context"), &SDClient::get_secrets);

	// Visual update commands
	ClassDB::bind_method(D_METHOD("set_image", "context", "image", "state", "target"), &SDClient::set_image, DEFVAL(String()), DEFVAL(-1), DEFVAL(SD_TARGET_HARDWARE_AND_SOFTWARE));
	ClassDB::bind_method(D_METHOD("set_title", "context", "title", "state", "target"), &SDClient::set_title, DEFVAL(String()), DEFVAL(-1), DEFVAL(SD_TARGET_HARDWARE_AND_SOFTWARE));
	ClassDB::bind_method(D_METHOD("set_state", "context", "state"), &SDClient::set_state);

	// Feedback system commands
	ClassDB::bind_method(D_METHOD("set_feedback", "context", "feedback"), &SDClient::set_feedback);
	ClassDB::bind_method(D_METHOD("set_feedback_layout", "context", "layout"), &SDClient::set_feedback_layout);
	ClassDB::bind_method(D_METHOD("set_trigger_description", "context", "rotate", "push", "touch", "long_touch"), &SDClient::set_trigger_description, DEFVAL(String()), DEFVAL(String()), DEFVAL(String()), DEFVAL(String()));

	// UI feedback commands
	ClassDB::bind_method(D_METHOD("show_alert", "context"), &SDClient::show_alert);
	ClassDB::bind_method(D_METHOD("show_ok", "context"), &SDClient::show_ok);

	// Communication commands
	ClassDB::bind_method(D_METHOD("send_to_property_inspector", "action", "context", "payload"), &SDClient::send_to_property_inspector);
	ClassDB::bind_method(D_METHOD("log_message", "message"), &SDClient::log_message);

	// System commands
	ClassDB::bind_method(D_METHOD("open_url", "url"), &SDClient::open_url);
	ClassDB::bind_method(D_METHOD("switch_to_profile", "context", "device", "profile", "page"), &SDClient::switch_to_profile, DEFVAL(String()), DEFVAL(-1));

	// Enums
	BIND_ENUM_CONSTANT(STATE_DISCONNECTED);
	BIND_ENUM_CONSTANT(STATE_CONNECTING);
	BIND_ENUM_CONSTANT(STATE_REGISTERED);

	// Signals - Application monitoring
	ADD_SIGNAL(MethodInfo("application_did_launch", PropertyInfo(Variant::STRING, "application")));
	ADD_SIGNAL(MethodInfo("application_did_terminate", PropertyInfo(Variant::STRING, "application")));

	// Signals - Device events
	ADD_SIGNAL(MethodInfo("device_did_change", PropertyInfo(Variant::STRING, "device"), PropertyInfo(Variant::DICTIONARY, "device_info")));
	ADD_SIGNAL(MethodInfo("device_did_connect", PropertyInfo(Variant::STRING, "device"), PropertyInfo(Variant::DICTIONARY, "device_info")));
	ADD_SIGNAL(MethodInfo("device_did_disconnect", PropertyInfo(Variant::STRING, "device")));

	// Signals - Dial events
	ADD_SIGNAL(MethodInfo("dial_down", PropertyInfo(Variant::STRING, "action"), PropertyInfo(Variant::STRING, "context"), PropertyInfo(Variant::STRING, "device"), PropertyInfo(Variant::DICTIONARY, "payload")));
	ADD_SIGNAL(MethodInfo("dial_rotate", PropertyInfo(Variant::STRING, "action"), PropertyInfo(Variant::STRING, "context"), PropertyInfo(Variant::STRING, "device"), PropertyInfo(Variant::DICTIONARY, "payload")));
	ADD_SIGNAL(MethodInfo("dial_up", PropertyInfo(Variant::STRING, "action"), PropertyInfo(Variant::STRING, "context"), PropertyInfo(Variant::STRING, "device"), PropertyInfo(Variant::DICTIONARY, "payload")));

	// Signals - Deep linking
	ADD_SIGNAL(MethodInfo("did_receive_deep_link", PropertyInfo(Variant::STRING, "url")));

	// Signals - Settings events
	ADD_SIGNAL(MethodInfo("did_receive_global_settings", PropertyInfo(Variant::DICTIONARY, "settings")));
	ADD_SIGNAL(MethodInfo("did_receive_settings", PropertyInfo(Variant::STRING, "action"), PropertyInfo(Variant::STRING, "context"), PropertyInfo(Variant::STRING, "device"), PropertyInfo(Variant::DICTIONARY, "payload")));
	ADD_SIGNAL(MethodInfo("did_receive_secrets", PropertyInfo(Variant::DICTIONARY, "secrets")));

	// Signals - Property inspector events
	ADD_SIGNAL(MethodInfo("did_receive_property_inspector_message", PropertyInfo(Variant::STRING, "action"), PropertyInfo(Variant::STRING, "context"), PropertyInfo(Variant::NIL, "payload")));
	ADD_SIGNAL(MethodInfo("property_inspector_did_appear", PropertyInfo(Variant::STRING, "action"), PropertyInfo(Variant::STRING, "context"), PropertyInfo(Variant::STRING, "device")));
	ADD_SIGNAL(MethodInfo("property_inspector_did_disappear", PropertyInfo(Variant::STRING, "action"), PropertyInfo(Variant::STRING, "context"), PropertyInfo(Variant::STRING, "device")));

	// Signals - Key events
	ADD_SIGNAL(MethodInfo("key_down", PropertyInfo(Variant::STRING, "action"), PropertyInfo(Variant::STRING, "context"), PropertyInfo(Variant::STRING, "device"), PropertyInfo(Variant::DICTIONARY, "payload")));
	ADD_SIGNAL(MethodInfo("key_up", PropertyInfo(Variant::STRING, "action"), PropertyInfo(Variant::STRING, "context"), PropertyInfo(Variant::STRING, "device"), PropertyInfo(Variant::DICTIONARY, "payload")));

	// Signals - Touch events
	ADD_SIGNAL(MethodInfo("touch_tap", PropertyInfo(Variant::STRING, "action"), PropertyInfo(Variant::STRING, "context"), PropertyInfo(Variant::STRING, "device"), PropertyInfo(Variant::DICTIONARY, "payload")));

	// Signals - Action lifecycle events
	ADD_SIGNAL(MethodInfo("will_appear", PropertyInfo(Variant::STRING, "action"), PropertyInfo(Variant::STRING, "context"), PropertyInfo(Variant::STRING, "device"), PropertyInfo(Variant::DICTIONARY, "payload")));
	ADD_SIGNAL(MethodInfo("will_disappear", PropertyInfo(Variant::STRING, "action"), PropertyInfo(Variant::STRING, "context"), PropertyInfo(Variant::STRING, "device"), PropertyInfo(Variant::DICTIONARY, "payload")));

	// Signals - System events
	ADD_SIGNAL(MethodInfo("system_did_wake_up"));

	// Signals - Title events
	ADD_SIGNAL(MethodInfo("title_parameters_did_change", PropertyInfo(Variant::STRING, "action"), PropertyInfo(Variant::STRING, "context"), PropertyInfo(Variant::STRING, "device"), PropertyInfo(Variant::DICTIONARY, "payload")));
}

SDClient::SDClient() {
	singleton = this;
	ws = Ref<WebSocketPeer>(WebSocketPeer::create());
}

SDClient::~SDClient() {
	if (ws.is_valid() && ws->get_ready_state() != WebSocketPeer::STATE_CLOSED) {
		ws->close();
	}
	singleton = nullptr;
}

// Connection Management

Error SDClient::parse_command_line_args() {
	List<String> args = OS::get_singleton()->get_cmdline_args();

	List<String>::Element *I = args.front();
	while (I) {
		String arg = I->get();
		List<String>::Element *N = I->next();

		if (arg == "-port" && N) {
			cmd_port = N->get();
			I = N->next();
			continue;
		} else if (arg == "-pluginUUID" && N) {
			cmd_plugin_uuid = N->get();
			I = N->next();
			continue;
		} else if (arg == "-registerEvent" && N) {
			cmd_register_event = N->get();
			I = N->next();
			continue;
		} else if (arg == "-info" && N) {
			cmd_info = N->get();
			I = N->next();
			continue;
		}

		I = N;
	}

	if (cmd_port.is_empty() || cmd_plugin_uuid.is_empty() || cmd_register_event.is_empty()) {
		return ERR_INVALID_PARAMETER;
	}

	return OK;
}

Error SDClient::connect_from_args() {
	if (cmd_port.is_empty() || cmd_plugin_uuid.is_empty() || cmd_register_event.is_empty()) {
		ERR_PRINT("Stream Deck: Command-line arguments not parsed. Call parse_command_line_args() first.");
		return ERR_UNCONFIGURED;
	}

	int port_num = cmd_port.to_int();
	if (port_num <= 0 || port_num > 65535) {
		ERR_PRINT("Stream Deck: Invalid port number.");
		return ERR_INVALID_PARAMETER;
	}

	// Parse registration info if provided
	if (!cmd_info.is_empty()) {
		JSON json;
		Error err = json.parse(cmd_info);
		if (err == OK) {
			registration_info = json.get_data();
		} else {
			ERR_PRINT("Stream Deck: Failed to parse registration info JSON.");
		}
	}

	return connect_manual(port_num, cmd_plugin_uuid, cmd_register_event);
}

Error SDClient::connect_manual(int p_port, const String &p_uuid, const String &p_register_event) {
	if (connection_state != STATE_DISCONNECTED) {
		ERR_PRINT("Stream Deck: Already connected or connecting.");
		return ERR_ALREADY_IN_USE;
	}

	port = p_port;
	plugin_uuid = p_uuid;
	register_event = p_register_event;

	String url = "ws://127.0.0.1:" + String::num_int64(port);
	Error err = ws->connect_to_url(url);

	if (err != OK) {
		ERR_PRINT("Stream Deck: Failed to connect to WebSocket at " + url);
		return err;
	}

	connection_state = STATE_CONNECTING;
	return OK;
}

void SDClient::disconnect_from_stream_deck() {
	if (ws.is_valid() && ws->get_ready_state() != WebSocketPeer::STATE_CLOSED) {
		ws->close();
	}
	connection_state = STATE_DISCONNECTED;
	contexts.clear();
}

void SDClient::poll() {
	if (!ws.is_valid()) {
		return;
	}

	ws->poll();

	WebSocketPeer::State state = ws->get_ready_state();

	// Handle connection state changes
	if (connection_state == STATE_CONNECTING && state == WebSocketPeer::STATE_OPEN) {
		// Send registration message
		Dictionary reg_msg;
		reg_msg["event"] = register_event;
		reg_msg["uuid"] = plugin_uuid;
		send_message(reg_msg);
		connection_state = STATE_REGISTERED;
	}

	if (state == WebSocketPeer::STATE_CLOSED) {
		if (connection_state != STATE_DISCONNECTED) {
			connection_state = STATE_DISCONNECTED;
		}
		return;
	}

	// Process incoming messages
	while (ws->get_available_packet_count() > 0) {
		const uint8_t *buffer;
		int buffer_size;

		if (ws->get_packet(&buffer, buffer_size) == OK) {
			if (ws->was_string_packet()) {
				String message = String::utf8((const char *)buffer, buffer_size);
				process_message(message);
			}
		}
	}
}

SDClient::ConnectionState SDClient::get_connection_state() const {
	return connection_state;
}

bool SDClient::is_stream_deck_connected() const {
	return connection_state == STATE_REGISTERED && ws.is_valid() && ws->get_ready_state() == WebSocketPeer::STATE_OPEN;
}

// Registration Info Access

Dictionary SDClient::get_registration_info() const {
	return registration_info;
}

Array SDClient::get_devices() const {
	if (registration_info.has("devices")) {
		return registration_info["devices"];
	}
	return Array();
}

Dictionary SDClient::get_application_info() const {
	if (registration_info.has("application")) {
		return registration_info["application"];
	}
	return Dictionary();
}

Dictionary SDClient::get_colors() const {
	if (registration_info.has("colors")) {
		return registration_info["colors"];
	}
	return Dictionary();
}

// Event Subscription

void SDClient::subscribe_to_event(const String &p_event_name, const Callable &p_callback) {
	if (!event_callbacks.has(p_event_name)) {
		event_callbacks[p_event_name] = Vector<Callable>();
	}
	event_callbacks[p_event_name].push_back(p_callback);
}

void SDClient::unsubscribe_from_event(const String &p_event_name, const Callable &p_callback) {
	if (!event_callbacks.has(p_event_name)) {
		return;
	}

	Vector<Callable> &callbacks = event_callbacks[p_event_name];
	for (int i = 0; i < callbacks.size(); i++) {
		if (callbacks[i] == p_callback) {
			callbacks.remove_at(i);
			break;
		}
	}

	if (callbacks.is_empty()) {
		event_callbacks.erase(p_event_name);
	}
}

// Settings Commands

void SDClient::get_settings(const String &p_context) {
	Dictionary msg;
	msg["event"] = "getSettings";
	msg["context"] = p_context;
	send_message(msg);
}

void SDClient::set_settings(const String &p_context, const Dictionary &p_settings) {
	Dictionary msg;
	msg["event"] = "setSettings";
	msg["context"] = p_context;
	msg["payload"] = p_settings;
	send_message(msg);
}

void SDClient::get_global_settings(const String &p_context) {
	Dictionary msg;
	msg["event"] = "getGlobalSettings";
	msg["context"] = p_context;
	send_message(msg);
}

void SDClient::set_global_settings(const String &p_context, const Dictionary &p_settings) {
	Dictionary msg;
	msg["event"] = "setGlobalSettings";
	msg["context"] = p_context;
	msg["payload"] = p_settings;
	send_message(msg);
}

void SDClient::get_secrets(const String &p_context) {
	Dictionary msg;
	msg["event"] = "getSecrets";
	msg["context"] = p_context;
	send_message(msg);
}

// Visual Update Commands

void SDClient::set_image(const String &p_context, const String &p_image, int p_state, int p_target) {
	Dictionary msg;
	msg["event"] = "setImage";
	msg["context"] = p_context;

	Dictionary payload;
	if (!p_image.is_empty()) {
		payload["image"] = p_image;
	}
	if (p_state >= 0) {
		payload["state"] = p_state;
	}
	if (p_target >= 0) {
		payload["target"] = p_target;
	}

	msg["payload"] = payload;
	send_message(msg);
}

void SDClient::set_title(const String &p_context, const String &p_title, int p_state, int p_target) {
	Dictionary msg;
	msg["event"] = "setTitle";
	msg["context"] = p_context;

	Dictionary payload;
	if (!p_title.is_empty()) {
		payload["title"] = p_title;
	}
	if (p_state >= 0) {
		payload["state"] = p_state;
	}
	if (p_target >= 0) {
		payload["target"] = p_target;
	}

	msg["payload"] = payload;
	send_message(msg);
}

void SDClient::set_state(const String &p_context, int p_state) {
	Dictionary msg;
	msg["event"] = "setState";
	msg["context"] = p_context;

	Dictionary payload;
	payload["state"] = p_state;

	msg["payload"] = payload;
	send_message(msg);
}

// Feedback System Commands

void SDClient::set_feedback(const String &p_context, const Dictionary &p_feedback) {
	Dictionary msg;
	msg["event"] = "setFeedback";
	msg["context"] = p_context;
	msg["payload"] = p_feedback;
	send_message(msg);
}

void SDClient::set_feedback_layout(const String &p_context, const String &p_layout) {
	Dictionary msg;
	msg["event"] = "setFeedbackLayout";
	msg["context"] = p_context;

	Dictionary payload;
	payload["layout"] = p_layout;

	msg["payload"] = payload;
	send_message(msg);
}

void SDClient::set_trigger_description(const String &p_context, const String &p_rotate, const String &p_push, const String &p_touch, const String &p_long_touch) {
	Dictionary msg;
	msg["event"] = "setTriggerDescription";
	msg["context"] = p_context;

	Dictionary payload;
	if (!p_rotate.is_empty()) {
		payload["rotate"] = p_rotate;
	}
	if (!p_push.is_empty()) {
		payload["push"] = p_push;
	}
	if (!p_touch.is_empty()) {
		payload["touch"] = p_touch;
	}
	if (!p_long_touch.is_empty()) {
		payload["longTouch"] = p_long_touch;
	}

	msg["payload"] = payload;
	send_message(msg);
}

// UI Feedback Commands

void SDClient::show_alert(const String &p_context) {
	Dictionary msg;
	msg["event"] = "showAlert";
	msg["context"] = p_context;
	send_message(msg);
}

void SDClient::show_ok(const String &p_context) {
	Dictionary msg;
	msg["event"] = "showOk";
	msg["context"] = p_context;
	send_message(msg);
}

// Communication Commands

void SDClient::send_to_property_inspector(const String &p_action, const String &p_context, const Variant &p_payload) {
	Dictionary msg;
	msg["event"] = "sendToPropertyInspector";
	msg["action"] = p_action;
	msg["context"] = p_context;
	msg["payload"] = p_payload;
	send_message(msg);
}

void SDClient::log_message(const String &p_message) {
	Dictionary msg;
	msg["event"] = "logMessage";

	Dictionary payload;
	payload["message"] = p_message;

	msg["payload"] = payload;
	send_message(msg);
}

// System Commands

void SDClient::open_url(const String &p_url) {
	Dictionary msg;
	msg["event"] = "openUrl";

	Dictionary payload;
	payload["url"] = p_url;

	msg["payload"] = payload;
	send_message(msg);
}

void SDClient::switch_to_profile(const String &p_context, const String &p_device, const String &p_profile, int p_page) {
	Dictionary msg;
	msg["event"] = "switchToProfile";
	msg["context"] = p_context;
	msg["device"] = p_device;

	Dictionary payload;
	if (!p_profile.is_empty()) {
		payload["profile"] = p_profile;
	}
	if (p_page >= 0) {
		payload["page"] = p_page;
	}

	msg["payload"] = payload;
	send_message(msg);
}

// Helper Methods

void SDClient::send_message(const Dictionary &p_message) {
	if (!is_stream_deck_connected()) {
		ERR_PRINT("Stream Deck: Cannot send message - not connected.");
		return;
	}

	String json_string = JSON::stringify(p_message);
	ws->send_text(json_string);
}

void SDClient::process_message(const String &p_message) {
	JSON json;
	Error err = json.parse(p_message);

	if (err != OK) {
		ERR_PRINT("Stream Deck: Failed to parse incoming message: " + json.get_error_message());
		return;
	}

	Variant data = json.get_data();
	if (data.get_type() != Variant::DICTIONARY) {
		ERR_PRINT("Stream Deck: Incoming message is not a dictionary.");
		return;
	}

	Dictionary event = data;
	handle_event(event);
}

void SDClient::handle_event(const Dictionary &p_event) {
	if (!p_event.has("event")) {
		ERR_PRINT("Stream Deck: Event missing 'event' field.");
		return;
	}

	String event_name = p_event["event"];

	// Extract common fields
	String action = p_event.has("action") ? String(p_event["action"]) : String();
	String context = p_event.has("context") ? String(p_event["context"]) : String();
	String device = p_event.has("device") ? String(p_event["device"]) : String();
	Dictionary payload = p_event.has("payload") ? Dictionary(p_event["payload"]) : Dictionary();

	// Route to callbacks
	if (event_callbacks.has(event_name)) {
		const Vector<Callable> &callbacks = event_callbacks[event_name];
		for (int i = 0; i < callbacks.size(); i++) {
			callbacks[i].call(p_event);
		}
	}

	// Emit signals based on event type
	if (event_name == "applicationDidLaunch") {
		if (payload.has("application")) {
			emit_signal("application_did_launch", payload["application"]);
		}
	} else if (event_name == "applicationDidTerminate") {
		if (payload.has("application")) {
			emit_signal("application_did_terminate", payload["application"]);
		}
	} else if (event_name == "deviceDidChange") {
		Dictionary device_info = p_event.has("deviceInfo") ? Dictionary(p_event["deviceInfo"]) : Dictionary();
		emit_signal("device_did_change", device, device_info);
	} else if (event_name == "deviceDidConnect") {
		Dictionary device_info = p_event.has("deviceInfo") ? Dictionary(p_event["deviceInfo"]) : Dictionary();
		emit_signal("device_did_connect", device, device_info);
	} else if (event_name == "deviceDidDisconnect") {
		emit_signal("device_did_disconnect", device);
	} else if (event_name == "dialDown") {
		emit_signal("dial_down", action, context, device, payload);
	} else if (event_name == "dialRotate") {
		emit_signal("dial_rotate", action, context, device, payload);
	} else if (event_name == "dialUp") {
		emit_signal("dial_up", action, context, device, payload);
	} else if (event_name == "didReceiveDeepLink") {
		if (payload.has("url")) {
			emit_signal("did_receive_deep_link", payload["url"]);
		}
	} else if (event_name == "didReceiveGlobalSettings") {
		Dictionary settings = payload.has("settings") ? Dictionary(payload["settings"]) : Dictionary();
		emit_signal("did_receive_global_settings", settings);
	} else if (event_name == "didReceiveSettings") {
		emit_signal("did_receive_settings", action, context, device, payload);
	} else if (event_name == "didReceiveSecrets") {
		Dictionary secrets = payload.has("secrets") ? Dictionary(payload["secrets"]) : Dictionary();
		emit_signal("did_receive_secrets", secrets);
	} else if (event_name == "sendToPlugin") {
		// This is the property inspector message event
		Variant pi_payload = p_event.has("payload") ? p_event["payload"] : Variant();
		emit_signal("did_receive_property_inspector_message", action, context, pi_payload);
	} else if (event_name == "propertyInspectorDidAppear") {
		emit_signal("property_inspector_did_appear", action, context, device);
	} else if (event_name == "propertyInspectorDidDisappear") {
		emit_signal("property_inspector_did_disappear", action, context, device);
	} else if (event_name == "keyDown") {
		emit_signal("key_down", action, context, device, payload);
	} else if (event_name == "keyUp") {
		emit_signal("key_up", action, context, device, payload);
	} else if (event_name == "touchTap") {
		emit_signal("touch_tap", action, context, device, payload);
	} else if (event_name == "willAppear") {
		// Store context info
		contexts[context] = p_event;
		emit_signal("will_appear", action, context, device, payload);
	} else if (event_name == "willDisappear") {
		emit_signal("will_disappear", action, context, device, payload);
		// Remove context info
		contexts.erase(context);
	} else if (event_name == "systemDidWakeUp") {
		emit_signal("system_did_wake_up");
	} else if (event_name == "titleParametersDidChange") {
		emit_signal("title_parameters_did_change", action, context, device, payload);
	}
}

String SDClient::event_type_to_string(SDEventType p_type) const {
	switch (p_type) {
		case SD_EVENT_APPLICATION_DID_LAUNCH:
			return "applicationDidLaunch";
		case SD_EVENT_APPLICATION_DID_TERMINATE:
			return "applicationDidTerminate";
		case SD_EVENT_DEVICE_DID_CHANGE:
			return "deviceDidChange";
		case SD_EVENT_DEVICE_DID_CONNECT:
			return "deviceDidConnect";
		case SD_EVENT_DEVICE_DID_DISCONNECT:
			return "deviceDidDisconnect";
		case SD_EVENT_DIAL_DOWN:
			return "dialDown";
		case SD_EVENT_DIAL_ROTATE:
			return "dialRotate";
		case SD_EVENT_DIAL_UP:
			return "dialUp";
		case SD_EVENT_DID_RECEIVE_DEEP_LINK:
			return "didReceiveDeepLink";
		case SD_EVENT_DID_RECEIVE_GLOBAL_SETTINGS:
			return "didReceiveGlobalSettings";
		case SD_EVENT_DID_RECEIVE_SETTINGS:
			return "didReceiveSettings";
		case SD_EVENT_DID_RECEIVE_SECRETS:
			return "didReceiveSecrets";
		case SD_EVENT_DID_RECEIVE_PROPERTY_INSPECTOR_MESSAGE:
			return "sendToPlugin";
		case SD_EVENT_PROPERTY_INSPECTOR_DID_APPEAR:
			return "propertyInspectorDidAppear";
		case SD_EVENT_PROPERTY_INSPECTOR_DID_DISAPPEAR:
			return "propertyInspectorDidDisappear";
		case SD_EVENT_KEY_DOWN:
			return "keyDown";
		case SD_EVENT_KEY_UP:
			return "keyUp";
		case SD_EVENT_TOUCH_TAP:
			return "touchTap";
		case SD_EVENT_WILL_APPEAR:
			return "willAppear";
		case SD_EVENT_WILL_DISAPPEAR:
			return "willDisappear";
		case SD_EVENT_SYSTEM_DID_WAKE_UP:
			return "systemDidWakeUp";
		case SD_EVENT_TITLE_PARAMETERS_DID_CHANGE:
			return "titleParametersDidChange";
		default:
			return "";
	}
}

SDEventType SDClient::string_to_event_type(const String &p_event_name) const {
	if (p_event_name == "applicationDidLaunch") {
		return SD_EVENT_APPLICATION_DID_LAUNCH;
	} else if (p_event_name == "applicationDidTerminate") {
		return SD_EVENT_APPLICATION_DID_TERMINATE;
	} else if (p_event_name == "deviceDidChange") {
		return SD_EVENT_DEVICE_DID_CHANGE;
	} else if (p_event_name == "deviceDidConnect") {
		return SD_EVENT_DEVICE_DID_CONNECT;
	} else if (p_event_name == "deviceDidDisconnect") {
		return SD_EVENT_DEVICE_DID_DISCONNECT;
	} else if (p_event_name == "dialDown") {
		return SD_EVENT_DIAL_DOWN;
	} else if (p_event_name == "dialRotate") {
		return SD_EVENT_DIAL_ROTATE;
	} else if (p_event_name == "dialUp") {
		return SD_EVENT_DIAL_UP;
	} else if (p_event_name == "didReceiveDeepLink") {
		return SD_EVENT_DID_RECEIVE_DEEP_LINK;
	} else if (p_event_name == "didReceiveGlobalSettings") {
		return SD_EVENT_DID_RECEIVE_GLOBAL_SETTINGS;
	} else if (p_event_name == "didReceiveSettings") {
		return SD_EVENT_DID_RECEIVE_SETTINGS;
	} else if (p_event_name == "didReceiveSecrets") {
		return SD_EVENT_DID_RECEIVE_SECRETS;
	} else if (p_event_name == "sendToPlugin") {
		return SD_EVENT_DID_RECEIVE_PROPERTY_INSPECTOR_MESSAGE;
	} else if (p_event_name == "propertyInspectorDidAppear") {
		return SD_EVENT_PROPERTY_INSPECTOR_DID_APPEAR;
	} else if (p_event_name == "propertyInspectorDidDisappear") {
		return SD_EVENT_PROPERTY_INSPECTOR_DID_DISAPPEAR;
	} else if (p_event_name == "keyDown") {
		return SD_EVENT_KEY_DOWN;
	} else if (p_event_name == "keyUp") {
		return SD_EVENT_KEY_UP;
	} else if (p_event_name == "touchTap") {
		return SD_EVENT_TOUCH_TAP;
	} else if (p_event_name == "willAppear") {
		return SD_EVENT_WILL_APPEAR;
	} else if (p_event_name == "willDisappear") {
		return SD_EVENT_WILL_DISAPPEAR;
	} else if (p_event_name == "systemDidWakeUp") {
		return SD_EVENT_SYSTEM_DID_WAKE_UP;
	} else if (p_event_name == "titleParametersDidChange") {
		return SD_EVENT_TITLE_PARAMETERS_DID_CHANGE;
	}

	// Return a default value (first enum value)
	return SD_EVENT_APPLICATION_DID_LAUNCH;
}
