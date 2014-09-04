#include "font.h"

#include "mathutil.h"
#include <string.h>
#include <malloc.h>

void generateUV(font_t* font,
	const char* chars,
	const u32 charWidth,
	const u32 charHeight,
	const f32 texSize) {
	/* Find out char count and allocate the quad UV array */
	u16 charCount = strlen(chars);
	font->charUV = malloc(sizeof(charuv_t)* charCount);

	f32 uvWidth = charWidth / texSize;
	f32 uvHeight = charHeight / texSize;
	f32 uvWStride = (charWidth+1) / texSize;
	f32 uvHStride = (charHeight+1) / texSize;

	u16 i;
	f32 x = 0, y = 0;
	for (i = 0; i < charCount; i++) {
		if ((x + uvWidth) > 1.0f) {
			y += uvHStride;
			x = 0;
		}

		//TL
		font->charUV[i].uvs[0] = x;
		font->charUV[i].uvs[1] = y;
		//BL
		font->charUV[i].uvs[2] = x;
		font->charUV[i].uvs[3] = y + uvHeight;
		//BR
		font->charUV[i].uvs[4] = x + uvWidth;
		font->charUV[i].uvs[5] = y + uvHeight;
		//TR
		font->charUV[i].uvs[6] = x + uvWidth;
		font->charUV[i].uvs[7] = y;

		font->charIndex[(u8)chars[i]] = i;

		x += uvWStride;
	}
	font->charRatio = (f32)charWidth / (f32)charHeight;
}

void FONT_draw(font_t* font, const char* message, f32 x, f32 y, f32 size) {
	u32 charCount = strlen(message);
	u32 currentCharId;
	f32 height = size;
	f32 width = height * font->charRatio;

	GX_ClearVtxDesc();
	GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
	GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);

	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XY, GX_F32, 0);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);

	//Set position to identity
	Mtx modelView;
	guMtxIdentity(modelView);
	GX_LoadPosMtxImm(modelView, GX_PNMTX0);

	// Set font texture
	GX_LoadTexObj(font->texture, GX_TEXMAP0);

	// Lighting off, Alpha blend
	GX_SetNumChans(1);
	GX_SetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT0, GX_DF_CLAMP, GX_AF_NONE);
	GX_SetTevOp(GX_TEVSTAGE0, GX_MODULATE);
	GX_SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);

	// Disable Z Reading/Writing
	GX_SetZMode(GX_FALSE, GX_LEQUAL, GX_FALSE);

	// Orthographical mode
	GXU_2DMode();
	GX_Begin(GX_QUADS, GX_VTXFMT0, 4 * charCount);
	for (currentCharId = 0; currentCharId < charCount; currentCharId++) {
		u8 index = font->charIndex[(u8)message[currentCharId]];
		f32 xoffset = currentCharId * width;

		// CCW
		// Top left
		GX_Position2f32(xoffset + x, y);
		GX_TexCoord2f32(font->charUV[index].uvs[0], font->charUV[index].uvs[1]);
		// Bottom left
		GX_Position2f32(xoffset + x, y + height - 1);
		GX_TexCoord2f32(font->charUV[index].uvs[2], font->charUV[index].uvs[3]);
		// Bottom right
		GX_Position2f32(xoffset + x + width - 1, y + height - 1);
		GX_TexCoord2f32(font->charUV[index].uvs[4], font->charUV[index].uvs[5]);
		// Top right
		GX_Position2f32(xoffset + x + width - 1, y);
		GX_TexCoord2f32(font->charUV[index].uvs[6], font->charUV[index].uvs[7]);

	}
	GX_End();

	// Reenable Z Reading/Writing
	GX_SetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
}

font_t* FONT_load(GXTexObj* texture,
	const char* chars,
	const u32 charWidth,
	const u32 charHeight,
	const f32 texSize) {
	font_t* font = malloc(sizeof(font_t));
	font->texture = texture;
	GX_InitTexObjWrapMode(texture, GX_CLAMP, GX_CLAMP);
	GX_InitTexObjFilterMode(texture, GX_LINEAR, GX_LINEAR);
	generateUV(font, chars, charWidth, charHeight, texSize);
	return font;
}

void FONT_free(font_t* font) {
	free(font->charUV);
	free(font);
}