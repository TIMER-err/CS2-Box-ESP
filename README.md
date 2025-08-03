# CS2 Box ESP

An external Box ESP for Counter-Strike 2 written in C++.
For educational purposes only.

## Features

- Draws boxes around players (enemies and teammates)
- Displays health bars with color indication (green/yellow/red)
- Team-based coloring (red for enemies, blue for allies)
- External overlay window (no code injection)
- Runs at 144 FPS

## How It Works

- Reads CS2 memory externally using `ReadProcessMemory`
- Retrieves entity list and player information via known offsets
- Uses the view matrix to convert 3D positions to 2D screen coordinates
- Renders an always-on-top transparent window using GDI for drawing

## Build Instructions

1. Open the project in Visual Studio
2. Set build target to `x64`
3. Link against the following libraries:
   - `User32.lib`
   - `Gdi32.lib`
   - `Kernel32.lib`
4. Compile and run as Administrator
5. Ensure Counter-Strike 2 is open before launching the tool

## Offsets

Game offsets are defined in `esp.h`. These may need to be updated after CS2 updates. You can find updated offsets using tools like Cheat Engine or community offset dumps.

## Disclaimer

This software is provided for educational and research purposes only.  
Do not use this in online matchmaking or to gain unfair advantages in multiplayer games.  
Use of this tool may result in account bans.
