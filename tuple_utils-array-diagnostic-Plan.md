# tuple_utils.h — nice diagnostic for add/remove-type on std::array (range-backed tuple-likes)

**Status: implemented and verified** (g++-14, clang++-20; full `linux-debug` and `linux-release`
builds incl. clang-tidy; all `test_runner`/`test_runner_mocked` tests pass).

## Context

`add_type_front/back` and `remove_type_front/back` in `include/sqlite_wrapper/tuple_utils.h` are
constrained on `tuple_like`, but the underlying `details::` structs are only specialized for
`std::tuple<...>` and `std::pair<...>`; the primary template is declared-but-undefined. Passing any
other tuple-like type — notably `std::array` — previously produced an opaque *"invalid use of
incomplete type"* hard error.

`std::array` is a homogeneous, range-backed container: its element type is fixed and it can only ever
hold one type. Adding a differently-typed element (or removing one) is not representable — so silent
support is the wrong goal. We want a clear compile error explaining why it's not possible and what to
do (convert to `std::tuple` first).

Scope note: this deliberately does NOT reject homogeneous *tuples*. `std::tuple<int,int>` satisfies
`array_like`, but `add_type_front<char, std::tuple<int,int>>` → `std::tuple<char,int,int>` is valid
and must keep working. The correct discriminator is therefore **`std::ranges::range`** (which
`std::array` satisfies and `std::tuple`/`std::pair`/tuple-like structs do not), NOT `array_like`.

## Decision: hybrid

Two gates, both verified to compile as expected on g++-14 and clang-20:

1. **Public-alias `requires`-clause on a self-documenting concept** — makes misuse SFINAE-detectable
   (so tests can positively assert rejection) and is what a normal caller hits.
   ```cpp
   template <typename T>
   concept heterogeneous_tuple_like =
       tuple_like<T> && !std::ranges::range<std::remove_cvref_t<T>>;

   template <typename T, heterogeneous_tuple_like Tuple>
   using add_type_front = details::add_type_front<T, std::remove_cvref_t<Tuple>>::type;
   ```
2. **`static_assert` in the `details::` primary/array-like specialization** — a readable prose
   explainer for anyone who reaches `details::` directly (bypassing the constrained alias).

### What each surface actually prints (verified)
- Normal caller `sqlite_wrapper::add_type_front<int, std::array<char,2>>`:
  `error: constraints not satisfied ... 'std::array<char,2>' does not satisfy 'heterogeneous_tuple_like'`
  → `note: because '!std::ranges::range<...>' evaluated to false`. The concept name carries the
  reason. (The prose `static_assert` does NOT fire here — the constraint blocks instantiation first;
  this is inherent to the hybrid and is the accepted trade-off.)
- Direct `details::add_type_front<int, std::array<char,2>>::type` use → the full prose
  `static_assert` message, e.g.: *"add_type_front: cannot add a type to std::array or another
  homogeneous, range-backed tuple-like container; its element type is fixed. Convert to std::tuple
  first."*
- `add_type_front<char, std::tuple<int,int>>` and all existing tuple/pair cases: compile unchanged.
- Test-side detection is SFINAE-clean:
  `static_assert(!requires { typename add_type_front<int, std::array<int,2>>; });` compiles.

## Changes made

1. `include/sqlite_wrapper/tuple_utils.h`
   - Added `details::always_false<...>` (dependent-false helper for `static_assert`) and the public
     `heterogeneous_tuple_like` concept.
   - Constrained the four public aliases `add_type_front`, `add_type_back`, `remove_type_front`,
     `remove_type_back` with `heterogeneous_tuple_like` (replacing `tuple_like`), and added
     `std::remove_cvref_t` to the `remove_type_front/back` aliases so cv/ref-qualified inputs
     normalize the same way `add_type_*` already did (this also resolves item #2/#8 from the broader
     `docs/tuple_utils-Plan.md` review).
   - For each of the four `details::` structs: the previously declared-only primary template is now
     defined with a generic `static_assert("... unsupported tuple-like type; only std::tuple and
     std::pair are supported")`, and a new `array_like`-constrained partial specialization carries an
     op-specific prose message. The structural `std::tuple<Args...>` / `std::pair<U,V>`
     specializations are unchanged and win by partial ordering, so homogeneous tuples are unaffected
     (verified for all four ops).
   - Added a direct `#include <ranges>` (previously only transitively available via `concepts.h`).

2. `test/tuple_utils_test.cpp`
   - Added positive `static_assert`s: homogeneous `std::tuple<int,int>` routing correctly through all
     four ops, and cv/ref-qualified (`const std::tuple<...>&`) inputs for all four ops (the latter
     previously hard-errored for `remove_type_front/back`, per item #2 above).
   - Added a `can_add_type_front`/`can_add_type_back`/`can_remove_type_front`/`can_remove_type_back`
     concept quartet (local, anonymous-namespace) used to assert via SFINAE that `std::tuple<char,
     float>` is accepted and `std::array<int,2>` (including `const std::array<int,2>&`) is rejected,
     for all four ops.

## Verification performed

- Compile probes on both g++-14 and clang++-20 confirmed: all pre-existing positive cases unchanged;
  homogeneous-tuple and cv/ref cases now pass; `std::array` cases are SFINAE-rejectable (no hard
  error) and produce the expected diagnostics at both the public-alias and direct-`details::` layers.
- Full project build via `ninja test_runner` (and `sqlite_wrapper.run_all_tests`) on both the
  `linux-debug` and `linux-release` presets, with clang-tidy (`clang-tidy-20`, `WarningsAsErrors: '*'`)
  wired into the compile step — no warnings, no clang-tidy findings.
- `test_runner --gtest_filter='tuple_utils_tests.*'` (10/10 passed) and the full
  `sqlite_wrapper.run_all_tests` target (`test_runner`: 21/21, `test_runner_mocked`: 34/34).

## Out of scope (tracked separately in `docs/tuple_utils-Plan.md`)

Zero-length `std::array` breaking `to_array`/`convert_to_array_type` (item #3/#7), and the
empty-tuple `remove_type_*` no-op semantics (item #4) — not addressed by this change.
