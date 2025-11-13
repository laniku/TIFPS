# TIFPS - TI-84 Plus CE FPS Game

![demo loop](https://raw.githubusercontent.com/laniku/TIFPS/main/demo.gif)

A first-person shooter game for the TI-84 Plus CE calculator featuring raycasting, procedural level generation, and multiple enemy types.

## Features

- **Raycasting Engine**: Rays are skipped for performance reaons but can be changed by modifying the source and recompiling
- **Procedural Levels**: Randomly generated levels based on RTC time seed
- **Multiple Enemy Types**: Work in Progress
- **Combat System**: Shooting with ammo management
- **UI**: HP and ammo bars
- **Controls**:
  - Arrow keys: Move forward/backward and rotate
  - 2nd: Shoot
  - CLEAR: Exit game

## Building

Requires the CE C/C++ Toolchain to be installed and configured. Make sure your PATH includes the toolchain binaries:

```bash
export PATH=<path to CEDEV>:$PATH
```

Then build:

```bash
make
```

This will create `bin/TIFPS.8xp` which can be transferred to your calculator.

The Makefile uses the standard CE toolchain build system via `cedev-config --makefile`.

## Controls

- **Up Arrow**: Move forward
- **Down Arrow**: Move backward
- **Left Arrow**: Rotate left
- **Right Arrow**: Rotate right
- **2nd**: Shoot
- **CLEAR**: Exit game

## Gameplay

Navigate through procedurally generated levels with rooms connected by hallways (similar to Wolf3D/Doom betas). Defeat enemies to progress. Reach the exit room to win. Manage your HP and ammo carefully!

## Optimizations

- Fixed-point math optimizations
- Efficient DDA raycasting
- Minimal memory allocations
- Optimized rendering loops