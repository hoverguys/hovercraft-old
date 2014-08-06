#ifndef _RAYCAST_H
#define _RAYCAST_H

#include "object.h"

BOOL Raycast(object_t* object, guVector* raydir, guVector* rayorigin, f32* distanceOut);

#endif