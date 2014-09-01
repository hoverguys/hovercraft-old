#include "font.h"
#include <malloc.h>

font_t* FONT_load(GXTexObj* texture,
				  const char* chars,
				  const u32 charWidth,
				  const u32 charHeight,
				  const u32 columns) {
	font_t* font = malloc(sizeof(font_t));
	font->texture = texture;

	return font;
}

void FONT_free(font_t* font) {
	free(font);
}