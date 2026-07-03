# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

A modern C++23 RAII/type-safe wrapper around the SQLite C API (`unofficial-sqlite3` via vcpkg), built as both a shared
and a static library. Binding/column extraction is done through concepts rather than templates-of-templates or macros.

## Build

Requires cmake >= 3.30, vcpkg (with `VCPKG_ROOT` set), a C++23 compiler. `ccache` and `mold` are auto-detected and used
if present.

Use CMake presets (see `CMakePresets.json`): `linux-debug`, `linux-release`, `linux-debug-newest-toolset` (gcc-14/g++-14/clang-tidy-20),
`linux-release-newest-toolset`, plus Windows/macOS presets. Linux binary/build dirs land outside the source tree at
`../bindir/sqlite_wrapper/build/<preset>`.

```sh
cmake --preset linux-debug
cmake --build --preset linux-debug   # or just: cmake --build <binaryDir>
```

Warnings are treated as errors (`CMAKE_COMPILE_WARNING_AS_ERROR`), and clang-tidy runs as part of the build by default
for the shared library and tests (see `static_analysis.cmake`); it is disabled for the static library target to speed
up builds. Set `-DENABLE_STATIC_ANALYSIS=OFF` locally for faster iteration, but never commit with it disabled.

Debug builds enable ASan+UBSan and (if `gcovr` is found) `--coverage` instrumentation; non-Debug builds enable LTO and
`-O3`.

## Running tests

Two independent test binaries are built (see `test/CMakeLists.txt`):
- `test_runner` — links the real shared `sqlite_wrapper` library and exercises it against a real SQLite (in-memory/temp
  file) via GTest/GMock.
- `test_runner_mocked` — links the static `sqlite_wrapper` library plus `sqlite_mock`, which mocks the raw `sqlite3_*`
  C API (see `test/mocks/sqlite_mock.h`, `test/mocks/free_function_mock.h`) to unit-test wrapper logic (error paths,
  binding/column dispatch, statement lifecycle) without touching real SQLite.

Both binaries live in `CMAKE_RUNTIME_OUTPUT_DIRECTORY` (`<binaryDir>/bin`). A convenience target runs both:

```sh
cmake --build <binaryDir> --target sqlite_wrapper.run_all_tests
```

To run a single test, invoke the binary directly with GTest filters, e.g.:

```sh
<binaryDir>/bin/test_runner --gtest_filter=SomeTestSuite.SomeTest
<binaryDir>/bin/test_runner_mocked --gtest_filter=SomeTestSuite.*
```

### Debug build gotchas (ASan/UBSan)
- If a test binary crashes at startup with `AddressSanitizer:DEADLYSIGNAL` / segfaults or loops printing that, it's
  likely an ASan-vs-high-entropy-ASLR kernel incompatibility. Workaround: `sudo sysctl vm.mmap_rnd_bits=28` (or set
  permanently in `/etc/sysctl.d/local.conf`).
- If a test binary segfaults or returns non-zero only when run under gdb, this is an asan/lsan-vs-gdb interaction; in
  CLion uncheck "Use visual representation for Sanitizer's output" to see the real sanitizer text output.
- `ASAN_OPTIONS=halt_on_error=true` (or `help=1` to list all options) is a useful env var when debugging.

## Coverage

In Debug builds, if `gcovr` is found, `cmake --build <binaryDir> --target sqlite_wrapper.coverage_report` runs both
test binaries and prints branch/line coverage tables (excludes `test/`).

## Docs

`docs/` builds Doxygen+graphviz HTML docs (target `sqlite_wrapper.build_documentation`) if both tools are found;
otherwise the target is silently skipped. Doxygen pulls in `doxygen-awesome-css` via `FetchContent`.

## Architecture

### Layering
- `include/sqlite_wrapper/config.h` — `SQLITE_WRAPPER_EXPORT` dllexport/import macro; only meaningful for the shared
  library build (`SQLITE_WRAPPER_SHARED`), a no-op for the static build.
- `include/sqlite_wrapper/raii.h` — owning handle aliases: `database = unique_ptr<sqlite3, database_deleter>` and
  `statement = unique_ptr<sqlite3_stmt, statement_deleter>`, plus non-owning `db_with_location` / `stmt_with_location`
  (raw pointer + `std::source_location`, see below).
- `include/sqlite_wrapper/with_location.h` — generic `with_location<T>` wrapper that pairs a value with a captured
  `std::source_location`. Used wherever a defaulted `source_location` parameter can't be the last parameter because a
  variadic/forwarding parameter pack follows it — every error path can report *where in caller code* things went wrong.
- `include/sqlite_wrapper/concepts.h` — generic, wrapper-independent concepts (`tuple_like`, `array_like`,
  `same_as_either`/`same_as_all`, `exception_with_stack_trace`, ...).
- `include/sqlite_wrapper/tuple_utils.h` — generic tuple/array manipulation helpers (`push_front`/`push_back`,
  `pop_front`/`pop_back`, `add_type_front`/`add_type_back`, `remove_type_front`/`remove_type_back`,
  `to_array`/`convert_to_array_type`) built on `tuple_like`/`array_like` from `concepts.h`.
- `include/sqlite_wrapper/sqlite_wrapper.h` — the main API surface: concepts for bindable/queryable types, free
  functions `open`, `create_prepared_statement`, `step`, `execute`, `execute_one_row`, `execute_no_data`, `get_row(s)`,
  and the `details::` namespace with the actual per-type bind/column-extraction overloads.
- `include/sqlite_wrapper/sqlite_error.h` — `sqlite_error` (derives `std::runtime_error`), carries a `source_location`
  and (if `SQLITE_WRAPPER_STACK_TRACE` is defined — enabled automatically in Debug builds, see `common.cmake`) a
  captured `std::stacktrace`.
- `include/sqlite_wrapper/create_table.h` — experimental/WIP compile-time table/column DSL (`table<>`, `column<>`,
  `primary_key`, `foreign_key`). Marked `// TODO: cleanup or remove altogether!` in the source — treat as unstable,
  don't build heavily on it without checking with the user first.
- `include/sqlite_wrapper/format.h` — `fmt`-based formatting helpers/macros used internally and for custom formatters
  (e.g. `open_flags`).

### Type-safety model (concepts, not virtuals)
Everything hinges on a small hierarchy of concepts defined in `sqlite_wrapper.h`:
- `basic_database_type` / `optional_database_type` / `database_type` — what a query result *column* can be
  (`int64_t`, `double`, `string`, `byte_vector`, or `optional<T>` of those).
- `row_type` — any tuple-like type whose elements are all `database_type` (checked via a fold over `has_tuple_element`
  in `concepts.h`). This is how `get_row<Row>()`/`get_rows<Row>()`/`execute<Row>()` accept arbitrary
  `std::tuple<...>` (or tuple-like struct) row shapes without codegen.
- `integral_binding_type` / `floation_point_binding_type` / `string_binding_type` / `blob_binding_type` /
  `null_binding_type` → `basic_binding_type` / `optional_binding_type` → `single_binding_type`, plus
  `multi_binding_type` (a range of `single_binding_type`, excluding things that are themselves `basic_binding_type`,
  e.g. so a `string` isn't treated as a range of chars to bind) → `binding_type` — what can be bound as a query
  *parameter*. `create_prepared_statement`/`execute`/etc. take `const binding_type auto&... params` and dispatch via
  `details::bind_value_and_increment_index`, so a single multi-value param (e.g. a `vector<int>` for an `IN (...)`
  clause) consumes multiple placeholders while a single-value param consumes one.

Adding support for a new bindable/queryable type means extending these concepts and adding the corresponding
`details::bind_value`/`details::get_column` overload — not adding new top-level API functions.

### RAII + non-owning handle pattern
`database`/`statement` (owning, via `unique_ptr` with custom deleters in `raii.cpp`) are what callers hold.
`db_with_location`/`stmt_with_location` (non-owning raw pointer + source_location) are what's threaded through
internal calls — constructed ad hoc at call sites like `{stmt.get(), database.location}`. This split exists so errors
deep in a call chain can still report the original caller's `source_location` without needing owning access.

### Testing strategy: real vs. mocked SQLite
The two test binaries are deliberately not redundant:
- `sqlite_wrapper_tests.cpp` (`test_runner`) exercises real behavior end-to-end against actual SQLite.
- `sqlite_wrapper_tests_mocked.cpp` (`test_runner_mocked`) defines its own fake `sqlite3`/`sqlite3_stmt` structs and
  uses GMock (`sqlite_mock.h`, `free_function_mock.h`) to intercept the raw C API, so it can assert on exact
  call sequences/arguments and force error paths that are hard to trigger against a real DB. `test_runner_mocked`
  links the static library variant (`sqlite_wrapper.sqlite_wrapper_static`) so it can substitute the underlying
  `sqlite3_*` symbols with mocks — but the static library is a general-purpose build output in its own right (an
  alternative to the shared `sqlite_wrapper.sqlite_wrapper` for consumers who want to link the wrapper statically),
  not something that exists solely for this test binary.

When adding wrapper features, consider whether the mocked test needs a corresponding fake/mock addition in
`test/mocks/` in addition to the real-SQLite test.