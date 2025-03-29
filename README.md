# datacoe

datacoe is a small, simple and generic C++ data management library template for game development,
It provides functionalities for data persistence, serialization, and encryption.

[![Windows](https://github.com/nircoe/datacoe/actions/workflows/ci-windows.yml/badge.svg?branch=main&event=push)](https://github.com/nircoe/datacoe/actions/workflows/ci-windows.yml)
[![Linux](https://github.com/nircoe/datacoe/actions/workflows/ci-linux.yml/badge.svg?branch=main&event=push)](https://github.com/nircoe/datacoe/actions/workflows/ci-linux.yml)
[![macOS](https://github.com/nircoe/datacoe/actions/workflows/ci-macos.yml/badge.svg?branch=main&event=push)](https://github.com/nircoe/datacoe/actions/workflows/ci-macos.yml)

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
git clone --recurse-submodules https://github.com/yourusername/datacoe.git
cd datacoe

# Create a build directory
mkdir build && cd build

# Configure and build
cmake ..
cmake --build .

# Run tests (optional)
./tests/all_tests
```

### Integration Steps

1. **Fork the Repository:**

   Fork this repository to your own GitHub account.

2. **Add as a Submodule:**

   Add your forked repository as a Git submodule to your game project.

3. **Modify the Files:**

   Modify the `GameData.hpp/cpp` and `DataManager.hpp/cpp` files to include your game's data structures and logic.

4. **Integrate into Your Project:**

   There are two ways to integrate datacoe into your project:
   
   ### 4.1. Use as Subdirectory (Recommended for development)
   
   ```cmake
   add_subdirectory(path/to/datacoe)
   target_link_libraries(your_game_executable PRIVATE datacoe)
   ```

   ### 4.2. Install and Use with find_package (Better for distribution)
   
   ```cmake
   # Build and install datacoe
   cd path/to/datacoe
   mkdir build && cd build
   cmake ..
   cmake --build .
   cmake --install . --prefix <install_path>
   
   # In your project's CMakeLists.txt
   find_package(datacoe REQUIRED)
   target_link_libraries(your_game_executable PRIVATE datacoe)
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
#include <datacoe/data_manager.hpp>

datacoe::DataManager manager;
bool loadSuccess = manager.init("save_game.json");
// Note: When init() returns false, it indicates no existing save was found,
// and a new default GameData instance was created internally

// Create and set game data
datacoe::GameData gameData;
gameData.setNickname("Player1");
gameData.setHighscore(1000);
manager.setGamedata(gameData);

// Save to disk
bool saveSuccess = manager.saveGame();
```

#### Load Game Data

```cpp
#include <datacoe/data_manager.hpp>

datacoe::DataManager manager;
bool loadSuccess = manager.init("save_game.json");

// Load from disk (this happens automatically on init, but can be called explicitly)
manager.loadGame();

// Access game data
const datacoe::GameData& data = manager.getGamedata();
std::string playerName = data.getNickname();
int score = data.getHighscore();
```

#### Create New Game

```cpp
#include <datacoe/data_manager.hpp>

datacoe::DataManager manager;
bool loadSuccess = manager.init("save_game.json");

// Reset to default values
manager.newGame();

// Create and set initial data
datacoe::GameData gameData("NewPlayer", 0);
manager.setGamedata(gameData);

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

All dependencies are automatically handled:

- **CryptoPP:** Added as a git submodule at external/cryptopp-cmake
- **nlohmann/json:** Added as a git submodule at external/json
- **Google Test:** Automatically fetched by CMake during configuration only if BUILD_TESTS is ON (currently version 1.16.0)

The nlohmann/json library is now included as a submodule (like cryptopp-cmake) rather than being fetched via CMake, providing more consistent dependency management and offline build capability.

### Updating Dependencies (optional)

#### Updating the cryptopp-cmake or nlohmann/json submodules

If you want to update either submodule to a different version:

```bash
# Navigate to the submodule directory
cd external/cryptopp-cmake  # or external/json

# Fetch all tags
git fetch --tags

# List available tags
git tag -l

# Checkout the specific tag you want
git checkout <tag_name>  # e.g., CRYPTOPP_8_9_0 or v3.11.3

# Return to the main project directory
cd ../..

# Now commit the submodule update
git add external/cryptopp-cmake  # or external/json
git commit -m "Update submodule to <tag_name>"
```

#### Updating Google Test

To update Google Test to a newer version, modify the FetchContent_Declare section in your CMakeLists.txt:

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
- Performance benchmarks
- Memory usage

### Running Tests

To run all tests:

```bash
cd build
./tests/all_tests
```

To build and run individual test executables, enable the `BUILD_INDIVIDUAL_TESTS` option:

```bash
cmake -DBUILD_INDIVIDUAL_TESTS=ON ..
cmake --build .
./tests/error_handling_tests  # Run a specific test
```

### Customizing Tests

You'll need to modify the test files to match your game's data structures. The test files are located in the `tests/` directory:

1. Update the test fixture classes to match your game data structure
2. Modify the test cases to use your specific data types and expected values
3. Add or remove tests as needed for your specific requirements

### Disabling Tests

If you don't need the tests in your project, you can disable them by using the `BUILD_TESTS` option when configuring CMake:

```bash
cmake -DBUILD_TESTS=OFF ..
```

This will prevent Google Test from being fetched and the test suite from being built, which can speed up the build process and reduce dependencies.

[Back to top](#table-of-contents)

## Continuous Integration

This project uses GitHub Actions for continuous integration to ensure code quality and compatibility across different platforms and compilers.

### CI Pipeline Status

[![Windows](https://github.com/nircoe/datacoe/actions/workflows/ci-windows.yml/badge.svg?branch=main&event=push)](https://github.com/nircoe/datacoe/actions/workflows/ci-windows.yml)
[![Linux](https://github.com/nircoe/datacoe/actions/workflows/ci-linux.yml/badge.svg?branch=main&event=push)](https://github.com/nircoe/datacoe/actions/workflows/ci-linux.yml)
[![macOS](https://github.com/nircoe/datacoe/actions/workflows/ci-macos.yml/badge.svg?branch=main&event=push)](https://github.com/nircoe/datacoe/actions/workflows/ci-macos.yml)

### Supported Platforms and Compilers

Our CI pipeline automatically builds and tests the project with the following configurations:

- **Windows**:
  - Visual Studio (MSVC)
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
- Comprehensive test suite with Google Test
- Proper installation targets and CMake configuration
- Cross-platform support (Windows, macOS, Linux)

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
- ✅ Optional encryption (ability to disable encryption if not needed)

### Planned Improvements
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