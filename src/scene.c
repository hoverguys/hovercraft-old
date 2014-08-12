/* System and SDK libraries */
#include <ogcsys.h>

/* Generated assets headers */
#include "menumusic_mod.h"
#include "hovercraft_bmb.h"
#include "plane_bmb.h"
#include "terrain_bmb.h"
#include "textures.h"

#include "model.h"
#include "object.h"
#include "mathutil.h"
#include "game.h"
#include "gxutils.h"

/* Model info */
model_t *modelHover, *modelTerrain, *modelPlane;
object_t *objectTerrain, *objectPlane;

/* Texture vars */
GXTexObj hoverTexObj, terrainTexObj, waterTexObj;
TPLFile TPLfile;

/* Light */
static GXColor lightColor[] = {
		{ 0x4f, 0x4f, 0x4f, 0xff }, // Light color
		{ 0x0f, 0x0f, 0x0f, 0xff }, // Ambient color
		{ 0xff, 0xff, 0xff, 0xff }  // Mat color
};

BOOL firstFrame = TRUE;
guVector speedVec;

void playMod();
void followCamera(transform_t* target, float distance);

void SCENE_load() {
	GXU_init();
	//playMod();
	GXU_loadTexture(hovercraftTex, &hoverTexObj);
	GXU_loadTexture(terrainTex, &terrainTexObj);
	GXU_loadTexture(waterTex, &waterTexObj);

	modelHover = MODEL_setup(hovercraft_bmb);
	modelTerrain = MODEL_setup(terrain_bmb);
	modelPlane = MODEL_setup(plane_bmb);

	MODEL_setTexture(modelHover, &hoverTexObj);
	MODEL_setTexture(modelTerrain, &terrainTexObj);
	MODEL_setTexture(modelPlane, &waterTexObj);

	objectTerrain = OBJECT_create(modelTerrain);
	OBJECT_scaleTo(objectTerrain, 200, 200, 200);

	objectPlane = OBJECT_create(modelPlane);
	OBJECT_scaleTo(objectPlane, 1000, 1, 1000);
	OBJECT_moveTo(objectPlane, -500, .5f, -500);

	GAME_init(objectTerrain, objectPlane);
	GAME_createPlayer(0, modelHover);
}

void SCENE_update() {
	u8 i;
	for (i = 0; i < 4; i++) {
		player_t* player = GAME_getPlayerData(i);
		if (player->isPlaying == TRUE) {
			GAME_updatePlayer(i);
			GAME_renderPlayerView(i);
		}
	}
}

void SCENE_render(Mtx viewMtx) {
	/* Render time */
	GX_SetNumChans(1);

	if (firstFrame) {
		firstFrame = FALSE;
		VIDEO_SetBlack(FALSE);
	}

	/* Enable Light */
	GXU_setLight(viewMtx, lightColor);

	/* Draw terrain */
	OBJECT_render(objectTerrain, viewMtx);
	OBJECT_render(objectPlane, viewMtx);

	u8 i;
	for (i = 0; i < 4; i++) {
		player_t* player = GAME_getPlayerData(i);
		if (player->isPlaying == TRUE)
			OBJECT_render(player->hovercraft, viewMtx);
	}

	/* Finish up */
	GXU_done();
}

/* TO MOVE IN audioutils
static MODPlay play;
void playMod() {
	MODPlay_Init(&play);
	MODPlay_SetMOD(&play, menumusic_mod);
	MODPlay_Start(&play);
}*/