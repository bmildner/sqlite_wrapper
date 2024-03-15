// sqlite_wrapper.cpp : Defines the entry point for the application.
//

#include "sqlite_wrapper.h"

#include <iostream>

#include <sqlite3.h>

#include "sqlite_wrapper_error.h"
#include "sqlite_wrapper_format.h"

namespace sqlite_wrapper
{
	namespace details
	{
		auto create_prepared_statement(const db_with_location& db, std::string_view sql) -> statement
		{
			sqlite3_stmt* stmt{ nullptr };

			if (const auto result{ ::sqlite3_prepare_v2(db.value, sql.data(), static_cast<int>(sql.size()), &stmt, nullptr) }; (result != SQLITE_OK) || (stmt == nullptr))
			{
				throw sqlite_error(sqlite_wrapper::format("failed to create prepated statement \"{}\"", sql), db, result);
			}

			return statement{ stmt };
		}

		void bind(const stmt_with_location& stmt, int index)
		{
			if (const auto result{ ::sqlite3_bind_null(stmt.value, index) }; result != SQLITE_OK)
			{
				throw sqlite_error(sqlite_wrapper::format("failed to bind null to index {}", index), stmt, result);
			}
		}

		void bind(const stmt_with_location& stmt, int index, std::int64_t value)
		{
			if (const auto result{ ::sqlite3_bind_int64(stmt.value, index, value) }; result != SQLITE_OK)
			{
				throw sqlite_error(sqlite_wrapper::format("failed to bind int64 to index {}", index), stmt, result);
			}
		}

		void bind(const stmt_with_location& stmt, int index, double value)
		{
			if (const auto result{ ::sqlite3_bind_double(stmt.value, index, value) }; result != SQLITE_OK)
			{
				throw sqlite_error(sqlite_wrapper::format("failed to bind double to index {}", index), stmt, result);
			}
		}

		void bind(const stmt_with_location& stmt, int index, std::string_view value)
		{
			if (const auto result{::sqlite3_bind_text64(stmt.value, index, value.data(), value.size(), nullptr, SQLITE_UTF8)}; result != SQLITE_OK)
			{
				throw sqlite_error(sqlite_wrapper::format("failed to bind string to index {}", index), stmt, result);
			}
		}

		void bind(const stmt_with_location& stmt, int index, const_byte_span value)
		{
			if (const auto result{::sqlite3_bind_blob64(stmt.value, index, value.data(), value.size(), nullptr)}; result != SQLITE_OK)
			{
				throw sqlite_error(sqlite_wrapper::format("failed to bind BLOB to index {}", index), stmt, result);
			}
		}

	}  // namespace details

	auto open(const std::string& file_name, const std::source_location& loc) -> database
	{
		sqlite3* db{nullptr};

		if (const auto result{::sqlite3_open(file_name.c_str(), &db)}; result != SQLITE_OK)
		{
			throw sqlite_error(sqlite_wrapper::format("failed to open database \"{}\"", file_name), result, loc);
		}

		return database{db};
	}

	auto step(const stmt_with_location& stmt) -> bool
	{
		const auto result{::sqlite3_step(stmt.value)};

		if ((result != SQLITE_DONE) && (result != SQLITE_ROW))
		{
			throw sqlite_error("failed to step", stmt, result);
		}

		return (result == SQLITE_ROW);
	}
}  // namespace sqlite_wrapper


auto to_byte_vector(const std::string_view& str) -> sqlite_wrapper::byte_vector
{
	return {reinterpret_cast<const std::byte*>(str.data()), reinterpret_cast<const std::byte*>(str.data() + str.size())};
}


int main()
{
	try
	{
		std::cout << "Hello CMake." << std::endl;


		auto test_table{ sqlite_wrapper::table("test_table",
																					 sqlite_wrapper::column<sqlite_wrapper::primary_key>("pk_test_table"),
																					 sqlite_wrapper::column<std::string>("text"),
																					 sqlite_wrapper::column<double>("double"),
																					 sqlite_wrapper::column<std::optional<sqlite_wrapper::byte_vector>>("blob")) };

		decltype(test_table)::full_row row;

		auto other_table{ sqlite_wrapper::table("other_table",
												sqlite_wrapper::column<sqlite_wrapper::primary_key>("pk_other_table"),
												sqlite_wrapper::column <sqlite_wrapper::foreign_key>("fk_test_table", test_table)) };

		const auto db{ sqlite_wrapper::open("Test.db") };

		sqlite_wrapper::execute_no_data(db.get(), "CREATE TABLE IF NOT EXISTS testtable (data)");

		constexpr auto* sql{ "SELECT * from testtable WHERE data = ?" };

		auto stmt{ sqlite_wrapper::create_prepared_statement(db.get(), sql) };
		stmt = sqlite_wrapper::create_prepared_statement(db.get(), sql, std::int64_t{1});
		stmt = sqlite_wrapper::create_prepared_statement(db.get(), sql, 1);
		stmt = sqlite_wrapper::create_prepared_statement(db.get(), sql, nullptr);
		stmt = sqlite_wrapper::create_prepared_statement(db.get(), sql, std::nullopt);
		stmt = sqlite_wrapper::create_prepared_statement(db.get(), sql, 1.23);
		stmt = sqlite_wrapper::create_prepared_statement(db.get(), sql, float{1.23});
		stmt = sqlite_wrapper::create_prepared_statement(db.get(), sql, "fgsdfg");
		stmt = sqlite_wrapper::create_prepared_statement(db.get(), sql, std::string("fgsdfg"));

		const auto vector{to_byte_vector("Hello")};

		stmt = sqlite_wrapper::create_prepared_statement(db.get(), sql, vector);
		stmt = sqlite_wrapper::create_prepared_statement(db.get(), sql, sqlite_wrapper::const_byte_span{vector});


		stmt = sqlite_wrapper::create_prepared_statement(db.get(), sql, std::optional<int>{});
	}
	catch (const std::exception& exception)
	{
		std::cout << "Caught execption, what: " << exception.what() << std::endl;
	}

	return 0;
}
