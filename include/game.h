/*! \file game.h
 *  \brief Game logic handling
 */
#ifndef _GAME_H
#define _GAME_H

#include <ogc/gu.h>
#include "object.h"
#include "scene.h"
#include "input.h"

/*! Maximum number of players */
#define MAX_PLAYERS 4

/*! Player structure*/
typedef struct {
	BOOL         isPlaying;  /*< Is the player... playing?     */
	BOOL         isGrounded; /*< Is is on the ground?          */
	object_t*    hovercraft; /*< Hovercraft object             */
	guVector     velocity;   /*< Current velocity              */
	camera_t     camera;     /*< Player's camera               */
	controller_t controller; /*< Player's controller data      */
} player_t;

typedef struct {
	player_t* players;     /*< Player data       */
	u8        playerCount; /*< Number of players */
} playerArray_t;

/*! \brief Initialize In-game data
 *  \param terrain Game terrain
 *  \param plane   Water plane
 */
void GAME_init(object_t* terrain, object_t* plane);

/*! \brief Create a player
 *  \param controllerInfo  Controller to bind player to
 *  \param hovercraftModel Hovercraft model
 *  \param startPosition   Starting position
 */
void GAME_createPlayer(controller_t controllerInfo, model_t* hovercraftModel, guVector startPosition);

/*! \brief Remove a player
 *  \param player Player to remove
 */
void GAME_removePlayer(player_t* player);

/*! \brief Update player's state (physics/logic)
 *  \param player Player to update
 */
void GAME_updatePlayer(player_t* player);

/*! \brief Update global/world state (physics/logic)
*/
void GAME_updateWorld();

/*! \brief Render a player's view
 *  \param player Player to render
 */
void GAME_renderPlayerView(player_t* player);

/*! \brief Proper way to retrieve player data
 *  \param playerId Player Id
 *  \return Pointer to requested player_t struct
 */
playerArray_t GAME_getPlayersData();

#endif
