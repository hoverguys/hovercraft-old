#include "model.h"

#include <malloc.h>
#include <string.h>
#include <stdio.h>

model_t* MODEL_setup(const u8* model_bmb) {
	binheader_t* header = (binheader_t*)model_bmb;

	u32 posOffset = sizeof(binheader_t);
	u32 nrmOffset = posOffset + (sizeof(f32)* header->vcount * 3);
	u32 texOffset = nrmOffset + (sizeof(f32)* header->vcount * 3);
	u32 indOffset = texOffset + (sizeof(f32)* header->vcount * 2);

	f32* positions = (f32*)(model_bmb + posOffset);
	f32* normals = (f32*)(model_bmb + nrmOffset);
	f32* texcoords = (f32*)(model_bmb + texOffset);
	u16* indices = (u16*)(model_bmb + indOffset);

	//Calculate cost
	u32 indicesCount = header->fcount * 3;
	u32 indicesSize = indicesCount * 3 * sizeof(u16); // 3 indices per vertex index (p,n,t) that are u16 in size
	printf("indicesSize is %u\n", indicesSize);
	u32 callSize = 89; // Size of setup vars
	u32 dispSize = indicesSize + callSize + 63;
	printf("guessed size is %u\n", dispSize);
	dispSize = ((dispSize >> 5) + 1) << 5; //Round up to nearest 32 multiplication

	// Build displaylist
	// Allocate and clear
	u32 i;
	void* modelList = memalign(32, dispSize);
	memset(modelList, 0, dispSize);

	// Set buffer data
	DCInvalidateRange(modelList, dispSize);
	GX_BeginDispList(modelList, dispSize);

	//GX_InvVtxCache();
	GX_ClearVtxDesc();
	GX_SetVtxDesc(GX_VA_POS, GX_INDEX16);
	GX_SetVtxDesc(GX_VA_NRM, GX_INDEX16);
	GX_SetVtxDesc(GX_VA_TEX0, GX_INDEX16);

	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);

	GX_SetArray(GX_VA_POS, positions, 3 * sizeof(GX_F32));
	GX_SetArray(GX_VA_NRM, normals, 3 * sizeof(GX_F32));
	GX_SetArray(GX_VA_TEX0, texcoords, 2 * sizeof(GX_F32));

	// Fill
	GX_Begin(GX_TRIANGLES, GX_VTXFMT0, indicesCount);
	for (i = 0; i < indicesCount; i++) {
		GX_Position1x16(indices[i]);
		GX_Normal1x16(indices[i]);
		GX_TexCoord1x16(indices[i]);
	}
	GX_End();

	// Close
	u32 modelListSize = GX_EndDispList();
	if (modelListSize == 0) {
		printf("Error: Display list not big enough [%u]\n", dispSize);
		return NULL;
	}

	printf("modelListSize is %u\n", modelListSize);

	// Return model info
	model_t* model = malloc(sizeof(model_t));
	memset(model, 0, sizeof(model_t));
	model->modelList = modelList;
	model->modelListSize = modelListSize;
	return model;
}

void MODEL_destroy(model_t* model) {
	free(model->modelList);
	memset(model, 0, sizeof(model_t));
}

void MODEL_render(model_t* model) {
	if (model == NULL) return;
	if (model->textureObject != NULL) {
		GX_LoadTexObj(model->textureObject, GX_TEXMAP0);
	}
	GX_CallDispList(model->modelList, model->modelListSize);
}

void MODEL_setTexture(model_t* model, GXTexObj* textureObject) {
	if (model == NULL) return;
	model->textureObject = textureObject;
}
