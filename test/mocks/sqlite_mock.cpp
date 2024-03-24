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

int sqlite3_prepare_v2(sqlite3* db, const char* zSql, int nByte, sqlite3_stmt** ppStmt, const char** pzTail)
{
  return get_global_mock<sqlite3_mock>()->sqlite3_prepare_v2(db, zSql, nByte, ppStmt, pzTail);
}