#ifndef _MODEL_H
#define _MODEL_H

#include <gctypes.h>
#include <gccore.h>

typedef struct {
	unsigned int	vcount;
	unsigned int	ncount;
	unsigned int	vtcount;
	unsigned int	fcount;
} binheader_t;

typedef struct {
	GXTexObj* textureObject;
	void *modelList; // Storage for the display lists
	u32 modelListSize;   // Real display list sizes
} model_t;

model_t* MODEL_setup(const u8* model_bmb);
void MODEL_destroy(model_t* model);
void MODEL_render(model_t* model);
void MODEL_setTexture(model_t* model, GXTexObj* textureObject);

#endif
