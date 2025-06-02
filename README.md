# Multiplayer Maze Game 
> The client-server maze game in course of System Coding with Xlib, pthreads and sockets

The game is a multi-player maze with a client-server architecture, where players must reach the exit, avoiding the enemy who changes the structure of the maze.


## Current Game Features

- **Procedural maze generation** with guaranteed passability
- **Up to 9 players** at the same time (indicated by numbers 0-8)
- ** A smart enemy** that changes the walls of the maze when moving
- **Multithreaded architecture** using pthread
- **Graphical interface** based on X11/Xlib
- **Network communication** via TCP sockets


### Dependecies
- Linux with X11
- GCC compiler
- Xlib11-dev


#### Installation of dependencies

**Ubuntu/Debian:**
```bash
sudo apt-get install libx11-dev build-essential
```


## Build

```bash
# full project (client+server)
make all

# separately
make server
make client

# with debug info
make debug-server
make debug-client
```


## Game start

### 1. Run server
```bash
./server
```
The server will start on **port 8080** and start accepting connections

> Warning: make sure that **port 8080** is free or set your own value for the `SERVER_PORT` macro in the *maze-game/game_commons.h* file.


### 2. Run clients
```bash
# Connect to local server
./client 127.0.0.1

# Connect to remote one
./client <ip-address-of-server>
```

### Clean
To remove buit files:
```bash
make clean
```


## Controls

- **WASD** or **arrows** - move player
- **ESC** - close the game


## Game Rules

### Goal
Get to the green cage (exit) before the other players and avoid getting caught by the enemy.


### Game Field Elements
- **Black cells** - walls (impassable)
- **White cells** - passageways
- **Green cage** - exit from the maze
- **Blue circles with numbers** - players (0-8, in current realization)
- **Red circle** - enemy


### Enemy Mechanics
- The enemy is moving in a random direction every second
- Upon exiting the cell, the enemy **inverts** its state:
- If there was a wall -> becomes a passage
- If there was a passage -> becomes a wall
- When colliding with a player -> the player loses

If the player gets caught by the enemy, he loses and loses control, but he can anyway follow the match.


## Architecture

### Server
- **Main thread** - accepts client connections
- **Enemy thread** - controls enemy movement (every second or with your delay at *maze-game/game_commons.h* if you host server)
- **Client threads** - process messages from each player
- **Mutexes** - protect the game state from racing threads

### Client
- **Main thread** - X11 event processing (keyboard, rendering)
- **Network thread** - receiving updates from the server
- **Mutexes** - synchronization of the playing field between threads

### Communication protocol
Messages between client and server:
- `MSG_CONNECT' - connecting a new player
- `MSG_MOVE' - player movement
- `MSG_GAME_STATE` - the state of the playing field
- `MSG_PLAYER_LOST' - notification of defeat
- `MSG_DISCONNECT' - disabling the player


## Debug
The project organizes the output of messages to the console.

### Server
- Information about player connections/disconnections
- Notifications of defeats and victories

### Client
- Server connection status
- Assigned to player ID
- Defeat notifications


## Extenstions
I'll think about it later...