#include "sqlite_mock.h"

#include "free_function_mock.h"

#include <sqlite3.h>

using sqlite_wrapper::mocks::sqlite3_mock;
using sqlite_wrapper::mocks::get_global_mock;

auto sqlite3_open_v2(const char *filename, sqlite3 **ppDb, int flags, const char* zVfs) -> int
{
  return get_global_mock<sqlite3_mock>()->sqlite3_open_v2(filename, ppDb, flags, zVfs);
}

auto sqlite3_close(sqlite3* pDb) -> int
{
  return get_global_mock<sqlite3_mock>()->sqlite3_close(pDb);
}

auto sqlite3_errmsg(sqlite3* pDb) -> const char*
{
  return get_global_mock<sqlite3_mock>()->sqlite3_errmsg(pDb);
}

auto sqlite3_errstr(int error) -> const char*
{
  return get_global_mock<sqlite3_mock>()->sqlite3_errstr(error);
}

auto sqlite3_prepare_v2(sqlite3* pDb, const char* zSql, int nByte, sqlite3_stmt** ppStmt, const char** pzTail) -> int
{
  return get_global_mock<sqlite3_mock>()->sqlite3_prepare_v2(pDb, zSql, nByte, ppStmt, pzTail);
}

auto sqlite3_finalize(sqlite3_stmt* pStmt) -> int
{
  return get_global_mock<sqlite3_mock>()->sqlite3_finalize(pStmt);
}

auto sqlite3_db_handle(sqlite3_stmt* pStmt) -> sqlite3*
{
  return get_global_mock<sqlite3_mock>()->sqlite3_db_handle(pStmt);
}

auto sqlite3_sql(sqlite3_stmt* pStmt) -> const char*
{
  return get_global_mock<sqlite3_mock>()->sqlite3_sql(pStmt);
}

auto sqlite3_step(sqlite3_stmt* pStmt) -> int
{
  return get_global_mock<sqlite3_mock>()->sqlite3_step(pStmt);
}

auto sqlite3_bind_null(sqlite3_stmt* pStmt, int index) -> int
{
  return get_global_mock<sqlite3_mock>()->sqlite3_bind_null(pStmt, index);
}

auto sqlite3_bind_int64(sqlite3_stmt* pStmt, int index, sqlite3_int64 value) -> int
{
  return get_global_mock<sqlite3_mock>()->sqlite3_bind_int64(pStmt, index, value);
}

auto sqlite3_bind_text64(sqlite3_stmt* pStmt, int index, const char* value, sqlite3_uint64 byteSize, void(*pDtor)(void*), unsigned char encoding) -> int
{
  return get_global_mock<sqlite3_mock>()->sqlite3_bind_text64(pStmt, index, value, byteSize, pDtor, encoding);
}

auto sqlite3_bind_blob64(sqlite3_stmt* pStmt, int index, const void* value, sqlite3_uint64 byteSize, void(*pDtor)(void*)) -> int
{
  return get_global_mock<sqlite3_mock>()->sqlite3_bind_blob64(pStmt, index, value, byteSize, pDtor);
}

auto sqlite3_bind_double(sqlite3_stmt* pStmt, int index, double value) -> int
{
  return get_global_mock<sqlite3_mock>()->sqlite3_bind_double(pStmt, index, value);
}

auto sqlite3_column_type(sqlite3_stmt* pStmt, int iCol) -> int
{
  return get_global_mock<sqlite3_mock>()->sqlite3_column_type(pStmt, iCol);
}

auto sqlite3_column_bytes(sqlite3_stmt* pStmt, int iCol) -> int
{
  return get_global_mock<sqlite3_mock>()->sqlite3_column_bytes(pStmt, iCol);
}

auto sqlite3_column_int64(sqlite3_stmt* pStmt, int iCol) -> sqlite3_int64
{
  return get_global_mock<sqlite3_mock>()->sqlite3_column_int64(pStmt, iCol);
}

auto sqlite3_column_text(sqlite3_stmt* pStmt, int iCol) -> const unsigned char*
{
  return get_global_mock<sqlite3_mock>()->sqlite3_column_text(pStmt, iCol);
}

auto sqlite3_column_blob(sqlite3_stmt* pStmt, int iCol) -> const void*
{
  return get_global_mock<sqlite3_mock>()->sqlite3_column_blob(pStmt, iCol);
}

auto sqlite3_column_double(sqlite3_stmt* pStmt, int iCol) -> double
{
  return get_global_mock<sqlite3_mock>()->sqlite3_column_double(pStmt, iCol);
}

auto sqlite3_reset(sqlite3_stmt *pStmt) -> int
{
  return get_global_mock<sqlite3_mock>()->sqlite3_reset(pStmt);
}

auto sqlite3_clear_bindings(sqlite3_stmt* pStmt) -> int
{
  return get_global_mock<sqlite3_mock>()->sqlite3_clear_bindings(pStmt);
}
