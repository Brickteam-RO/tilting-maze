# BrickMaze

An electromechanical 10×10 maze project controlled by an STM32 microcontroller. The system generates and solves mazes, drives the maze platform with DC motors, and streams maze state to a companion application.

## Features

- 10×10 maze generation and solving, powered by 100 linear actuators and 35 Infineon TLE94112 Half-bridge drivers
- STM32F103C8T6 firmware
- Dual L298N motor-driver control for platform tilting
- UART/JSON state streaming at 115200 baud
- Raspberry Pi or desktop companion application
- Offline simulation and serial replay support
- KiCad motherboard, daughterboard, and testboard designs

## Repository layout

```text
firmware/
  main/       Main STM32 firmware
  testboard/  Motor-driver test firmware

pcb/
  motherboard/  Main controller and power board
  daughterboard/ Motor and driver board
  testboard/    Hardware test board

external_tools/
  pi_companion/    Python GUI, simulator, serial reader, and protocol tools
  maze_visualizer/ Experimental maze visualization utilities
