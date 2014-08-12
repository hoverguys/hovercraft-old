/*! \file gxutils.h
 *  \brief Utility functions and wrappers for GX
 */

#ifndef _GXUTILS_H 
#define _GXUTILS_H

#include <ogcsys.h>

void GXU_init();
void GXU_loadTexture(s32 texId, GXTexObj* texObj);
void GXU_setLight(Mtx view, GXColor lightColor[]);
void GXU_done();

#endif