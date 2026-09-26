#ifndef MINESWEEPER_H
#define MINESWEEPER_H

#include <stddef.h>

#define MAX_ROWS 16
#define MAX_COLS 30
#define MAX_CELLS (MAX_ROWS * MAX_COLS)

typedef enum {
    DIFFICULTY_BEGINNER = 1,
    DIFFICULTY_INTERMEDIATE = 2,
    DIFFICULTY_EXPERT = 3
} Difficulty;

typedef struct {
    const char *name;
    int rows;
    int cols;
    int mine_count;
} DifficultyConfig;

typedef struct {
    int rows;
    int cols;
    int mine_count;
    int revealed_count;
    int flag_count;
    int first_move;
    int game_over;
    int win;
    int mine_map[MAX_ROWS][MAX_COLS];
    int adjacent_map[MAX_ROWS][MAX_COLS];
    char visible_map[MAX_ROWS][MAX_COLS];
} GameState;

typedef struct {
    int row;
    int col;
} Position;

const DifficultyConfig *get_difficulty_config(Difficulty difficulty);
void set_random_seed(unsigned int seed);
void init_game(GameState *game, int rows, int cols, int mine_count);
void reset_board(GameState *game);
void place_mines(GameState *game, int safe_row, int safe_col);
void calculate_adjacent_mines(GameState *game);
void print_board(const GameState *game, int reveal_mines);
int reveal_cell(GameState *game, int row, int col);
int toggle_flag(GameState *game, int row, int col);
int is_win(const GameState *game);
int is_valid_position(const GameState *game, int row, int col);
void reveal_all_mines(GameState *game);

#endif
