#include "game.h"

#include <malloc.h>
#include <math.h>

#include "raycast.h"
#include "input.h"

object_t* mapTerrain;
object_t* mapPlane;

const int MAX_PLAYERS = 4;
player_t* players;

void GAME_init(object_t* terrain, object_t* plane) {
	mapTerrain = terrain;
	mapPlane = plane;

	players = calloc(MAX_PLAYERS, sizeof(player_t));
	u8 i;
	for (i = 0; i < MAX_PLAYERS; i++) {
		players[i].isPlaying = FALSE;
	}
}

void GAME_createPlayer(u8 playerId, model_t* hovercraftModel) {
	/* Create player hovercraft object and position it */
	players[playerId].hovercraft = OBJECT_create(hovercraftModel);
	OBJECT_moveTo(players[playerId].hovercraft, 1, 0.6f, 1);

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
	/* Move hovercraft */
	f32 rot = INPUT_AnalogX(0) * .033f;
	OBJECT_rotate(players[playerId].hovercraft, 0, rot, 0);
	f32 accel = INPUT_TriggerR(0) * .02f;
	f32 decel = .02f + INPUT_TriggerL(0) * .033f;
	const f32 maxspeed = .3f;
	guVector momem, decelVec;
	guVector *speedVec = &players[playerId].speed;
	guVecScale(&players[playerId].hovercraft->transform.forward, &momem, accel);
	guVecScale(speedVec, &decelVec, decel);
	guVecSub(speedVec, &decelVec, speedVec);
	guVecAdd(speedVec, &momem, speedVec);
	if (sqrtf(speedVec->x*speedVec->x + speedVec->y*speedVec->y + speedVec->z*speedVec->z) > maxspeed) {
		guVecNormalize(speedVec);
		guVecScale(speedVec, speedVec, maxspeed);
	}
	OBJECT_move(players[playerId].hovercraft, speedVec->x, speedVec->y, speedVec->z);

	guVector raydir = { 0, -1, 0 };
	guVector raypos = { 0, 200, 0 };
	guVector rayhit;
	guVecAdd(&raypos, &players[playerId].hovercraft->transform.position, &raypos);
	f32 dist = 0;
	f32 minHeight = mapPlane->transform.position.y;

	if (Raycast(mapTerrain, &raydir, &raypos, &dist)) {
		guVecScale(&raydir, &rayhit, dist);
		guVecAdd(&rayhit, &raypos, &rayhit);
		f32 h = rayhit.y + .1f;
		if (h > .5f) {
			OBJECT_moveTo(players[playerId].hovercraft, rayhit.x, rayhit.y + .1f, rayhit.z);
		}
	} else {
		//printf("Out of bounds \n");
	}
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