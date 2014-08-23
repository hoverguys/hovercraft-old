/* System and SDK libraries */
#include <gccore.h>
#include <stdlib.h>

/* Generated assets headers */
#include "hovercraft_bmb.h"
#include "plane_bmb.h"
#include "terrain_bmb.h"
#include "ray_bmb.h"
#include "ring_bmb.h"
#include "textures.h"

#include "model.h"
#include "object.h"
#include "game.h"
#include "gxutils.h"
#include "input.h"

/* Model info */
model_t *modelHover, *modelTerrain, *modelPlane, *modelRay, *modelRing;
object_t *objectTerrain, *objectPlane, *planeRay, *firstRing, *secondRing;

/* Texture vars */
GXTexObj hoverTexObj, terrainTexObj, waterTexObj, rayTexObj, ringTexObj;

/* Light */
static GXColor lightColor[] = {
		{ 0xF0, 0xF0, 0xF0, 0xff }, /* Light color   */
		{ 0xB0, 0xB0, 0xB0, 0xff }, /* Ambient color */
		{ 0xFF, 0xFF, 0xFF, 0xff }  /* Mat color     */
};

BOOL firstFrame = TRUE;
guVector speedVec;

void SCENE_load() {
	GXU_init();
	//playMod();
	GXU_loadTexture(hovercraftTex, &hoverTexObj);
	GXU_loadTexture(terrainTex, &terrainTexObj);
	GXU_loadTexture(waterTex, &waterTexObj);
	GXU_loadTexture(rayTex, &rayTexObj);
	GXU_loadTexture(ringTex, &ringTexObj);

	modelHover = MODEL_setup(hovercraft_bmb);
	modelTerrain = MODEL_setup(terrain_bmb);
	modelPlane = MODEL_setup(plane_bmb);
	modelRay = MODEL_setup(ray_bmb);
	modelRing = MODEL_setup(ring_bmb);

	MODEL_setTexture(modelHover, &hoverTexObj);
	MODEL_setTexture(modelTerrain, &terrainTexObj);
	MODEL_setTexture(modelPlane, &waterTexObj);
	MODEL_setTexture(modelRay, &rayTexObj);
	MODEL_setTexture(modelRing, &ringTexObj);

	objectTerrain = OBJECT_create(modelTerrain);
	OBJECT_scaleTo(objectTerrain, 200, 200, 200);

	objectPlane = OBJECT_create(modelPlane);
	OBJECT_scaleTo(objectPlane, 1000, 1, 1000);
	OBJECT_moveTo(objectPlane, -500, 6.1f, -500);

	planeRay = OBJECT_create(modelRay);
	OBJECT_moveTo(planeRay, 0, 6.1f, 0);
	OBJECT_scaleTo(planeRay, 1.5f, 4, 1.5f);

	firstRing = OBJECT_create(modelRing);
	secondRing = OBJECT_create(modelRing);
	OBJECT_moveTo(firstRing, 0, 6.1f, 0);
	OBJECT_moveTo(secondRing, 0, 6.1f, 0);
	OBJECT_scaleTo(firstRing, 1.4f, 1, 1.4f);
	OBJECT_scaleTo(secondRing, 1.7f, 0.7f, 1.7f);

	GAME_init(objectTerrain, objectPlane);

	/* Wait for controllers */
	INPUT_waitForControllers();

	/* Check for Gamecube pads */
	u8 i;
	for (i = 0; i < MAX_PLAYERS; i++) {
		if (INPUT_isConnected(INPUT_CONTROLLER_GAMECUBE, i) == TRUE) {
			guVector position = { rand() % 200, 30.f, rand() % 200 };
			controller_t controller = { INPUT_CONTROLLER_GAMECUBE, i };
			GAME_createPlayer(controller, modelHover, position);
		}
	}

#ifdef WII
	/* Check for Gamecube pads */
	for (i = 0; i < MAX_PLAYERS; i++) {
		if (INPUT_isConnected(INPUT_CONTROLLER_WIIMOTE, i) == TRUE) {
			guVector position = { rand() % 200, 30.f, rand() % 200 };
			controller_t controller = { INPUT_CONTROLLER_WIIMOTE, i };
			GAME_createPlayer(controller, modelHover, position);
		}
	}
#endif

	/* We went through all players, so we know how to split the screen */
	playerArray_t players = GAME_getPlayersData();
	for (i = 0; i < players.playerCount; i++) {
		GXU_setupCamera(&players.players[i].camera, players.playerCount, i+1);
	}
}

void SCENE_render() {
	/* Render time */
	GX_SetNumChans(1);

	/* Animate scene models */
	OBJECT_rotate(firstRing, 0, 0.3f / 60.f, 0);
	OBJECT_rotate(secondRing, 0, -0.2f / 60.f, 0);

	u8 i;
	playerArray_t players = GAME_getPlayersData();
	for (i = 0; i < players.playerCount; i++) {
		if (players.players[i].isPlaying == TRUE) {
			GAME_updatePlayer(&players.players[i]);
			GAME_renderPlayerView(&players.players[i]);
		}
	}

	GAME_updateWorld();

	/* Flip framebuffer */
	GXU_done();
}

void SCENE_renderPlayer(Mtx viewMtx) {
	/* Set default blend mode*/
	GX_SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);

	/* Enable Zbuf */
	GX_SetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);

	/* Enable Light */
	GXU_setLight(viewMtx, lightColor);

	/* Draw terrain */
	OBJECT_render(objectTerrain, viewMtx);

	/* Draw players */
	u8 i;
	playerArray_t players = GAME_getPlayersData();
	for (i = 0; i < players.playerCount; i++) {
		if (players.players[i].isPlaying == TRUE) {
			OBJECT_render(players.players[i].hovercraft, viewMtx);
		}
	}


	/* Draw ray */
	/* Lighting off */
	GX_SetTevOp(GX_TEVSTAGE0, GX_MODULATE);
	GX_SetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT0, GX_DF_CLAMP, GX_AF_NONE);

	/* Draw water */
	OBJECT_render(objectPlane, viewMtx);

	/* Special blend mode */
	/* Disable Zbuf */
	GX_SetZMode(GX_TRUE, GX_LEQUAL, GX_FALSE);
	GX_SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_ONE, GX_LO_CLEAR);
	OBJECT_render(planeRay, viewMtx);
	OBJECT_render(firstRing, viewMtx);
	OBJECT_render(secondRing, viewMtx);
}