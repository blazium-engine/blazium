/**************************************************************************/
/*  pogr_client.h                                                         */
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

#ifndef POGR_CLIENT_H
#define POGR_CLIENT_H

#include "../blazium_client.h"
#include "core/io/json.h"
#include "core/templates/vector.h"
#include "core/version.h"
#include "main/performance.h"
#include "scene/main/http_request.h"

class POGRClient : public BlaziumClient {
	GDCLASS(POGRClient, BlaziumClient);

private:
	String session_id;
	String POGR_URL = "https://api.pogr.io/v1/intake";
	String POGR_CLIENT = "Blazium";
	String POGR_BUILD = EXTERNAL_VERSION_FULL_BUILD;

	Vector<String> get_init_headers() {
		Vector<String> headers;
		headers.append("POGR_CLIENT: " + POGR_CLIENT);
		headers.append("POGR_BUILD: " + POGR_BUILD);
		return headers;
	}

	Vector<String> get_session_headers() {
		Vector<String> headers;
		headers.append("INTAKE_SESSION_ID: " + session_id);
		return headers;
	}

protected:
	static void _bind_methods() {
		ClassDB::bind_method(D_METHOD("init"), &POGRClient::init);
		ClassDB::bind_method(D_METHOD("data", "data"), &POGRClient::data);
		ClassDB::bind_method(D_METHOD("event", "event_name", "event_data", "event_flag", "event_key", "event_type", "event_sub_type"), &POGRClient::event);
		ClassDB::bind_method(D_METHOD("logs", "tags", "data", "environment", "log", "service", "severity", "type"), &POGRClient::logs);
		ClassDB::bind_method(D_METHOD("metrics", "tags", "environment", "metrics", "service"), &POGRClient::metrics);
		ClassDB::bind_method(D_METHOD("monitor", "settings"), &POGRClient::monitor);

		ClassDB::bind_method(D_METHOD("get_client_id"), &POGRClient::get_client_id);
		ClassDB::bind_method(D_METHOD("get_build_id"), &POGRClient::get_build_id);
		ClassDB::bind_method(D_METHOD("get_pogr_url"), &POGRClient::get_pogr_url);
		ClassDB::bind_method(D_METHOD("get_session_id"), &POGRClient::get_session_id);
	}

public:
	class POGRResult : public RefCounted {
		GDCLASS(POGRResult, RefCounted);
		String error;
		String result;

	protected:
		static void _bind_methods() {
			ClassDB::bind_method(D_METHOD("has_error"), &POGRResult::has_error);
			ClassDB::bind_method(D_METHOD("get_error"), &POGRResult::get_error);
			ClassDB::bind_method(D_METHOD("get_result"), &POGRResult::get_result);
			ADD_PROPERTY(PropertyInfo(Variant::STRING, "error"), "", "get_error");
			ADD_PROPERTY(PropertyInfo(Variant::STRING, "result"), "", "get_result");
		}

	public:
		void set_error(String p_error) { error = p_error; }
		void set_result(String p_result) { result = p_result; }

		bool has_error() const { return error != ""; }
		String get_error() const { return error; }
		String get_result() const { return result; }
	};
	class POGRResponse : public RefCounted {
		GDCLASS(POGRResponse, RefCounted);
		HTTPRequest *request;

	protected:
		static void _bind_methods() {
			ADD_SIGNAL(MethodInfo("finished", PropertyInfo(Variant::OBJECT, "result", PROPERTY_HINT_RESOURCE_TYPE, "POGRResult")));
		}

	public:
		void _on_request_completed(int p_status, int p_code, const PackedStringArray &p_headers, const PackedByteArray &p_data) {
			Ref<POGRResult> result;
			result.instantiate();
			String result_str = String::utf8((const char *)p_data.ptr(), p_data.size());
			if (p_code != 200 || result_str == "") {
				result->set_error("Request failed with code: " + String::num(p_code) + " " + result_str);
			} else {
				if (result_str != "") {
					Dictionary result_dict = JSON::parse_string(result_str);
					if (!result_dict.get("success", false)) {
						result->set_error("Request failed with code: " + String::num(p_code) + " " + result_str);
					} else {
						result->set_result(result_str);
					}
				}
			}
			emit_signal(SNAME("finished"), result);
		}
		void post_request(String p_url, Vector<String> p_headers, Dictionary p_data, POGRClient *p_client) {
			request = memnew(HTTPRequest);
			p_client->add_child(request);
			request->connect("request_completed", callable_mp(this, &POGRResponse::_on_request_completed));
			request->request(p_url, p_headers, HTTPClient::METHOD_POST, JSON::stringify(p_data));
		}
	};

	Ref<POGRResponse> init() {
		Ref<POGRResponse> response;
		response.instantiate();
		response->connect("finished", callable_mp(this, &POGRClient::init_finished));
		Dictionary dict_data;
		dict_data["association_id"] = OS::get_singleton()->get_unique_id();
		response->post_request(POGR_URL + "/init", get_init_headers(), dict_data, this);
		return response;
	}

	void init_finished(Ref<POGRResult> result) {
		String result_str = result->get_result();
		if (result.is_valid() && result_str != "") {
			Dictionary result_dict = JSON::parse_string(result_str);
			Dictionary payload = result_dict.get("payload", Dictionary());
			set_session_id(payload.get("session_id", ""));
		}
	}

	Ref<POGRResponse> data(Dictionary p_data) {
		Ref<POGRResponse> response;
		response.instantiate();
		response->post_request(POGR_URL + "/data", get_session_headers(), p_data, this);
		return response;
	}

	Ref<POGRResponse> end() {
		Ref<POGRResponse> response;
		response.instantiate();
		response->post_request(POGR_URL + "/end", get_session_headers(), Dictionary(), this);
		return response;
	}

	Ref<POGRResponse> event(String event_name, Dictionary event_data, String event_flag, String event_key, String event_type, String event_sub_type) {
		Ref<POGRResponse> response;
		response.instantiate();
		Dictionary data;
		data["event"] = event_name;
		data["event_data"] = event_data;
		data["event_flag"] = event_flag;
		data["event_key"] = event_key;
		data["event_type"] = event_type;
		data["sub_event"] = event_sub_type;
		response->post_request(POGR_URL + "/end", get_session_headers(), data, this);
		return response;
	}

	Ref<POGRResponse> logs(Dictionary p_tags, Dictionary p_data, String p_environment, String p_log, String p_service, String p_severity, String p_type) {
		Ref<POGRResponse> response;
		response.instantiate();
		Dictionary data;
		data["tags"] = p_tags;
		data["data"] = p_data;
		data["environment"] = p_environment;
		data["log"] = p_log;
		data["service"] = p_service;
		data["severity"] = p_severity;
		data["type"] = p_type;
		response->post_request(POGR_URL + "/logs", get_session_headers(), data, this);
		return response;
	}

	Ref<POGRResponse> metrics(Dictionary p_tags, String p_environment, Dictionary p_metrics, String p_service) {
		Ref<POGRResponse> response;
		response.instantiate();
		Dictionary data;
		data["tags"] = p_tags;
		data["environment"] = p_environment;
		data["metrics"] = p_metrics;
		data["service"] = p_service;
		response->post_request(POGR_URL + "/logs", get_session_headers(), data, this);
		return response;
	}

	Ref<POGRResponse> monitor(Dictionary p_settings) {
		Ref<POGRResponse> response;
		response.instantiate();
		Dictionary data;
		data["settings"] = p_settings;
		data["cpu_usage"] = Performance::get_singleton()->get_monitor(Performance::Monitor::TIME_FPS);
		data["dlls_loaded"] = Array();
		data["memory_usage"] = OS::get_singleton()->get_static_memory_usage();
		response->post_request(POGR_URL + "/logs", get_session_headers(), data, this);
		return response;
	}

	String get_client_id() const { return POGR_CLIENT; }
	String get_build_id() const { return POGR_BUILD; }
	String get_pogr_url() const { return POGR_URL; }
	String get_session_id() {
		return session_id;
	}
	void set_session_id(String p_session_id) {
		if (p_session_id == "") {
			return;
		}
		session_id = p_session_id;
	}
};

#endif // POGR_CLIENT_H
