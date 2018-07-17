#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "board.h"
#include "stack.h"
#include "cell.h"
#include "../watcher/watcher.h"

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))

void board_print_cell(Cell *cell);
void board_set_priority(Board *board, Cell *cell, int priority);
/* Helper function that returns the a pointer to the cell of row and col  */
Cell *board_get_cell(Board *board, int row, int col)
{
    return (board->cells) + (board->width) * row + col;
}

/* Helper function. We reset all the colors in the cell to 0 so that we can start DFS */
void board_reset_color(Board *board)
{
    for (int row = 0; row < board->height; row++)
    {
        for (int col = 0; col < board->width; col++)
        {
            board_set_color(board, row, col, 0);
        }
    }
}

/* Helper function to return left neighbour, returns NULL if it doesn't have one */
Cell *board_left_neighbour(Board *board, Cell *cell)
{
    Cell *neighbour = NULL;
    if (cell->col >= 1)
    {
        neighbour = board_get_cell(board, cell->row, cell->col - 1);
    }
    return neighbour;
}

/* Helper function to return right neighbour, returns NULL if it doesn't have one */
Cell *board_right_neighbour(Board *board, Cell *cell)
{
    Cell *neighbour = NULL;
    if (cell->col < board->width - 1)
    {
        neighbour = board_get_cell(board, cell->row, cell->col + 1);
    }
    return neighbour;
}
/* Helper function to return upper neighbour, returns NULL if it doesn't have one */
Cell *board_upper_neighbour(Board *board, Cell *cell)
{
    Cell *neighbour = NULL;
    if (cell->row >= 1)
    {
        neighbour = board_get_cell(board, cell->row - 1, cell->col);
    }
    return neighbour;
}

/* Helper function to return bottom neighbour, returns NULL if it doesn't have one */
Cell *board_bottom_neighbour(Board *board, Cell *cell)
{
    Cell *neighbour = NULL;
    if (cell->row < board->height - 1)
    {
        neighbour = board_get_cell(board, cell->row + 1, cell->col);
    }
    return neighbour;
}

/* Helper funcion, returns a neighbour based on the cell and and int */
Cell *board_iter_neighbours(Board *board, Cell *cell, int neighbour)
{
    /* The idea is to use this function to iterate over the neighbours more easily
    * 0: left neighbour
    * 1: right neighbour
    * 2: top neighbour
    * 3: bottom neighbour
    */
    if (neighbour == 0)
    {
        return board_left_neighbour(board, cell);
    }
    else if (neighbour == 1)
    {
        return board_right_neighbour(board, cell);
    }
    else if (neighbour == 2)
    {
        return board_upper_neighbour(board, cell);
    }
    else if (neighbour == 3)
    {
        return board_bottom_neighbour(board, cell);
    }
    else
    {
        printf("Error, accesing out of index neighbour!\n");
        return NULL;
    }
}

/* Helper function to update the count of a cells neighbour when changing status */
void board_update_neighbours(Board *board, Cell *cell, int old_status, int new_status)
{
    for (int i = 0; i < 4; i++)
    {
        Cell *neighbour = board_iter_neighbours(board, cell, i);
        if (neighbour && old_status != new_status)
        {
            if (old_status == 0)
            {
                neighbour->empty -= 1;
            }
            else if (old_status == 1)
            {
                neighbour->loyals -= 1;
            }
            else if (old_status == 2)
            {
                neighbour->rebels -= 1;
            }
            if (new_status == 0)
            {
                neighbour->empty += 1;
            }
            else if (new_status == 1)
            {
                neighbour->loyals += 1;
            }
            else if (new_status == 2)
            {
                neighbour->rebels += 1;
            }
        }
    }
}

/* Helper function, we evaluate whether to add neighbour to the DFS stack */
void dfs_process_neighbours(Board *board, Cell *cell, Cell *neighbour, Stack *stack, Cell **rebel, int current_status)
{
    // we check to assign the first rebel cell we find
    if (!(*rebel) && neighbour->status == 2)
    {
        *rebel = neighbour;
        //printf("rebel found at (%d, %d)\n", *rebel ->row, *rebel ->col);
    }
    // Neighbour hasn't been visited and its either null status, or the same status as current_cell
    if (!neighbour->color && (neighbour->status == 0 || neighbour->status == current_status))
    {
        neighbour->color = 1; // we mark the neighbour as visited
        if (neighbour->status == 1)
        {
            board->count_loyalist += 1;
        }
        else if (neighbour->status == 2)
        {
            board->count_rebel += 1;
        }
        stack_push(stack, neighbour); // we add the neighbour into the stack
    }
}

/* Helper function. We insert into the stack all the cells that have the same status and haven't been visited */
void dfs_search_neighbours(Board *board, Cell *cell, Stack *stack, Cell **rebel, int current_status)
{
    for (int i = 0; i < 4; i++)
    {
        Cell *neighbour = board_iter_neighbours(board, cell, i);
        if (neighbour)
        {
            dfs_process_neighbours(board, cell, neighbour, stack, rebel, current_status);
        }
    }
    return;
}

/* DFS prunning to check if we have more than one group per status, we return true if it is, false in another case */
bool board_check_groups(Board *board)
{
    /* color 0: unvisited node (white)
     * color 1: visited   node (grey)
     * color 2: closed    node (black)
     */
    // Starting params
    board_reset_color(board);
    board->count_loyalist = 0;
    board->count_rebel = 0;

    /* Pointer to the first cell that we find that is rebel */
    Cell *rebel = NULL;

    /* We add the top right corner to the stack, which is garanteed to be a loyalist cell */
    Stack *stack = stack_init();
    stack_push(stack, board_get_cell(board, 0, 0));

    /* We count the loyalist cells first */
    Cell *current_cell = stack_pop(stack);
    current_cell->color = 1;
    board->count_loyalist += 1;

    while (current_cell)
    {
        current_cell->color = 2;
        dfs_search_neighbours(board, current_cell, stack, &rebel, 1);
        current_cell = stack_pop(stack);
    }
    board_reset_color(board);
    /* Now we count the rebel cells starting from the pointer to rebel 
     * empty cells are being counted twice, we also check if any rebel cells 
     * have been assigned
     */
    if (rebel != NULL)
    {
        current_cell = rebel;
        current_cell->color = 1;
        board->count_rebel += 1;
        while (current_cell)
        {
            current_cell->color = 2;
            dfs_search_neighbours(board, current_cell, stack, &rebel, 2);
            current_cell = stack_pop(stack);
        }
    }

    // free the memmory from our stack
    stack_destroy(stack);

    // return our value
    //printf("count empty: %d\n", board->count_empty);
    //printf("count rebels: %d\n", board->count_rebel);
    //printf("count loyals: %d\n", board->count_loyalist);
    if (board->count_empty + board->count_rebel + board->count_loyalist == board->width * board->height)
    {
        return true;
    }
    else
    {
        return false;
    }
}

/* Helper function, we check if an individual cell meets the degree requirement 
 * */
bool cell_degree_check(Cell *cell)
{
    //board_print_cell(cell);
    if (!cell->degree)
    {
        return true;
    }
    if (!cell->status && ((cell->loyals <= cell->degree && cell->loyals + cell->empty >= cell->degree) || (cell->rebels <= cell->degree && cell->rebels + cell->empty >= cell->degree)))
    {
        return true;
    }
    if (cell->status == 1 && cell->loyals <= cell->degree && cell->loyals + cell->empty >= cell->degree)
    {
        return true;
    }
    if (cell->status == 2 && cell->rebels <= cell->degree && cell->rebels + cell->empty >= cell->degree)
    {
        return true;
    }
    return false;
}

/* Neighbour prunning, we iter through the cell and its neighbours calling cell_degree_check */
bool board_degree_prune(Board *board, int row, int col)
{
    Cell *cell = board_get_cell(board, row, col);
    // first we check the cell its self
    if (!cell_degree_check(cell))
    {
        return false;
    }
    for (int i = 0; i < 4; i++)
    {
        Cell *neighbour = board_iter_neighbours(board, cell, i);
        if (neighbour)
        {
            //printf("checking neighbour:\n");
            //board_print_cell(neighbour);
            if (!cell_degree_check(neighbour))
            {
                return false;
            }
        }
    }
    return true;
}

bool board_degree_check(Board *board)
{
    for (int row = 1; row < board->height - 1; row++)
    {
        for (int col = 1; col < board->width - 1; col++)
        {
            if (!board_degree_prune(board, row, col))
            {
                return false;
            }
        }
    }
    return true;
}

bool board_check_restrictions3(Board *board, Cell * cell)
{
    return cell_degree_check(cell);
}

bool board_check_restrictions2(Board *board)
{
    return board_check_groups(board) && board_degree_check(board);
}
/* Convinience method to do logic && on all restricion */
bool board_check_restrictions(Board *board, Cell *cell)
{
    return board_check_groups(board) && board_degree_prune(board, cell->row, cell->col);
}

/* we move the cells from the stacks acording to the new heuristics from the assignation */
void board_update_heuristics(Board *board, Cell *cell)
{
    if (!cell->status)
    {
        cell->priority = 2;
    }
    for (int i = 0; i < 4; i++)
    {
        Cell *neighbour = board_iter_neighbours(board, cell, i);

        if (neighbour && !neighbour->status)
        {
            if (neighbour -> degree == 4)
            {
                neighbour->priority = 1;
            }
            if (neighbour -> degree == 3)
            {
                if (neighbour -> loyals == 1)
                {
                    
                }
            }
        }
    }
}

/* We get the next cell for assignation, TODO: implement heuristics ordering */
Cell *board_next_assignation(Board *board)
{
    for (int row = 1; row < board->height - 1; row++)
    {
        for (int col = 1; col < board->width - 1; col++)
        {
            Cell * cell = board_get_cell(board, row, col);
            if (cell -> status == 0)
            {
                return cell;
            }
        }
    }
    return NULL;
    if (board->stack_a->count)
    {
        return stack_pop(board->stack_a);
    }
    else if (board->stack_b->count)
    {
        return stack_pop(board->stack_b);
    }
    else
    {
        return NULL; // in case we don't have any assignations left
    }
}

/* Create our board and return a pointer to our object */
Board *board_init(int height, int width)
{
    Board *board = malloc(sizeof(Board));
    board->height = height;
    board->width = width;
    board->count_empty = 0;
    board->count_loyalist = 0;
    board->count_rebel = 0;
    board->count_empty = height * width;
    /* We allocate enough memory for the cells in our board
     * we store a simple array of pointers and use basic arithmetic two get the correct cell
     */
    board->cells = (Cell *)malloc(board->height * board->width * sizeof(Cell));
    board->stack_a = stack_init();
    board->stack_b = stack_init();
    /* We set the initial values of the cells in our board
     * on the first pass we just create an empty board
     */
    for (int row = 0; row < board->height; row++)
    {
        for (int col = 0; col < board->width; col++)
        {
            Cell *cell = board_get_cell(board, row, col);
            cell->row = row;
            cell->col = col;
            cell->loyals = 0;
            cell->rebels = 0;
            cell->color = 0;
            cell->degree = 0;
            cell->empty = 4;
            cell->status = 0;
            // Initial values of the corners
            if ((!row && !col) || (!row && col + 1 == board->width) || (row + 1 == board->height && !col) || (row + 1 == board->height && col + 1 == board->width))
            {
                cell->empty = 2;
            } //values of the edges - corners
            else if (!row || !col || row + 1 == board->height || col + 1 == board->width)
            {
                cell->empty = 3;
            }
            else
            {
                stack_push(board->stack_b, cell);
            }
        }
    }
    for (int row = 0; row < board->height; row++)
    {
        for (int col = 0; col < board->width; col++)
        {
            if (!row || !col || row + 1 == board->height || col + 1 == board->width)
            {
                board_set_status(board, row, col, 1);
            }
        }
    }
    return board;
}

/* we change some priorities of the values before starting our backtrack */
void board_optimize(Board *board)
{
    for (int row = 1; row < board->height - 1; row++)
    {
        for (int col = 1; col < board->width - 1; col++)
        {
            Cell *cell = board_get_cell(board, row, col);
            // Initial values of the corners
            if ((row == 1 && col == 1) || (row == 1 && col == board->width - 2) || (row == board->height - 2 && col == 1) || (row == board->height - 2 && col == board->width - 2))
            {
                if (cell->degree == 1)
                {
                    board_set_priority(board, cell, 1);
                }
            } //values of the edges - corners
            else if (row == 1 || col == 1 || row == board->height - 2 || col == board->width - 2)
            {
                if (cell->degree == 4)
                {
                    board_set_priority(board, cell, 1);
                    for (int i = 0; i < 4; i++)
                    {
                        Cell *neighbour = board_iter_neighbours(board, cell, i);
                        if (neighbour)
                        {
                            board_set_priority(board, neighbour, 1);
                        }
                    }
                }
            }
        }
    }
}

/* Used to free the memory of a board and all associated cells */
void board_destroy(Board *board)
{
    free(board->cells);
    free(board);
    stack_destroy(board->stack_a);
    stack_destroy(board->stack_b);
}

/* Get the degree of a given cell */
int board_get_degree(Board *board, int row, int col)
{
    Cell *cell = board_get_cell(board, row, col);
    return cell->degree;
}

/* Used when creating out board, we set the apropiate info */
void board_set_degree(Board *board, int row, int col, int degree)
{
    Cell *cell = board_get_cell(board, row, col);
    cell->degree = degree;
    if (cell->degree > 0)
    {
        watcher_set_cell_degree(row, col, cell->degree);
    }
}

/* Get the current status of a given cell */
int board_get_status(Board *board, int row, int col)
{
    Cell *cell = board_get_cell(board, row, col);
    return cell->status;
}

/* Set the prioryty of a cell TODO move the cell of queue if needed */
void board_set_priority(Board *board, Cell *cell, int priority)
{
    if (cell->priority != priority)
    {
        cell->priority = priority;
        if (priority == 1)
        {
            stack_push(board->stack_a, stack_remove(board->stack_b, cell));
        }
        else
        {
            stack_push(board->stack_b, stack_remove(board->stack_a, cell));
        }
    }
}

/* Set the status of a given cell
 * with each assignation, we update the grafical interface and
 * update the count of the neighbours used in the neighbours branch prune
 * TODO: remove grafical interface update for performance? check this
 */
void board_set_status(Board *board, int row, int col, int status)
{
    Cell *cell = board_get_cell(board, row, col);
    int old_status = cell->status;
    // we first update all of our neighbours with the appropiate info about the change in status
    board_update_neighbours(board, cell, old_status, status);
    cell->status = status;
    // we re-prioritize the neighbours and the cell, depending if it's empty or not
    // board_update_heuristics(board, cell);
    if (status)
    {
        if (old_status == 0)
        {
            board->count_empty -= 1;
        }
        watcher_set_cell_status(row, col, status - 1);
    }
    else
    {
        // TODO change this to work with two stacks
        if (cell->priority == 1)
        {
            stack_push(board->stack_a, cell);
        }
        else
        {
            stack_push(board->stack_b, cell);
        }
        board->count_empty += 1;
        watcher_clear_cell(row, col);
    }
}

/* Get the color of a given cell */
int board_get_color(Board *board, int row, int col)
{
    Cell *cell = board_get_cell(board, row, col);
    return cell->color;
}

/* Set the color of a given cell */
void board_set_color(Board *board, int row, int col, int color)
{
    Cell *cell = board_get_cell(board, row, col);
    cell->color = color;
}

/* Debuging function */
void board_print_status(Board *board)
{
    for (int row = 0; row < board->height; row++)
    {
        for (int col = 0; col < board->width; col++)
        {
            printf("%d ", board_get_status(board, row, col));
        }
        printf("\n");
    }
    printf("\n");
}

/* Debuging function */
void board_print_loyal_count(Board *board)
{
    for (int row = 0; row < board->height; row++)
    {
        for (int col = 0; col < board->width; col++)
        {
            printf("%d ", board_get_cell(board, row, col)->loyals);
        }
        printf("\n");
    }
    printf("\n");
}

/* Debuging function */
void board_print_rebel_count(Board *board)
{
    for (int row = 0; row < board->height; row++)
    {
        for (int col = 0; col < board->width; col++)
        {
            printf("%d ", board_get_cell(board, row, col)->rebels);
        }
        printf("\n");
    }
    printf("\n");
}

/* Debuging function */
void board_print_empty_count(Board *board)
{
    for (int row = 0; row < board->height; row++)
    {
        for (int col = 0; col < board->width; col++)
        {
            printf("%d ", board_get_cell(board, row, col)->empty);
        }
        printf("\n");
    }
    printf("\n");
}

/* Debuging function */
void board_print_color(Board *board)
{
    for (int row = 0; row < board->height; row++)
    {
        for (int col = 0; col < board->width; col++)
        {
            printf("%d ", board_get_color(board, row, col));
        }
        printf("\n");
    }
    printf("\n");
}

/* Debuging function */
void board_print_cell(Cell *cell)
{
    printf("[%d, %d] degree: %d, status: %d, rebels: %d, loyals: %d, empty: %d\n",
           cell->row, cell->col, cell->degree, cell->status, cell->rebels, cell->loyals, cell->empty);
}