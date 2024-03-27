#include <gtest/gtest.h>

extern "C"
{
  void __ubsan_on_report()  // NOLINT [bugprone-reserved-identifier]
  {
    FAIL() << "Encountered an undefined behavior sanitizer error";
  }

  void __asan_on_error()  // NOLINT [bugprone-reserved-identifier]
  {
    FAIL() << "Encountered an address sanitizer error";
  }

  void __tsan_on_report()  // NOLINT [bugprone-reserved-identifier]
  {
    FAIL() << "Encountered a thread sanitizer error";
  }
}  // extern "C"

// the following code turns ASSERT-failure into an exception, allows nested function in a test to abort test execution immediately
class ThrowListener : public testing::EmptyTestEventListener
{
  void OnTestPartResult(const testing::TestPartResult& result) override
  {
    if (result.type() == testing::TestPartResult::kFatalFailure)
    {
      throw testing::AssertionException(result);
    }
  }
};

auto main(int argc, char** argv) -> int
{
  testing::InitGoogleTest(&argc, argv);

  // GTest takes ownership and frees object NOLINTNEXTLINE [cppcoreguidelines-owning-memory]
  testing::UnitTest::GetInstance()->listeners().Append(new ThrowListener);

  return RUN_ALL_TESTS();
}
