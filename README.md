# project lemmings

A game like Lemmings or Mario vs Donkey Kong : Mini-Land Mayhem

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

## Hotreload

```console
export LD_LIBRARY_PATH=./raylib/lib/:./
$ make glfw
$ make raylib-shared
$ make debug
```
