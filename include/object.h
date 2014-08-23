/*! \file object.h
 *  \brief (Game)Object creation and handling
 */

#ifndef _OBJECT_H
#define _OBJECT_H

#include <ogc/gu.h>
#include "model.h"

/*! Transform data */
typedef struct {
	guVector     position; /*< World position                       */
	guQuaternion rotation; /*< Rotation                             */
	guVector     scale;    /*< Scale                                */
	BOOL         dirty;    /*< Dirty flag for matrix recalculation  */
	Mtx          matrix;   /*< Transform matrix   (AUTO-GENERATED)  */
	guVector     forward;  /*< Forward vector     (AUTO-GENERATED)  */
	guVector     up;       /*< Up vector          (AUTO-GENERATED)  */
	guVector     right;    /*< Right vector       (AUTO-GENERATED)  */
} transform_t;

/*! Object structure */
typedef struct {
	model_t*    mesh;      /*< Model                                    */
	transform_t transform; /*< Transform data (Position, Rotation, etc) */
} object_t;

/*! \brief Create Object from mesh with default transforms
*  \param mesh Model to use
*  \return Pointer to Object structure
*/
object_t* OBJECT_create(model_t* mesh);

/*! \brief Create Object from mesh and basic transform data
 *  \param mesh     Model to use
 *  \param position Initial position in the world
 *  \param rotation Initial rotation
 *  \param scale    Initial scale
 *  \return Pointer to Object structure
 */
object_t* OBJECT_createEx(model_t*     mesh,
						  const guVector     position,
						  const guQuaternion rotation,
						  const guVector     scale);

/*! \brief Force Matrix regeneration on object
 *  \param object Object to process
 */
void OBJECT_flush(object_t* object);

/*! \brief Render the object
 *  \param object  Object to render
 *  \param viewMtx Camera's view matrix
 */
void OBJECT_render(object_t* object, Mtx viewMtx);

/*! \brief Destroy and object and free its allocated memory
 *  \param object Object to destroy
 *  \remarks This doesn't free the mesh, don't forget to MODEL_destroy(1)!
 */
void OBJECT_destroy(object_t* object);

/*! \brief Move an object to a specified position
 *  \param object Object to move
 *  \param tX     X coordinate
 *  \param tY     Y coordinate
 *  \param tZ     Z coordinate
 */
void OBJECT_moveTo(object_t* object, const f32 tX, const f32 tY, const f32 tZ);

/*! \brief Move an object of a certain position (RELATIVE)
 *  \param object Object to move
 *  \param tX     X coordinate
 *  \param tY     Y coordinate
 *  \param tZ     Z coordinate
 */
void OBJECT_move(object_t* object, const f32 tX, const f32 tY, const f32 tZ);

/*! \brief Set a specific rotation an object
 *  \param object Object to rotate
 *  \param rX     X coordinate
 *  \param rY     Y coordinate
 *  \param rZ     Z coordinate
 */
void OBJECT_rotateTo(object_t* object, const f32 rX, const f32 rY, const f32 rZ);

/*! \brief Set a specific rotation an object
 *  \param object   Object to rotate
 *  \param rotation Rotation to set
 */
void OBJECT_rotateSet(object_t* object, guQuaternion *rotation);

/*! \brief Set rotation by axis angle
 *  \param object Object to rotate
 *  \param axis   Axis to rotate
 *  \param angle  Angle
 */
void OBJECT_rotateAxis(object_t* object, guVector* axis, const f32 angle);

/*! \brief Rotate an object of a certain rotation (RELATIVE)
 *  \param object Object to rotate
 *  \param rX     X coordinate
 *  \param rY     Y coordinate
 *  \param rZ     Z coordinate
 */
void OBJECT_rotate(object_t* object, const f32 rX, const f32 rY, const f32 rZ);

/*! \brief Scale an object to a specific size
*  \param object Object to scale
*  \param sX     X coordinate
*  \param sY     Y coordinate
*  \param sZ     Z coordinate
*/
void OBJECT_scaleTo(object_t* object, const f32 sX, const f32 sY, const f32 sZ);

/*! \brief Scale an object of a specific size (RELATIVE)
*  \param object Object to scale
*  \param sX     X coordinate
*  \param sY     Y coordinate
*  \param sZ     Z coordinate
*/
void OBJECT_scale(object_t* object, const f32 sX, const f32 sY, const f32 sZ);

#endif
