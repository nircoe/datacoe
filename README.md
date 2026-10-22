# datacoe

datacoe is a small, generic C++ data management library for game development.
It handles data persistence, serialization and optional encryption.

[![Windows](https://github.com/nircoe/datacoe/actions/workflows/ci-windows.yml/badge.svg?branch=main&event=push)](https://github.com/nircoe/datacoe/actions/workflows/ci-windows.yml)
[![Linux](https://github.com/nircoe/datacoe/actions/workflows/ci-linux.yml/badge.svg?branch=main&event=push)](https://github.com/nircoe/datacoe/actions/workflows/ci-linux.yml)
[![macOS](https://github.com/nircoe/datacoe/actions/workflows/ci-macos.yml/badge.svg?branch=main&event=push)](https://github.com/nircoe/datacoe/actions/workflows/ci-macos.yml)

## Current State - Refactoring towards a Generic Data API

The generic, templated `save`/`load` API has landed. The library no longer ships its own data type.

---

## Table of Contents
- [Overview](#overview)
- [Features](#features)
- [Architecture](#architecture)
- [Building and Usage](#building-and-usage)
- [API Overview and Examples](#api-overview-and-examples)
- [Dependencies](#dependencies)
- [Testing](#testing)
- [Continuous Integration](#continuous-integration)
- [Repository Organization](#repository-organization)
- [Version History](#version-history)
- [Contributing](#contributing)
- [Roadmap](#roadmap)
- [License](#license)

## Overview

The library covers:

- Reading and writing game data to files
- Serialization and deserialization of game data using [nlohmann/json](https://github.com/nlohmann/json)
- Optional encryption and decryption of data using [CryptoPP](https://github.com/weidai11/cryptopp) (built with [CryptoPP-CMake](https://github.com/abdes/cryptopp-cmake))
- Test suite using [Google Test](https://github.com/google/googletest), with [testcoe](https://github.com/nircoe/testcoe) adding grid visualization and crash reporting

[Back to top](#table-of-contents)

## Features

- Basic error handling for file operations
- JSON-based serialization and deserialization
- Optional AES encryption, off by default (see [Encryption](#encryption))
- Test suite covering:
  - Saveable concept and the I/O layer
  - Save/load round trips
  - Error handling
  - Integration
  - Memory usage
  - Performance benchmarks

[Back to top](#table-of-contents)

## Architecture

The library is a header-only `save`/`load` API on top of a small compiled I/O layer:

```
┌─────────────────────────────────────┐
│        Your saveable struct         │
│       (defined in your game)        │
└──────────────────┬──────────────────┘
                   │
┌──────────────────▼──────────────────┐
│             save / load             │
│       (datacoe/save_load.hpp)       │
└──────────────────┬──────────────────┘
                   │
┌──────────────────▼──────────────────┐
│     Internal I/O and AES layer      │
│     (src/datacoe/utils/io.cpp)      │
└─────────────────────────────────────┘
```

- `datacoe/save_load.hpp` is the only header you include
- Any struct that is default constructible, copyable and has nlohmann/json `to_json`/`from_json` is `saveable`
- The I/O layer (`datacoe::internal`) is not public API

[Back to top](#table-of-contents)

## Building and Usage

### Prerequisites

- CMake 3.22 or higher
- C++23 compatible compiler, with exceptions enabled (the public header uses them). Linking `datacoe` sets the C++23
  requirement on your target
- Git

### Build Steps

```bash
# Clone the repository
git clone https://github.com/yourusername/datacoe.git
cd datacoe

# Create a build directory
mkdir build && cd build

# Configure (with tests and encryption) and build
cmake -DDATACOE_BUILD_TESTS=ON -DDATACOE_ENABLE_ENCRYPTION=ON ..
cmake --build .

# Run tests (only built with DATACOE_BUILD_TESTS=ON)
./tests/datacoe_tests
```

Dependencies are downloaded with FetchContent at configure time, so the first configure needs network access.

### Encryption

Encryption is off by default.
`DATACOE_ENABLE_ENCRYPTION` turns it on, and CryptoPP is downloaded, built and linked only then.
With it off:

- `save(path, data, true)` returns `encryption_failure` and writes nothing.
- `load` of an encrypted file returns `decryption_failure`.

Plain saves work the same either way. When using datacoe through FetchContent, set the option before
`FetchContent_MakeAvailable(datacoe)`:

```cmake
set(DATACOE_ENABLE_ENCRYPTION ON)
```

### Integration Steps

1. **Fork the Repository:**

   Fork this repository to your own GitHub account.

2. **Modify the Files:**

   You don't edit any library files. Define your own `saveable` struct in your game (see
   [Your own data](#your-own-data)).

3. **Add to Your Project with FetchContent:**

   Point FetchContent at your fork in your game's `CMakeLists.txt`:

   ```cmake
   include(FetchContent)
   FetchContent_Declare(
       datacoe
       GIT_REPOSITORY https://github.com/<your-username>/datacoe.git
       GIT_TAG main
   )
   FetchContent_MakeAvailable(datacoe)

   target_link_libraries(your_game_executable PRIVATE datacoe)
   ```

[Back to top](#table-of-contents)

## API Overview and Examples

### Define a saveable struct

Define your own struct and add one macro line right after it, at namespace scope in the same namespace:

```cpp
#include <datacoe/save_load.hpp>
#include <string>
#include <vector>

namespace game
{
    struct save_data
    {
        std::string nickname;
        int highscore = 0;
        std::vector<int> unlocked_levels;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(save_data, nickname, highscore, unlocked_levels)
}
```

A key missing from an old save falls back to the field's default. Passing a type that is not `saveable` to `save`
or `load` gives a single `static_assert` message.

- Members must own their data (no `std::string_view`, pointers or other views), because `load` builds the value from
  a temporary json.
- A struct derived from a saveable struct needs its own macro call. Otherwise the base's functions are found and the
  derived members are silently not saved.
- Values are converted as leniently as nlohmann does, so a hand-edited `12.9` loads as `12`, `true` as `1`, and
  out-of-range numbers are undefined behavior. If you want strict loading, write your own `from_json`.

### Save

```cpp
game::save_data data{"Player1", 1000, {1, 2}};

// Plain JSON
auto saved = datacoe::save("save_game.json", data);

// AES encrypted (needs DATACOE_ENABLE_ENCRYPTION=ON)
auto encrypted = datacoe::save("save_game.json", data, true);
```

`save` returns `std::expected<void, datacoe::error>`. It writes `<path>.datacoe.tmp` and then renames it over the
target, so a crash or a write error never destroys the old save. Power loss is not covered, the standard library has
no fsync.

- `<path>.datacoe.tmp` is reserved, don't keep other files under that name.
- The save file is replaced, so a symlink pointing at it is not supported (it becomes a regular file and the
  permissions reset to the default). A symlinked directory works.
- Values that couldn't be loaded back, like NaN/Inf floats and nlohmann binary values, return
  `serialization_failure` and the old save stays.

### Load

```cpp
auto loaded = datacoe::load<game::save_data>("save_game.json");
if (loaded)
{
    game::save_data data = *loaded;
}
else if (loaded.error().code == datacoe::error_code::file_not_found)
{
    // No save yet, start a new game
    game::save_data data;
}
else
{
    // Any other failure, loaded.error().message says what went wrong
}
```

`load` returns `std::expected<T, datacoe::error>`. Encryption is detected from the file, there is no flag to pass.

### Your own data

Define your struct and add the macro line. That is it, no library files to change.

To store several data types, either compose them into one struct, or use one file per concern (for example
`settings.json` plus a save slot file). With a composed struct, each nested struct needs its own macro call and
must be default constructible:

```cpp
namespace game
{
    struct everything
    {
        save_data save;
        int volume = 5;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(everything, save, volume)
}
```

[Back to top](#table-of-contents)

## Dependencies

All dependencies are automatically handled:

- **[CryptoPP-CMake](https://github.com/abdes/cryptopp-cmake):** Fetched and built by CMake during configuration only if `DATACOE_ENABLE_ENCRYPTION` is ON, via `cmake/cryptopp.cmake` - currently pinned to CRYPTOPP_8_9_0
- **[nlohmann/json](https://github.com/nlohmann/json):** Fetched by CMake during configuration, via `cmake/json.cmake` - currently pinned to v3.11.3
- **[testcoe](https://github.com/nircoe/testcoe):** Adds grid visualization and crash reporting to Google Test. Automatically fetched by CMake during configuration only if `DATACOE_BUILD_TESTS` is ON, via `cmake/testcoe.cmake` - currently pinned to v0.2.0. Pulls in Google Test transitively.

If your project already provides `nlohmann_json` or `cryptopp` (with encryption ON) before datacoe is added, datacoe skips fetching its own copy and uses yours. Fetch yours first.

### Updating Dependencies (optional)

To update a dependency, bump the `GIT_TAG` in `cmake/cryptopp.cmake`, `cmake/json.cmake` or `cmake/testcoe.cmake`.
For example, testcoe:

```cmake
FetchContent_Declare(
    testcoe
    GIT_REPOSITORY https://github.com/nircoe/testcoe.git
    GIT_TAG v0.2.0  # Update to desired version
    GIT_SHALLOW TRUE
)
```

[Back to top](#table-of-contents)

## Testing

The project includes a test suite built with Google Test, run through a CLI dispatcher (`tests/main.cpp`) built on testcoe. Tests cover:

- Saveable concept and the I/O layer
- Save/load round trips
- Error handling
- Integration
- Memory usage
- Performance benchmarks

### Running Tests

To run all tests:

```bash
cd build
./tests/datacoe_tests
```

Filter to a suite or a single test:

```bash
./tests/datacoe_tests --suite=SaveLoadTests
./tests/datacoe_tests --test=SaveLoadTests.PlainRoundTrip
```

To build and run individual test executables, enable the `DATACOE_BUILD_INDIVIDUAL_TESTS` option before `FetchContent_MakeAvailable(datacoe)`:

```cmake
set(DATACOE_BUILD_INDIVIDUAL_TESTS ON)
```

Then build and run a specific test:

```bash
cmake --build .
./tests/error_handling_tests
```

When building datacoe standalone, pass it on the command line instead: `cmake -DDATACOE_BUILD_TESTS=ON -DDATACOE_BUILD_INDIVIDUAL_TESTS=ON ..`

The encryption tests are skipped when `DATACOE_ENABLE_ENCRYPTION` is OFF.

### Customizing Tests

The tests use their own test-local `saveable` structs, defined in `tests/support/test_support.hpp`. There is
nothing game-specific to rewrite.

### Enabling Tests

Tests are off by default. To build them, enable the `DATACOE_BUILD_TESTS` option before `FetchContent_MakeAvailable(datacoe)`:

```cmake
set(DATACOE_BUILD_TESTS ON)
```

This fetches testcoe (and Google Test) and builds the test suite.

[Back to top](#table-of-contents)

## Continuous Integration

CI runs on GitHub Actions on pushes and pull requests.

### CI Pipeline Status

[![Windows](https://github.com/nircoe/datacoe/actions/workflows/ci-windows.yml/badge.svg?branch=main&event=push)](https://github.com/nircoe/datacoe/actions/workflows/ci-windows.yml)
[![Linux](https://github.com/nircoe/datacoe/actions/workflows/ci-linux.yml/badge.svg?branch=main&event=push)](https://github.com/nircoe/datacoe/actions/workflows/ci-linux.yml)
[![macOS](https://github.com/nircoe/datacoe/actions/workflows/ci-macos.yml/badge.svg?branch=main&event=push)](https://github.com/nircoe/datacoe/actions/workflows/ci-macos.yml)

### Supported Platforms and Compilers

CI builds and tests the project with the following configurations:

- **Windows**:
  - Visual Studio (MSVC)
  - GCC (MinGW)

- **Linux**:
  - GCC
  - Clang

- **macOS**:
  - Apple Clang

Each configuration compiles the project with `DATACOE_ENABLE_ENCRYPTION=ON`
and runs the full test suite to verify compatibility and functionality.
The OFF build is not in CI. The Linux workflow also runs a clang-format check job.

### For [Contributors](#contributing)

Pull requests trigger the CI pipeline. All checks must pass before a PR can be merged.

[Back to top](#table-of-contents)

## Repository Organization

### Game-Specific Branches

The main repository contains several game-specific branches that demonstrate how this library has been customized for different games:

- `game/worm`: Contains the data management implementation for my game "Worm"
- `game/future-game`: Example of another game-specific branch

Note: While I use this branching strategy for my own projects, if you fork the repository, you may simply customize the main branch for your specific game.

### Tag Releases

The main repository uses tags to mark stable releases of the template library:

- `v0.1.0`: Initial release with basic functionality

Users should select their desired version of the template by checking out the appropriate tag before making modifications:

```bash
# Clone the repository
git clone https://github.com/yourusername/datacoe.git

# List available tags
git tag -l

# Checkout specific version
git checkout v0.1.0
```

Game-specific implementations will have their own tags (e.g., `worm-v1.0.0`) to track which version of the template they were built from.

[Back to top](#table-of-contents)

## Version History

### [v0.1.0](https://github.com/nircoe/datacoe/releases/tag/v0.1.0) (Initial Release)
- Basic data management functionality
- JSON serialization using nlohmann/json
- AES encryption/decryption using CryptoPP
- Optional encryption with automatic format detection
- Test suite with Google Test
- Proper installation targets and CMake configuration
- Cross-platform support (Windows, macOS, Linux)

[Back to top](#table-of-contents)

## Contributing

Contributions are welcome: roadmap features, bug fixes, docs and tests.

To contribute:

1. Fork the repository
2. Create a new branch for your feature (`git checkout -b feature/amazing-feature`)
3. Format the changed C++ files with `clang-format -i` (config is `.clang-format` at the repo root)
4. Commit your changes (`git commit -m 'Add some amazing feature'`)
5. Push to the branch (`git push origin feature/amazing-feature`)
6. Open a Pull Request

[Back to top](#table-of-contents)

## Roadmap

### Implemented Features
- Basic file input/output operations
- Crash-safe save (write to a temp file, then rename)
- JSON serialization using nlohmann/json
- AES encryption/decryption using CryptoPP
- Test suite with Google Test
- Automated dependency management
- Optional encryption (ability to disable encryption if not needed)

### Planned Improvements
- Secure encryption key management (replacing fixed keys with secure storage and derivation)
- Backup of the previous save to recover from corrupted files
- Thread-safe operations for concurrent data access
- Asynchronous save/load operations
- Performance optimizations for large data sets
- Auto-save functionality with configurable intervals
- Save data compression
- Save data versioning and migration
- Multiple save slot system with profile management
- Support for additional build systems (Make, Visual Studio, Meson, etc.)
- Cloud save integration capabilities
- Save data analytics and statistics

[Back to top](#table-of-contents)

## License

This library is licensed under the [MIT License](LICENSE).

[Back to top](#table-of-contents)
