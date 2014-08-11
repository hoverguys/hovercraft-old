#include "game.h"

#include <malloc.h>

#include "input.h"

object_t* mapTerrain;
object_t* mapPlane;

const int MAX_PLAYERS = 4;
player_t* players;

void GAME_init(object_t* terrain, object_t* plane) {
	mapTerrain = terrain;
	mapPlane = plane;

	players = calloc(MAX_PLAYERS, sizeof(player_t));
	for (u8 i = 0; i < MAX_PLAYERS; i++) {
		players[i].isPlaying = FALSE;
	}
}

void GAME_createPlayer(u8 playerId, model_t* hovercraftModel) {
	players[playerId].hovercraft = OBJECT_create(hovercraftModel);
	OBJECT_moveTo(players[playerId].hovercraft, 1, 0.6f, 1);
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
	guVector speedVec = players[playerId].speed;
	guVecScale(&players[playerId].hovercraft->transform.forward, &momem, accel);
	guVecScale(&speedVec, &decelVec, decel);
	guVecSub(&speedVec, &decelVec, &speedVec);
	guVecAdd(&speedVec, &momem, &speedVec);
	if (sqrtf(speedVec.x*speedVec.x + speedVec.y*speedVec.y + speedVec.z*speedVec.z) > maxspeed) {
		guVecNormalize(&speedVec);
		guVecScale(&speedVec, &speedVec, maxspeed);
	}
	OBJECT_move(players[playerId].hovercraft, speedVec.x, speedVec.y, speedVec.z);

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