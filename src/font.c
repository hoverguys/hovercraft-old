#include "font.h"

font_t* FONT_load(GXTexObj* texture) {
	font_t* font = malloc(sizeof(font_t));
	font->texture = texture;

	return font;
}

void FONT_free(font_t* font) {
	free(font);
}