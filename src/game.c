/* System and SDK libraries */
#include <malloc.h>
#include <gccore.h>
#include <stdio.h>

/* Internal headers */
#include "game.h"
#include "model.h"
#include "object.h"
#include "font.h"
#include "audioutil.h"
#include "gxutils.h"
#include "mathutil.h"
#include "raycast.h"
#include "input.h"

/* Generated assets headers */
#include "hovercraft_bmb.h"
#include "plane_bmb.h"
#include "terrain_bmb.h"
#include "ray_bmb.h"
#include "ring_bmb.h"
#include "pickup_bmb.h"
#include "textures.h"

#include "menumusic_mod.h"
#include "gamemusic_mod.h"

player_t players[MAX_PLAYERS];
u8 playerCount = 0;

/* Game settings */
const f32 maxSpeed = 0.3f;
guVector gravity = { 0, -.8f / 60.f, 0 };

/* Model info */
model_t *modelHover, *modelTerrain, *modelPlane, *modelRay, *modelRing, *modelPickup;
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

/* Pickup points */
static const guVector pickupPoints[] = {
	{ 76, 7, 136 },
	{ 90, 7, 59 },
	{ 148, 7, 21 },
	{ 200, 7, 82 },
	{ 113, 22, 134 },
	{ 6, 7, 136 }
};
static const int pickupPointsCount = sizeof(pickupPoints) / sizeof(pickupPoints[0]);
pickup_t pickups[sizeof(pickupPoints) / sizeof(pickupPoints[0])];

f32 pickupTimeout = 10;

guVector checkpoint;

BOOL isWaiting;

font_t* font;

/* Util functions */
void _moveCheckpoint();
void _createPlayers();
void _getPickup(u8 playerId, u8 pickupId);

void GAME_init() {
	GXU_init();

	GXU_loadTexture(hovercraftTex, &hoverTexObj);
	GXU_loadTexture(terrainTex, &terrainTexObj);
	GXU_loadTexture(waterTex, &waterTexObj);
	GXU_loadTexture(rayTex, &rayTexObj);
	GXU_loadTexture(ringTex, &ringTexObj);
	GXU_loadTexture(ubuntuFontTex, &fontTexObj);
	GX_InitTexObjWrapMode(&fontTexObj, GX_CLAMP, GX_CLAMP); //Point filtering

	GXU_closeTPL();

	modelHover = MODEL_setup(hovercraft_bmb);
	modelTerrain = MODEL_setup(terrain_bmb);
	modelPlane = MODEL_setup(plane_bmb);
	modelRay = MODEL_setup(ray_bmb);
	modelRing = MODEL_setup(ring_bmb);
	modelPickup = MODEL_setup(pickup_bmb);

	MODEL_setTexture(modelHover, &hoverTexObj);
	MODEL_setTexture(modelTerrain, &terrainTexObj);
	MODEL_setTexture(modelPlane, &waterTexObj);
	MODEL_setTexture(modelRay, &rayTexObj);
	MODEL_setTexture(modelRing, &ringTexObj);
	MODEL_setTexture(modelPickup, &rayTexObj);

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

	/* Setup pickup points */
	u8 pickupIndex;
	for (pickupIndex = 0; pickupIndex < pickupPointsCount; pickupIndex++) {
		pickup_t currentPickup;
		currentPickup.enable = TRUE;

		// Create the pickup object and move it to its position
		object_t* pickupObject = OBJECT_create(modelPickup);
		currentPickup.object = pickupObject;
		guVector pickupPosition = pickupPoints[pickupIndex];
		OBJECT_moveTo(pickupObject, pickupPosition.x, pickupPosition.y, pickupPosition.z);

		pickups[pickupIndex] = currentPickup;
	}

	/* Setup spectator matrix */
	GXU_setupCamera(&spectatorCamera, 1, 1);
	GX_SetViewport(spectatorCamera.offsetLeft, spectatorCamera.offsetTop, spectatorCamera.width, spectatorCamera.height, 0, 1);
	guVector spectatorPos = { -30, 40, -10 };
	guVector targetPos = { 100, 0, 100 };
	guVector spectatorUp = { 0, 1, 0 };
	guLookAt(spectatorView, &spectatorPos, &spectatorUp, &targetPos);

	FONT_init();
	font = FONT_load(&fontTexObj, " !,.0123456789:<>?ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz", 12, 22, 256, 1.f);

	_moveCheckpoint();
	isWaiting = TRUE;
}


void GAME_createPlayer(controller_t controllerInfo, model_t* hovercraftModel, guVector startPosition) {
	/* We should tell the parent that we didn't actually make the player */
	if (playerCount > MAX_PLAYERS) return;

	/* Create player hovercraft object and position it */
	player_t* player = &players[playerCount];
	player->hovercraft = OBJECT_create(hovercraftModel);
	OBJECT_moveTo(player->hovercraft, startPosition.x, startPosition.y, startPosition.z);
	OBJECT_flush(player->hovercraft);

	player->isPlaying = TRUE;
	player->controller = controllerInfo;
	playerCount++;
}

void GAME_removePlayer(player_t* player) {
	OBJECT_destroy(player->hovercraft);
	playerCount--;
}

void GAME_updateWorld() {
	/* Check for collisions
	 * To optimize and avoid glitches, collisions are calculated on the world loop
	 * to both minimize physics getting in the way and assure that calculations
	 * between different players are only evaluated once per frame.
	 */

	u8 playerId, otherPlayerId;
	for (playerId = 0; playerId < playerCount; playerId++) {
		player_t* actor = &players[playerId];

		/* Collisions against other players */
		for (otherPlayerId = playerId + 1; otherPlayerId < playerCount; otherPlayerId++) {
			player_t* target = &players[otherPlayerId];

			/* Check for collision between current player and other */
			CalculateBounce(actor, target);
		}

		/* Collisions with the checkpoint */
		checkpoint.y = actor->hovercraft->transform.position.y;
		f32 distance = vecDistance(&actor->hovercraft->transform.position, &checkpoint);
		if (distance < 3.5f) {
			_moveCheckpoint();
		}

		/* Collisions with pickups */
		u8 pickupId;
		for (pickupId = 0; pickupId < pickupPointsCount; pickupId++) {
			if (pickups[pickupId].enable == TRUE) {
				distance = vecDistance(&actor->hovercraft->transform.position, &pickups[pickupId].object->transform.position);
				if (distance < 2.f) {
					_getPickup(playerId, pickupId);
				}
			}
		}
	}

	/* Update pickup timers
	 * If a pickup is disabled (has been taken) it will have a positive "timeout" value.
	 * Each update loop should decrease the timeout by the delta time value and re-enable
	 * the pickup when it reaches zero (or lower).
	 */
	u8 pickupId;
	for (pickupId = 0; pickupId < pickupPointsCount; pickupId++) {
		if (pickups[pickupId].enable == FALSE) {
			pickups[pickupId].timeout -= 1.f/60.f;

			if (pickups[pickupId].timeout <= 0) {
				pickups[pickupId].enable = TRUE;
			}
		}
	}
}

void GAME_updatePlayer(player_t* player) {
	/* Data */
	guVector acceleration = { 0, 0, 0 };
	guVector jump = { 0, 0.3f, 0 };
	guVector *velocity = &player->velocity;
	guVector *position = &player->hovercraft->transform.position;
	guVector *right = &player->hovercraft->transform.right;
	guVector *playerForward = &player->hovercraft->transform.forward;
	guVector forward, worldUp = { 0, 1, 0 };

	/* Get input */
	f32 rot = INPUT_steering(&player->controller) * .033f;
	f32 accel = INPUT_acceleration(&player->controller) * .02f;

	/* Apply rotation */
	OBJECT_rotateAxis(player->hovercraft, &worldUp, rot);
	OBJECT_flush(player->hovercraft);

	/* Calculate forward */
	guVecCross(right, &worldUp, &forward);
	guVecNormalize(&forward);

	/* Calculate physics */
	guVecScale(playerForward, &acceleration, accel);

	/* Apply physics */
	guVecScale(velocity, velocity, 0.95f);
	guVecAdd(velocity, &acceleration, velocity);
	guVecAdd(velocity, &gravity, velocity);
	if (player->isGrounded && INPUT_jump(&player->controller) == TRUE) {
		guVecAdd(velocity, &jump, velocity);
	}

	/* Limit speed */
	/*if (guVecDotProduct(velocity, velocity) > (maxSpeed*maxSpeed)) {
		guVecNormalize(velocity);
		guVecScale(velocity, velocity, maxSpeed);
		}*/

	/* Move Player */
	OBJECT_move(player->hovercraft, velocity->x, velocity->y, velocity->z);

	/* Collision check*/
	const f32 rayoffset = 200;
	guVector raydir = { 0, -1, 0 };
	guVector raypos = { 0, rayoffset, 0 };
	guVector rayhit, normalhit;
	guVecAdd(&raypos, position, &raypos);
	f32 dist = 0;
	f32 minHeight = objectPlane->transform.position.y;
	guQuaternion rotation;

	/* Raycast track */
	if (Raycast(objectTerrain, &raydir, &raypos, &dist, &normalhit)) {
		/* Get hit position */
		guVecScale(&raydir, &rayhit, dist);
		guVecAdd(&rayhit, &raypos, &rayhit);
		f32 height = rayhit.y;

		if (dist < rayoffset) {
			/* Moved into the terrain, snap */
			guVector f;
			guVecCross(right, &normalhit, &f);
			guVecNormalize(&f);
			QUAT_lookat(&f, &normalhit, &rotation);
			QUAT_slerp(&rotation, &player->hovercraft->transform.rotation, .9f, &rotation);
			OBJECT_moveTo(player->hovercraft, rayhit.x, height, rayhit.z);

			/* Since we hit the ground, reset the gravity */
			player->isGrounded = TRUE;
			velocity->y = 0.0f;
		} else {
			/* We didn't move into the terrain */
			player->isGrounded = FALSE;

			/* Rotate back to level*/
			QUAT_lookat(&forward, &worldUp, &rotation);
			QUAT_slerp(&rotation, &player->hovercraft->transform.rotation, .9f, &rotation);
		}
	} else {
		/* Ray misses, we're up really high or on water, code below will make use*/
		player->isGrounded = FALSE;

		/* This should be avoided somehow */
		QUAT_lookat(&forward, &worldUp, &rotation);
		QUAT_slerp(&rotation, &player->hovercraft->transform.rotation, .9f, &rotation);
	}

	/* Rotate player again */
	OBJECT_rotateSet(player->hovercraft, &rotation);

	/* Make sure we do not move underwater */
	if (position->y < minHeight) {
		player->isGrounded = TRUE;
		velocity->y = 0.0f;
		OBJECT_moveTo(player->hovercraft, position->x, minHeight, position->z);
	}

	OBJECT_flush(player->hovercraft);
}

void GAME_render() {
	/* Render time */
	GX_SetNumChans(1);

	/* Animate scene models */
	OBJECT_rotate(firstRing, 0, 0.3f / 60.f, 0);
	OBJECT_rotate(secondRing, 0, -0.2f / 60.f, 0);

	/* Wait for controllers */
	if (isWaiting) {
		GX_LoadProjectionMtx(spectatorCamera.perspectiveMtx, GX_PERSPECTIVE);
		GAME_renderView(spectatorView);

		GXRModeObj* rmode = GXU_getMode();
		FONT_draw(font, "Connect at least one controller\nPress START or A to play", rmode->viWidth / 2, rmode->viHeight - 200, TRUE);

		if (INPUT_checkControllers()) {
			_createPlayers();
		}
	} else {
		u8 i;;
		for (i = 0; i < playerCount; i++) {
			GAME_updatePlayer(&players[i]);
			GAME_renderPlayerView(&players[i]);
			FONT_draw(font, "Score: 0000", 1, 1, FALSE);
			char debugPos[30];
			guVector* playerPosition = &(players[i].hovercraft->transform.position);
			sprintf(debugPos, "X %.2f Y %.2f Z %.2f", playerPosition->x, playerPosition->y, playerPosition->z);
			FONT_draw(font, debugPos, 1, 30, FALSE);
		}
	}

	GAME_updateWorld();

	/* Flip framebuffer */
	GXU_done();
}

void GAME_renderView(Mtx viewMtx) {
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
	for (i = 0; i < playerCount; i++) {
		if (players[i].isPlaying == TRUE) {
			OBJECT_render(players[i].hovercraft, viewMtx);
		}
	}

	/* Draw pickups */
	for (i = 0; i < pickupPointsCount; i++) {
		if (pickups[i].enable == TRUE) {
			OBJECT_render(pickups[i].object, viewMtx);
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

void GAME_renderPlayerView(player_t* player) {
	/* Setup camera view and perspective */
	transform_t target = player->hovercraft->transform;
	camera_t* camera = &player->camera;

	/* Settings */
	const float targetHeight = 1.6f;
	const float cameraHeight = 2.0f;
	const float cameraMinHeight = 0.1f;
	const float cameraDistance = -5.0f;
	const float t = 1.f / 5.f;
	guVector up = { 0, 1, 0 };

	/* Calculate camera position */
	guVector posTemp, targetCameraPos = { 0, cameraHeight, 0 };
	guVecScale(&target.forward, &posTemp, cameraDistance);
	guVecAdd(&targetCameraPos, &posTemp, &targetCameraPos);
	guVecAdd(&target.position, &targetCameraPos, &targetCameraPos);

	/* Calculate camera target */
	guVector targetPos;
	guVecScale(&target.up, &targetPos, targetHeight);
	guVecAdd(&targetPos, &target.position, &targetPos);

	/* Lerp between old camera position and target */
	guVector camPos;
	guVecSub(&targetCameraPos, &camera->position, &camPos);
	guVecScale(&camPos, &camPos, t);
	guVecAdd(&camera->position, &camPos, &camPos);

	/* Make sure the camera does not enter the ground */
	const f32 rayoffset = 400;
	guVector raydir = { 0, -1, 0 };
	guVector raypos = { 0, rayoffset, 0 };
	guVecAdd(&raypos, &camPos, &raypos);
	guVector normalhit;
	f32 dist = 0;
	if (Raycast(objectTerrain, &raydir, &raypos, &dist, &normalhit)) {
		if (dist < (rayoffset + cameraMinHeight)) {
			/* the camera is lower then it should be, move up */
			camPos.y += (rayoffset + cameraMinHeight) - dist;
		}
	} else {
		if (camPos.y < cameraMinHeight) {
			camPos.y = cameraMinHeight;
		}
	}

	/* Create camera matrix */
	Mtx viewMtx;
	guLookAt(viewMtx, &camPos, &up, &targetPos);

	GX_LoadProjectionMtx(camera->perspectiveMtx, GX_PERSPECTIVE);
	camera->position = camPos;

	/* Viewport setup */
	GXU_SetViewport(camera->offsetLeft, camera->offsetTop, camera->width, camera->height, 0, 1);

	/* Render the player's hovercraft */
	GAME_renderView(viewMtx);
}

void _moveCheckpoint() {
	f32 distance = 0;
	f32 minHeight = objectPlane->transform.position.y - 0.9f;
	const f32 rayoffset = 200;
	guVector raydir = { 0, -1, 0 };
	guVector position;
	while (rayoffset - distance > minHeight) {
		position = (guVector) { fioraRand() * 200.f, rayoffset, fioraRand() * 200.f };
		Raycast(objectTerrain, &raydir, &position, &distance, NULL);
	}

	checkpoint = (guVector) { position.x, 0, position.z };

	OBJECT_moveTo(planeRay, checkpoint.x, objectPlane->transform.position.y + 4, checkpoint.z);
	OBJECT_flush(planeRay);
	OBJECT_moveTo(firstRing, checkpoint.x, objectPlane->transform.position.y + 0.5f, checkpoint.z);
	OBJECT_moveTo(secondRing, checkpoint.x, objectPlane->transform.position.y + 0.5f, checkpoint.z);
}

void _createPlayers() {
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
	for (i = 0; i < playerCount; i++) {
		GXU_setupCamera(&players[i].camera, playerCount, i + 1);
	}

	isWaiting = FALSE;

	AU_playMusic(menumusic_mod);
}

void _getPickup(u8 playerId, u8 pickupId) {
	pickups[pickupId].enable = FALSE;
	pickups[pickupId].timeout = pickupTimeout;
}