#include "game.h"

#include <malloc.h>

#include "gxutils.h"
#include "mathutil.h"
#include "raycast.h"
#include "input.h"

object_t* mapTerrain;
object_t* mapPlane;

player_t players[MAX_PLAYERS];
u8 playerCount = 0;

/* Game settings */
const f32 maxSpeed = 0.3f;
guVector gravity = { 0, -.8f / 60.f, 0 };

void GAME_init(object_t* terrain, object_t* plane) {
	mapTerrain = terrain;
	mapPlane = plane;
}

void GAME_createPlayer(controller_t controllerInfo, model_t* hovercraftModel, guVector startPosition) {
	/* We should tell the parent that we didn't actually make the player */
	if (playerCount > MAX_PLAYERS) return;

	/* Create player hovercraft object and position it */
	player_t* player = &GAME_getPlayersData().players[playerCount];
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
	 * to both minimize physics getting in the way and assure that calculations between
	 * different players are only evaluated once per frame
	 */

	playerArray_t players = GAME_getPlayersData();
	u8 playerId, otherPlayerId;
	guVector checkpoint = SCENE_getCheckpoint();
	for (playerId = 0; playerId < players.playerCount; playerId++) {
		player_t* actor = &players.players[playerId];

		/* Collisions against other players */
		for (otherPlayerId = playerId + 1; otherPlayerId < players.playerCount; otherPlayerId++) {
			player_t* target = &players.players[otherPlayerId];

			/* Check for collision between current player and other */
			CalculateBounce(actor, target);
		}

		/* Collisions with the checkpoint */
		guVector player2d = actor->hovercraft->transform.position;
		player2d.y = 0;
		checkpoint.y = 0;
		

		f32 distance = vecDistance(&player2d, &checkpoint);
		if (distance < 3.5f) {
			SCENE_moveCheckpoint();
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
	if (Raycast(mapTerrain, &raydir, &raypos, &dist, &normalhit)) {
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
	GX_SetViewport(camera->offsetLeft, camera->offsetTop, camera->width, camera->height, 0, 1);

	/* Render the player's hovercraft */
	SCENE_renderView(viewMtx);
}

inline playerArray_t GAME_getPlayersData() {
	return (playerArray_t) { players, playerCount };
}