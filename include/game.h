/*! \file game.h
 *  \brief Game logic handling
 */
#ifndef _GAME_H
#define _GAME_H

#include <ogc/gu.h>
#include "object.h"
#include "scene.h"

typedef struct {
	BOOL		isPlaying;
	BOOL		isGrounded;
	object_t	*hovercraft;
	guVector	velocity;
	camera_t	*camera;
} player_t;

void GAME_init(object_t* terrain, object_t* plane);
void GAME_createPlayer(u8 playerId, model_t* hovercraftModel);
void GAME_updatePlayer(u8 playerId);
void GAME_renderPlayerView(u8 playerId);
player_t* GAME_getPlayerData(u8 playerId);

#endif
