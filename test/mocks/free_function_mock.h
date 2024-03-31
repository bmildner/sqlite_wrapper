#pragma once

#include <gmock/gmock.h>

#include <concepts>
#include <type_traits>
#include <memory>
#include <stdexcept>

namespace sqlite_wrapper::mocks
{
  template <typename T>
  concept mock = std::is_class_v<T> && std::has_virtual_destructor_v<T>;

  template <mock Mock>
  using strict_mock = testing::StrictMock<Mock>;

  template <mock Mock>
  using mock_ptr = std::shared_ptr<strict_mock<Mock>>;

  namespace details
  {
    template <mock Mock>
    auto get_set_reset_global_mock(const mock_ptr<Mock>& ptr, bool reset) -> const mock_ptr<Mock>&
    {
      static mock_ptr<Mock> g_mock{};

      if (reset)
      {
        g_mock.reset();
      }
      else if (ptr != nullptr)
      {
        g_mock = ptr;
      }

      return g_mock;
    }
  }  // namespace details

  template <mock Mock>
  void reset_global_mock()
  {
    details::get_set_reset_global_mock<Mock>(nullptr, true);
  }

  template <mock Mock>
  void set_global_mock(const mock_ptr<Mock>& ptr)
  {
    if (ptr == nullptr)
    {
      reset_global_mock<Mock>();
    }
    else
    {
      details::get_set_reset_global_mock(ptr, false);
    }
  }

  template <mock Mock, typename... Args>
  auto create_and_set_global_mock(Args&&... args) -> mock_ptr<Mock>
  {
    auto ptr{std::make_shared<strict_mock<Mock>>(std::forward<Args>(args)...)};

    set_global_mock(ptr);

    return ptr;
  }

  template <mock Mock>
  auto get_global_mock() -> const mock_ptr<Mock>&
  {
    // false-positive for dangling reference from GCC 13, clang-tidy 16 does not know the warning option
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wdangling-reference"
#endif
    const auto& ptr{details::get_set_reset_global_mock<Mock>(nullptr, false)};
#if defined(__GNUC__) && !defined(__clang__)
# pragma GCC diagnostic pop
#endif
    if (ptr == nullptr)
    {
      // TODO: maybe add caller source_location ...
      throw std::runtime_error(std::string{"failed to get global mock object for type "} + typeid(Mock).name());
    }

    return ptr;
  }
}  // namespace sqlite_wrapper::mocks
