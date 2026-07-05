# tuple_utils.h — bugs, problems & improvements

## Context

`include/sqlite_wrapper/tuple_utils.h` provides generic tuple/array type-level ops
(`add_type_front/back`, `remove_type_front/back`, `try_to_convert_to_array_type`) and
value-level ops (`pop_front/back`, `push_front/back`, `to_array`). All public aliases/functions
are constrained on the `tuple_like` / `array_like` concepts from `concepts.h`, which accept
**any** tuple-like type (`std::tuple`, `std::pair`, `std::array`, tuple-like structs) plus
cv/ref-qualified forms. The review below was verified by compiling probes against the real
header with `g++-14 -std=c++23`. Findings are ordered by severity.

## Confirmed bugs

### 1. Type-level ops break the promise of their `tuple_like` constraint (hard errors) — RESOLVED
`details::add_type_front/back` and `details::remove_type_front/back` are only specialized for
`std::tuple<...>` and `std::pair<...>`; the primary template is declared but undefined. Yet the
public aliases are constrained merely on `tuple_like`. Passing any other tuple-like type
(e.g. `std::array`, a tuple-like struct) satisfies the constraint but then hits an
**incomplete-type hard error** — no clean SFINAE/diagnostic.

Verified: `add_type_front<int, std::array<char,2>>` →
`error: invalid use of incomplete type 'struct details::add_type_front<int, std::array<char,2>>'`.

- Files: `include/sqlite_wrapper/tuple_utils.h:15-99, 128-138`.

**Resolution (implemented):** rather than generalizing add/remove-type to *all* `tuple_like` types
(the `#6` sketch below), the decision was that `std::array` (and other homogeneous, range-backed
containers) genuinely can't support add/remove-type — their element type/size is fixed — so the fix
is a clear, SFINAE-friendly compile-time rejection instead of silent support. See
`docs/tuple_utils-array-diagnostic-Plan.md` for the detailed design (a `heterogeneous_tuple_like`
concept gating the four public aliases, plus a `static_assert` explainer in `details::` for anyone
bypassing the alias directly). Homogeneous **tuples** like `std::tuple<int,int>` remain fully
supported — only range-backed containers are rejected.

### 2. `remove_type_front/back` are not cv/ref-robust (inconsistent with `add_type_*`) — RESOLVED
`add_type_front/back` normalize input with `std::remove_cvref_t<Tuple>` (line 129, 132), but
`remove_type_front/back` forward `Tuple` **raw** into `details::remove_type_front<Tuple>`
(line 135, 138). Since `details::remove_type_front` has no cv/ref specializations, a
`const&`-qualified tuple — which `tuple_like` accepts — hard-errors.

Verified: `remove_type_front<const std::tuple<char,float>&>` →
`error: invalid use of incomplete type ...`, whereas the analogous `add_type_front<int, const std::tuple<char>&>` compiles.

- Files: `include/sqlite_wrapper/tuple_utils.h:135, 138`.

**Resolution (implemented):** fixed alongside #1/#8 — the `remove_type_front/back` public aliases
now apply `std::remove_cvref_t<Tuple>` before forwarding to `details::`, matching `add_type_*`.

### 3. `to_array` / `convert_to_array_type` hard-error on zero-length `std::array`
`std::array<T,0>` satisfies `array_like` (it is a `range`), but both `to_array` (line 185) and
the array-conversion specialization (line 115) compute `std::tuple_element_t<0, T>`, which is
ill-formed for a zero-size array → hard error inside `<array>`.

Verified for both `convert_to_array_type<std::array<int,0>>` and `to_array(std::array<int,0>{})`.

- Files: `include/sqlite_wrapper/tuple_utils.h:115, 185`.

## Problems / questionable semantics

### 4. `remove_type_front/back` on an empty tuple silently no-op
`remove_type_front<std::tuple<>>` and `remove_type_back<std::tuple<>>` both yield `std::tuple<>`
(lines 49-53, 71-75). Removing from an empty tuple arguably should be ill-formed. The tests
themselves flag this: `test/tuple_utils_test.cpp:51,64` carry `// TODO: desired behavior?`.
Needs a decision, not necessarily a code change.

### 5. Reference-carrying tuples are silently decayed by `pop_front/back`
`pop_*` build their result via `std::make_pair`/`std::make_tuple`, which decay element types.
For `push_*` this is documented ("use `std::ref` to add reference types", lines 163, 172), but
`pop_*` have no such note while behaving the same way. Minor: add a matching doc comment.

## Possible improvements

### 6. Generalize the type-level ops to all `tuple_like` types (fixes #1 and #2 at once) — NOT ADOPTED
This sketch was superseded: `std::array` add/remove-type doesn't have a sensible meaning (its element
type/size is fixed), so it was rejected with a clear diagnostic instead of being made to "work". See
the resolution notes on #1/#2/#8 above and `docs/tuple_utils-array-diagnostic-Plan.md`.

Replace the per-shape (`tuple`/`pair`) specializations with a single normalization step that maps
any `tuple_like` to a `std::tuple` of its elements, then does the front/back manipulation with an
`index_sequence`. Sketch:
```cpp
template <tuple_like T>
using as_tuple = decltype([]<std::size_t... I>(std::index_sequence<I...>)
  -> std::tuple<std::tuple_element_t<I, std::remove_cvref_t<T>>...> { return {}; }
  (std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<T>>>()));
```
Then define `add_type_front/back`, `remove_type_front/back` in terms of `as_tuple<T>`, dropping the
special `std::pair` overloads (a normalized `std::pair` becomes `std::tuple<U,V>` automatically).
This makes the impl honor the `tuple_like` constraint and become cv/ref-robust uniformly. Keep the
existing static_asserts in `test/tuple_utils_test.cpp` green and add cases for `std::array` /
cv-ref inputs.

### 7. Guard array conversions against zero size
Add a `requires (std::tuple_size_v<...> >= 1)` (or a size-0 → `std::array<T,0>` passthrough) to the
`try_to_convert_to_array_type_impl` array specialization and to `to_array`, so empty arrays don't
hard-error. Decide whether empty is supported or cleanly rejected.

### 8. Minor consistency: missing `remove_cvref_t` — RESOLVED
Even if #6 isn't adopted, at minimum apply `std::remove_cvref_t<Tuple>` in `remove_type_front/back`
(lines 135, 138) to match `add_type_*`.

**Resolution (implemented):** see #2 above — done as part of the array-diagnostic change instead of
the broader `#6` generalization.

## Verification

- Add/extend `static_assert`s in `test/tuple_utils_test.cpp` covering: `std::array` and tuple-like
  struct inputs to `add_type_*`/`remove_type_*`; cv/ref-qualified inputs; zero-length `std::array`
  into `to_array`/`convert_to_array_type`.
- Build + run the tuple-utils tests both ways:
  `cmake --build <binaryDir> --target sqlite_wrapper.run_all_tests`, or directly
  `<binaryDir>/bin/test_runner --gtest_filter='tuple_utils_tests.*'` and the `_mocked` variant.
- Sanity: re-run the compile probes from this review (they currently error) and confirm they
  compile after the fix.

## Note

This started as a read-only review. Item #4 is a design decision for the user; the rest are
concrete fixes. Recommend implementing #6 (subsumes #1, #2, #8) + #7, then resolving #4.