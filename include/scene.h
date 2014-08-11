#ifndef _SCENE_H
#define _SCENE_H

#include <ogc/gu.h>

typedef struct {
	guVector    position;
	f32         width, height, offsetTop, offsetLeft;
	Mtx         perspectiveMtx;
} camera_t;

void SCENE_load();
void SCENE_render();

#endif