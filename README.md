# data-management

This repository contains a generic C++ data management library designed for game projects. It provides functionalities for data persistence, serialization, and deserialization.

## Overview

The library offers a base implementation for data management tasks, including:

-   Reading and writing game data to files.
-   Serialization and deserialization of game data using [nlohmann/json](https://github.com/nlohmann/json).
-   Encryption and decryption of data using [CryptoPP](https://github.com/weidai11/cryptopp).

## Usage

1.  **Fork the Repository:**

    Fork this repository to your own GitHub account.

2.  **Add as a Submodule:**

    Add your forked repository as a Git submodule to your game project.

3.  **Create a Game-Specific Branch:**

    Create a new branch in your forked repository (e.g., `game/your-game-name`) to customize the data management library for your specific game.

4.  **Modify the Files:**

    Modify the `GameData.hpp/cpp`, and `DataManager.hpp/cpp` files in your game-specific branch to include your game's data structures and logic.

5.  **Modify CMakeLists.txt:**

    Modify the `CMakeLists.txt` file inside your game specific branch to include the correct paths for cryptopp and json.

6.  **Build and Integrate:**

    Build the library and integrate it into your game project.

## Dependencies

-   **CryptoPP:**
    -   Download the desired version from [CryptoPP GitHub](https://github.com/weidai11/cryptopp).
    -   Build the library using Visual Studio.
    -   Link the resulting `.lib` file to your project.
    -   Refer to this guide for building CryptoPP in Visual Studio: [AES Encryption Example using CryptoPP .lib in Visual Studio C++](https://github.com/ustayready/tradecraft/blob/master/miscellaneous-reversing-forensics/aes-encryption-example-using-cryptopp-.lib-in-visual-studio-c++.md)
-   **nlohmann/json:**
    -   Download the single header file from [nlohmann/json](https://github.com/nlohmann/json).
    -   Place the header file in a directory that is included in your project's include paths.

## Game-Specific Branches

-   `game/worm`: Contains the data management implementation for my "Worm" game.
-   `game/future-game`: Example of another game specific branch.
-   And so on.

## Tag Releases

Use tags to mark stable releases of the data management library for each game-specific branch (e.g., `worm-v1.0.0`).

## License

This library is licensed under the [MIT License](LICENSE).

## Building

Use CMake to build the library.