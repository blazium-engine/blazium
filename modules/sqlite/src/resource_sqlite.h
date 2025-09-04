/**************************************************************************/
/*  resource_sqlite.h                                                     */
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

#include "core/io/resource.h"
#include "core/variant/typed_array.h"
#include "core/variant/variant.h"
#include "godot_sqlite.h"

class SQLiteDatabase : public Resource {
	GDCLASS(SQLiteDatabase, Resource);
	Ref<SQLiteAccess> db;

protected:
	static void _bind_methods();

public:
	void set_resource(const String &p_path);
	void set_data(const PackedByteArray &p_data);
	Ref<SQLiteQuery> create_table(const String &p_table_name, const TypedArray<SQLiteColumnSchema> &p_columns);
	Ref<SQLiteQuery> drop_table(const String &p_table_name);

	Ref<SQLiteQuery> insert_row(const String &p_name, const Dictionary &p_row_dict);
	Ref<SQLiteQuery> insert_rows(const String &p_name, const TypedArray<Dictionary> &p_row_array);

	Ref<SQLiteQuery> select_rows(const String &p_name, const String &p_conditions);
	Ref<SQLiteQuery> delete_rows(const String &p_name, const String &p_conditions);
	Dictionary get_tables() const;
	TypedArray<SQLiteColumnSchema> get_columns(const String &p_name) const;
	Ref<SQLiteQuery> create_query(const String &p_query_string, const Array &p_args = Array());
	Ref<SQLiteQueryResult> execute_query(const String &p_query_string, const Array &p_args = Array());
	String get_last_error_message() const;
	int get_last_error_code() const;
	Ref<SQLiteAccess> get_sqlite();

	SQLiteDatabase();
	~SQLiteDatabase();
};
