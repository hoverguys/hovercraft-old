/*! \file font.h
 *  \brief Bitmap font management and drawing
 */

#ifndef _FONT_H
#define _FONT_H

#include <ogcsys.h>

typedef struct {
	GXTexObj* texture;
	u32 charHeight, charWidth;
	u32 columns;
} font_t;

/*! \brief Load a font from texture and data
 *  \param texture    Font texture
 *  \param chars      Character order (required for UV generation)
 *  \param charWidth  Width of each character
 *  \param charHeight Height of each character
 *  \param columns    Number of characters per row
 */
font_t* FONT_load(GXTexObj* texture, const char* chars, const u32 charWidth, const u32 charHeight, const u32 columns);

/*! \brief Frees font data 
 *  \param font Font structure to destroy
 */
void FONT_free(font_t* font);

#endif