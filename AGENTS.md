# SheenBidi — Agent Guide

This document guides AI assistants working on the SheenBidi codebase. For user-facing documentation, see [README.md](README.md).

## Project Summary

SheenBidi is a lightweight, fast, thread-safe C implementation of the [Unicode Bidirectional Algorithm (UBA)](https://unicode.org/reports/tr9/). Version is defined in `Headers/SheenBidi/SBVersion.h`. Licensed under Apache 2.0.

**Key constraints:**

- Object-based, reference-counted API
- Depends only on `stddef.h`, `stdint.h`, `stdlib.h`, and `string.h`
- No third-party runtime dependencies

**Typical API data flow:**

```
SBCodepointSequence → SBAlgorithm → SBParagraph → SBLine → SBRun
```

## Repository Layout

| Path | Role |
|------|------|
| `Headers/SheenBidi/` | Public, installable API headers |
| `Source/API/` | Public API implementations + internal API headers |
| `Source/Core/` | Memory, objects, lists, atomics, TLS |
| `Source/UBA/` | Bidirectional algorithm engine |
| `Source/Data/` | **Generated** Unicode lookup tables — do not hand-edit |
| `Source/Text/` | Experimental text API internals |
| `Source/Script/` | Script resolution |
| `Source/SheenBidi.c` | Unity-build aggregator |
| `Tests/` | C++14 test harness (23 suites) |
| `Tools/` | Build-time tools: Parser (Unicode data files), Generator (regenerates `Source/Data/`) |

**Header duality:** Public declarations live in `Headers/SheenBidi/SB*.h`. Internal struct layouts and helpers live in `Source/API/SB*.h` and other `Source/*/` headers. Never expose internal struct definitions in public headers.

## Language Standards

### C Library — C89 (ANSI C)

The library targets **strict C89**. CI enforces this:

- CMake: `-DCMAKE_C_STANDARD=90 -DCMAKE_C_EXTENSIONS=OFF`
- Makefile: `-ansi -pedantic`
- Meson CI: `-Dc_std=c89`

**Rules for new and changed library code:**

- Use `/* */` comments only — no `//`
- Declare variables at the start of a block when possible
- Use braced `case` blocks to introduce locals inside `switch` (see `Source/API/SBCodepoint.c`)
- `stdint.h` fixed-width types are acceptable (`SBInt8`, `SBUInt32`, etc.)
- C11 atomics are feature-detected at compile time (`Source/API/SBBase.h`) — do not assume C11 for portable logic
- Avoid VLAs, compound literals outside macros, and other C99+ features

Some existing files contain C99-style declarations after statements. Match the nearest file when editing, but prefer C89-compliant patterns in new code.

### Tests — C++14

Tests use C++14 (`CMAKE_CXX_STANDARD 14`, Meson `cpp_std=c++14`, Makefile `-std=c++14`).

- Custom harness — no Google Test, Catch2, or similar
- Each suite is a `*Tests` class with a public `run()` and private `testXxx()` methods
- Assertions via `<cassert>`; progress output via `std::cout`
- Namespace: `SheenBidi`

## C Code Style

No automated formatter is configured. Match the conventions below, derived from existing headers and sources.

### Formatting

- **4 spaces** indentation, no tabs
- **K&R braces:** opening `{` on the same line as the function or control statement
- **`switch`:** `case` labels indented one level under `switch`; multi-statement cases use braced blocks
- Spaces around binary operators (`==`, `&&`, `||`)
- Align consecutive assignments in struct initialization and typedef blocks
- Soft line-length target: 80–100 columns

```c
SB_INTERNAL void SBCodepointSkipToStart(const void *buffer, SBUInteger length,
    SBStringEncoding encoding, SBUInteger *index)
{
    /* Index MUST be valid */
    SBAssert(*index < length);

    switch (encoding) {
    case SBStringEncodingUTF8: {
        const SBUInt8 *codeUnits = buffer;
        SBUInteger start = *index;
        /* ... */
        break;
    }
    default:
        break;
    }
}
```

### Naming

| Kind | Convention | Example |
|------|------------|---------|
| Public functions, types, macros | `SB` + PascalCase | `SBAttributeRegistryCreate`, `SBBidiTypeL` |
| Opaque references | `SB` + Name + `Ref` | `SBParagraphRef` |
| Internal structs | `_Tag` + short name + `Ref` | `BidiChain`, `BidiChainRef` |
| Internal functions | PascalCase module prefix | `InitializeList`, `BidiChainAdd` |
| Private struct fields | Leading `_` | `_base`, `_algorithm` |
| Local variables | camelCase | `isInitialized`, `dictIndex` |
| Boolean locals | `is` prefix | `isAllocated`, `isEnsured` |
| Config macros | `SB_CONFIG_*` | `SB_CONFIG_UNITY` |

### Visibility

- `SB_PUBLIC` — exported public API (in `Headers/SheenBidi/`)
- `SB_INTERNAL` / `SB_PRIVATE` — internal symbols; becomes `static` in unity builds

```c
#ifdef SB_CONFIG_UNITY
#define SB_INTERNAL static
#else
#define SB_INTERNAL
#endif
```

### Header Guards

| Scope | Pattern | Example |
|-------|---------|---------|
| Public | `_SB_PUBLIC_<NAME>_H` | `_SB_PUBLIC_ATTRIBUTE_REGISTRY_H` |
| Internal | `_SB_INTERNAL_<NAME>_H` | `_SB_INTERNAL_LIST_H` |
| Umbrella | `_SHEENBIDI_H` | `Headers/SheenBidi/SheenBidi.h` |

### Include Order

**Public headers** include siblings as `<SheenBidi/SBBase.h>` and wrap declarations in `SB_EXTERN_C_BEGIN` / `SB_EXTERN_C_END`.

**`.c` implementation files:**

1. Standard headers (`<stddef.h>`, `<stdlib.h>`, …)
2. Internal module headers via angle brackets: `<API/...>`, `<Core/...>`, `<UBA/...>`, `<Data/...>`
3. Quoted local companion: `#include "SBCodepoint.h"`

### Comments

- Apache 2.0 license block at the top of every file
- **Public API:** Doxygen `/** ... */` with `@param`, `@return`; backtick-quoted type names
- **Implementation:** `/* ... MUST ... */` before `SBAssert` calls
- **Section banners** in larger files: `/* ========= Section Name ========= */`
- **Generated data blocks:** `/* DATA_BLOCK: -- 0x0000..0x000F -- */`

```c
/**
 * Creates an attribute registry from an array of attribute infos.
 *
 * @param attributeInfos
 *      Pointer to an array of `SBAttributeInfo` entries.
 * @return
 *      A reference to the attribute registry instance, or `NULL` on failure.
 */
SB_PUBLIC SBAttributeRegistryRef SBAttributeRegistryCreate(...);
```

### Typedef and Enum Patterns

Fixed-width aliases with column-aligned names:

```c
typedef int8_t                      SBInt8;
typedef uint32_t                    SBUInt32;
```

Enum constants with a separate typedef (not combined `typedef enum`):

```c
enum {
    BidiFlagNone   = 0x00,
    BidiFlagSingle = 0x01
};
typedef SBUInt8 BidiFlag;
```

Struct with tag, short name, and pointer typedef:

```c
typedef struct _BidiChain {
    SBBidiType *types;
    SBLevel *levels;
    /* ... */
} BidiChain, *BidiChainRef;
```

Boolean constants:

```c
enum { SBFalse = 0, SBTrue = 1 };
typedef SBUInt8 SBBoolean;
```

Statement macros use `do { ... } while (0)`:

```c
#define SetItemValue(list_, index_, value_)             \
do {                                                    \
    CheckItemIndex(list_, index_);                      \
    (list_)->items[index_] = (value_);                  \
} while (0)
```

### Generated Files

Files in `Source/Data/` (e.g. `BidiTypeLookup.c`) are autogenerated with a `DO NOT EDIT!!` banner. Regenerate via `Tools/Generator` when Unicode data changes — never hand-edit these files.

Optional APIs are gated with `#if SB_TEXT_API_SUPPORTED` … `#endif` in public headers.

## C++ Test Code Style

- One suite per component: `FooTests.h` + `FooTests.cpp`, class `FooTests`
- Header guard: `_SHEENBIDI__FOO_TESTS_H`
- Member prefix `m_` for injected dependencies (e.g. `m_bidiTest`)
- Include public API as `<SheenBidi/...>`; internal C headers inside `extern "C" { ... }`
- C++14 features in use: `auto`, `constexpr`, `nullptr`, `u"..."` / `U"..."` literals, `std::thread`, `std::atomic`
- Two entry modes:
  - Monolithic: `Tests/main.cpp` runs all suites (Makefile build)
  - Standalone: `#ifdef STANDALONE_TESTING` per-file `main()` (CMake/Meson)

```cpp
namespace SheenBidi {

class AlgorithmTests {
public:
    void run();
private:
    void testBidiAlgorithm();
    Parser::BidiTest &m_bidiTest;
};

}
```

## Build and Test

### CMake (preferred for development)

```bash
cmake -S. -Bbuild -DCMAKE_BUILD_TYPE=Debug \
  -DSB_CONFIG_UNITY=OFF \
  -DSB_CONFIG_EXPERIMENTAL_TEXT_API=ON \
  -DCMAKE_C_STANDARD=90 -DCMAKE_C_EXTENSIONS=OFF
cmake --build build
ctest --test-dir build --output-on-failure
```

Run a single suite:

```bash
./build/AlgorithmTests Tools/Unicode
```

### Makefile (monolithic test binary)

```bash
make debug && make tests && make check
# runs: ./Debug/Tests Tools/Unicode
```

### Meson

```bash
meson setup builddir -Dunity_mode=disabled -Dtext_api=enabled
meson compile -C builddir
meson test -C builddir --print-errorlogs
```

### Important Build Flags

| Flag | Effect |
|------|--------|
| `SB_CONFIG_UNITY=ON` (default) | Single translation unit via `Source/SheenBidi.c` — CMake/Meson **skip building test targets entirely** when this is on |
| `SB_CONFIG_UNITY=OFF` | Per-file compilation — required for running tests |
| `SB_CONFIG_EXPERIMENTAL_TEXT_API` | Enables text editing API and 7 additional test suites |
| `SB_CONFIG_DLL_EXPORT` / `SB_CONFIG_DLL_IMPORT` | Windows shared library export/import |

Tests require `Tools/Unicode/` data passed as `argv[1]`.

CMake/Meson build **one standalone executable per suite** (e.g. `AlgorithmTests`, `OnceTests`), each compiled with a `STANDALONE_TESTING` define that enables the `#ifdef STANDALONE_TESTING` / `int main(...)` block at the bottom of each `Tests/*.cpp` file. The Makefile instead links every suite plus `Tests/main.cpp` into one monolithic `Debug/Tests` binary (no `STANDALONE_TESTING` define, so the per-file `main`s stay compiled out and `main.cpp`'s dispatcher runs all suites).

## Compile-Time Configuration

Defined in `Headers/SheenBidi/SBConfig.h`:

| Macro | Purpose |
|-------|---------|
| `SB_CONFIG_UNITY` | Unity build (single `.c` aggregator) |
| `SB_CONFIG_EXPERIMENTAL_TEXT_API` | Text editing and analysis API |
| `SB_CONFIG_DLL_EXPORT` / `SB_CONFIG_DLL_IMPORT` | Windows DLL linkage |
| `SB_CONFIG_DISABLE_SCRATCH_MEMORY` | Disable scratch memory in default allocator |
| `SB_CONFIG_SCRATCH_BUFFER_SIZE` | Scratch buffer size (default 8192) |
| `SB_CONFIG_SCRATCH_POOL_SIZE` | Number of scratch buffers (default 3) |
| `SB_CONFIG_ALLOW_NON_ATOMIC_FALLBACK` | Allow non-atomic fallback for threading primitives |

## Keeping Build Systems in Sync

CMake (`CMakeLists.txt`), Meson (`meson.build`), and Make (`Makefile`, `Tests/Makefile`, `Tools/Parser/Makefile`) each enumerate library, test, and parser source files **explicitly** (no globbing for the checked-in file lists that matter for releases). When adding, removing, or renaming a `.c`/`.h`/`.cpp` file under `Source/`, `Tests/`, or `Tools/Parser/`, update all three build definitions, plus the CMake `set(<TestName> ...)` / `TEST_TARGETS` list and the Meson test-target loop if it's a new test suite.

## Continuous Integration

GitHub Actions (`.github/workflows/{linux,macos,windows}.yml`) run on push/PR to `master` and `develop`:

- **Linux & macOS:** CMake and Meson, each in Release (unity, best-effort/`continue-on-error`) and Debug (non-unity, `ENABLE_ASAN=ON`, `ENABLE_UBSAN=ON`) configurations; Debug tests run via `ctest`/`meson test`.
- **Linux only:** a third job builds with plain `make` and runs the monolithic `Debug/Tests` binary under Valgrind (`--leak-check=full`, `--track-origins=yes`); GCC's Debug CMake build also collects coverage (`lcov`) uploaded to Coveralls.
- **Windows:** CMake and Meson with MSVC, both `x64` and `Win32`, Debug builds use `-Db_sanitize=address` (Meson) / `ENABLE_ASAN=ON` (CMake); no UBSan (MSVC doesn't support it).

All CI jobs build with `SB_CONFIG_EXPERIMENTAL_TEXT_API` **on**, so changes must keep the experimental text API compiling and passing even though it defaults to off for library consumers.

## Commits and Branches

- `master` is the stable/release branch; `develop` is the integration branch for ongoing work — base new work on `develop` unless told otherwise.
- Subject lines commonly use a bracketed scope tag followed by an imperative summary: `[lib] ...`, `[test] ...`, `[cmake] ...`, `[ci] ...`. Omit the tag only when a change doesn't fit a single scope (e.g. `Update README`).

## Agent Guidelines

### Do

- Match naming, brace style, and include patterns in the nearest file
- Add Doxygen documentation to new public API in `Headers/SheenBidi/`
- Put internal struct definitions in `Source/` headers only
- Run tests with unity mode **off** after substantive library changes
- Use `SBAssert` with a preceding `/* ... MUST ... */` comment for invariants
- Keep changes minimal and focused on the task at hand
- Update CMake, Meson, *and* Make file lists together when files are added/removed/renamed
- Base branch work on `develop`; use a `[scope]` commit tag matching the area touched

### Don't

- Hand-edit `Source/Data/*Lookup.c` files
- Use `//` comments in C sources
- Introduce third-party test frameworks or runtime dependencies
- Break C89 compatibility in library code
- Add dependencies beyond the four standard C headers
- Expose internal struct layouts in public headers

## Tools (Brief)

`Tools/Parser/` — C++ library that parses Unicode conformance data files in `Tools/Unicode/`. Linked by tests to load BidiTest, BidiCharacterTest, and related data.

`Tools/Generator/` — Regenerates `Source/Data/*Lookup.c` from Unicode data. Run when updating Unicode version data, not during routine feature work.
