#include "assert_throw_with_msg.h"

#if (defined(__GNUC__) || defined(__clang__)) && __has_include(<cxxabi.h>)
#  include <cstdlib>
#  include <memory>
#  include <string>

#  include <cxxabi.h>

namespace
{
  struct c_str_deleter
  {
    void operator()(char* name) const noexcept
    {
      ::free(name);  // NOLINT(*-owning-memory, *-no-malloc)
    }
  };
}  // namespace

namespace sqlite_wrapper::details
{
  using c_str_ptr = std::unique_ptr<char, c_str_deleter>;

  auto demangle(const char* name) -> std::string
  {
    if (name == nullptr)
    {
      return {};
    }

    int status{0};

    const c_str_ptr demangled_str{abi::__cxa_demangle(name, nullptr, nullptr, &status)};

    if ((status != 0) || !demangled_str)
    {
      return {name};
    }

    return {demangled_str.get()};
  }
}  // namespace sqlite_wrapper::details

#else
namespace sqlite_wrapper::details
{
  auto demangle(const char* name) -> std::string
  {
    if (name == nullptr)
    {
      return {};
    }

    return {name};
  }
}  // namespace sqlite_wrapper::details
#endif
