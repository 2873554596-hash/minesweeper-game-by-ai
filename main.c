#include "minesweeper.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static void clear_screen(void) {
    fputs("\033[2J\033[H", stdout);
}

static int read_line(char *line, size_t size) {
    if (fgets(line, (int)size, stdin) == NULL) {
        return 0;
    }
    return 1;
}

static int parse_single_number(const char *line, int *value) {
    char extra;
    return sscanf(line, " %d %c", value, &extra) == 1;
}

static void print_menu(void) {
    printf("=== C语言扫雷 ===\n\n");
    printf("1. 初级 (9 x 9, 10雷)\n");
    printf("2. 中级 (16 x 16, 40雷)\n");
    printf("3. 高级 (16 x 30, 99雷)\n");
    printf("4. 退出\n");
    printf("请选择：");
}

static Difficulty choose_difficulty(void) {
    char line[64];
    int choice;

    for (;;) {
        clear_screen();
        print_menu();
        if (!read_line(line, sizeof(line))) {
            return 0;
        }
        if (parse_single_number(line, &choice) && choice >= 1 && choice <= 4) {
            return (Difficulty)choice;
        }
        printf("输入无效，请输入 1-4。\n");
        printf("按回车继续...");
        (void)read_line(line, sizeof(line));
    }
}

static int parse_game_command(const char *line, char *command, int *row,
                              int *col) {
    char extra;
    int count = sscanf(line, " %c %d %d %c", command, row, col, &extra);

    if (count == 1 && tolower((unsigned char)*command) == 'q') {
        return 1;
    }
    if (count == 3 && (tolower((unsigned char)*command) == 'r' ||
                       tolower((unsigned char)*command) == 'f')) {
        return 1;
    }
    return 0;
}

static int play_game(const DifficultyConfig *config) {
    GameState game;
    char line[128];

    init_game(&game, config->rows, config->cols, config->mine_count);
    while (!game.game_over) {
        char command;
        int row;
        int col;
        int result;

        clear_screen();
        printf("=== %s ===\n", config->name);
        print_board(&game, 0);
        printf("操作：r 行 列 揭示，f 行 列 插旗/取消旗，q 退出本局\n> ");

        if (!read_line(line, sizeof(line))) {
            return 0;
        }
        if (!parse_game_command(line, &command, &row, &col)) {
            printf("输入格式错误，请使用 r 行 列、f 行 列 或 q。\n");
            printf("按回车继续...");
            (void)read_line(line, sizeof(line));
            continue;
        }
        if (tolower((unsigned char)command) == 'q') {
            return 1;
        }

        row--;
        col--;
        if (!is_valid_position(&game, row, col)) {
            printf("坐标超出棋盘范围。\n");
            printf("按回车继续...");
            (void)read_line(line, sizeof(line));
            continue;
        }

        if (tolower((unsigned char)command) == 'r') {
            result = reveal_cell(&game, row, col);
            if (result == 0) {
                printf("该格不能揭示，可能已经揭示或已插旗。\n");
            } else if (result < 0) {
                clear_screen();
                print_board(&game, 1);
                printf("你踩到了雷，游戏失败。\n");
            } else if (game.win) {
                clear_screen();
                print_board(&game, 1);
                printf("恭喜你，成功排除所有安全格！\n");
            }
        } else {
            if (!toggle_flag(&game, row, col)) {
                printf("无法插旗：格子可能已揭示，或旗帜数量已达到雷数。\n");
            }
        }

        if (!game.game_over) {
            continue;
        }
        if (!game.win) {
            clear_screen();
            print_board(&game, 1);
        }
        printf("按回车返回菜单...");
        (void)read_line(line, sizeof(line));
    }
    return 1;
}

int main(void) {
    set_random_seed((unsigned int)time(NULL));

    for (;;) {
        Difficulty difficulty = choose_difficulty();
        const DifficultyConfig *config;

        if (difficulty == 0 || difficulty == 4) {
            printf("游戏结束。\n");
            return 0;
        }
        config = get_difficulty_config(difficulty);
        if (config == NULL) {
            return EXIT_FAILURE;
        }
        if (!play_game(config)) {
            return 0;
        }
    }
}
