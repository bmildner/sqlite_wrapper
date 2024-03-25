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
    MOCK_METHOD(int, sqlite3_finalize, (sqlite3_stmt* pStmt), (const));

    MOCK_METHOD(int, sqlite3_bind_null, (sqlite3_stmt* pStmt, int index), (const));
    MOCK_METHOD(int, sqlite3_bind_int64, (sqlite3_stmt* pStmt, int index, sqlite3_int64 value), (const));
    MOCK_METHOD(int, sqlite3_bind_text64, (sqlite3_stmt* pStmt, int index, const char* value, sqlite3_uint64 byteSize,void(*pDtor)(void*), unsigned char encoding), (const));
    MOCK_METHOD(int, sqlite3_bind_blob64, (sqlite3_stmt* pStmt, int index, const void* value, sqlite3_uint64 byteSize, void(*pDtor)(void*)), (const));
    MOCK_METHOD(int, sqlite3_bind_double, (sqlite3_stmt* pStmt, int index, double value), (const));

  };

}  // namespace sqlite_wrapper::mocks
