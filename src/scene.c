/* System and SDK libraries */
#include <gccore.h>

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
#include "audioutil.h"
#include "raycast.h"
#include "mathutil.h"
#include "font.h"

#include "menumusic_mod.h"
#include "gamemusic_mod.h"

/* Model info */
model_t *modelHover, *modelTerrain, *modelPlane, *modelRay, *modelRing;
object_t *objectTerrain, *objectPlane, *planeRay, *firstRing, *secondRing;

/* Texture vars */
GXTexObj hoverTexObj, terrainTexObj, waterTexObj, rayTexObj, ringTexObj, fontTexObj;

/* Light */
static GXColor lightColor[] = {
		{ 0xF0, 0xF0, 0xF0, 0xff }, /* Light color   */
		{ 0xB0, 0xB0, 0xB0, 0xff }, /* Ambient color */
		{ 0xFF, 0xFF, 0xFF, 0xff }  /* Mat color     */
};

/* Spectator */
camera_t spectatorCamera;
Mtx spectatorView;

guVector checkpoint;

BOOL firstFrame = TRUE;
guVector speedVec;
BOOL isWaiting;

font_t* font;

void SCENE_load() {
	GXU_init();

	GXU_loadTexture(hovercraftTex, &hoverTexObj);
	GXU_loadTexture(terrainTex, &terrainTexObj);
	GXU_loadTexture(waterTex, &waterTexObj);
	GXU_loadTexture(rayTex, &rayTexObj);
	GXU_loadTexture(ringTex, &ringTexObj);
	GXU_loadTexture(ubuntuFontTex, &fontTexObj);

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
	OBJECT_scaleTo(firstRing, 1.4f, 1, 1.4f);
	OBJECT_scaleTo(secondRing, 1.7f, 0.7f, 1.7f);

	GAME_init(objectTerrain, objectPlane);

	/* Setup spectator matrix */
	GXU_setupCamera(&spectatorCamera, 1, 1);
	GX_SetViewport(spectatorCamera.offsetLeft, spectatorCamera.offsetTop, spectatorCamera.width, spectatorCamera.height, 0, 1);
	guVector spectatorPos = { -30, 40, -10 };
	guVector targetPos = { 100, 0, 100 };
	guVector spectatorUp = { 0, 1, 0 };
	guLookAt(spectatorView, &spectatorPos, &spectatorUp, &targetPos);
	
	FONT_init();
	font = FONT_load(&fontTexObj, " !,.0123456789:<>?ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz", 12, 22, 256);

	SCENE_moveCheckpoint();
	isWaiting = TRUE;
}

void SCENE_render() {
	/* Render time */
	GX_SetNumChans(1);

	/* Animate scene models */
	OBJECT_rotate(firstRing, 0, 0.3f / 60.f, 0);
	OBJECT_rotate(secondRing, 0, -0.2f / 60.f, 0);

	/* Wait for controllers */
	if (isWaiting) {
		GX_LoadProjectionMtx(spectatorCamera.perspectiveMtx, GX_PERSPECTIVE);
		SCENE_renderView(spectatorView);

		GXRModeObj* rmode = GXU_getMode();
		FONT_draw(font, "Connect at least one controller\nPress START or A to play", rmode->viWidth / 2, rmode->viHeight - 200, TRUE);

		if (INPUT_checkControllers()) {
			SCENE_createPlayers();
		}
	} else {
		u8 i;
		playerArray_t players = GAME_getPlayersData();
		for (i = 0; i < players.playerCount; i++) {
			GAME_updatePlayer(&players.players[i]);
			GAME_renderPlayerView(&players.players[i]);
			FONT_draw(font, "Score: 0000", 1, 1, FALSE);
		}
	}

	GAME_updateWorld();

	/* Flip framebuffer */
	GXU_done();
}

void SCENE_createPlayers() {
	/* Check for Gamecube pads */
	u8 i;
	for (i = 0; i < MAX_PLAYERS; i++) {
		if (INPUT_isConnected(INPUT_CONTROLLER_GAMECUBE, i) == TRUE) {
			guVector position = { fioraRand() * 200.f, 30.f, fioraRand() * 200.f };
			controller_t controller = { INPUT_CONTROLLER_GAMECUBE, i, 0 };
			GAME_createPlayer(controller, modelHover, position);
		}
	}

#ifdef WII
	/* Check for Wiimotes */
	for (i = WPAD_CHAN_0; i < WPAD_MAX_WIIMOTES; i++) {
		if (INPUT_isConnected(INPUT_CONTROLLER_WIIMOTE, i) == TRUE) {
			guVector position = { fioraRand() * 200.f, 30.f, fioraRand() * 200.f };
			controller_t controller = { INPUT_CONTROLLER_WIIMOTE, i, 0 };
			INPUT_getExpansion(&controller);
			GAME_createPlayer(controller, modelHover, position);
		}
	}
#endif

	/* We went through all players, so we know how to split the screen */
	playerArray_t players = GAME_getPlayersData();
	for (i = 0; i < players.playerCount; i++) {
		GXU_setupCamera(&players.players[i].camera, players.playerCount, i + 1);
	}

	isWaiting = FALSE;

	//AU_playMusic(menumusic_mod);
}

void SCENE_renderView(Mtx viewMtx) {
	/* Set default blend mode */
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

void SCENE_moveCheckpoint() {
	f32 distance = 0;
	f32 minHeight = objectPlane->transform.position.y - 0.9f;
	const f32 rayoffset = 200;
	guVector raydir = { 0, -1, 0 };
	guVector position;
	while (rayoffset - distance > minHeight) {
		position = (guVector) { fioraRand() * 200.f, rayoffset, fioraRand() * 200.f };
		Raycast(objectTerrain, &raydir, &position, &distance, NULL);
	}
	
	checkpoint = (guVector){ position.x, 0, position.z };

	OBJECT_moveTo(planeRay, checkpoint.x, objectPlane->transform.position.y + 4, checkpoint.z);
	OBJECT_flush(planeRay);
	OBJECT_moveTo(firstRing, checkpoint.x, objectPlane->transform.position.y + 0.5f, checkpoint.z);
	OBJECT_moveTo(secondRing, checkpoint.x, objectPlane->transform.position.y + 0.5f, checkpoint.z);
}

guVector SCENE_getCheckpoint() {
	return checkpoint;
}