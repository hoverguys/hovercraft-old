#include "game.h"

#include <stdio.h>
#include <malloc.h>
#include <math.h>

#include "mathutil.h"
#include "raycast.h"
#include "input.h"

object_t* mapTerrain;
object_t* mapPlane;

const int MAX_PLAYERS = 4;
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
	camera_t* camera = malloc(sizeof(camera_t));
	//TODO PROPER SPLITSCREEN
	GXRModeObj* rmode = VIDEO_GetPreferredMode(NULL);
	camera->width = rmode->viWidth;
	camera->height = rmode->viHeight;
	camera->offsetTop = camera->offsetLeft = 0;
	guPerspective(camera->perspectiveMtx, 60, (f32) camera->width / camera->height, 0.1f, 300.0f);
	players[playerId].camera = camera;

	/* Set player as playing */
	players[playerId].isPlaying = TRUE;
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
	f32 rot = INPUT_AnalogX(0) * .033f;
	f32 accel = INPUT_TriggerR(0) * .02f;
	f32 decel = INPUT_TriggerL(0) * .033f;

	/* Apply rotation */
	//OBJECT_rotate(players[playerId].hovercraft, 0, rot, 0);
	OBJECT_rotateAxis(players[playerId].hovercraft, &worldUp, rot);
	OBJECT_flush(players[playerId].hovercraft);

	/* calculate forward */
	guVecCross(right, &worldUp, &forward);
	guVecNormalize(&forward);

	/* Calculate physics */
	//if (players[playerId].isGrounded) {
		guVecScale(&forward, &acceleration, accel);
		guVecScale(&forward, &deacceleration, -decel);
	//}
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

	//printf("velocity %f %f %f\n", velocity->x, velocity->y, velocity->z);

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
			QUAT_lookat(&forward, &normalhit, &rotation);
			QUAT_slerp(&rotation, &players[playerId].hovercraft->transform.rotation, .9f, &rotation);
			OBJECT_moveTo(players[playerId].hovercraft, rayhit.x, height, rayhit.z);

			/* Since we hit the ground, reset the gravity*/
			players[playerId].isGrounded = TRUE;
			velocity->y = 0.0f;
		} else {
			/* We didnt move into the terrain */
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

void GAME_renderPlayer(u8 playerId, Mtx viewMtx) {
	/* Setup camera view and perspective */
	transform_t* target = &players[playerId].hovercraft->transform;
	camera_t* camera = players[playerId].camera;
	const float distance = 5.f;
	guVector targetPos = { target->position.x - target->forward.x * distance,
		target->position.y - target->forward.y * distance + distance * .5f,
		target->position.z - target->forward.z * distance };
	float t = 1.f / 10.f;
	guVector up = { 0, 1, 0 };
	guVector cam = { camera->position.x + t * (targetPos.x - camera->position.x),
		camera->position.y + t * (targetPos.y - camera->position.y),
		camera->position.z + t * (targetPos.z - camera->position.z) };

	guLookAt(viewMtx, &cam, &up, &target->position);

	GX_LoadProjectionMtx(camera->perspectiveMtx, GX_PERSPECTIVE);
	camera->position = cam;

	/* Render the player's hovercraft */
	OBJECT_render(players[playerId].hovercraft, viewMtx);
}

void followCamera(transform_t* target, float distance, Mtx viewMtx) {

}