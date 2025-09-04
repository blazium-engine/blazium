/**************************************************************************/
/*  godot_sqlite.h                                                        */
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

#include "../thirdparty/spmemvfs/spmemvfs.h"
#include "../thirdparty/sqlite/sqlite3.h"
#include "core/object/ref_counted.h"
#include "core/templates/local_vector.h"
#include "core/variant/typed_array.h"

class SQLiteColumnSchema : public RefCounted {
	GDCLASS(SQLiteColumnSchema, RefCounted);
	String name;
	Variant::Type type;
	Variant default_value;
	bool primary_key = false;
	bool auto_increment = false;
	bool not_null = false;
	bool unique = false;

protected:
	static void _bind_methods() {
		ClassDB::bind_method(D_METHOD("get_name"), &SQLiteColumnSchema::get_name);
		ClassDB::bind_method(D_METHOD("get_type"), &SQLiteColumnSchema::get_type);
		ClassDB::bind_method(D_METHOD("get_default_value"), &SQLiteColumnSchema::get_default_value);
		ClassDB::bind_method(D_METHOD("is_primary_key"), &SQLiteColumnSchema::is_primary_key);
		ClassDB::bind_method(D_METHOD("is_auto_increment"), &SQLiteColumnSchema::is_auto_increment);
		ClassDB::bind_method(D_METHOD("is_not_null"), &SQLiteColumnSchema::is_not_null);
		ClassDB::bind_method(D_METHOD("is_unique"), &SQLiteColumnSchema::is_unique);

		ClassDB::bind_method(D_METHOD("set_name", "name"), &SQLiteColumnSchema::set_name);
		ClassDB::bind_method(D_METHOD("set_type", "type"), &SQLiteColumnSchema::set_type);
		ClassDB::bind_method(D_METHOD("set_default_value", "default_value"), &SQLiteColumnSchema::set_default_value);
		ClassDB::bind_method(D_METHOD("set_primary_key", "primary_key"), &SQLiteColumnSchema::set_primary_key);
		ClassDB::bind_method(D_METHOD("set_auto_increment", "auto_increment"), &SQLiteColumnSchema::set_auto_increment);
		ClassDB::bind_method(D_METHOD("set_not_null", "not_null"), &SQLiteColumnSchema::set_not_null);
		ClassDB::bind_method(D_METHOD("set_unique", "unique"), &SQLiteColumnSchema::set_unique);

		ADD_PROPERTY(PropertyInfo(Variant::STRING, "name"), "set_name", "get_name");
		ADD_PROPERTY(PropertyInfo(Variant::INT, "type"), "set_type", "get_type");
		ADD_PROPERTY(PropertyInfo(Variant::STRING, "default_value"), "set_default_value", "get_default_value");
		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "primary_key"), "set_primary_key", "is_primary_key");
		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "auto_increment"), "set_auto_increment", "is_auto_increment");
		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "not_null"), "set_not_null", "is_not_null");
		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "unique"), "set_unique", "is_unique");

		ClassDB::bind_static_method("SQLiteColumnSchema", D_METHOD("create", "name", "type", "default_value", "primary_key", "auto_increment", "not_null", "unique"), &SQLiteColumnSchema::create, DEFVAL(Variant::Type::STRING), DEFVAL(Variant()), DEFVAL(false), DEFVAL(false), DEFVAL(false), DEFVAL(false));
	}

public:
	static Ref<SQLiteColumnSchema> create(const String &p_name, Variant::Type p_type, const Variant &p_default_value, bool p_primary_key, bool p_auto_increment, bool p_not_null, bool p_unique) {
		Ref<SQLiteColumnSchema> schema;
		schema.instantiate();
		schema->set_name(p_name);
		schema->set_type(p_type);
		schema->set_default_value(p_default_value);
		schema->set_primary_key(p_primary_key);
		schema->set_auto_increment(p_auto_increment);
		schema->set_not_null(p_not_null);
		schema->set_unique(p_unique);
		return schema;
	}
	String get_name() const { return name; }
	Variant::Type get_type() const { return type; }
	Variant get_default_value() const { return default_value; }
	bool is_primary_key() const { return primary_key; }
	bool is_auto_increment() const { return auto_increment; }
	bool is_not_null() const { return not_null; }
	bool is_unique() const { return unique; }

	void set_name(const String &p_name) { name = p_name; }
	void set_type(Variant::Type p_type) { type = p_type; }
	void set_default_value(const Variant &p_default_value) { default_value = p_default_value; }
	void set_primary_key(bool p_primary_key) { primary_key = p_primary_key; }
	void set_auto_increment(bool p_auto_increment) { auto_increment = p_auto_increment; }
	void set_not_null(bool p_not_null) { not_null = p_not_null; }
	void set_unique(bool p_unique) { unique = p_unique; }
};

class SQLiteAccess;

class SQLiteQueryResult : public RefCounted {
	GDCLASS(SQLiteQueryResult, RefCounted);
	TypedArray<Dictionary> result;
	Array arguments;
	String query;
	String error;
	int error_code = 0;

protected:
	static void _bind_methods() {
		ClassDB::bind_method(D_METHOD("get_result"), &SQLiteQueryResult::get_result);
		ClassDB::bind_method(D_METHOD("get_error"), &SQLiteQueryResult::get_error);
		ClassDB::bind_method(D_METHOD("get_error_code"), &SQLiteQueryResult::get_error_code);
		ClassDB::bind_method(D_METHOD("get_query"), &SQLiteQueryResult::get_query);
		ClassDB::bind_method(D_METHOD("get_arguments"), &SQLiteQueryResult::get_arguments);

		ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "result", PROPERTY_HINT_ARRAY_TYPE, "Dictionary"), "", "get_result");
		ADD_PROPERTY(PropertyInfo(Variant::STRING, "error"), "", "get_error");
		ADD_PROPERTY(PropertyInfo(Variant::INT, "error_code"), "", "get_error_code");
		ADD_PROPERTY(PropertyInfo(Variant::STRING, "query"), "", "get_query");
		ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "arguments"), "", "get_arguments");
	}

public:
	Array get_arguments() const { return arguments; }
	TypedArray<Dictionary> get_result() const { return result; }
	String get_error() const { return error; }
	int get_error_code() const { return error_code; }
	String get_query() const { return query; }

	void set_result(TypedArray<Dictionary> p_result) { result = p_result; }
	void set_error(String p_error) { error = p_error; }
	void set_error_code(int p_error_code) { error_code = p_error_code; }
	void set_query(String p_query) { query = p_query; }
	void set_arguments(Array p_arguments) { arguments = p_arguments; }
};

class SQLiteQuery : public RefCounted {
	GDCLASS(SQLiteQuery, RefCounted);

	Array arguments;
	SQLiteAccess *db = nullptr;
	sqlite3_stmt *stmt = nullptr;
	String query;

protected:
	static void _bind_methods();

public:
	SQLiteQuery();
	~SQLiteQuery();
	void init(SQLiteAccess *p_db, const String &p_query, Array p_args);
	bool is_ready() const;
	String get_query() const { return query; }
	String get_last_error_message() const;
	Array get_arguments() const { return arguments; }
	void set_arguments(Array p_arguments) { arguments = p_arguments; }
	TypedArray<SQLiteColumnSchema> get_columns();
	void finalize();
	Ref<SQLiteQueryResult> execute(const Array p_args);
	TypedArray<SQLiteQueryResult> batch_execute(TypedArray<Array> p_rows);

private:
	bool prepare();
};

class SQLiteAccess : public RefCounted {
	GDCLASS(SQLiteAccess, RefCounted);

	friend SQLiteQuery;

private:
	sqlite3 *db = nullptr;
	spmemvfs_db_t spmemvfs_db{};
	bool memory_read = false;

	::LocalVector<WeakRef *, uint32_t, true> queries;

	sqlite3_stmt *prepare(const char *statement);
	Array fetch_rows(const String &query, const Array &args, int result_type = RESULT_BOTH);
	sqlite3 *get_handler() const { return memory_read ? spmemvfs_db.handle : db; }

public:
	static String bind_args(sqlite3_stmt *stmt, const Array &args);

protected:
	static void _bind_methods();

public:
	enum { RESULT_BOTH = 0,
		RESULT_NUM,
		RESULT_ASSOC };

	SQLiteAccess();
	~SQLiteAccess();

	bool open(const String &path);
	bool open_in_memory();
	bool open_buffered(const String &name, const PackedByteArray &buffers, int64_t size);
	bool backup(const String &path);
	bool close();

	Ref<SQLiteQuery> create_query(String p_query, Array p_args = Array());

	String get_last_error_message() const;
	int get_last_error_code() const;
};
