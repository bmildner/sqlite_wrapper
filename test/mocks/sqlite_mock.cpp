#include "sqlite_mock.h"
#include "free_function_mock.h"

using sqlite_wrapper::mocks::sqlite3_mock;
using sqlite_wrapper::mocks::get_global_mock;

int sqlite3_open(const char* filename, sqlite3** ppDb)
{
  return get_global_mock<sqlite3_mock>()->sqlite3_open(filename, ppDb);
}

int sqlite3_close(sqlite3* pDb)
{
  return get_global_mock<sqlite3_mock>()->sqlite3_close(pDb);
}

const char* sqlite3_errmsg(sqlite3* pDb)
{
  return get_global_mock<sqlite3_mock>()->sqlite3_errmsg(pDb);
}

const char* sqlite3_errstr(int error)
{
  return get_global_mock<sqlite3_mock>()->sqlite3_errstr(error);
}

int sqlite3_prepare_v2(sqlite3* db, const char* zSql, int nByte, sqlite3_stmt** ppStmt, const char** pzTail)
{
  return get_global_mock<sqlite3_mock>()->sqlite3_prepare_v2(db, zSql, nByte, ppStmt, pzTail);
}

int sqlite3_finalize(sqlite3_stmt* pStmt)
{
  return get_global_mock<sqlite3_mock>()->sqlite3_finalize(pStmt);
}

sqlite3* sqlite3_db_handle(sqlite3_stmt* pStmt)
{
  return get_global_mock<sqlite3_mock>()->sqlite3_db_handle(pStmt);
}

const char* sqlite3_sql(sqlite3_stmt* pStmt)
{
  return get_global_mock<sqlite3_mock>()->sqlite3_sql(pStmt);
}

int sqlite3_step(sqlite3_stmt* pStmt)
{
  return get_global_mock<sqlite3_mock>()->sqlite3_step(pStmt);
}

int sqlite3_bind_null(sqlite3_stmt* pStmt, int index)
{
  return get_global_mock<sqlite3_mock>()->sqlite3_bind_null(pStmt, index);
}

int sqlite3_bind_int64(sqlite3_stmt* pStmt, int index, sqlite3_int64 value)
{
  return get_global_mock<sqlite3_mock>()->sqlite3_bind_int64(pStmt, index, value);
}

int sqlite3_bind_text64(sqlite3_stmt* pStmt, int index, const char* value, sqlite3_uint64 byteSize, void(*pDtor)(void*), unsigned char encoding)
{
  return get_global_mock<sqlite3_mock>()->sqlite3_bind_text64(pStmt, index, value, byteSize, pDtor, encoding);
}

int sqlite3_bind_blob64(sqlite3_stmt* pStmt, int index, const void* value, sqlite3_uint64 byteSize, void(*pDtor)(void*))
{
  return get_global_mock<sqlite3_mock>()->sqlite3_bind_blob64(pStmt, index, value, byteSize, pDtor);
}

int sqlite3_bind_double(sqlite3_stmt* pStmt, int index, double value)
{
  return get_global_mock<sqlite3_mock>()->sqlite3_bind_double(pStmt, index, value);
}

int sqlite3_column_type(sqlite3_stmt* pStmt, int iCol)
{
  return get_global_mock<sqlite3_mock>()->sqlite3_column_type(pStmt, iCol);
}

int sqlite3_column_bytes(sqlite3_stmt* pStmt, int iCol)
{
  return get_global_mock<sqlite3_mock>()->sqlite3_column_bytes(pStmt, iCol);
}

sqlite3_int64 sqlite3_column_int64(sqlite3_stmt* pStmt, int iCol)
{
  return get_global_mock<sqlite3_mock>()->sqlite3_column_int64(pStmt, iCol);
}

const unsigned char* sqlite3_column_text(sqlite3_stmt* pStmt, int iCol)
{
  return get_global_mock<sqlite3_mock>()->sqlite3_column_text(pStmt, iCol);
}

const void* sqlite3_column_blob(sqlite3_stmt* pStmt, int iCol)
{
  return get_global_mock<sqlite3_mock>()->sqlite3_column_blob(pStmt, iCol);
}

double sqlite3_column_double(sqlite3_stmt* pStmt, int iCol)
{
  return get_global_mock<sqlite3_mock>()->sqlite3_column_double(pStmt, iCol);
}
