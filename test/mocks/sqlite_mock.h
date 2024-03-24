#pragma once

#include <gmock/gmock.h>

#define SQLITE_API  // do not mark SQLite3 functions for export!
#include <sqlite3.h>

namespace sqlite_wrapper::mocks
{
  class sqlite3_mock
  {
  public:
    virtual ~sqlite3_mock() = default;

    MOCK_METHOD(int, sqlite3_open, (const char* filename, sqlite3** ppDb), (const));
    MOCK_METHOD(int, sqlite3_close, (sqlite3* pDb), (const));

    MOCK_METHOD(int, sqlite3_prepare_v2, (sqlite3* db, const char* zSql, int nByte, sqlite3_stmt** ppStmt, const char** pzTail), (const));
  };

}  // namespace sqlite_wrapper::mocks
