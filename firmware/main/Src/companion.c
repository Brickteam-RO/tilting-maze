#include "companion.h"

#include "uart.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

static bool transmit_text(const char* text)
{
    return uart_printf("%s", text);
}

static bool transmit_u32(uint32_t value)
{
    return uart_printf("%lu", (unsigned long)value);
}

static bool transmit_int(int value)
{
    return uart_printf("%d", value);
}

static bool transmit_char(char value)
{
    return uart_printf("%c", value);
}

static bool direction_delta(int direction, int* row_delta, int* col_delta)
{
    switch (direction) {
    case 0: // down
        *row_delta = 1;
        *col_delta = 0;
        return true;

    case 1: // up
        *row_delta = -1;
        *col_delta = 0;
        return true;

    case 2: // right
        *row_delta = 0;
        *col_delta = 1;
        return true;

    case 3: // left
        *row_delta = 0;
        *col_delta = -1;
        return true;

    default:
        return false;
    }
}

static int path_length(const int* solve_directions)
{
    int length = 0;

    while (length < COMPANION_MAX_PATH_LENGTH &&
           solve_directions[length] != -1) {
        if (solve_directions[length] < 0 ||
            solve_directions[length] > 3) {
            return -1;
        }

        ++length;
    }

    if (length == COMPANION_MAX_PATH_LENGTH &&
        solve_directions[length] != -1) {
        return -1;
    }

    return length;
}

static bool transmit_maze(
    int** maze,
    const int* solve_directions,
    uint32_t seed,
    int path_len,
    bool solvable
)
{
    if (!transmit_text("{\"type\":\"maze\",\"seed\":")) return false;
    if (!transmit_u32(seed)) return false;

    if (!transmit_text(",\"side\":")) return false;
    if (!transmit_u32(COMPANION_MAZE_SIDE)) return false;

    if (!transmit_text(",\"start\":[0,0],\"goal\":[")) return false;
    if (!transmit_u32(COMPANION_MAZE_SIDE - 1u)) return false;
    if (!transmit_text(",")) return false;
    if (!transmit_u32(COMPANION_MAZE_SIDE - 1u)) return false;

    if (!transmit_text("],\"solvable\":")) return false;
    if (!transmit_text(solvable ? "true" : "false")) return false;

    if (!transmit_text(",\"walls\":[")) return false;

    for (uint32_t row = 0; row < COMPANION_MAZE_SIDE; ++row) {
        if (row != 0u && !transmit_char(',')) return false;
        if (!transmit_char('[')) return false;

        for (uint32_t col = 0; col < COMPANION_MAZE_SIDE; ++col) {
            if (col != 0u && !transmit_char(',')) return false;

            if (!transmit_int(maze[row][col] != 0 ? 1 : 0)) {
                return false;
            }
        }

        if (!transmit_char(']')) return false;
    }

    if (!transmit_text("],\"path\":[")) return false;

    for (int i = 0; i < path_len; ++i) {
        if (i != 0 && !transmit_char(',')) return false;
        if (!transmit_int(solve_directions[i])) return false;
    }

    return transmit_text("]}\n");
}

static bool transmit_status(
    const int* solve_directions,
    int path_len,
    int solve_position,
    uint32_t tick,
    uint8_t diagnostics,
    bool button_pressed
)
{
    int ball_row = 0;
    int ball_col = 0;
    const char* state = "waiting";
    const char* tilt = "none";

    if (path_len > 0) {
        state = "solving";

        for (int i = 0; i < solve_position; ++i) {
            int row_delta;
            int col_delta;

            if (!direction_delta(
                    solve_directions[i],
                    &row_delta,
                    &col_delta)) {
                return false;
            }

            ball_row += row_delta;
            ball_col += col_delta;
        }

        if (solve_position >= path_len) {
            state = "solved";
        } else {
            switch (solve_directions[solve_position]) {
            case 0:
                tilt = "down";
                break;
            case 1:
                tilt = "up";
                break;
            case 2:
                tilt = "right";
                break;
            case 3:
                tilt = "left";
                break;
            default:
                return false;
            }
        }
    }

    if (!transmit_text("{\"type\":\"status\",\"tick\":")) return false;
    if (!transmit_u32(tick)) return false;

    if (!transmit_text(",\"diagnostics\":")) return false;
    if (!transmit_u32(diagnostics)) return false;

    if (!transmit_text(",\"button\":")) return false;
    if (!transmit_text(button_pressed ? "true" : "false")) return false;

    if (!transmit_text(",\"ball\":[")) return false;
    if (!transmit_int(ball_row)) return false;
    if (!transmit_char(',')) return false;
    if (!transmit_int(ball_col)) return false;

    if (!transmit_text("],\"tilt\":\"")) return false;
    if (!transmit_text(tilt)) return false;

    if (!transmit_text("\",\"state\":\"")) return false;
    if (!transmit_text(state)) return false;

    return transmit_text("\"}\n");
}

bool transmit_state(
    int** maze,
    const int* solve_directions,
    uint32_t seed,
    int solve_position,
    uint32_t tick,
    uint8_t diagnostics,
    bool button_pressed
)
{
    if (maze == NULL || solve_directions == NULL) {
        return false;
    }

    int path_len = path_length(solve_directions);
    if (path_len < 0) {
        return false;
    }

    if (solve_position < 0) {
        solve_position = 0;
    }

    if (solve_position > path_len) {
        solve_position = path_len;
    }

    if (!transmit_maze(
            maze,
            solve_directions,
            seed,
            path_len,
            path_len > 0)) {
        return false;
    }

    return transmit_status(
        solve_directions,
        path_len,
        solve_position,
        tick,
        diagnostics,
        button_pressed
    );
}