#include "sprite.h"
#include "mathutil.h"
#include <malloc.h>

sprite_t* SPRITE_create() {
	sprite_t* sprite = malloc(sizeof(sprite_t));
	return sprite;
}

void SPRITE_setTexture(sprite_t* sprite, GXTexObj* texture, f32 position[2], f32 spriteSize[2], TexSize texSize) {
	sprite->texture = texture;
	f32 texRepr = 1.0f / texSize;
	f32 res[2] = { 0, 0 };
	f32 size[2] = { texRepr, texRepr };
	ps_float2Mul(position, size, res);
	sprite->UVoffsetX = res[0];
	sprite->UVoffsetY = res[1];
	ps_float2Mul(spriteSize, size, res);
	sprite->UVwidth = res[0];
	sprite->UVheight = res[1];
	sprite->transform.position = (guVector) { 0, 0, 0 };
	sprite->transform.scale = (guVector) { 1, 1, 1 }; 
	guQuaternion rotation;
	EulerToQuaternion(&rotation, 0, 0, 0);
	sprite->transform.rotation = rotation;

	MakeMatrix(&sprite->transform);
}

void SPRITE_flush(sprite_t* sprite) {
	if (sprite->transform.dirty == TRUE) {
		MakeMatrix(&sprite->transform);
	}
}

void SPRITE_render(sprite_t* sprite) {
	if (sprite->texture == NULL) return;

	/* Check dirty flag */
	SPRITE_flush(sprite);

	/* Set position to identity */
	Mtx viewMtx, modelviewMtx;
	guMtxIdentity(viewMtx);
	guMtxConcat(viewMtx, sprite->transform.matrix, modelviewMtx);
	GX_LoadPosMtxImm(modelviewMtx, GX_PNMTX0);

	/* Orthographic mode */
	GXU_2DMode();

	GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
	
	/* Top left */
	GX_Position2f32(0, 0);
	GX_TexCoord2f32(sprite->UVoffsetX, sprite->UVoffsetY);
	/* Bottom left */
	GX_Position2f32(0, sprite->UVheight);
	GX_TexCoord2f32(sprite->UVoffsetX, sprite->UVoffsetY + sprite->UVheight);
	/* Bottom right */
	GX_Position2f32(sprite->UVwidth, sprite->UVheight);
	GX_TexCoord2f32(sprite->UVoffsetX + sprite->UVwidth, sprite->UVoffsetY + sprite->UVheight);
	/* Top right */
	GX_Position2f32(sprite->UVwidth, 0);
	GX_TexCoord2f32(sprite->UVoffsetX + sprite->UVwidth, sprite->UVoffsetY);

	GX_End();
}

