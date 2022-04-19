# MazeWar-Game

The goal of this assignment is to become familiar with low-level POSIX threads, multi-threading safety, concurrency guarantees, and networking.
The overall objective is to implement a simple multi-threaded network game server.

## Description

"MazeWar" is a real-time network game in which the players control avatars that move around a maze and shoot lasers at each other. A maze is
a two-dimensional array, each cell of which can be occupied by an avatar, a solid object (i.e. a "wall"), or empty space. At any given time, each
avatar has a particular location in the maze and a particular direction of gaze. There are four such directions: NORTH , SOUTH , EAST , and WEST .
An avatar can be moved forward and backward in the direction of gaze, and the direction of gaze can be rotated "left" (counterclockwise) and
"right" (clockwise). An avatar can only be moved into a cell that was previously empty; if an attempt is made to move into a cell that is occupied,
either by another avatar or a wall, then the avatar is blocked and the motion fails. Each avatar is equipped with a laser that it can be commanded
to fire. The laser beam shoots in the direction of gaze and it will destroy the first other avatar (if any) that it encounters. The object of the game
is to command your avatar to move around the maze, shoot other avatars, and accumulate points.

## Getting Started

### Executing program
```c
util/mazewar -p 3333
util/gclient -p <port> [-h <host>] [-u <username>] [-a <avatar>]
```
