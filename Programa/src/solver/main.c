#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../watcher/watcher.h"

int main(int argc, char** argv)
{
	/* Revisamos que los parámetros sean correctos */
	if(argc != 2)
	{
		printf("Uso: %s <test>\nDonde\n", argv[0]);
		printf("\t<test> es la ruta al archivo a resolver\n");
		return 1;
	}

	/* Abrimos el archivo en modo lectura */
	FILE* input_file = fopen(argv[1], "r");

	/* Falló la apertura del archivo */
	if(!input_file)
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

	/* Leemos el resto del archivo fila por fila */
	for(int row = 0; row < height; row++)
	{
		for(int col = 0; col < width; col++)
		{
			/* El grado de la celda actual */
			int degree;

			/* Leemos el grado de la celda */
			fscanf(input_file, "%d", &degree);

			/* Si el grado de la celda es conocido */
			if(degree > 0)
			{
				/* Lo dibujamos en la ventana */
				watcher_set_cell_degree(row, col, degree);
			}
		}
	}

	/* Cerramos el archivo */
	fclose(input_file);

	/* Paramos por 5 segundos para poder ver la ventana */
	/* OJO: borra los sleeps o tu código puede dar timeout */
	sleep(5);

	/* Cerramos la ventana */
	watcher_close();

	/* Retornamos 0 indicando que todo salió bien */
	return 0;
}
