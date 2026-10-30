#pragma once

#include <stdbool.h>
#include <stdint.h>

#define COMPANION_MAZE_SIDE 10u
#define COMPANION_CELL_COUNT \
    (COMPANION_MAZE_SIDE * COMPANION_MAZE_SIDE)
#define COMPANION_MAX_PATH_LENGTH \
    (COMPANION_CELL_COUNT - 1u)

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Transmit the maze and current solving state as newline-delimited JSON.
 *
 * The maze must be the matrix returned by MazeCore::generate_maze().
 * The solve directions must be the array returned by MazeCore::solve_maze().
 *
 * Direction codes:
 *   0 = down
 *   1 = up
 *   2 = right
 *   3 = left
 *
 * @param maze              Row-major maze matrix, using 0 for free and 1 for wall.
 * @param solve_directions  Direction array terminated by -1.
 * @param seed              Seed used to generate the maze.
 * @param solve_position    Number of completed solution steps.
 * @param tick              Millisecond uptime value.
 * @param diagnostics       Diagnostic bitfield.
 * @param button_pressed    Current button state.
 *
 * @return true if all UART transmissions succeed.
 */
bool transmit_state(
    int** maze,
    const int* solve_directions,
    uint32_t seed,
    int solve_position,
    uint32_t tick,
    uint8_t diagnostics,
    bool button_pressed
);

#ifdef __cplusplus
}
#endif