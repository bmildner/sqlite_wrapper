// sqlite_wrapper.cpp : Defines the entry point for the application.
//

#include "sqlite_wrapper.h"

#include <iostream>

#include <fmt/format.h>
#include <sqlite3.h>


int main()
{
	std::cout << "Hello CMake." << std::endl;


	auto test_table{sqlite_wrapper::table("test_table",
		                                     sqlite_wrapper::column<sqlite_wrapper::primary_key>("pk_test_table"),
		                                     sqlite_wrapper::column<std::string>("text"),
		                                     sqlite_wrapper::column<double>("double"),
		                                     sqlite_wrapper::column<std::optional<sqlite_wrapper::byte_vector>>("blob"))};

	decltype(test_table)::row_type row;

	auto other_table{ sqlite_wrapper::table("other_table",
											sqlite_wrapper::column<sqlite_wrapper::primary_key>("pk_other_table"),
											sqlite_wrapper::column <sqlite_wrapper::foreign_key>("fk_test_table", test_table))};


	return 0;
}
