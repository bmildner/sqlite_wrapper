#pragma once

#include <string>

namespace sqlite_wrapper
{
  auto to_wstring(const char* str) -> std::wstring;

  inline auto to_wstring(const std::string& str) -> std::wstring
  {
    return to_wstring(str.c_str());
  }

}