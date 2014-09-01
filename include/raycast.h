/*! \file raycast.h
 *  \brief Raycasting module (for sticking to terrains and collisions)
 */

#ifndef _RAYCAST_H
#define _RAYCAST_H

#include "object.h"

/*! \brief Create Object from mesh with default transforms
 *  \param[in]  object      GameObject to raycast against
 *  \param[in]  raydir      Ray Direction vector (euler angles)
 *  \param[in]  rayorigin   Ray Origin (World space)
 *  \param[out] distanceOut Ray length (distance to hitpoint)
 *  \param[out] normalOut   Normal of the surface hit (NULL if you don't need it)
 *  \return TRUE if the ray hit somewhere, FALSE otherwise
 */
BOOL Raycast(object_t* object, guVector* raydir, guVector* rayorigin, f32* distanceOut, guVector* normalOut);

#endif