#pragma once

/****************************************************************************/
/*                                Watcher                                   */
/*                                                                          */
/* Modulo a cargo de la visualizacion del problema.                         */
/****************************************************************************/

#include <stdbool.h>

/** Abre una ventana para mostrar una matriz de las dimensiones dadas */
void watcher_open            (int height, int width);
/** Escribe un número entre 1 y 4 inclusive dentro de esa celda */
void watcher_set_cell_degree (int row, int col, int degree);
/** Indica si una celda es o no rebelde */
void watcher_set_cell_status (int row, int col, bool type);
/** Deja la celda en blanco */
void watcher_clear_cell      (int row, int col);
/** Imprime la ventana al 100% de tamaño en formato PDF */
void watcher_snapshot        (char* filename);
/** Cierra y libera los recursos de la ventana */
void watcher_close           ();
