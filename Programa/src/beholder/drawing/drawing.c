#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <cairo-pdf.h>
#include "drawing.h"

#define WINDOW_MAX_SIZE 640.0

/* El color para marcar las celdas leales al régimen */
Color loyal_color;
/* El color para marcar las celdas rebeldes */
Color rebel_color;

/** Modifica el pixel de la imagen**/
static void change_pixel_color(BG* bg, int row, int col, Color color)
{
  /* Nos aseguramos de que se terminen los cambios en la imagen */
  cairo_surface_flush(bg -> image);

  /* Cambiamos el pixel al color correspondiente*/
  bg -> data[bg -> stride * row + col * 4 + 0] = color.B;
  bg -> data[bg -> stride * row + col * 4 + 1] = color.G;
  bg -> data[bg -> stride * row + col * 4 + 2] = color.R;

  /* Marcamos que la imagen fue modificada para que Cairo la cargue */
  cairo_surface_mark_dirty(bg -> image);
}

/** Pinta la celda del color correspondiente */
void drawing_cell_status  (Content* cont, int row, int col, bool status)
{
  if(status)
  {
    change_pixel_color(cont -> status_image, row, col, rebel_color);
  }
  else
  {
    change_pixel_color(cont -> status_image, row, col, loyal_color);
  }
}
/** Escribe el número correspondiente sobre la celda */
void drawing_cell_degree  (Content* cont, int row, int col, int degree)
{
  abort();
}
/** Deja la celda sin color, como venía originalmente */
void drawing_cell_clear   (Content* cont, int row, int col)
{
  abort();
}

bool drawing_draw(cairo_t* restrict cr, Content* restrict cont)
{
  cairo_scale(cr, cont -> scale, cont -> scale);

	cairo_set_source(cr, cont -> status_image -> pattern);

  cairo_paint(cr);

  cairo_scale(cr, 1, 1);

	return true;
}

static BG* init_background(int height, int width)
{
  BG* bg = malloc(sizeof(BG));

  /* El formato de imagen: R G B de 8 bits cada uno */
  cairo_format_t format = CAIRO_FORMAT_RGB24;
  /* El ancho en bits de la imagen */
  bg -> stride = cairo_format_stride_for_width (format, width);
  /* La información de los pixeles de la imagen completa */
  bg -> data   = malloc(bg -> stride * height);
  /* La imagen en sí */
  bg -> image  = cairo_image_surface_create_for_data(bg -> data, format, width, height, bg -> stride);

	/* Inicializamos la imagen */
  for(int j = 0; j < height; j++)
  {
    for(int i = 0; i <  width; i++)
    {
			/* Blue channel */
      bg -> data[bg -> stride * j + i * 4 + 0] = 241;
			/* Green channel */
      bg -> data[bg -> stride * j + i * 4 + 1] = 255;
			/* Red channel */
      bg -> data[bg -> stride * j + i * 4 + 2] = 253;
			/* Alpha channel */
      bg -> data[bg -> stride * j + i * 4 + 3] = 255;
    }
  }

	/* Pasamos la imagen a patron dado que necesitamos usar NEAREST NEIGHBOURS */
	bg -> pattern = cairo_pattern_create_for_surface(bg -> image);
	cairo_pattern_set_filter(bg -> pattern, CAIRO_FILTER_NEAREST);

  return bg;
}


/** Genera los contenedores para las dos imagenes superpuestas */
Content* drawing_init(int height, int width)
{
  Content* cont = malloc(sizeof(Content));

  cont -> matrix_height = height;
  cont -> matrix_width = width;

  cont -> status_image = init_background(height, width);

  /* Las dimensiones de la ventana deben ajustarse a la matriz */
	cont -> scale = WINDOW_MAX_SIZE / fmax(height, width);
	cont -> image_height = cont -> scale * height;
	cont -> image_width  = cont -> scale * width;

  //TODO

  loyal_color = color_init(210, 220, 255);
  rebel_color = color_init(202, 255, 219);

  return cont;
}

/** Geenera una imagen en pdf para un estado en particular */
void drawing_snapshot(Content* cont, char* filename)
{
	double width = cont -> image_width;
	double height = cont -> image_height;

	/* Imprimimos las imagenes del tablero */
	cairo_surface_t* surface;
	cairo_t *cr;

  surface = cairo_image_surface_create(CAIRO_FORMAT_RGB24, width, height);


  //TODO
	// surface = cairo_pdf_surface_create (filename, width, height);
	cr = cairo_create(surface);

	/* Reseteamos los parámetros para generar correctamente la imagen */
	Content aux = *cont;

	/* Dibuja el estado actual */
	drawing_draw(cr, &aux);

  cairo_surface_write_to_png(surface, filename);

	cairo_surface_destroy(surface);
	cairo_destroy(cr);
}

/** Libera los recursos asociados a las herramientas de dibujo */
void drawing_free(Content* cont)
{
  /* Liberamos la imagen */
  cairo_surface_destroy(cont -> status_image -> image);
  cairo_pattern_destroy(cont -> status_image -> pattern);

  //TODO

  free(cont);
}
