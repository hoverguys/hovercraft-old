#include "font.h"

#include "mathutil.h"
#include <string.h>
#include <malloc.h>
#include <math.h>

f32 fontRatio;

void _FONT_GenerateUV(font_t* font,
	const char* chars,
	const u16 charWidth,
	const u16 charHeight,
	const u16 texSize) {
	/* Find out char count and allocate the quad UV array */
	u16 charCount = strlen(chars);
	font->charUV = malloc(sizeof(charuv_t)* charCount);

	f32 texRepr = 1.0f / texSize;

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
}

void _FONT_Prep(font_t* font) {
	GX_ClearVtxDesc();
	GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
	GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);

	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XY, GX_F32, 0);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);

	/* Set position to identity */
	Mtx modelView;
	guMtxIdentity(modelView);
	GX_LoadNrmMtxImm(modelView, GX_PNMTX0); //dummies required
	guMtxTransApply(modelView, modelView, 0, 0, -1); //0 isnt allowed, -1 is minimum TODO: sync with near plane value
	GX_LoadPosMtxImm(modelView, GX_PNMTX0);

	/* Set color */
	GX_SetChanAmbColor(GX_COLOR0A0, font->color);
	GX_SetChanMatColor(GX_COLOR0A0, font->color);

	/* Set font texture */
	GX_LoadTexObj(font->texture, GX_TEXMAP0);

	/* Lighting off, Alpha blend */
	GX_SetNumChans(1);
	GX_SetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT0, GX_DF_CLAMP, GX_AF_NONE);
	GX_SetTevOp(GX_TEVSTAGE0, GX_MODULATE);
	GX_SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);

	/* Orthographic mode */
	GXU_2DMode();
}

void _FONT_Rect(font_t* font, u32 index, f32 x, f32 y) {
	const f32 height = font->height * font->scale;
	const f32 width = font->width * font->scale;

	GX_Position2f32(x, y);
	GX_TexCoord2f32(font->charUV[index].uvs[0], font->charUV[index].uvs[1]);
	/* Bottom left */
	GX_Position2f32(x, y + height);
	GX_TexCoord2f32(font->charUV[index].uvs[2], font->charUV[index].uvs[3]);
	/* Bottom right */
	GX_Position2f32(x + width, y + height);
	GX_TexCoord2f32(font->charUV[index].uvs[4], font->charUV[index].uvs[5]);
	/* Top right */
	GX_Position2f32(x + width, y);
	GX_TexCoord2f32(font->charUV[index].uvs[6], font->charUV[index].uvs[7]);
}

void FONT_draw(font_t* font, const char* message, f32 x, f32 y, BOOL center) {
	const u16 messagelength = strlen(message);
	const char* msgpointer = message;
	const f32 height = font->height * font->scale;
	const f32 width = font->width * font->scale;

	_FONT_Prep(font);

	u16 charCount = 0, offset = 0;
	f32 xoffset = 0, yoffset = 0, centreoffset = 0;
	do {
		charCount = strcspn(msgpointer, "\n"); //length till newline (exclusive)

		if (center) {
			centreoffset = -(charCount / 2) * width;
		}

		//Skip recurring
		if (charCount > 0) {
			GX_Begin(GX_QUADS, GX_VTXFMT0, 4 * charCount);
			u16 i;
			for (i = 0; i < charCount; i++) {
				u8 index = font->charIndex[(u8)msgpointer[i]];
				f32 xx = centreoffset + xoffset + x;
				f32 yy = yoffset + y;

				//Build vertices
				_FONT_Rect(font, index, xx, yy);

				xoffset += width;
			}
			GX_End();
		}

		// Newline
		xoffset = 0;
		yoffset += height;

		msgpointer += charCount + 1;
		offset += charCount + 1;
	} while (offset < messagelength);
}

void FONT_drawScroller(font_t* font, const char* message, f32 x, f32 y, f32 padding, f32 freq, f32 amplitude, f32 progress) {
	const u16 messagelength = strlen(message);
	const char* msgpointer = message;
	const f32 width = font->width * font->scale;

	_FONT_Prep(font);

	u16 charCount = 0, offset = 0;
	f32 xoffset = 0;
	do {
		charCount = strcspn(msgpointer, "\n"); //length till newline (exclusive)

											   //Skip recurring
		if (charCount > 0) {
			GX_Begin(GX_QUADS, GX_VTXFMT0, 4 * charCount);
			u16 i;
			for (i = 0; i < charCount; i++) {
				u8 index = font->charIndex[(u8)msgpointer[i]];
				f32 xx = xoffset + x + (i * padding);
				f32 yy = y + cos(progress + i * freq) * amplitude;

				//Build vertices
				_FONT_Rect(font, index, xx, yy);

				xoffset += width;
			}
			GX_End();
		}

		msgpointer += charCount + 1;
		offset += charCount + 1;
	} while (offset < messagelength);
}

void FONT_init() {
	fontRatio = 1 / GXU_getAspectRatio();
}

font_t* FONT_load(GXTexObj* texture,
	const char* chars,
	const u16 charWidth,
	const u16 charHeight,
	const u16 texSize,
	const f32 scale) {
	font_t* font = malloc(sizeof(font_t));
	font->width = charWidth;
	font->height = charHeight;
	font->scale = scale;
	font->color = (GXColor) { 0xFF, 0xFF, 0xFF, 0xFF };

	font->texture = texture;
	GX_InitTexObjWrapMode(texture, GX_CLAMP, GX_CLAMP);
	GX_InitTexObjFilterMode(texture, GX_NEAR, GX_NEAR);

	_FONT_GenerateUV(font, chars, charWidth, charHeight, texSize);
	return font;
}

void FONT_color(font_t* font, GXColor color) {
	font->color = color;
}

void FONT_free(font_t* font) {
	free(font->charUV);
	free(font);
}