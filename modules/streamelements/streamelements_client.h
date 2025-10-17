/**************************************************************************/
/*  streamelements_client.h                                               */
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
#include "core/templates/hash_map.h"
#include "modules/websocket/websocket_peer.h"

class StreamElementsClient : public Object {
	GDCLASS(StreamElementsClient, Object);

	static StreamElementsClient *singleton;

public:
	enum ConnectionState {
		STATE_DISCONNECTED,
		STATE_CONNECTING,
		STATE_CONNECTED,
		STATE_RECONNECTING
	};

	enum TokenType {
		TOKEN_JWT,
		TOKEN_APIKEY,
		TOKEN_OAUTH
	};

private:
	// WebSocket connection
	Ref<WebSocketPeer> ws;
	ConnectionState connection_state = STATE_DISCONNECTED;

	// Authentication
	String auth_token;
	TokenType token_type = TOKEN_JWT;

	// Subscription management
	HashMap<String, bool> subscribed_topics; // topic -> is_subscribed
	HashMap<String, Dictionary> pending_requests; // nonce -> request_data

	// Reconnection logic
	bool auto_reconnect = true;
	float reconnect_delay_current = 1.0;
	float reconnect_delay_min = 1.0;
	float reconnect_delay_max = 30.0;
	float time_since_disconnect = 0.0;

	// Helper methods
	void _process_websocket_messages();
	void _handle_message(const Dictionary &p_message);
	void _handle_response(const Dictionary &p_response);
	void _handle_notification(const Dictionary &p_notification);
	void _emit_specific_signals(const String &p_topic, const Dictionary &p_data);
	String _generate_nonce();
	Error _send_request(const String &p_type, const Dictionary &p_data);
	void _attempt_reconnect(float p_delta);
	void _reset_reconnect_delay();
	void _resubscribe_all();

protected:
	static void _bind_methods();

public:
	static StreamElementsClient *get_singleton();

	// Connection management
	Error connect_to_service(const String &p_token, TokenType p_token_type = TOKEN_JWT);
	void close();
	void poll();

	// Subscription management
	Error subscribe(const String &p_topic, const String &p_room = "");
	Error unsubscribe(const String &p_topic, const String &p_room = "");
	Array get_subscribed_topics() const;

	// Connection info
	bool is_service_connected() const;
	ConnectionState get_connection_state() const;

	// Reconnection settings
	void set_auto_reconnect(bool p_enabled);
	bool get_auto_reconnect() const;
	void set_reconnect_delay_min(float p_seconds);
	float get_reconnect_delay_min() const;
	void set_reconnect_delay_max(float p_seconds);
	float get_reconnect_delay_max() const;

	StreamElementsClient();
	~StreamElementsClient();
};

VARIANT_ENUM_CAST(StreamElementsClient::ConnectionState);
VARIANT_ENUM_CAST(StreamElementsClient::TokenType);

