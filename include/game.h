/*! \file game.h
 *  \brief Game logic handling
 */
#ifndef _GAME_H
#define _GAME_H

#include <ogc/gu.h>
#include "object.h"
#include "scene.h"

typedef struct {
	BOOL      isPlaying;
	object_t* hovercraft;
	guVector  speed;
	camera_t* camera;
} player_t;

void GAME_init(object_t* terrain, object_t* plane);
void GAME_createPlayer(u8 playerId, model_t* hovercraftModel);
void GAME_updatePlayer(u8 playerId);
void GAME_renderPlayer(u8 playerId, Mtx viewMtx);

#endif
