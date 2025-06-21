/**************************************************************************/
/*  resource_sqlite.cpp                                                   */
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

#include "resource_sqlite.h"

void SQLiteDatabase::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_sqlite"), &SQLiteDatabase::get_sqlite);
	ClassDB::bind_method(D_METHOD("create_table", "table_name", "columns"), &SQLiteDatabase::create_table);
	ClassDB::bind_method(D_METHOD("drop_table", "table_name"), &SQLiteDatabase::drop_table);
	ClassDB::bind_method(D_METHOD("create_query", "query", "arguments"), &SQLiteDatabase::create_query);
	ClassDB::bind_method(D_METHOD("execute_query", "query", "arguments"), &SQLiteDatabase::execute_query);
	ClassDB::bind_method(D_METHOD("get_columns", "table_name"), &SQLiteDatabase::get_columns);
	ClassDB::bind_method(D_METHOD("insert_row", "table_name", "value"), &SQLiteDatabase::insert_row);
	ClassDB::bind_method(D_METHOD("insert_rows", "table_name", "values"), &SQLiteDatabase::insert_rows);
	ClassDB::bind_method(D_METHOD("delete_rows", "table_name", "condition"), &SQLiteDatabase::delete_rows, DEFVAL(String()));
	ClassDB::bind_method(D_METHOD("select_rows", "table_name", "condition"), &SQLiteDatabase::select_rows, DEFVAL(String()));
	ClassDB::bind_method(D_METHOD("get_tables"), &SQLiteDatabase::get_tables);

	ClassDB::bind_method(D_METHOD("set_data", "data"), &SQLiteDatabase::set_data);

	ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "tables"), "", "get_tables");
}

void SQLiteDatabase::set_resource(const String &p_path) {
	db->open(p_path);
	emit_changed();
}

void SQLiteDatabase::set_data(const PackedByteArray &p_data) {
	db->open_buffered(get_name(), p_data, p_data.size());
	emit_changed();
}

Ref<SQLiteAccess> SQLiteDatabase::get_sqlite() {
	return db;
}

Ref<SQLiteQuery> SQLiteDatabase::create_table(const String &p_table_name, const TypedArray<SQLiteColumnSchema> &p_columns) {
	String query_string, type_string, key_string, primary_string;
	/* Create SQL statement */
	query_string = "CREATE TABLE IF NOT EXISTS " + p_table_name + " (";
	key_string = "";
	primary_string = "";

	Dictionary column_dict;
	int primary_key_columns = 0;
	for (int64_t i = 0; i < p_columns.size(); i++) {
		Ref<SQLiteColumnSchema> schema = p_columns[i];
		if (schema->is_primary_key()) {
			primary_key_columns++;
		}
	}
	for (int64_t i = 0; i < p_columns.size(); i++) {
		Ref<SQLiteColumnSchema> schema = p_columns[i];
		query_string += (const String &)schema->get_name() + String(" ");
		switch (schema->get_type()) {
			case Variant::Type::NIL:
				type_string = "NULL";
				break;
			case Variant::Type::INT:
				type_string = "INTEGER";
				break;
			case Variant::Type::FLOAT:
				type_string = "REAL";
				break;
			case Variant::Type::STRING:
				type_string = "TEXT";
				break;
			case Variant::Type::PACKED_BYTE_ARRAY:
				type_string = "BLOB";
				break;
			default:
				ERR_PRINT(vformat("Invalid column type. %s", schema->get_type()));
				type_string = "NULL";
				break;
		}
		query_string += type_string;

		/* Primary key check */
		if (schema->is_primary_key()) {
			if (primary_key_columns == 1) {
				query_string += String(" PRIMARY KEY");
				/* Autoincrement check */
				if (schema->is_auto_increment()) {
					query_string += String(" AUTOINCREMENT");
				}
			} else {
				if (primary_string.is_empty()) {
					primary_string = schema->get_name();
				} else {
					primary_string += String(", ") + schema->get_name();
				}
			}
		}
		/* Not null check */
		if (schema->is_not_null()) {
			query_string += String(" NOT NULL");
		}
		/* Unique check */
		if (schema->is_unique()) {
			query_string += String(" UNIQUE");
		}
		/* Default check */
		if (!schema->get_default_value().is_null()) {
			query_string += String(" DEFAULT ") + schema->get_default_value().to_json_string();
		}

		if (i != p_columns.size() - 1) {
			query_string += ",";
		}
	}

	if (primary_key_columns > 1) {
		query_string += String(", PRIMARY KEY (") + primary_string + String(")");
	}

	query_string += key_string + ");";
	return db->create_query(query_string);
}

Ref<SQLiteQuery> SQLiteDatabase::drop_table(const String &p_name) {
	String query_string;
	/* Create SQL statement */
	query_string = "DROP TABLE " + p_name + ";";

	return db->create_query(query_string);
}

Ref<SQLiteQuery> SQLiteDatabase::insert_row(const String &p_name, const Dictionary &p_row_dict) {
	String query_string, key_string, value_string = "";
	Array keys = p_row_dict.keys();
	Array param_bindings = p_row_dict.values();

	/* Create SQL statement */
	query_string = "INSERT INTO " + p_name;

	int64_t number_of_keys = p_row_dict.size();
	for (int64_t i = 0; i < number_of_keys; i++) {
		key_string += (const String &)keys[i];
		value_string += "?";
		if (i != number_of_keys - 1) {
			key_string += ",";
			value_string += ",";
		}
	}
	query_string += " (" + key_string + ") VALUES (" + value_string + ");";

	return db->create_query(query_string, param_bindings);
}

Ref<SQLiteQuery> SQLiteDatabase::insert_rows(const String &p_name, const TypedArray<Dictionary> &p_row_array) {
	String query_string, key_string, value_string = "", values_string = "";
	Dictionary row0 = p_row_array[0];
	Array keys = row0.keys();
	Array param_bindings;

	/* Create SQL statement */
	query_string = "INSERT INTO " + p_name;

	int64_t number_of_keys = row0.size();
	for (int64_t i = 0; i < number_of_keys; i++) {
		key_string += (const String &)keys[i];
		value_string += "?";
		if (i != number_of_keys - 1) {
			key_string += ",";
			value_string += ",";
		}
	}
	for (int64_t i = 0; i < p_row_array.size(); i++) {
		values_string += "(";
		values_string += value_string;
		values_string += ")";
		if (i != p_row_array.size() - 1) {
			values_string += ",";
		}
		Dictionary row = p_row_array[i];
		Array values = row.values();
		param_bindings.append_array(values);
	}
	query_string += " (" + key_string + ") VALUES " + values_string + ";";

	return db->create_query(query_string, param_bindings);
}

Ref<SQLiteQuery> SQLiteDatabase::select_rows(const String &p_name, const String &p_conditions) {
	String query_string;

	/* Create SQL statement */
	query_string = "SELECT * FROM " + p_name;
	/* If it's empty or * everything is to be selected */
	if (!p_conditions.is_empty() && (p_conditions != (const String &)"*")) {
		query_string += " WHERE " + p_conditions;
	}

	return db->create_query(query_string);
}

Ref<SQLiteQuery> SQLiteDatabase::delete_rows(const String &p_name, const String &p_conditions) {
	String query_string;

	/* Create SQL statement */
	query_string = "DELETE FROM " + p_name;
	/* If it's empty or * everything is to be deleted */
	if (!p_conditions.is_empty() && (p_conditions != (const String &)"*")) {
		query_string += " WHERE " + p_conditions;
	}
	query_string += ";";

	return db->create_query(query_string);
}

TypedArray<SQLiteColumnSchema> SQLiteDatabase::get_columns(const String &p_name) const {
	Ref<SQLiteQuery> query = db->create_query("PRAGMA table_info(" + p_name + ")");
	Ref<SQLiteQueryResult> result = query->execute(Array());
	if (result->get_error() != "") {
		ERR_PRINT("Error getting column names: " + result->get_error() + " " + result->get_error_code());
		return TypedArray<String>();
	}
	TypedArray<SQLiteColumnSchema> column_schemas;
	for (int i = 0; i < result->get_result().size(); i++) {
		Dictionary row = result->get_result()[i];
		Ref<SQLiteColumnSchema> schema;
		schema.instantiate();
		schema->set_name(row["name"]);
		String data_type = row["type"];
		// data type
		if (data_type == "INTEGER") {
			schema->set_type(Variant::Type::INT);
		} else if (data_type == "REAL") {
			schema->set_type(Variant::Type::FLOAT);
		} else if (data_type == "TEXT") {
			schema->set_type(Variant::Type::STRING);
		} else if (data_type == "BLOB") {
			schema->set_type(Variant::Type::PACKED_BYTE_ARRAY);
		} else {
			schema->set_type(Variant::Type::NIL);
		}
		int not_null = row["notnull"];
		schema->set_not_null(not_null == 1 ? true : false);
		Variant default_value = row["dflt_value"];
		if (!default_value.is_null()) {
			schema->set_default_value(default_value);
		}
		int primary_key = row["pk"];
		schema->set_primary_key(primary_key == 1 ? true : false);
		column_schemas.append(schema);
	}
	return column_schemas;
}

Ref<SQLiteQuery> SQLiteDatabase::create_query(const String &p_query_string, const Array &p_args) {
	return db->create_query(p_query_string, p_args);
}

Ref<SQLiteQueryResult> SQLiteDatabase::execute_query(const String &p_query_string, const Array &p_args) {
	return db->create_query(p_query_string, p_args)->execute(Array());
}

String SQLiteDatabase::get_last_error_message() const {
	return db->get_last_error_message();
}
int SQLiteDatabase::get_last_error_code() const {
	return db->get_last_error_code();
}

Dictionary SQLiteDatabase::get_tables() const {
	Ref<SQLiteQuery> query = db->create_query("SELECT name FROM sqlite_master WHERE type = \"table\"");
	Ref<SQLiteQueryResult> result = query->execute(Array());
	if (result->get_error() != "") {
		ERR_PRINT("Error getting table names: " + result->get_error() + " " + result->get_error_code());
		return Dictionary();
	}
	Dictionary result_dict;
	for (int i = 0; i < result->get_result().size(); i++) {
		Dictionary row = result->get_result()[i];
		for (Variant key : row.keys()) {
			String name = row.get("name", String());
			if (!name.begins_with("sqlite_")) {
				TypedArray<SQLiteColumnSchema> columns = get_columns(name);
				TypedArray<String> column_names;
				for (int k = 0; k < columns.size(); k++) {
					Ref<SQLiteColumnSchema> column = columns[k];
					column_names.append(column->get_name());
				}
				result_dict[name] = column_names;
			}
		}
	}
	return result_dict;
}

SQLiteDatabase::SQLiteDatabase() {
	db.instantiate();
	db->open_in_memory();
}

SQLiteDatabase::~SQLiteDatabase() {
	db.unref();
}
