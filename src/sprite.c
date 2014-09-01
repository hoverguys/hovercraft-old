#include "sprite.h"
#include "gxutils.h"
#include "mathutil.h"

#include <malloc.h>

Mtx44 perspective;
void SPRITE_init() {
	GXRModeObj* rmode = GXU_getMode();
	guOrtho(perspective, 0, rmode->viHeight - 1, 0, rmode->viWidth - 1, 0, 300);
}

sprite_t* SPRITE_create() {
	sprite_t* sprite = malloc(sizeof(sprite_t));
	return sprite;
}

void SPRITE_setTexture(sprite_t* sprite, GXTexObj* texture, f32* position, f32* spriteSize, f32 texSize) {
	sprite->texture = texture;
	f32 res[2] = { 0, 0 };
	f32 size[2] = { texSize, texSize };
	ps_float2Mul(position, &size, &res);
	sprite->UVoffsetX = res[0];
	sprite->UVoffsetY = res[1];
	ps_float2Mul(spriteSize, &size, &res);
	sprite->UVwidth = res[0];
	sprite->UVheight = res[1];
}