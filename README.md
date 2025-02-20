# project lemmings

A game like Lemmings or Mario vs Donkey Kong : Mini-Land Mayhem

## Dependencies

To build and run this project, you need the following dependencies:

- **libGL**: OpenGL library for rendering graphics.
- **GLFW**: A multi-platform library for creating windows with OpenGL contexts and managing inputs.

You can install these dependencies on a Debian-based system using:

```console
sudo apt-get install libgl1-mesa-dev libglfw3-dev
```

## Quick Start

```console
$ make
```

## Controls (QWERTY layout)

| keyboard | action                                 |
|----------|----------------------------------------|
| E        | activate/deactivate eraser             |
| A        | show dialog in both editor and game    |
| T        | toggle item menu (only in editor mode) |
| D        | delete all tiles (only in editor mode) |

## Highlights

### Dynamic Array Implementation

- **File**: `array.h`
- **Description**: A dynamic array implementation in the style of stb libraries. It can be easily integrated into any project by defining `ARRAY_IMPLEMENTATION`
- **Usage**: This array implementation is used throughout the project for managing dynamic collections of objects.

### Custom XML Parser

- **File**: `xml.c`
- **Description**: A custom XML parser built from scratch, used for saving and loading game data.
- **Usage**: This parser is essential for the game's save and load functionality, demonstrating the ability to handle custom file formats efficiently.

### Hot Reload Feature

- **Description**: The hot reload feature allows for rapid iteration of changes without the need to recompile the entire project.
- **Usage**: This is particularly useful during development, enabling quick testing of modifications to the game logic or assets.

Usage
```console
export LD_LIBRARY_PATH=./raylib/lib/:./raylib/lib64/:./
$ make debug
```

## Platform Compatibility

- **Linux**: This project is fully functional on Linux.
- **Note**: The project has not been tested on Windows or macOS
