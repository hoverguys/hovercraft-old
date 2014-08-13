#include "game.h"

#include <malloc.h>
#include <math.h>
#include <string.h>

#include "gxutils.h"
#include "mathutil.h"
#include "raycast.h"
#include "input.h"

object_t* mapTerrain;
object_t* mapPlane;

player_t* players;

/* Game settings */
const f32 maxSpeed = 0.3f;
guVector gravity = { 0, -.8f / 60.f, 0 };

void GAME_init(object_t* terrain, object_t* plane) {
	mapTerrain = terrain;
	mapPlane = plane;

	players = calloc(MAX_PLAYERS, sizeof(player_t));
	u8 i;
	for (i = 0; i < MAX_PLAYERS; i++) {
		players[i].isPlaying = FALSE;
		players[i].isGrounded = FALSE;
	}
}

void GAME_createPlayer(u8 playerId, model_t* hovercraftModel) {
	/* Create player hovercraft object and position it */
	players[playerId].hovercraft = OBJECT_create(hovercraftModel);
	OBJECT_moveTo(players[playerId].hovercraft, 100, 50.f, 100);
	OBJECT_flush(players[playerId].hovercraft);

	/* Setup player's camera */
	players[playerId].camera = malloc(sizeof(camera_t));
	memset(players[playerId].camera, 0, sizeof(camera_t));

	/* Set player as playing */
	players[playerId].isPlaying = TRUE;
}

void GAME_removePlayer(u8 playerId) {
	free(players[playerId].camera);
	OBJECT_destroy(players[playerId].hovercraft);
}

void GAME_updatePlayer(u8 playerId) {
	/* Data */
	guVector acceleration = {0,0,0}, deacceleration = { 0, 0, 0 };
	guVector jump = { 0, 0.3f, 0 };
	guVector *velocity = &players[playerId].velocity;
	guVector *position = &players[playerId].hovercraft->transform.position;
	guVector *right = &players[playerId].hovercraft->transform.right;
	guVector forward, worldUp = {0,1,0};

	/* Get input */
	f32 rot = INPUT_AnalogX(playerId) * .033f;
	f32 accel = INPUT_TriggerR(playerId) * .02f;
	f32 decel = INPUT_TriggerL(playerId) * .033f;

	/* Apply rotation */
	OBJECT_rotateAxis(players[playerId].hovercraft, &worldUp, rot);
	OBJECT_flush(players[playerId].hovercraft);

	/* calculate forward */
	guVecCross(right, &worldUp, &forward);
	guVecNormalize(&forward);

	/* Calculate physics */
	guVecScale(&forward, &acceleration, accel);
	guVecScale(&forward, &deacceleration, -decel);

	/* Apply physics */
	guVecScale(velocity, velocity, 0.95f);
	guVecAdd(velocity, &acceleration, velocity);
	guVecAdd(velocity, &deacceleration, velocity);
	guVecAdd(velocity, &gravity, velocity);
	if (players[playerId].isGrounded && PAD_ButtonsDown(playerId) & PAD_BUTTON_X) {
		guVecAdd(velocity, &jump, velocity);
	}

	/* Limit speed */
	/*if (guVecDotProduct(velocity, velocity) > (maxSpeed*maxSpeed)) {
		guVecNormalize(velocity);
		guVecScale(velocity, velocity, maxSpeed);
	}*/

	/* Move Player */
	OBJECT_move(players[playerId].hovercraft, velocity->x, velocity->y, velocity->z);

	/* Collision check*/
	const f32 rayoffset = 200;
	guVector raydir = { 0, -1, 0 };
	guVector raypos = { 0, rayoffset, 0 };
	guVector rayhit, normalhit;
	guVecAdd(&raypos, position, &raypos);
	f32 dist = 0;
	f32 minHeight = mapPlane->transform.position.y;
	guQuaternion rotation;

	/* Raycast track */
	if (Raycast(mapTerrain, &raydir, &raypos, &dist, &normalhit)) {
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
			QUAT_slerp(&rotation, &players[playerId].hovercraft->transform.rotation, .9f, &rotation);
			OBJECT_moveTo(players[playerId].hovercraft, rayhit.x, height, rayhit.z);

			/* Since we hit the ground, reset the gravity*/
			players[playerId].isGrounded = TRUE;
			velocity->y = 0.0f;
		} else {
			/* We didn't move into the terrain */
			players[playerId].isGrounded = FALSE;

			/* Rotate back to level*/
			QUAT_lookat(&forward, &worldUp, &rotation);
			QUAT_slerp(&rotation, &players[playerId].hovercraft->transform.rotation, .9f, &rotation);
		}
	} else {
		/* Ray misses, we're up really high or on water, code below will make use*/
		players[playerId].isGrounded = FALSE;

		/* This should be avoided somehow */
		QUAT_lookat(&forward, &worldUp, &rotation);
		QUAT_slerp(&rotation, &players[playerId].hovercraft->transform.rotation, .9f, &rotation);
	}

	/* Rotate player again */
	OBJECT_rotateSet(players[playerId].hovercraft, &rotation);

	/* Make sure we do not move underwater */
	if (position->y < minHeight) {
		players[playerId].isGrounded = TRUE;
		velocity->y = 0.0f;
		OBJECT_moveTo(players[playerId].hovercraft, position->x, minHeight, position->z);
	}

	OBJECT_flush(players[playerId].hovercraft);
}

void GAME_renderPlayerView(u8 playerId) {
	/* Setup camera view and perspective */
	transform_t* target = &players[playerId].hovercraft->transform;
	camera_t* camera = players[playerId].camera;

	/* Settings */
	const float cameraHeight = 2.5;
	const float cameraDistance = -5.f;
	const float t = 1.f / 10.f;

	/* Calculate camera position*/
	guVector posTemp, targetCameraPos = { 0, cameraHeight, 0 };
	guVecScale(&target->forward, &posTemp, cameraDistance);
	guVecAdd(&targetCameraPos, &posTemp, &targetCameraPos);
	guVecAdd(&target->position, &targetCameraPos, &targetCameraPos);

	/* Lerp between old camera position and target */
	guVector camPos, up = { 0, 1, 0 };
	guVecSub(&targetCameraPos, &camera->position, &camPos);
	guVecScale(&camPos, &camPos, t);
	guVecAdd(&camera->position, &camPos, &camPos);

	/* make sure the camera does not enter the ground*/
	const f32 rayoffset = 400;
	guVector raydir = { 0, -1, 0 };
	guVector raypos = { 0, rayoffset, 0 };
	guVecAdd(&raypos, &camPos, &raypos);
	guVector normalhit;
	f32 dist = 0;
	if (Raycast(mapTerrain, &raydir, &raypos, &dist, &normalhit)) {
		if (dist < (rayoffset + cameraHeight)) {
			/* the camera is lower then it should be, move up*/
			camPos.y += (rayoffset + cameraHeight) - dist;
		}
	}

	/* Create camera matrix */
	Mtx viewMtx;
	guLookAt(viewMtx, &camPos, &up, &target->position);

	GX_LoadProjectionMtx(camera->perspectiveMtx, GX_PERSPECTIVE);
	camera->position = camPos;

	/* Viewport setup */
	GX_SetViewport(camera->offsetLeft, camera->offsetTop, camera->width, camera->height, 0, 1);

	/* Render the player's hovercraft */
	SCENE_renderPlayer(viewMtx);
}

player_t* GAME_getPlayerData(u8 playerId) {
	return &players[playerId];
}