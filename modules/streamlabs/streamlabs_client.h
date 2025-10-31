/**************************************************************************/
/*  streamlabs_client.h                                                   */
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

#pragma once

#include "core/object/object.h"
#include "modules/socketio/socketio_client.h"
#include "modules/socketio/socketio_namespace.h"

class StreamlabsClient : public Object {
	GDCLASS(StreamlabsClient, Object);

	static StreamlabsClient *singleton;

public:
	enum ConnectionState {
		STATE_DISCONNECTED,
		STATE_CONNECTING,
		STATE_CONNECTED
	};

private:
	// Internal Socket.IO client
	Ref<SocketIONamespace> socketio_namespace;
	String socket_token;
	ConnectionState connection_state = STATE_DISCONNECTED;

	// Event handling
	void _on_socketio_connected();
	void _on_socketio_disconnected(const String &p_reason);
	void _on_socketio_connect_error(const Dictionary &p_error);
	void _on_event_received(const String &p_event_name, const Array &p_data);
	void _emit_platform_event(const String &p_platform, const String &p_type, const Dictionary &p_message);

protected:
	static void _bind_methods();

public:
	// Public for testing
	void _handle_streamlabs_event(const Dictionary &p_event_data);
	static StreamlabsClient *get_singleton();

	// Connection management
	Error connect_to_streamlabs(const String &p_socket_token);
	void disconnect_from_streamlabs();
	void poll();

	// Connection info
	bool is_streamlabs_connected() const;
	ConnectionState get_connection_state() const;

	StreamlabsClient();
	~StreamlabsClient();
};

VARIANT_ENUM_CAST(StreamlabsClient::ConnectionState);
