#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../watcher/watcher.h"
#include "board.h"
#include "stack.h"

int backtracks = 0;

bool backtracking(Board *board, int depth)
{
	depth += 1;
	Cell *cell = board_next_assignation(board);
	if (!cell) // We've done all posible asignations
	{
		//return board_check_restrictions2(board);
		return true;
	}

	for (int i = 1; i <= 2; i++)
	{
		// make assignment
		//sleep(1);
		board_set_status(board, cell->row, cell->col, i);
		// check assignment is valid
		if (board_check_restrictions(board, cell))// (board_check_restrictions(board, cell))
		{
			// if we are able to reach a solution from this state, return true
			if (backtracking(board, depth))
			{
				return true;
			} 
		}
	}
	//sleep(1);
	board_set_status(board, cell->row, cell->col, 0); // backtrack
	backtracks += 1;
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
	//board_optimize(board);

	/* Cerramos el archivo */
	fclose(input_file);

	sleep(5);
	backtracking(board, 0);
	/* Paramos por 5 segundos para poder ver la ventana */
	/* OJO: borra los sleeps o tu código puede dar timeout */
	//printf("backtracing finished \n");
	sleep(3);
	/* Cerramos la ventana */
	//char *filename = "test.pdf";
	//watcher_snapshot(filename);
	watcher_close();


	/* We free the board struct memory */
	board_destroy(board);

	printf("N desasignaciones: %d\n", backtracks);

	/* Retornamos 0 indicando que todo salió bien */
	return 0;
}
