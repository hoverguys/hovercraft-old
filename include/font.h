/*! \file font.h
 *  \brief Bitmap font management and drawing
 */

#ifndef _FONT_H
#define _FONT_H

#include <ogcsys.h>

#define CHAR_HEIGHT 36
#define CHAR_WIDTH  24

typedef struct {
	GXTexObj* texture;
} font_t;

font_t* FONT_load(GXTexObj* texture);
void FONT_free(font_t* font);

#endif