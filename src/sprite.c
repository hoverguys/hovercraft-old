#include "sprite.h"
#include <ogcsys.h>
#include "gxutils.h"

Mtx44 perspective;
void SPRITE_init() {
	GXRModeObj* rmode = GXU_getMode();
	guOrtho(perspective, 0, rmode->viHeight - 1, 0, rmode->viWidth - 1, 0, 300);
}