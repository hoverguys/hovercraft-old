#include "font.h"

#include "mathutil.h"
#include <string.h>
#include <malloc.h>

f32 fontRatio;

void generateUV(font_t* font,
	const char* chars,
	const u32 charWidth,
	const u32 charHeight,
	const f32 texSize) {
	/* Find out char count and allocate the quad UV array */
	u16 charCount = strlen(chars);
	font->charUV = malloc(sizeof(charuv_t)* charCount);
	
	f32 texRepr = 1 / texSize;

	f32 texRepr2[2] = { texRepr, texRepr };
	f32 uvSize[2] = { charWidth, charHeight };
	f32 uvStride[2] = { uvSize[0] + 1, uvSize[1] + 1 };
	ps_float2Mul(uvSize, texRepr2, uvSize);
	ps_float2Mul(uvStride, texRepr2, uvStride);

	u16 i;
	f32 x = 0, y = 0;
	for (i = 0; i < charCount; i++) {
		if ((x + uvSize[0]) > 1.0f) {
			y += uvStride[1];
			x = 0;
		}

		//TL
		font->charUV[i].uvs[0] = x;
		font->charUV[i].uvs[1] = y;
		//BL
		font->charUV[i].uvs[2] = x;
		font->charUV[i].uvs[3] = y + uvSize[1];
		//BR
		font->charUV[i].uvs[4] = x + uvSize[0];
		font->charUV[i].uvs[5] = y + uvSize[1];
		//TR
		font->charUV[i].uvs[6] = x + uvSize[0];
		font->charUV[i].uvs[7] = y;

		font->charIndex[(u8)chars[i]] = i;

		x += uvStride[0];
	}
	font->charRatio = (f32)charWidth / (f32)charHeight;
}

void FONT_draw(font_t* font, const char* message, f32 x, f32 y, f32 size) {
	u32 charCount = strlen(message);
	u32 currentCharId;
	f32 height = size;
	f32 width = height * font->charRatio * fontRatio;

	GX_ClearVtxDesc();
	GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
	GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);

	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XY, GX_F32, 0);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);

	/* Set position to identity */
	Mtx modelView;
	guMtxIdentity(modelView);
	GX_LoadPosMtxImm(modelView, GX_PNMTX0);

	/* Set font texture */
	GX_LoadTexObj(font->texture, GX_TEXMAP0);

	/* Lighting off, Alpha blend */
	GX_SetNumChans(1);
	GX_SetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT0, GX_DF_CLAMP, GX_AF_NONE);
	GX_SetTevOp(GX_TEVSTAGE0, GX_MODULATE);
	GX_SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);

	/* Disable Z Reading/Writing */
	GX_SetZMode(GX_FALSE, GX_LEQUAL, GX_FALSE);

	/* Orthographic mode */
	GXU_2DMode();
	GX_Begin(GX_QUADS, GX_VTXFMT0, 4 * charCount);
	f32 xoffset = 0, yoffset = 0;
	for (currentCharId = 0; currentCharId < charCount; currentCharId++) {
		u8 index;
		/* Check for control characters */
		if (message[currentCharId] == '\n') {
			xoffset = - width;
			yoffset += height;
			index = font->charIndex[(u8)' '];
		} else {
			index = font->charIndex[(u8) message[currentCharId]];
		}

		/* CCW */
		/* Top left */
		GX_Position2f32(xoffset + x, yoffset + y);
		GX_TexCoord2f32(font->charUV[index].uvs[0], font->charUV[index].uvs[1]);
		/* Bottom left */
		GX_Position2f32(xoffset + x, yoffset + y + height - 1);
		GX_TexCoord2f32(font->charUV[index].uvs[2], font->charUV[index].uvs[3]);
		/* Bottom right */
		GX_Position2f32(xoffset + x + width - 1, yoffset + y + height - 1);
		GX_TexCoord2f32(font->charUV[index].uvs[4], font->charUV[index].uvs[5]);
		/* Top right */
		GX_Position2f32(xoffset + x + width - 1, yoffset + y);
		GX_TexCoord2f32(font->charUV[index].uvs[6], font->charUV[index].uvs[7]);

		xoffset += width;
	}
	GX_End();

	/* Re-enable Z Reading/Writing */
	GX_SetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
}

void FONT_init() {
	fontRatio = 1 / GXU_getAspectRatio();
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