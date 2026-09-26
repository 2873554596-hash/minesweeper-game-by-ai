#include "minesweeper.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static const int directions[8][2] = {
    {-1, -1}, {-1, 0}, {-1, 1},
    {0, -1},            {0, 1},
    {1, -1},  {1, 0},  {1, 1}
};

static const DifficultyConfig difficulty_configs[] = {
    {"初级", 9, 9, 10},
    {"中级", 16, 16, 40},
    {"高级", 16, 30, 99}
};

const DifficultyConfig *get_difficulty_config(Difficulty difficulty) {
    if (difficulty < DIFFICULTY_BEGINNER || difficulty > DIFFICULTY_EXPERT) {
        return NULL;
    }
    return &difficulty_configs[difficulty - DIFFICULTY_BEGINNER];
}

void set_random_seed(unsigned int seed) {
    srand(seed);
}

int is_valid_position(const GameState *game, int row, int col) {
    return game != NULL && row >= 0 && row < game->rows &&
           col >= 0 && col < game->cols;
}

void reset_board(GameState *game) {
    int row;
    int col;

    for (row = 0; row < MAX_ROWS; row++) {
        for (col = 0; col < MAX_COLS; col++) {
            game->mine_map[row][col] = 0;
            game->adjacent_map[row][col] = 0;
            game->visible_map[row][col] = '#';
        }
    }
}

void init_game(GameState *game, int rows, int cols, int mine_count) {
    if (game == NULL || rows < 1 || rows > MAX_ROWS || cols < 1 ||
        cols > MAX_COLS || mine_count < 1 || mine_count >= rows * cols) {
        return;
    }

    game->rows = rows;
    game->cols = cols;
    game->mine_count = mine_count;
    game->revealed_count = 0;
    game->flag_count = 0;
    game->first_move = 1;
    game->game_over = 0;
    game->win = 0;
    reset_board(game);
}

void place_mines(GameState *game, int safe_row, int safe_col) {
    int placed = 0;

    while (placed < game->mine_count) {
        int row = rand() % game->rows;
        int col = rand() % game->cols;

        if (row == safe_row && col == safe_col) {
            continue;
        }
        if (game->mine_map[row][col] != 0) {
            continue;
        }

        game->mine_map[row][col] = 1;
        placed++;
    }
}

void calculate_adjacent_mines(GameState *game) {
    int row;
    int col;

    for (row = 0; row < game->rows; row++) {
        for (col = 0; col < game->cols; col++) {
            int direction;
            int count = 0;

            if (game->mine_map[row][col]) {
                game->adjacent_map[row][col] = -1;
                continue;
            }

            for (direction = 0; direction < 8; direction++) {
                int next_row = row + directions[direction][0];
                int next_col = col + directions[direction][1];

                if (is_valid_position(game, next_row, next_col) &&
                    game->mine_map[next_row][next_col]) {
                    count++;
                }
            }
            game->adjacent_map[row][col] = count;
        }
    }
}

void print_board(const GameState *game, int reveal_mines) {
    int row;
    int col;

    printf("\n剩余雷数：%d\n\n", game->mine_count - game->flag_count);
    printf("    ");
    for (col = 0; col < game->cols; col++) {
        printf("%2d", col + 1);
    }
    putchar('\n');

    for (row = 0; row < game->rows; row++) {
        printf("%3d ", row + 1);
        for (col = 0; col < game->cols; col++) {
            char cell = game->visible_map[row][col];
            if (reveal_mines && game->mine_map[row][col]) {
                cell = '*';
            }
            printf("%2c", cell);
        }
        putchar('\n');
    }
    putchar('\n');
}

static void reveal_safe_area(GameState *game, int start_row, int start_col) {
    Position queue[MAX_CELLS];
    int head = 0;
    int tail = 0;

    queue[tail++] = (Position){start_row, start_col};
    while (head < tail) {
        Position current = queue[head++];
        int direction;

        for (direction = 0; direction < 8; direction++) {
            int row = current.row + directions[direction][0];
            int col = current.col + directions[direction][1];

            if (!is_valid_position(game, row, col) ||
                game->mine_map[row][col] ||
                game->visible_map[row][col] != '#') {
                continue;
            }

            game->visible_map[row][col] =
                (char)('0' + game->adjacent_map[row][col]);
            game->revealed_count++;

            if (game->adjacent_map[row][col] == 0 && tail < MAX_CELLS) {
                queue[tail++] = (Position){row, col};
            }
        }
    }
}

int is_win(const GameState *game) {
    return game != NULL &&
           game->revealed_count == game->rows * game->cols - game->mine_count;
}

int reveal_cell(GameState *game, int row, int col) {
    if (game == NULL || game->game_over || !is_valid_position(game, row, col)) {
        return 0;
    }
    if (game->visible_map[row][col] == 'F' ||
        game->visible_map[row][col] != '#') {
        return 0;
    }

    if (game->first_move) {
        place_mines(game, row, col);
        calculate_adjacent_mines(game);
        game->first_move = 0;
    }

    if (game->mine_map[row][col]) {
        game->game_over = 1;
        game->win = 0;
        reveal_all_mines(game);
        return -1;
    }

    game->visible_map[row][col] =
        (char)('0' + game->adjacent_map[row][col]);
    game->revealed_count++;
    if (game->adjacent_map[row][col] == 0) {
        reveal_safe_area(game, row, col);
    }

    if (is_win(game)) {
        game->game_over = 1;
        game->win = 1;
        reveal_all_mines(game);
    }
    return 1;
}

int toggle_flag(GameState *game, int row, int col) {
    if (game == NULL || game->game_over || !is_valid_position(game, row, col) ||
        (game->visible_map[row][col] >= '0' &&
         game->visible_map[row][col] <= '8')) {
        return 0;
    }

    if (game->visible_map[row][col] == 'F') {
        game->visible_map[row][col] = '#';
        game->flag_count--;
        return 1;
    }
    if (game->flag_count >= game->mine_count) {
        return 0;
    }

    game->visible_map[row][col] = 'F';
    game->flag_count++;
    return 1;
}

void reveal_all_mines(GameState *game) {
    int row;
    int col;

    for (row = 0; row < game->rows; row++) {
        for (col = 0; col < game->cols; col++) {
            if (game->mine_map[row][col]) {
                game->visible_map[row][col] = '*';
            }
        }
    }
}
