/*! \file gxutils.h
 *  \brief Utility functions and wrappers for GX
 */

#ifndef _GXUTILS_H 
#define _GXUTILS_H

#include <ogcsys.h>
#include "scene.h"

void GXU_init();
void GXU_loadTexture(s32 texId, GXTexObj* texObj);
void GXU_setLight(Mtx view, GXColor lightColor[]);
GXRModeObj* GXU_getMode();
void GXU_done();
void GXU_setupCamera(camera_t* camera, u8 playerId, u8 splitType);

#endif