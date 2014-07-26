#ifndef _OBJECT_H
#define _OBJECT_H

#include <ogc/gu.h>
#include "model.h"

typedef struct {
	guVector position;     //! World position
	guQuaternion rotation; //! Rotation
	guVector scale;        //! Scale
	u8 dirty;              //! Dirty flags for matrix operations
} transform_t;

typedef struct {
	model_t* mesh;         //! Model
	transform_t transform; //! Transform data (Position, Rotation, etc)
} object_t;


#define DIRTY_POSITION 1<<0
#define DIRTY_ROTATION 1<<1
#define DIRTY_SCALE    1<<2

/*! \brief Create Object from mesh and basic transform data
 *  \param mesh Model to use
 *  \param position Initial position in the world
 *  \param rotation Initial rotation
 *  \param scale    Initial scale
 *  \return Pointer to Object structure
 */
object_t* OBJECT_create(model_t*     mesh,
						guVector     position,
						guQuaternion rotation,
						guVector     scale);

/*! \brief Render the object
 *  \param object Object to render
 */
void OBJECT_render(object_t* object);

/*! \brief Destory and object and free its allocated memory
 *  \param object Object to destroy
 *  \remarks This doesn't free the mesh, don't forget to MODEL_destroy(1)!
 */
void OBJECT_destroy(object_t* object);

#endif
