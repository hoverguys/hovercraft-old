/*! \file game.h
 *  \brief Game logic handling
 */
#ifndef _GAME_H
#define _GAME_H

#include <ogc/gu.h>
#include "object.h"
#include "scene.h"

/*! Maximum number of players */
#define MAX_PLAYERS 4

/*! Player structure*/
typedef struct {
	BOOL		isPlaying;	/*< Is the player active/playing? */
	BOOL		isGrounded; /*< Is is on the ground?          */
	object_t*	hovercraft; /*< Hovercraft object             */
	guVector	velocity;	/*< Current velocity              */
	camera_t	camera;		/*< Player's camera               */
} player_t;

/*! \brief Initialize In-game data
 *  \param terrain Game terrain
 *  \param plane   Water plane
 */
void GAME_init(object_t* terrain, object_t* plane);

/*! \brief Create a player
 *  \param playerId        Player Id
 *  \param hovercraftModel Hovercraft model
 *  \param startPosition   Starting position
 */
void GAME_createPlayer(u8 playerId, model_t* hovercraftModel, guVector startPosition);

/*! \brief Create a player
 *  \param playerId Player Id
 */
void GAME_removePlayer(u8 playerId);

/*! \brief Update player state (physics/logic)
 *  \param playerId Player Id
 */
void GAME_updatePlayer(u8 playerId);

/*! \brief Render a player's view
 *  \param playerId Player Id
 */
void GAME_renderPlayerView(u8 playerId);

/*! \brief Proper way to retrieve player data
 *  \param playerId Player Id
 *  \return Pointer to requested player_t struct
 */
player_t* GAME_getPlayerData(u8 playerId);

#endif
