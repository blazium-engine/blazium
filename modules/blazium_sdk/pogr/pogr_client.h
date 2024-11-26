#ifndef BLAZIUM_SDK_POGR_CLIENT_H
#define BLAZIUM_SDK_POGR_CLIENT_H

#include "../blazium_client.h"
#include "scene/main/http_request.h"
#include "core/version.h"
#include "core/templates/vector.h"
#include "core/io/json.h"
#include "main/performance.h"

class POGRClient : public BlaziumClient {
    GDCLASS(POGRClient, BlaziumClient);

private:
    String session_id;
    String POGR_URL = "https://api.pogr.io/v1";
    String POGR_CLIENT = "Blazium";
    String POGR_BUILD = VERSION_FULL_BUILD;

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
        ClassDB::bind_method(D_METHOD("data", "data"), &POGRClient::data);
        ClassDB::bind_method(D_METHOD("end"), &POGRClient::end);
        ClassDB::bind_method(D_METHOD("event", "event_data", "event_flag", "event_key", "event_type", "event_sub_type"), &POGRClient::event);
        ClassDB::bind_method(D_METHOD("init"), &POGRClient::init);
        ClassDB::bind_method(D_METHOD("logs"), &POGRClient::logs);
        ClassDB::bind_method(D_METHOD("metrics"), &POGRClient::metrics);
        ClassDB::bind_method(D_METHOD("monitor"), &POGRClient::monitor);

        ClassDB::bind_method(D_METHOD("get_client_id"), &POGRClient::get_client_id);
        ClassDB::bind_method(D_METHOD("get_build_id"), &POGRClient::get_build_id);
        ClassDB::bind_method(D_METHOD("get_pogr_url"), &POGRClient::get_pogr_url);
        ClassDB::bind_method(D_METHOD("get_session_id"), &POGRClient::get_session_id);

        ADD_PROPERTY(PropertyInfo(Variant::STRING, "client_id", PROPERTY_HINT_NONE, ""), "", "get_client_id");
        ADD_PROPERTY(PropertyInfo(Variant::STRING, "build_id", PROPERTY_HINT_NONE, ""), "", "get_build_id");
        ADD_PROPERTY(PropertyInfo(Variant::STRING, "pogr_url", PROPERTY_HINT_NONE, ""), "", "get_pogr_url");
        ADD_PROPERTY(PropertyInfo(Variant::STRING, "session_id", PROPERTY_HINT_NONE, ""), "", "get_session_id");
    }

public:
    class POGRResult: public RefCounted {
        GDCLASS(POGRResult, RefCounted);
        String error;
        String result;
        Ref<POGRClient> client;

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
        Ref<POGRClient> client;
        Ref<HTTPRequest> request;
        Ref<POGRResult> result;
    protected:
        static void _bind_methods() {
            ClassDB::bind_method(D_METHOD("_on_request_completed", "status", "code", "headers", "data"), &POGRResponse::_on_request_completed);
            ADD_SIGNAL(MethodInfo("finished", PropertyInfo(Variant::OBJECT, "result", PROPERTY_HINT_RESOURCE_TYPE, "POGRResult")));
        }

    public:
        void _on_request_completed(int p_status, int p_code, const PackedStringArray &p_headers, const PackedByteArray &p_data) {
            Ref<POGRResult> result;
            result.instantiate();
            String result_str = String::utf8((const char *)p_data.ptr(), p_data.size());
            if (p_status != 200) {
                result->set_error("Request failed with status code: " + String::num(p_status));
            } else {
                result->set_result(result_str);
            }
            if (client.is_valid()) {
                Dictionary result = JSON::parse_string(result_str);
                Dictionary payload = result.get("payload", Dictionary());
                client->set_session_id(payload.get("session_id", ""));
            }
            emit_signal(SNAME("finished"), result);
        }
        void set_pogr_client(Ref<POGRClient> p_client) { client = p_client; }
        void post_request(String url, Vector<String> headers, Dictionary data) {
            request->request(url, headers, HTTPClient::METHOD_POST, JSON::stringify(data));
        }
        POGRResponse() {
            request.instantiate();
            request->connect("request_completed", Callable(this, "_on_request_completed"));
            result.instantiate();
        }
    };

    Ref<POGRResponse> init() {
        Ref<POGRResponse> response;
        response.instantiate();
        response->set_pogr_client(this);
        response->post_request(POGR_URL + "/init", get_init_headers(), Dictionary());
        return response;
    }

    Ref<POGRResponse> data(Dictionary p_data) {
        Ref<POGRResponse> response;
        response.instantiate();
        response->set_pogr_client(this);
        response->post_request(POGR_URL + "/data", get_session_headers(), p_data);
        return response;
    }


    Ref<POGRResponse> end() {
        Ref<POGRResponse> response;
        response.instantiate();
        response->set_pogr_client(this);
        response->post_request(POGR_URL + "/end", get_session_headers(), Dictionary());
        return response;
    }

    Ref<POGRResponse> event(String event_name, Dictionary event_data, String event_flag, String event_key, String event_type, String event_sub_type) {
        Ref<POGRResponse> response;
        Dictionary data;
        data["event"] = event_name;
        data["event_data"] = event_data;
        data["event_flag"] = event_flag;
        data["event_key"] = event_key;
        data["event_type"] = event_type;
        data["sub_event"] = event_sub_type;
        response.instantiate();
        response->set_pogr_client(this);
        response->post_request(POGR_URL + "/end", get_session_headers(), data);
        return response;
    }

    Ref<POGRResponse> logs(Dictionary p_tags, Dictionary p_data, String p_environment, String p_log, String p_service, String p_severity, String p_type) {
        Ref<POGRResponse> response;
        Dictionary data;
        data["tags"] = p_tags;
        data["data"] = p_data;
        data["environment"] = p_environment;
        data["log"] = p_log;
        data["service"] = p_service;
        data["severity"] = p_severity;
        data["type"] = p_type;
        response.instantiate();
        response->set_pogr_client(this);
        response->post_request(POGR_URL + "/logs", get_session_headers(), data);
        return response;
    }

    Ref<POGRResponse> metrics(Dictionary p_tags, String p_environment, Dictionary p_metrics, String p_service) {
        Ref<POGRResponse> response;
        Dictionary data;
        data["tags"] = p_tags;
        data["environment"] = p_environment;
        data["metrics"] = p_metrics;
        data["service"] = p_service;
        response.instantiate();
        response->set_pogr_client(this);
        response->post_request(POGR_URL + "/logs", get_session_headers(), data);
        return response;
    }

    Ref<POGRResponse> monitor(Dictionary p_settings) {
        Ref<POGRResponse> response;
        Dictionary data;
        data["settings"] = p_settings;
        data["cpu_usage"] = Performance::get_singleton()->get_monitor(Performance::Monitor::TIME_FPS);
        data["dlls_loaded"] = Array();
        data["memory_usage"] = OS::get_singleton()->get_static_memory_usage();
        response.instantiate();
        response->set_pogr_client(this);
        response->post_request(POGR_URL + "/logs", get_session_headers(), data);
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
    POGRClient() {}
    ~POGRClient() {}
};

#endif // BLAZIUM_SDK_POGR_CLIENT_H