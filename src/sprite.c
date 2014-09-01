#include "sprite.h"
#include <ogcsys.h>
#include "gxutils.h"

Mtx44 perspective;
void SPRITE_init() {
	GXRModeObj* rmode = GXU_getMode();
	guOrtho(perspective, 0, rmode->viHeight - 1, 0, rmode->viWidth - 1, 0, 300);
}

sprite_t* SPRITE_create() {
	sprite_t* sprite = malloc(sizeof(sprite_t));
	return sprite;
}

void SPRITE_setTexture(sprite_t* sprite, GXTexObj* texture, f32* position, f32* spriteSize, f32* texSize) {
	sprite->texture = texture;
	sprite->UVoffsetX = position[0] / texSize[0];
	sprite->UVoffsetY = position[1] / texSize[1];
	sprite->UVwidth  = spriteSize[0] / texSize[0];
	sprite->UVheight = spriteSize[1] / texSize[1];
}