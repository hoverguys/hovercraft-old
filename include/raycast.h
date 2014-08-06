#ifndef _RAYCAST_H
#define _RAYCAST_H

#include "object.h"

u8 Raycast(object_t* object, guVector* raydir, guVector* rayorigin, f32* distanceOut);
u8 Raycast2(object_t* object, guVector* raydir, guVector* rayorigin, f32* distanceOut);

#endif