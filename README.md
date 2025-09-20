# Simple CS2 Box ESP

A simple Box ESP for Counter-Strike 2 running on Windows, for educational purposes only.

# Requirements

*   **System**: Windows 10 or Windows 11.
*   **Compiler**: A C++ compiler with support for C++17 or newer.
    *   [MinGW-w64](https://www.mingw-w64.org/) (GCC)
*   **Build Tools**:
    *   [CMake](https://cmake.org/download/) (version 3.10 or higher).
    *   [Ninja](https://github.com/ninja-build/ninja/releases): A fast, small build system.

# Build

```bash
cmake . -G "Ninja" -B build
cd build
ninja -j($env:NUMBER_OF_PROCESSORS)
```

# Usage
1.  Launch `cs2.exe` and make sure your game is displayed in **Windowed Maximized** mode.
2.  Download the latest `offsets.json` and `client.dll.json` files from the [a2x/cs2-dumper GitHub repository](https://github.com/a2x/cs2-dumper).
3.  Place both of the downloaded `.json` files in the **same folder** as the compiled `main.exe`.
4.  Run `main.exe`, and the ESP should be working.

# Credits
This project is based on the work from [Cr0mb/CS2-Box-ESP](https://github.com/Cr0mb/CS2-Box-ESP/).