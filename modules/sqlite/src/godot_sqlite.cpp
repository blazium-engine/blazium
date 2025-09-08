/**************************************************************************/
/*  godot_sqlite.cpp                                                      */
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

#include "../thirdparty/sqlite/sqlite3.h"
#include "core/config/project_settings.h"
#include "core/core_bind.h"
#include "core/error/error_macros.h"
#include "core/variant/variant.h"

#include "godot_sqlite.h"

Array fast_parse_row(sqlite3_stmt *stmt) {
	Array result;

	const int column_count = sqlite3_column_count(stmt);

	for (int i = 0; i < column_count; i++) {
		const int column_type = sqlite3_column_type(stmt, i);
		Variant value;
		switch (column_type) {
			case SQLITE_INTEGER:
				value = Variant(sqlite3_column_int(stmt, i));
				break;

			case SQLITE_FLOAT:
				value = Variant(sqlite3_column_double(stmt, i));
				break;

			case SQLITE_TEXT: {
				int size = sqlite3_column_bytes(stmt, i);
				String str =
						String::utf8((const char *)sqlite3_column_text(stmt, i), size);
				value = Variant(str);
				break;
			}
			case SQLITE_BLOB: {
				PackedByteArray arr;
				int size = sqlite3_column_bytes(stmt, i);
				arr.resize(size);
				memcpy(arr.ptrw(), sqlite3_column_blob(stmt, i), size);
				value = Variant(arr);
				break;
			}
			case SQLITE_NULL: {
			} break;
			default:
				ERR_PRINT("This kind of data is not yet supported: " + itos(column_type));
				break;
		}

		result.push_back(value);
	}

	return result;
}

SQLiteQuery::SQLiteQuery() {}

SQLiteQuery::~SQLiteQuery() {
	finalize();
}

void SQLiteQuery::init(SQLiteAccess *p_db, const String &p_query, Array p_args) {
	db = p_db;
	query = p_query;
	arguments = p_args;
	stmt = nullptr;
}

bool SQLiteQuery::is_ready() const {
	return stmt != nullptr;
}

String SQLiteQuery::get_last_error_message() const {
	ERR_FAIL_COND_V(db == nullptr, "Database is undefined.");
	return db->get_last_error_message();
}

TypedArray<SQLiteColumnSchema> SQLiteQuery::get_columns() {
	ERR_FAIL_NULL_V(stmt, TypedArray<SQLiteColumnSchema>());

	if (is_ready() == false) {
		ERR_FAIL_COND_V(prepare() == false, Array());
	}

	TypedArray<SQLiteColumnSchema> res;
	const int col_count = sqlite3_column_count(stmt);
	res.resize(col_count);

	// Fetch all column
	for (int i = 0; i < col_count; i++) {
		Ref<SQLiteColumnSchema> schema;
		schema.instantiate();
		// Key name
		const char *col_name = sqlite3_column_name(stmt, i);
		schema->set_name(String(col_name));
		switch (sqlite3_column_type(stmt, i)) {
			case SQLITE_INTEGER:
				schema->set_type(Variant::Type::INT);
				break;
			case SQLITE_FLOAT:
				schema->set_type(Variant::Type::FLOAT);
				break;
			case SQLITE_TEXT:
				schema->set_type(Variant::Type::STRING);
				break;
			case SQLITE_BLOB:
				schema->set_type(Variant::Type::PACKED_BYTE_ARRAY);
				break;
			case SQLITE_NULL:
				schema->set_type(Variant::Type::NIL);
				break;
			default:
				ERR_PRINT("This kind of data is not yet supported: " + itos(sqlite3_column_type(stmt, i)));
				schema->set_type(Variant::Type::NIL);
				break;
		}
		res[i] = schema;
	}

	return res;
}

bool SQLiteQuery::prepare() {
	ERR_FAIL_COND_V(stmt != nullptr, SQLITE_ERROR);
	ERR_FAIL_COND_V(db == nullptr, SQLITE_ERROR);
	ERR_FAIL_COND_V(db->get_handler() == nullptr, SQLITE_ERROR);
	ERR_FAIL_COND_V(query == "", SQLITE_ERROR);
	// Prepare the statement
	int result = sqlite3_prepare_v2(db->get_handler(), query.utf8().ptr(), -1,
			&stmt, nullptr);

	// Cannot prepare query!
	ERR_FAIL_COND_V_MSG(result != SQLITE_OK, false,
			"SQL Error: " + db->get_last_error_message());

	return true;
}

void SQLiteQuery::finalize() {
	if (stmt) {
		sqlite3_finalize(stmt);
		stmt = nullptr;
	}
}

void SQLiteQuery::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_last_error_message"), &SQLiteQuery::get_last_error_message);
	ClassDB::bind_method(D_METHOD("execute", "arguments"), &SQLiteQuery::execute, DEFVAL(Array()));
	ClassDB::bind_method(D_METHOD("batch_execute", "rows"), &SQLiteQuery::batch_execute);
	ClassDB::bind_method(D_METHOD("get_columns"), &SQLiteQuery::get_columns);
	ClassDB::bind_method(D_METHOD("get_query"), &SQLiteQuery::get_query);
	ClassDB::bind_method(D_METHOD("get_arguments"), &SQLiteQuery::get_arguments);
	ClassDB::bind_method(D_METHOD("set_arguments", "arguments"), &SQLiteQuery::set_arguments);

	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "arguments"), "set_arguments", "get_arguments");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "query"), "", "get_query");
}

SQLiteAccess::SQLiteAccess() {
}

bool SQLiteAccess::open_in_memory() {
	if (sqlite3_open_v2(":memory:", &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr) != SQLITE_OK) {
		return false;
	}
	return true;
}

bool SQLiteAccess::close() {
	// Finalize all queries before close the DB.
	// Reverse order because I need to remove the not available queries.
	for (uint32_t i = queries.size(); i > 0; i -= 1) {
		SQLiteQuery *query =
				Object::cast_to<SQLiteQuery>(queries[i - 1]->get_ref());
		if (query != nullptr) {
			query->finalize();
		} else {
			memdelete(queries[i - 1]);
			queries.remove_at(i - 1);
		}
	}

	if (db) {
		// Cannot close database!
		int result = sqlite3_close_v2(db);
		db = nullptr;
		return result;
	}

	if (memory_read) {
		// Close virtual filesystem database
		int result = spmemvfs_close_db(&spmemvfs_db);
		spmemvfs_env_fini();
		memory_read = false;
		return result;
	}

	return true;
}

sqlite3_stmt *SQLiteAccess::prepare(const char *query) {
	// Get database pointer
	sqlite3 *dbs = get_handler();

	ERR_FAIL_COND_V_MSG(dbs == nullptr, nullptr,
			"Cannot prepare query. The database was not opened.");

	// Prepare the statement
	sqlite3_stmt *stmt = nullptr;
	sqlite3_prepare_v2(dbs, query, -1, &stmt, nullptr);
	return stmt;
}

Dictionary parse_row(sqlite3_stmt *stmt) {
	Dictionary result;

	// Get column count
	int col_count = sqlite3_column_count(stmt);

	// Fetch all column
	for (int i = 0; i < col_count; i++) {
		// Key name
		const char *col_name = sqlite3_column_name(stmt, i);
		String key = String(col_name);

		// Value
		int col_type = sqlite3_column_type(stmt, i);
		Variant value;

		// Get column value
		switch (col_type) {
			case SQLITE_INTEGER:
				value = Variant(sqlite3_column_int(stmt, i));
				break;

			case SQLITE_FLOAT:
				value = Variant(sqlite3_column_double(stmt, i));
				break;

			case SQLITE_TEXT: {
				int size = sqlite3_column_bytes(stmt, i);
				String str =
						String::utf8((const char *)sqlite3_column_text(stmt, i), size);
				value = Variant(str);
				break;
			}
			case SQLITE_BLOB: {
				PackedByteArray arr;
				int size = sqlite3_column_bytes(stmt, i);
				arr.resize(size);
				memcpy((void *)arr.ptr(), sqlite3_column_blob(stmt, i), size);
				value = Variant(arr);
				break;
			}

			default:
				break;
		}
		result[key] = value;
	}

	return result;
}

String SQLiteAccess::get_last_error_message() const {
	return sqlite3_errmsg(get_handler());
}

int SQLiteAccess::get_last_error_code() const {
	return sqlite3_errcode(get_handler());
}

SQLiteAccess::~SQLiteAccess() {
	close();
	for (uint32_t i = 0; i < queries.size(); i += 1) {
		SQLiteQuery *query = Object::cast_to<SQLiteQuery>(queries[i]->get_ref());
		if (query != nullptr) {
			query->init(nullptr, "", Array());
		}
	}
}

void SQLiteAccess::_bind_methods() {
	ClassDB::bind_method(D_METHOD("open", "database"), &SQLiteAccess::open);
	ClassDB::bind_method(D_METHOD("open_in_memory"), &SQLiteAccess::open_in_memory);
	ClassDB::bind_method(D_METHOD("open_buffered", "path", "buffers", "size"), &SQLiteAccess::open_buffered);
	ClassDB::bind_method(D_METHOD("backup", "path"), &SQLiteAccess::backup);
	ClassDB::bind_method(D_METHOD("get_last_error_message"), &SQLiteAccess::get_last_error_message);
	ClassDB::bind_method(D_METHOD("get_last_error_code"), &SQLiteAccess::get_last_error_code);
	ClassDB::bind_method(D_METHOD("close"), &SQLiteAccess::close);
	ClassDB::bind_method(D_METHOD("create_query", "statement", "arguments"), &SQLiteAccess::create_query, DEFVAL(Array()));
}

bool SQLiteAccess::open(const String &path) {
	if (!path.strip_edges().length()) {
		print_error("Path is wrong!");
		return false;
	}
	Engine *engine_singleton = Engine::get_singleton();
	if (!engine_singleton) {
		print_error("Cannot get engine singleton!");
		return false;
	}
	if (!engine_singleton->is_editor_hint() && path.begins_with("res://")) {
		Ref<FileAccess> dbfile = FileAccess::open(path, FileAccess::READ);
		if (dbfile.is_null()) {
			print_error("Cannot open packed database!");
			return false;
		}
		int64_t size = dbfile->get_length();
		PackedByteArray buffer;
		buffer.resize(size);
		buffer.fill(0);
		dbfile->get_buffer(buffer.ptrw(), size);
		return open_buffered(path, buffer, size);
	}
	ProjectSettings *project_settings_singleton = ProjectSettings::get_singleton();
	if (!project_settings_singleton) {
		print_error("Cannot get project settings!");
		return SQLITE_ERROR;
	}
	String real_path = project_settings_singleton->globalize_path(path.strip_edges());

	return sqlite3_open(real_path.utf8().get_data(), &db);
}

String SQLiteAccess::bind_args(sqlite3_stmt *stmt, const Array &args) {
	int param_count = sqlite3_bind_parameter_count(stmt);
	if (param_count != args.size()) {
		return "SQLiteQuery failed; expected " + itos(param_count) + " arguments, got " + itos(args.size());
	}

	/**
	 * SQLite data types:
	 * - NULL
	 * - INTEGER (signed, max 8 bytes)
	 * - REAL (stored as a double-precision float)
	 * - TEXT (stored in database encoding of UTF-8, UTF-16BE or UTF-16LE)
	 * - BLOB (1:1 storage)
	 */

	for (int i = 0; i < param_count; i++) {
		int retcode;
		switch (args[i].get_type()) {
			case Variant::Type::NIL:
				retcode = sqlite3_bind_null(stmt, i + 1);
				break;
			case Variant::Type::BOOL:
			case Variant::Type::INT:
				retcode = sqlite3_bind_int(stmt, i + 1, (int)args[i]);
				break;
			case Variant::Type::FLOAT:
				retcode = sqlite3_bind_double(stmt, i + 1, (double)args[i]);
				break;
			case Variant::Type::STRING:
				retcode = sqlite3_bind_text(
						stmt, i + 1, String(args[i]).utf8().get_data(), -1, SQLITE_TRANSIENT);
				break;
			case Variant::Type::PACKED_BYTE_ARRAY:
				retcode =
						sqlite3_bind_blob(stmt, i + 1, PackedByteArray(args[i]).ptr(),
								PackedByteArray(args[i]).size(), SQLITE_TRANSIENT);
				break;
			default:
				return "SQLite was passed unhandled Variant with TYPE_* enum " +
						itos(args[i].get_type()) +
						". Please serialize your object into a String or a PoolByteArray.";
		}

		if (retcode != SQLITE_OK) {
			return "SQLiteQuery failed, an error occurred while binding argument" +
					itos(i + 1) + " of " + itos(args.size()) + " (SQLite errcode " +
					itos(retcode) + ")";
		}
	}

	return "";
}

bool SQLiteAccess::open_buffered(const String &name, const PackedByteArray &buffers, int64_t size) {
	if (!name.strip_edges().length()) {
		return false;
	}

	if (!buffers.size() || !size) {
		return false;
	}

	spmembuffer_t *p_mem = (spmembuffer_t *)calloc(1, sizeof(spmembuffer_t));
	p_mem->total = p_mem->used = size;
	p_mem->data = (char *)malloc(size + 1);
	memcpy(p_mem->data, buffers.ptr(), size);
	p_mem->data[size] = '\0';

	spmemvfs_env_init();
	int err = spmemvfs_open_db(&spmemvfs_db, name.utf8().get_data(), p_mem);

	if (err != SQLITE_OK || spmemvfs_db.mem != p_mem) {
		print_error("Cannot open buffered database!");
		return false;
	}

	memory_read = true;
	return true;
}

bool SQLiteAccess::backup(const String &path) {
	String destination_path = ProjectSettings::get_singleton()->globalize_path(path.strip_edges());
	sqlite3 *destination_db;
	int result = sqlite3_open_v2(destination_path.utf8().get_data(), &destination_db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_URI, NULL);

	if (result != SQLITE_OK) {
		ERR_PRINT("Cannot create backup database, error:" + itos(result));
		sqlite3_close_v2(destination_db);
		return false;
	}

	// Get database pointer
	sqlite3 *dbs = get_handler();

	if (dbs == nullptr) {
		ERR_PRINT("Cannot backup. The database was not opened.");
		sqlite3_close_v2(destination_db);
		return false;
	}

	sqlite3_backup *backup = sqlite3_backup_init(destination_db, "main", dbs, "main");
	if (backup) {
		sqlite3_backup_step(backup, -1);
		sqlite3_backup_finish(backup);
	}

	sqlite3_close_v2(destination_db);
	return true;
}

Ref<SQLiteQueryResult> SQLiteQuery::execute(const Array p_args) {
	Ref<SQLiteQueryResult> result;
	result.instantiate();
	result->set_query(query);
	if (!is_ready()) {
		if (!prepare()) {
			result->set_error("Query is not ready");
			result->set_error_code(db->get_last_error_code());
			return result;
		}
	}

	ERR_FAIL_NULL_V(stmt, Variant());
	Array args = p_args;
	if (args.is_empty()) {
		args = arguments;
	}
	String bind_err_msg = SQLiteAccess::bind_args(stmt, args);
	if (bind_err_msg != "") {
		result->set_error_code(db->get_last_error_code());
		result->set_error(bind_err_msg);
		return result;
	}

	TypedArray<Dictionary> results;
	while (true) {
		const int res = sqlite3_step(stmt);
		if (res == SQLITE_ROW) {
			results.append(parse_row(stmt));
		} else if (res == SQLITE_DONE) {
			break;
		} else {
			result->set_error_code(res);
			result->set_error(get_last_error_message());
			ERR_BREAK_MSG(true, "There was an error during an SQL execution: " + get_last_error_message());
		}
	}
	result->set_result(results);

	if (SQLITE_OK != sqlite3_reset(stmt)) {
		finalize();
		ERR_FAIL_V_MSG(result, "Was not possible to reset the query: " + get_last_error_message());
		result->set_error_code(db->get_last_error_code());
		result->set_error(get_last_error_message());
	}
	return result;
}

TypedArray<SQLiteQueryResult> SQLiteQuery::batch_execute(TypedArray<Array> p_rows) {
	TypedArray<SQLiteQueryResult> res;
	TypedArray<Array> rows = p_rows;
	if (rows.is_empty()) {
		rows = arguments;
	}
	for (int i = 0; i < p_rows.size(); i += 1) {
		Ref<SQLiteQueryResult> r = execute(p_rows[i]);
		res.push_back(r);
	}
	return res;
}

Ref<SQLiteQuery> SQLiteAccess::create_query(String p_query, Array p_args) {
	Ref<SQLiteQuery> query;
	query.instantiate();
	query->init(this, p_query, p_args);

	WeakRef *wr = memnew(WeakRef);
	wr->set_obj(query.ptr());
	queries.push_back(wr);

	return query;
}
