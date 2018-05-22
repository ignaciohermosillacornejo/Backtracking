#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../watcher/watcher.h"
#include "board.h"
// #include "backtracking.h"
#include "stack.h"

bool backtracking(Board *board, int count)
{
	count += 1;
	Cell *cell = board_next_assignation(board);
	if (!cell)
	{
		// printf("End of assignations reached!\n");
		return true;
	}

	for (int i = 1; i <= 2; i++)
	{
		// make assignment
		int old_status = cell -> status;
		board_set_status(board, cell->row, cell->col, i);
		//sleep(1);
		//printf("\nAssign cell color %d\n", i);
		//board_print_cell(cell);
		//printf("backtracking depth: %d\n", count);
		// check assignment is valid
		if (board_check_restrictions(board, cell))
		{
			// if all other assignment is valid
			if (backtracking(board, count))
			{
				return true;
			} 
		}
		board_set_status(board, cell->row, cell->col, old_status); // backtrack
		//sleep(1);
	}
	return false;
}

int main(int argc, char **argv)
{
	/* Revisamos que los parámetros sean correctos */
	if (argc != 2)
	{
		printf("Uso: %s <test>\nDonde\n", argv[0]);
		printf("\t<test> es la ruta al archivo a resolver\n");
		return 1;
	}

	/* Abrimos el archivo en modo lectura */
	FILE *input_file = fopen(argv[1], "r");

	/* Falló la apertura del archivo */
	if (!input_file)
	{
		printf("¡El archivo %s no existe!\n", argv[1]);
		return 2;
	}

	/* Las dimensiones del problema */
	int height;
	int width;

	/* Leemos las dimensiones del  a partir del archivo */
	fscanf(input_file, "%d %d", &height, &width);

	/* Abrimos una ventana de las dimensiones dadas */
	watcher_open(height, width);

	/* Create our board instance */
	Board *board = board_init(height, width);

	/* Leemos el resto del archivo fila por fila */
	for (int row = 0; row < height; row++)
	{
		for (int col = 0; col < width; col++)
		{
			/* El grado de la celda actual */
			int degree;
			/* Leemos el grado de la celda */
			fscanf(input_file, "%d", &degree);
			/* We set the degree of the board */
			board_set_degree(board, row, col, degree);
		}
	}
	// board_optimize(board);

	/* Cerramos el archivo */
	fclose(input_file);

	// board_set_status(board, 1, 1, 2);
	// board_set_status(board, 2, 1, 1);
	// board_set_status(board, 2, 2, 1);
	// board_set_status(board, 1, 2, 1);
	// board_set_status(board, 5, 5, 2);
	// board_set_status(board, 1, 3, 2);
	// board_print_status(board);
	// board_print_loyal_count(board);
	// board_print_rebel_count(board);
	// board_print_empty_count(board);
	// sleep(15);
	backtracking(board, 0);
	/* Paramos por 5 segundos para poder ver la ventana */
	/* OJO: borra los sleeps o tu código puede dar timeout */
	//printf("backtracing finished \n");
	// sleep(60);
	/* Cerramos la ventana */
	//char *filename = "test.pdf";
	//watcher_snapshot(filename);
	watcher_close();


	/* We free the board struct memory */
	board_destroy(board);

	/* Retornamos 0 indicando que todo salió bien */
	return 0;
}
