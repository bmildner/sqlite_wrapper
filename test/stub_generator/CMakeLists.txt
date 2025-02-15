set(LLVM_LINK_COMPONENTS support)

# install clang + libclang-dev on ubuntu!
find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)


set(SRC "stub_generator.cpp")

add_executable(stub_generator ${SRC})

target_link_libraries(stub_generator PRIVATE
    clangAST
    clangASTMatchers
    clangBasic
    clangFrontend
    clangSerialization
    clangTooling)

target_include_directories(stub_generator PRIVATE
    ${CLANG_INCLUDE_DIRS}
    ${LLVM_INCLUDE_DIRS})