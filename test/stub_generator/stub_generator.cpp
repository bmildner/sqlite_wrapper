#include <clang/Tooling/Tooling.h>
#include <clang/Tooling/CommonOptionsParser.h>

using namespace clang::tooling;
using namespace llvm;

// Define code generation tool option category
static cl::OptionCategory CodeGenCategory("Code generator options");
// Define option for output file name
cl::opt<std::string> OutputFilename("o", cl::desc("Specify output filename"), cl::value_desc("filename"));
// Define common help message printer
static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);
// Define specific help message printer
static cl::extrahelp MoreHelp("\nCode generation tool help text...");


int main(int argc, const char** argv)
{
  CommonOptionsParser optionsParser(argc, argv, CodeGenCategory);
  //ClangTool tool(optionsParser.getCompilations(), optionsParser.getSourcePathList());

  return 0;
}
