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

  cairo_scale(cr, 1.0/cont -> scale, 1.0/cont -> scale);

  /* Dibujamos lineas claras y delgadas */
  cairo_set_line_width(cr, cont -> scale / 32);
  cairo_set_source_rgba(cr,0.6,0.6,0.6,0.6);

  for(int row = 0; row < cont -> matrix_height; row++)
  {
    /* Lineas horizontales */
    cairo_move_to(cr, cont -> scale, row * cont -> scale);
    cairo_rel_line_to(cr, cont -> image_width - 2*cont -> scale, 0);
    cairo_stroke(cr);
  }

  for(int col = 0; col < cont -> matrix_width; col++)
  {
    /* Lineas horizontales */
    cairo_move_to(cr, col * cont -> scale,cont -> scale);
    cairo_rel_line_to(cr, 0, cont -> image_height - 2*cont -> scale);
    cairo_stroke(cr);
  }

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
      if(row == 0 || col == 0 || row == height - 1 || col == width -1)
      {
        change_pixel_color(bg, row, col, loyal_color);
      }
      else
      {
        if(((row ^ col)) % 2 == 0)
        {
          change_pixel_color(bg, row, col, loyal_color);
        }
        else
        {
          change_pixel_color(bg, row, col, rebel_color);

        }

      }

			/* Alpha channel */
      bg -> data[bg -> stride * row + col * 4 + 3] = 255;
    }
  }

	/* Pasamos la imagen a patron dado que necesitamos usar NEAREST NEIGHBOURS */
	bg -> pattern = cairo_pattern_create_for_surface(bg -> image);
	cairo_pattern_set_filter(bg -> pattern, CAIRO_FILTER_NEAREST);

  return bg;
}

static cairo_surface_t* init_vector_image(int height, int width)
{
  /* El formato de imagen: R G B de 8 bits cada uno */
  cairo_format_t format = CAIRO_FORMAT_ARGB32;
  /* La imagen vectorial misma */
  cairo_surface_t* img = cairo_image_surface_create(format, width, height);
  /* Inicialmente la imagen es completamente transparente */
  cairo_t* cr = cairo_create(img);
  cairo_set_source_rgba(cr, 0, 0, 0, 0);
  cairo_paint(cr);
  cairo_destroy(cr);

  return img;
}

/** Genera los contenedores para las dos imagenes superpuestas */
Content* drawing_init(int height, int width)
{
  height += 2;
  width += 2;

  Content* cont = malloc(sizeof(Content));

  cont -> matrix_height = height;
  cont -> matrix_width = width;

  loyal_color = color_init(205, 221, 255);
  rebel_color = color_init(202, 255, 219);
  blank_color = color_init(253, 255, 241);

  cont -> status_image = init_background(height, width);

  /* Las dimensiones de la ventana deben ajustarse a la matriz */
	cont -> scale = WINDOW_MAX_SIZE / fmax(height, width);
  printf("%lf\n", cont -> scale);
	cont -> image_height = cont -> scale * height;
	cont -> image_width  = cont -> scale * width;

  cont -> number_image = init_vector_image(cont -> image_height, cont -> image_width);


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
