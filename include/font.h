/*! \file font.h
 *  \brief Bitmap font management and drawing
 */

#ifndef _FONT_H
#define _FONT_H

#include <ogcsys.h>
#include "gxutils.h"

typedef struct {
	f32 uvs[8];
} charuv_t;

typedef struct {
	GXTexObj* texture;
	u8        charIndex[128];
	charuv_t* charUV;
	u16 width, height;
} font_t;

/*! \brief Initialize font subsystem (requires GXU) 
 */
void FONT_init();

/*! \brief Load a font from texture and data
 *  \param texture    Font texture
 *  \param chars      Character order (required for UV generation)
 *  \param charWidth  Width of each character
 *  \param charHeight Height of each character
 *  \param columns    Number of characters per row
 *  \param texSize    Texture size
 */
font_t* FONT_load(GXTexObj* texture, const char* chars, const u16 charWidth, const u16 charHeight, const u16 texSize);

/*! \brief Draws a message using the provided font 
 *  \param font    Font to use
 *  \param message Message to write
 *  \param x       X coordinate
 *  \param y       Y coordinate
 *  \param size    Font size
 */
void FONT_draw(font_t* font, const char* message, f32 x, f32 y, BOOL centre);

/*! \brief Frees font data 
 *  \param font Font structure to destroy
 */
void FONT_free(font_t* font);

#endif