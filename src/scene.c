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


/* Matrices */
Mtx viewMtx;
Mtx44 perspectiveMtx;

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
void SetLight(Mtx view);
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
		if (player->isPlaying) {
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
	SetLight(viewMtx);

	/* Draw terrain */
	OBJECT_render(objectTerrain, viewMtx);
	OBJECT_render(objectPlane, viewMtx);

	u8 i;
	for (i = 0; i < 4; i++) {
		player_t* player = GAME_getPlayerData(i);
		if (player->isPlaying)
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

void SetLight(Mtx view) {
	guVector lpos;
	GXLightObj lobj;

	lpos.x = 0.0f;	lpos.y = -1.0f; lpos.z = -0.3f;
	guVecNormalize(&lpos);
	guVecMultiplySR(view, &lpos, &lpos);

	GX_InitSpecularDirv(&lobj, &lpos);
	GX_InitLightColor(&lobj, lightColor[0]);
	GX_LoadLightObj(&lobj, GX_LIGHT0);

	// set number of rasterized color channels
	GX_SetNumChans(1);
	GX_SetChanCtrl(GX_COLOR0A0, GX_ENABLE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT0, GX_DF_CLAMP, GX_AF_NONE);
	GX_SetChanAmbColor(GX_COLOR0A0, lightColor[1]);
	GX_SetChanMatColor(GX_COLOR0A0, lightColor[2]);
	GX_SetTevOp(GX_TEVSTAGE0, GX_BLEND);
}
