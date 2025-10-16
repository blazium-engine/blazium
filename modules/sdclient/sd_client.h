/**************************************************************************/
/*  sd_client.h                                                           */
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

#pragma once

#include "core/object/object.h"
#include "core/templates/hash_map.h"
#include "core/templates/vector.h"
#include "modules/websocket/websocket_peer.h"
#include "sd_enums.h"

class SDClient : public Object {
	GDCLASS(SDClient, Object);

	static SDClient *singleton;

public:
	enum ConnectionState {
		STATE_DISCONNECTED,
		STATE_CONNECTING,
		STATE_REGISTERED,
	};

private:
	// WebSocket connection
	Ref<WebSocketPeer> ws;
	ConnectionState connection_state = STATE_DISCONNECTED;

	// Connection parameters
	int port = 0;
	String plugin_uuid;
	String register_event;
	Dictionary registration_info;

	// Command-line args storage
	String cmd_port;
	String cmd_plugin_uuid;
	String cmd_register_event;
	String cmd_info;

	// Context tracking (action instances)
	HashMap<String, Dictionary> contexts;

	// Event callbacks (by event name)
	HashMap<String, Vector<Callable>> event_callbacks;

	// Helper methods
	void send_message(const Dictionary &p_message);
	void process_message(const String &p_message);
	void handle_event(const Dictionary &p_event);
	String event_type_to_string(SDEventType p_type) const;
	SDEventType string_to_event_type(const String &p_event_name) const;

protected:
	static void _bind_methods();

public:
	static SDClient *get_singleton();

	// Connection management (dual mode)
	Error parse_command_line_args();
	Error connect_from_args();
	Error connect_manual(int p_port, const String &p_uuid, const String &p_register_event);
	void disconnect_from_stream_deck();
	void poll();
	ConnectionState get_connection_state() const;
	bool is_stream_deck_connected() const;

	// Registration info access
	Dictionary get_registration_info() const;
	Array get_devices() const;
	Dictionary get_application_info() const;
	Dictionary get_colors() const;

	// Event subscription (callback system)
	void subscribe_to_event(const String &p_event_name, const Callable &p_callback);
	void unsubscribe_from_event(const String &p_event_name, const Callable &p_callback);

	// Settings commands
	void get_settings(const String &p_context);
	void set_settings(const String &p_context, const Dictionary &p_settings);
	void get_global_settings(const String &p_context);
	void set_global_settings(const String &p_context, const Dictionary &p_settings);
	void get_secrets(const String &p_context);

	// Visual update commands
	void set_image(const String &p_context, const String &p_image = String(), int p_state = -1, int p_target = SD_TARGET_HARDWARE_AND_SOFTWARE);
	void set_title(const String &p_context, const String &p_title = String(), int p_state = -1, int p_target = SD_TARGET_HARDWARE_AND_SOFTWARE);
	void set_state(const String &p_context, int p_state);

	// Feedback system commands
	void set_feedback(const String &p_context, const Dictionary &p_feedback);
	void set_feedback_layout(const String &p_context, const String &p_layout);
	void set_trigger_description(const String &p_context, const String &p_rotate = String(), const String &p_push = String(), const String &p_touch = String(), const String &p_long_touch = String());

	// UI feedback commands
	void show_alert(const String &p_context);
	void show_ok(const String &p_context);

	// Communication commands
	void send_to_property_inspector(const String &p_action, const String &p_context, const Variant &p_payload);
	void log_message(const String &p_message);

	// System commands
	void open_url(const String &p_url);
	void switch_to_profile(const String &p_context, const String &p_device, const String &p_profile = String(), int p_page = -1);

	// Constructor/Destructor
	SDClient();
	~SDClient();
};

VARIANT_ENUM_CAST(SDClient::ConnectionState);
