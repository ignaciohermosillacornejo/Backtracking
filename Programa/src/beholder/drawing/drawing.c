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
/* El color para marcar las celdas sin asociación */
Color blank_color;

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
    cont -> status_matrix[row][col] = 1;
  }
  else
  {
    change_pixel_color(cont -> status_image, row, col, loyal_color);
    cont -> status_matrix[row][col] = -1;
  }
}
/** Escribe el número correspondiente sobre la celda */
void drawing_cell_degree  (Content* cont, int row, int col, int degree)
{
  cont -> degree_matrix[row][col] = degree;
}
/** Deja la celda sin color, como venía originalmente */
void drawing_cell_clear   (Content* cont, int row, int col)
{
  change_pixel_color(cont -> status_image, row, col, blank_color);
  cont -> status_matrix[row][col] = 0;
}

/** Dibuja el problema en la ventana */
bool drawing_draw(cairo_t* restrict cr, Content* restrict cont)
{
  /* Para prevenir cambios mientras dibujamos */
  pthread_mutex_lock(&drawing_mutex);

  /* Hacemos calzar la ventana con la imagen  */
  cairo_scale(cr, cont -> scale, cont -> scale);
  /* Dibujaremos la imagen de fondo */
  cairo_set_source(cr, cont -> status_image -> pattern);
  /* Pintamos la imagen */
  cairo_paint(cr);
  /* Restauramos la ventana */
  cairo_scale(cr, 1.0/cont -> scale, 1.0/cont -> scale);

  /* Dibujamos lineas claras y delgadas */
  cairo_set_line_width(cr, cont -> scale / 32);
  cairo_set_source_rgb(cr, 0.9, 0.9, 1);

  /* Lineas verticales */
  for(int row = 0; row < cont -> matrix_height; row++)
  {
    for(int col = 1; col < cont -> matrix_width; col++)
    {
      /* Solo se dibujan si las celdas son de distinto color o ambas estan en blanco */
      if(cont -> status_matrix[row][col - 1] != cont -> status_matrix[row][col] || !cont -> status_matrix[row][col])
      {
        cairo_move_to(cr, col * cont -> scale, row * cont -> scale);
        cairo_rel_line_to(cr, 0, cont -> scale);
        cairo_stroke(cr);
      }
    }
  }

  /* Lineas horizontales */
  for(int col = 0; col < cont -> matrix_width; col++)
  {
    for(int row = 1; row < cont -> matrix_height; row++)
    {
      /* Solo se dibujan si las celdas son de distinto color o ambas estan en blanco */
      if(cont -> status_matrix[row - 1][col] != cont -> status_matrix[row][col] || !cont -> status_matrix[row][col])
      {
        cairo_move_to(cr, col * cont -> scale, row * cont -> scale);
        cairo_rel_line_to(cr, cont -> scale, 0);
        cairo_stroke(cr);
      }
    }
  }

  /* Numbers */

  cairo_text_extents_t te;
  cairo_select_font_face (cr, "monospace",
      CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
  cairo_set_font_size (cr, cont -> scale / 2);
  char text[2];

  for(int row = 1; row < cont -> matrix_height - 1; row++)
  {
    for(int col = 1; col < cont -> matrix_width - 1; col++)
    {
      if(!cont -> degree_matrix[row][col]) continue;

      sprintf(text,"%hhu",cont -> degree_matrix[row][col]);

      cairo_text_extents (cr, text, &te);

      double x = cont -> scale / 2 + cont -> scale * col - te.width / 2 - te.x_bearing;
      double y = cont -> scale / 2 + cont -> scale * row - te.height / 2 - te.y_bearing;
      cairo_move_to (cr, x, y);
      cairo_text_path(cr, text);
      cairo_set_source_rgb(cr, 0.9, 0.9, 1);
      cairo_fill_preserve(cr);
      cairo_set_line_width(cr, cont -> scale / 256);
      cairo_set_source_rgb(cr, 0, 0, 0);
      cairo_stroke(cr);
    }
  }

  pthread_mutex_unlock(&drawing_mutex);

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
  for(int row = 0; row < height; row++)
  {
    for(int col = 0; col <  width; col++)
    {
      change_pixel_color(bg, row, col, blank_color);
			/* Alpha channel */
      bg -> data[bg -> stride * row + col * 4 + 3] = 255;
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

  pthread_mutex_init(&drawing_mutex, NULL);

  /* Agregamos celdas en blanco a cada lado de la matriz */
  // height += 2;
  // width += 2;

  /* Dimensiones de la matriz */
  cont -> matrix_height = height;
  cont -> matrix_width = width;

  /* Los colores usados para representar los distintos tipos de celda */
  loyal_color = color_init(249,202,30);
  rebel_color = color_init(70,79,171);
  blank_color = color_init(27,27,67);

  /* La imagen con los colores de cada celda */
  cont -> status_image = init_background(height, width);

  /* Las dimensiones de la ventana deben ajustarse a la matriz */
	cont -> scale = WINDOW_MAX_SIZE / fmax(height, width);

  /* Dimensiones de la ventana */
	cont -> image_height = cont -> scale * height;
	cont -> image_width  = cont -> scale * width;

  /* Inicializamos las matrices */
  cont -> degree_matrix = calloc(height, sizeof(uint8_t*));
  cont -> status_matrix = calloc(height, sizeof(int8_t*));
  for(int row = 0; row < height; row++)
  {
    cont -> degree_matrix[row] = calloc(width, sizeof(uint8_t));
    cont -> status_matrix[row] = calloc(width, sizeof(int8_t));
  }

  return cont;
}

/** Geenera una imagen en pdf para un estado en particular */
void drawing_snapshot(Content* cont, char* filename)
{
	double width = cont -> matrix_width * 64;
	double height = cont -> matrix_height * 64;

	/* Imprimimos las imagenes del tablero */
	cairo_surface_t* surface;
	cairo_t *cr;

  // surface = cairo_image_surface_create(CAIRO_FORMAT_RGB24, width, height);

	surface = cairo_pdf_surface_create (filename, width, height);
	cr = cairo_create(surface);

	/* Reseteamos los parámetros para generar correctamente la imagen */
	Content aux = *cont;

  aux.scale = 64;
  aux.image_height = height;
  aux.image_width = width;

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

  for(int row = 0; row < cont -> matrix_height; row++)
  {
    free(cont -> status_matrix[row]);
    free(cont -> degree_matrix[row]);
  }
  free(cont -> degree_matrix);
  free(cont -> status_matrix);

  free(cont -> status_image -> data);
  free(cont -> status_image);

  free(cont);
}
