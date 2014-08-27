/*! \file scene.h
 *  \brief InGame scene handling
 */

#ifndef _SCENE_H
#define _SCENE_H

#include <ogc/gu.h>

/*! Camera structure */
typedef struct {
	guVector position;       /*< Camera position    */
	f32      width,          /*< Viewport width     */
			 height,         /*< Viewport height    */
			 offsetTop,      /*< Viewport Y         */
			 offsetLeft;     /*< Viewport X         */
	Mtx44    perspectiveMtx; /*< Perspective Matrix */
} camera_t;

/*! Loads the game scene and models */
void SCENE_load();

/*! Updates the scene and renders all the player views inside it */
void SCENE_render();

/*! \brief Renders the scene
 *  \param viewMtx View matrix to use for rendering
 */
void SCENE_renderView(Mtx viewMtx);

/*! \brief Create all the players
 */
void SCENE_createPlayers();

/*! \brief Get checkpoint position
 *  \return Checkpoint position as a guVector
 */
guVector SCENE_getCheckpoint();

/*! \brief Move checkpoint to a random new position
 */
void SCENE_moveCheckpoint();

#endif