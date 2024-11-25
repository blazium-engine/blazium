#ifndef BLAZIUM_SDK_POGR_CLIENT_H
#define BLAZIUM_SDK_POGR_CLIENT_H

#include "scene/main/node.h"
#include "core/io/json.h"

class POGRClient : public Node {
    GDCLASS(POGRClient, Node);

private:
    String session_id;
    String POGR_CLIENT = "pogr_client";
    String POGR_BUILD = "pogr_build";

protected:
    static void _bind_methods() {
        ClassDB::bind_method(D_METHOD("data", "data"), &POGRClient::data);
        ClassDB::bind_method(D_METHOD("end"), &POGRClient::end);
        ClassDB::bind_method(D_METHOD("event", "event_data"), &POGRClient::event);
        ClassDB::bind_method(D_METHOD("init"), &POGRClient::init);
        ClassDB::bind_method(D_METHOD("logs"), &POGRClient::logs);
        ClassDB::bind_method(D_METHOD("metrics"), &POGRClient::metrics);
        ClassDB::bind_method(D_METHOD("monitor"), &POGRClient::monitor);

        ADD_PROPERTY(PropertyInfo(Variant::STRING, "POGR_CLIENT", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_CLASS_CONSTANT), "", "");
        ADD_PROPERTY(PropertyInfo(Variant::STRING, "POGR_BUILD", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_CLASS_CONSTANT), "", "");
    }

public:
    class POGRResponse : public RefCounted {
        GDCLASS(POGRResponse, RefCounted);

        String error;
        String result;

    protected:
        static void _bind_methods() {
            ClassDB::bind_method(D_METHOD("get_error"), &POGRResponse::get_error);
            ClassDB::bind_method(D_METHOD("get_result"), &POGRResponse::get_result);
            ADD_PROPERTY(PropertyInfo(Variant::STRING, "error"), "", "get_error");
            ADD_PROPERTY(PropertyInfo(Variant::STRING, "result"), "", "get_result");
        }

    public:
        void set_error(String p_error) { error = p_error; }
        void set_result(String p_result) { result = p_result; }

        String get_error() const { return error; }
        String get_result() const { return result; }

        POGRResponse() {}
    };

    String data(Dictionary data) {
        // Generate and return a unique data ID (example logic)
        return String::num_int64(OS::get_singleton()->get_unix_time() + data.hash());
    }

    void end() {
        // End the session logic
        session_id = "";
    }

    void event(Dictionary event_data) {
        // Example event structure (you can further process event_data)
        Dictionary event = {
            {"event", "player_login"},
            {"event_data", event_data},
            {"event_flag", "completed"},
            {"event_key", "level_5_unlocked"},
            {"event_type", "achievement"},
            {"sub_event", "level_up"},
        };
    }

    Ref<Response> init() {
        // Initialize the session and return a response object
        Ref<Response> response;
        response.instantiate();
        session_id = "blazium_" + OS::get_singleton()->get_unique_id();
        response->set_result(session_id);
        return response;
    }

    Dictionary logs() {
        return {
            {"tags", Dictionary()},
            {"data", Dictionary()},
            {"environment", "live"},
            {"log", "User authentication successful"},
            {"service", "authentication"},
            {"severity", "info"},
            {"type", "user-authentication"},
        };
    }

    Dictionary metrics() {
        return {
            {"tags", Dictionary()},
            {"environment", "production"},
            {"metrics", Dictionary()},
            {"service", "game_server"},
        };
    }

    Dictionary monitor() {
        return {
            {"cpu_usage", OS::get_singleton()->get_processor_usage(0)},
            {"dlls_loaded", Array()},
            {"memory_usage", OS::get_singleton()->get_static_memory_usage()},
            {"settings", Dictionary()},
        };
    }

    POGRClient() {}
    ~POGRClient() {}
};

#endif // BLAZIUM_SDK_POGR_CLIENT_H