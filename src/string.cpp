#include "sqlite_wrapper/string.h"

#include <sstream>

namespace sqlite_wrapper
{
  auto to_wstring(const char* str) -> std::wstring
  {
    if (str == nullptr)
    {
      return {};
    }

    std::wostringstream stream;

    stream << str;

    return stream.str();
  }
}
