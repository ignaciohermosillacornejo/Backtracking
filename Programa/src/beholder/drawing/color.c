#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "color.h"

/** Inicializa un color dentro de los rangos establecidos */
Color color_init(uint8_t R, uint8_t G, uint8_t B)
{
	return (Color)
	{
		.R = R,
		.G = G,
		.B = B,
	};
}
