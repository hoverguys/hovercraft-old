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
	u32 currentCharId;
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

		font->charIndex[(u8) chars[currentCharId]] = currentCharId;
	}
	font->charRatio = charWidth / charHeight;
}

void FONT_draw(font_t* font, const char* message, f32 x, f32 y, f32 size) {
	u32 charCount = strlen(message);
	u32 currentCharId;
	f32 height = size;
	f32 width = height * font->charRatio;

	GX_SetCullMode(GX_CULL_NONE);

	GX_ClearVtxDesc();
	GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
	GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);

	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XY, GX_F32, 0);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);

	Mtx44 modelView;
	guMtxIdentity(modelView);
	GX_LoadPosMtxImm(modelView, GX_PNMTX0);

	GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
	for (currentCharId = 0; currentCharId < charCount; currentCharId++) {
		u8 index = font->charIndex[(u8) message[currentCharId]];
		GX_Position2f32(x, y);
		GX_TexCoord2f32(font->charUV[index].uvTop[0], font->charUV[index].uvTop[1]);
		GX_Position2f32(x + width - 1, y);
		GX_TexCoord2f32(font->charUV[index].uvBottom[0], font->charUV[index].uvTop[1]);
		GX_Position2f32(x + width - 1, y + height - 1);
		GX_TexCoord2f32(font->charUV[index].uvBottom[0], font->charUV[index].uvBottom[1]);
		GX_Position2f32(x, y + height - 1);
		GX_TexCoord2f32(font->charUV[index].uvTop[0], font->charUV[index].uvBottom[1]);
	}
	GX_End();
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
	free(font->charUV);
	free(font);
}