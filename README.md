# data-management

This repository contains a generic C++ data management library designed for game projects. It provides functionalities for data persistence, serialization, and deserialization.

![Windows](https://github.com/nircoe/data-management/actions/workflows/ci.yml/badge.svg?branch=main&event=push&label=Windows)
![Linux](https://github.com/nircoe/data-management/actions/workflows/ci.yml/badge.svg?branch=main&event=push&label=Linux)
![macOS](https://github.com/nircoe/data-management/actions/workflows/ci.yml/badge.svg?branch=main&event=push&label=macOS)

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

The library offers a base implementation for data management tasks, including:

- Reading and writing game data to files
- Serialization and deserialization of game data using [nlohmann/json](https://github.com/nlohmann/json)
- Encryption and decryption of data using [CryptoPP](https://github.com/weidai11/cryptopp)
- Comprehensive test suite using [Google Test](https://github.com/google/googletest)

[Back to top](#table-of-contents)

## Features

- Basic error handling for file operations
- JSON-based serialization and deserialization
- AES encryption for secure data storage
- Memory-safe implementation
- Extensive test suite including:
  - Basic functionality
  - Error handling
  - Performance benchmarks
  - Memory usage

[Back to top](#table-of-contents)

## Architecture

The library follows a layered architecture:

```
┌─────────────────────────────────────┐
│           Game Application          │
└───────────────┬─────────────────────┘
                │
┌───────────────▼─────────────────────┐
│           DataManager               │
│  (High-level data management API)   │
└───────────────┬─────────────────────┘
                │
┌───────────────▼─────────────────────┐
│         DataReaderWriter            │
│    (File I/O and Encryption)        │
└───────────────┬─────────────────────┘
                │
┌───────────────▼─────────────────────┐
│            GameData                 │
│      (Data Structure Model)         │
└─────────────────────────────────────┘
```

- **GameData**: Core data model with serialization/deserialization
- **DataReaderWriter**: Handles reading/writing and encryption/decryption
- **DataManager**: Provides high-level interface for game integration

[Back to top](#table-of-contents)

## Building and Usage

### Prerequisites

- CMake 3.14 or higher
- C++17 compatible compiler
- Git

### Build Steps

```bash
# Clone the repository with submodules
git clone --recurse-submodules https://github.com/yourusername/data-management.git
cd data-management

# Create a build directory
mkdir build && cd build

# Configure and build
cmake ..
cmake --build .

# Run tests (optional)
./all_tests
```

### Integration Steps

1. **Fork the Repository:**

   Fork this repository to your own GitHub account.

2. **Add as a Submodule:**

   Add your forked repository as a Git submodule to your game project.

3. **Modify the Files:**

   Modify the `GameData.hpp/cpp` and `DataManager.hpp/cpp` files to include your game's data structures and logic.

4. **Integrate into Your Project:**

   Add the library to your project's CMakeLists.txt:
   ```cmake
   add_subdirectory(path/to/data-management)
   ```
   
   Then link your executable with the library:
   ```cmake
   target_link_libraries(your_game_executable PRIVATE data_management)
   ```

[Back to top](#table-of-contents)

## API Overview and Examples

### Core Components

The library consists of three main components:

1. **GameData**: Represents the game's save data structure
2. **DataReaderWriter**: Handles low-level file I/O, encryption, and decryption
3. **DataManager**: High-level interface for managing game data (recommended entry point)

### Basic Usage Examples

#### Initialize and Save Game Data

```cpp
#include "data_manager.hpp"

// Initialize with a file path
DataManagement::DataManager manager;
manager.init("save_game.json");

// Set game data
manager.setNickName("Player1");
manager.setHighScore(1000);

// Save to disk
bool saveSuccess = manager.saveGame();
```

#### Load Game Data

```cpp
#include "data_manager.hpp"

// Initialize with the same file path
DataManagement::DataManager manager;
manager.init("save_game.json");

// Load from disk (this happens automatically on init, but can be called explicitly)
manager.loadGame();

// Access game data
const DataManagement::GameData& data = manager.getGameData();
std::string playerName = data.getNickName();
int score = data.getHighscore();
```

#### Create New Game

```cpp
#include "data_manager.hpp"

DataManagement::DataManager manager;
manager.init("save_game.json");

// Reset to default values
manager.newGame();

// Set initial values
manager.setNickName("NewPlayer");
manager.setHighScore(0);

// Save the new game
manager.saveGame();
```

### Extending for Your Game

To adapt this library for your game, you'll need to modify the core components to fit your specific needs:

1. **GameData**: 
   - Update `game_data.hpp` and `game_data.cpp` with your game's data fields
   - Modify the `toJson()` and `fromJson()` methods to handle your custom data

2. **DataManager**:
   - Extend `data_manager.hpp` and `data_manager.cpp` if you need additional management functionality
   - Add game-specific methods for manipulating your custom data

3. **DataReaderWriter**:
   - Customize encryption/decryption settings if needed
   - Add additional file formats or I/O methods as required

4. **Tests**:
   - Update the tests to reflect your data structures and functionality

[Back to top](#table-of-contents)

## Dependencies

All dependencies are now automatically handled:

- **CryptoPP:** Automatically fetched via the cryptopp-cmake Git submodule (currently points to version 8.9.0)
- **nlohmann/json:** Automatically fetched by CMake during configuration (currently version 3.11.3)
- **Google Test:** Automatically fetched by CMake during configuration (currently version 1.16.0)

You no longer need to manually download or build these dependencies.

### Updating Dependencies (optional)

#### Updating the cryptopp-cmake submodule

If you want to update the cryptopp-cmake submodule to a different version:

```bash
# Navigate to the cryptopp-cmake directory
cd external/cryptopp-cmake

# Fetch all tags
git fetch --tags

# List available tags
git tag -l

# Checkout the specific tag you want
git checkout <tag_name>  # e.g., CRYPTOPP_8_9_0

# Return to the main project directory
cd ../..

# Now commit the submodule update
git add external/cryptopp-cmake
git commit -m "Update cryptopp-cmake to <tag_name>"
```

#### Updating nlohmann/json and Google Test

To update nlohmann/json or Google Test to newer versions, modify the FetchContent_Declare section in your CMakeLists.txt:

For nlohmann/json:
```cmake
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3  # Change this to the desired version
)
```

For Google Test:
```cmake
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/<commit_hash>.zip  # Update URL with desired version
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
```

[Back to top](#table-of-contents)

## Testing

The project includes a comprehensive test suite built with Google Test. Tests cover:

- Basic data operations
- Error handling and recovery
- Thread safety and concurrency
- Performance benchmarks
- Memory usage

### Running Tests

To run all tests:

```bash
cd build
./all_tests
```

To build and run individual test executables, enable the `BUILD_INDIVIDUAL_TESTS` option:

```bash
cmake -DBUILD_INDIVIDUAL_TESTS=ON ..
cmake --build .
./error_handling_tests  # Run a specific test
```

### Customizing Tests

You'll need to modify the test files to match your game's data structures. The test files are located in the `tests/` directory:

1. Update the test fixture classes to match your game data structure
2. Modify the test cases to use your specific data types and expected values
3. Add or remove tests as needed for your specific requirements

### Disabling Tests

If you don't need the tests in your project, you can disable them by:

1. Removing or commenting out these lines in your CMakeLists.txt:
   ```cmake
   # Remove these lines to disable tests
   FetchContent_Declare(googletest ...)
   FetchContent_MakeAvailable(googletest)
   add_subdirectory(tests)
   ```

2. Or, you can conditionally include tests using a CMake option:
   ```cmake
   option(BUILD_TESTS "Build the test suite" ON)
   if(BUILD_TESTS)
     FetchContent_Declare(googletest ...)
     FetchContent_MakeAvailable(googletest)
     add_subdirectory(tests)
   endif()
   ```
   Then use `-DBUILD_TESTS=OFF` when configuring CMake to disable tests.

[Back to top](#table-of-contents)

## Continuous Integration

This project uses GitHub Actions for continuous integration to ensure code quality and compatibility across different platforms and compilers.

### CI Pipeline Status

[![Data Management Build and Test](https://github.com/nircoe/data-management/actions/workflows/ci.yml/badge.svg)](https://github.com/nircoe/data-management/actions/workflows/ci.yml)

### Supported Platforms and Compilers

Our CI pipeline automatically builds and tests the project with the following configurations:

- **Windows**:
  - Visual Studio (MSVC)
  - Clang
  - GCC (MinGW)
  
- **Linux**:
  - GCC
  - Clang
  
- **macOS**:
  - Apple Clang

Each configuration compiles the project and runs the full test suite to verify compatibility and functionality. All supported platforms use C++17 features for maximum portability.

Platform-specific code (such as file permissions handling) is wrapped in conditional compilation blocks to ensure proper behavior across different operating systems.

### For [Contributors](#contributing)

Pull requests will automatically trigger the CI pipeline. All checks must pass before a PR can be merged to ensure the codebase remains stable. If you encounter CI failures in your PR, please check the build logs for details on what went wrong.

[Back to top](#table-of-contents)

## Repository Organization

### Game-Specific Branches

The main repository contains several game-specific branches that demonstrate how this library has been customized for different games:

- `game/worm`: Contains the data management implementation for the "Worm" game
- `game/future-game`: Example of another game-specific branch

Note: While I use this branching strategy for my own projects, if you fork the repository, you may simply customize the main branch for your specific game.

### Tag Releases

The main repository uses tags to mark stable releases of the template library:

- `v0.1.0`: Initial release with basic functionality

Users should select their desired version of the template by checking out the appropriate tag before making modifications:

```bash
# Clone the repository
git clone https://github.com/yourusername/data-management.git

# List available tags
git tag -l

# Checkout specific version
git checkout v0.1.0  # Or other version tag
```

Game-specific implementations will have their own tags (e.g., `worm-v1.0.0`) to track which version of the template they were built from.

[Back to top](#table-of-contents)

## Version History

### v0.1.0 (Initial Development)
- Basic data management functionality
- JSON serialization
- AES encryption
- Comprehensive test suite
- Automated dependency management

[Back to top](#table-of-contents)

## Contributing

Contributions to this library are welcome! Here are some ways you can contribute:

- Implementing features from the roadmap
- Fixing bugs
- Improving documentation
- Adding new features not listed in the roadmap
- Enhancing test coverage

If you'd like to contribute, please:

1. Fork the repository
2. Create a new branch for your feature (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add some amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

[Back to top](#table-of-contents)

## Roadmap

### Implemented Features
- ✅ Basic file input/output operations
- ✅ JSON serialization using nlohmann/json
- ✅ AES encryption/decryption using CryptoPP
- ✅ Comprehensive test suite with Google Test
- ✅ Automated dependency management

### Planned Improvements
- ⏳ Optional encryption (ability to disable encryption if not needed)
- ⏳ Secure encryption key management (replacing fixed keys with secure storage and derivation)
- ⏳ Graceful recovery from corrupted files with backup system
- ⏳ Thread-safe operations for concurrent data access
- ⏳ Asynchronous save/load operations
- ⏳ Performance optimizations for large data sets
- ⏳ Auto-save functionality with configurable intervals
- ⏳ Save data compression
- ⏳ Save data versioning and migration
- ⏳ Multiple save slot system with profile management
- ⏳ Support for additional build systems (Make, Visual Studio, Meson, etc.)
- ⏳ Cloud save integration capabilities
- ⏳ Save data analytics and statistics

[Back to top](#table-of-contents)

## License

This library is licensed under the [MIT License](LICENSE).

[Back to top](#table-of-contents)