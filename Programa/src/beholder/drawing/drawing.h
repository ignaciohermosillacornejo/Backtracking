#pragma once

#include <cairo.h>
#include <stdbool.h>
#include "color.h"
#include <pthread.h>

/** Contiene toda la información de la imagen de fondo */
struct background_grid
{
	/** La imagen que estamos usando */
	cairo_surface_t* image;
	/** Metadata de la imagen */
	int stride;
	uint8_t* data;
	/** La imagen (en formato patron) para poder dibujarla cuadriculada */
	cairo_pattern_t* pattern;
};

/** Contiene toda la información de la imagen de fondo */
typedef struct background_grid BG;

/** Contiene la información de lo que ha de ser dibujado en el canvas */
struct content
{
	/** Ancho de la ventana */
	int image_width;
	/** Alto de la ventana */
	int image_height;
	/** La imagen de los estados de cada celda */
	BG* status_image;
	/** La imagen de los números */
	cairo_surface_t* number_image;
	/** Ancho de la matriz */
	int matrix_width;
	/** Alto de la matriz */
	int matrix_height;
	/** Indica la escala en la que se está trabajando */
	double scale;
};
/** Contiene la información de lo que ha de ser dibujado en el canvas */
typedef struct content Content;

/** Inicializa las herramientas de dibujo*/
Content* drawing_init(int height, int width);
/** Dibuja sobre el canvas dado */
bool     drawing_draw         (cairo_t* cr, Content* cont);
/** Pinta la celda del color correspondiente */
void     drawing_cell_status  (Content* cont, int row, int col, bool status);
/** Escribe el número correspondiente sobre la celda */
void     drawing_cell_degree  (Content* cont, int row, int col, int degree);
/** Deja la celda sin color, como venía originalmente */
void     drawing_cell_clear   (Content* cont, int row, int col);
/** Genera una imagen en pdf para un estado en particular */
void     drawing_snapshot     (Content* cont, char* filename);
/** Libera los recursos asociados a las herramientas de dibujo */
void     drawing_free         (Content* cont);
