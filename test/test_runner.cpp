#include <gtest/gtest.h>

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
