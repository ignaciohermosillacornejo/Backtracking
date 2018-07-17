#pragma once
#include "stack.h"
#include "cell.h"
#include <stdbool.h>
// module to contain all board operations, and make assignations and revert them


typedef struct board Board;


struct board
{
    int width;
    int height;
    int count_loyalist;
    int count_rebel;
    int count_empty;
    Stack *stack_a;
    Stack *stack_b;
    Cell *cells; // Simple array of cells to represent a matrix
};

/* Board creation and destruction */
Board *board_init(int height, int width);
void board_destroy(Board *board); 

/* Setters and getters for our board */
int  board_get_degree        (Board *board, int row, int col             );
void board_set_degree        (Board *board, int row, int col, int  degree);
int  board_get_status        (Board *board, int row, int col             );
void board_set_status        (Board *board, int row, int col, int status );
int  board_get_color         (Board *board, int row, int col             );
void board_set_color         (Board *board, int row, int col, int  color );
bool board_check_groups      (Board *board                               );
bool board_degree_prune      (Board *board, int row, int col             );
Cell * board_next_assignation(Board *board                               );
bool board_check_restrictions(Board *board, Cell *cell                   );
void board_optimize          (Board *board                               );
void board_update_heuristics (Board *board, Cell *cell                   );
/* Debuging functions */
void board_print_status(Board *board);
void board_print_color (Board *board);
void board_print_rebel_count(Board *board);
void board_print_loyal_count(Board *board);
void board_print_empty_count(Board *board);
void board_print_cell(Cell *cell);
bool board_check_restrictions2(Board *board);
bool board_check_restrictions3(Board *board, Cell * cell);
