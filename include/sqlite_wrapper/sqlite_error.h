#pragma once

#include "sqlite_wrapper/config.h"
#include "sqlite_wrapper/raii.h"

#include <cstdint>
#include <source_location>
#include <stacktrace>
#include <stdexcept>
#include <string_view>

namespace sqlite_wrapper
{
  [[nodiscard]] consteval auto are_stack_traces_supported() -> bool
  {
#ifdef SQLITE_WRAPPER_STACK_TRACE
    return true;
#else
    return false;
#endif
  }

  [[nodiscard]] SQLITE_WRAPPER_EXPORT auto get_stack_trace([[maybe_unused]] std::uint16_t skip = 0) noexcept -> std::stacktrace;

  class sqlite_error : public std::runtime_error
  {
   public:
    SQLITE_WRAPPER_EXPORT sqlite_error(std::string_view what, const db_with_location& database, int error,
                                       std::stacktrace&& stacktrace = get_stack_trace(1));

    SQLITE_WRAPPER_EXPORT sqlite_error(std::string_view what, const stmt_with_location& stmt, int error,
                                       std::stacktrace&& stacktrace = get_stack_trace(1));

    sqlite_error(std::string_view what, const stmt_with_location& stmt, std::stacktrace&& stacktrace = get_stack_trace(1))
        : sqlite_error(what, stmt, 0, std::move(stacktrace))
    {
    }

    sqlite_error(std::string_view what, int error, const std::source_location& loc = std::source_location::current(),
                 std::stacktrace&& stacktrace = get_stack_trace(1))
        : sqlite_error(what, db_with_location{nullptr, loc}, error, std::move(stacktrace))
    {
    }

    SQLITE_WRAPPER_EXPORT ~sqlite_error() override = default;

    [[nodiscard]] SQLITE_WRAPPER_EXPORT auto where() const -> const std::source_location&;
    [[nodiscard]] SQLITE_WRAPPER_EXPORT auto stack_trace() const -> const std::stacktrace&;

    SQLITE_WRAPPER_EXPORT sqlite_error(const sqlite_error& other) = default;
    SQLITE_WRAPPER_EXPORT sqlite_error(sqlite_error&& other) = default;

    SQLITE_WRAPPER_EXPORT auto operator=(const sqlite_error& other) -> sqlite_error& = default;
    SQLITE_WRAPPER_EXPORT auto operator=(sqlite_error&& other) -> sqlite_error& = default;

   private:
    std::source_location m_location;
    std::stacktrace m_stacktrace;
  };

}  // namespace sqlite_wrapper
