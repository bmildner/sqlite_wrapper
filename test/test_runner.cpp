#include <gtest/gtest.h>

extern "C"
{
  void __ubsan_on_report()
  {
    FAIL() << "Encountered an undefined behavior sanitizer error";
  }

  void __asan_on_error()
  {
    FAIL() << "Encountered an address sanitizer error";
  }

  void __tsan_on_report()
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

int main(int argc, char** argv)
{
  testing::InitGoogleTest(&argc, argv);

  testing::UnitTest::GetInstance()->listeners().Append(new ThrowListener);

  return RUN_ALL_TESTS();
}
