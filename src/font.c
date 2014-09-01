#include "font.h"
#include "mathutil.h"
#include <string.h>
#include <malloc.h>

void generateUV(font_t* font,
				const char* chars,
				const u32 charWidth,
				const u32 charHeight,
				const u32 columns,
				const TexSize texSize) {
	/* Find out char count and allocate the quad UV array */
	u32 charCount = strlen(chars);
	font->charUV = malloc(sizeof(charuv_t) * charCount);

	u8 currentColumn = 0;
	f32 spacing[2] = { charWidth + 1, charHeight + 1 };
	f32 uvsize[2] = { texSize, texSize };
	u8 currentCharId;
	for (currentCharId = 0; currentCharId < charCount; currentCharId++) {
		/* Avoid a nasty division each cycle */
		while (currentCharId > columns * (currentColumn + 1)) {
			currentColumn++;
		}

		f32 position[2] = { currentCharId % columns, currentColumn };

		/* Top left corner of the character */
		f32* uvTop = font->charUV[currentCharId].uvTop;
		ps_float2Mul(position, spacing, uvTop);
		ps_float2Mul(uvTop, uvsize, uvTop);

		/* Bottom right corner of the character */
		font->charUV[currentCharId].uvBottom[0] = uvTop[0] + charWidth;
		font->charUV[currentCharId].uvBottom[1] = uvTop[1] + charHeight;

		font->charIndex[(u8)chars[currentCharId]] = currentCharId;
	}
}

font_t* FONT_load(GXTexObj* texture,
				  const char* chars,
				  const u32 charWidth,
				  const u32 charHeight,
				  const u32 columns,
				  const TexSize texSize) {
	font_t* font = malloc(sizeof(font_t));
	font->texture = texture;
	generateUV(font, chars, charWidth, charHeight, columns, texSize);
	return font;
}

void FONT_free(font_t* font) {
	free(font);
}