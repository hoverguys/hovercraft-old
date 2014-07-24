#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <ogcsys.h>
#include <gccore.h>

// include generated headers
// rename from modfile.mod to modfile_mod.h
#include "menumusic_mod.h"
#include "hovercraft_bmb.h"

// Models
#include "modeldata.h"

// Textures
#include "textures_tpl.h"
#include "textures.h"

#include "input.h"

// Audio
#include <aesndlib.h>
#include <gcmodplay.h>
static MODPlay play;

// GX
#define DEFAULT_FIFO_SIZE	(256*1024)
static void *xfb[2] = { NULL, NULL};
static u32 fbi = 0;
static GXRModeObj *rmode = NULL;
void *gpfifo = NULL;

// Matrices
Mtx viewMtx;
Mtx44 perspectiveMtx;

// Model info
void *modelList; // Storage for the display lists
u32 modelListSize;   // Real display list sizes

GXTexObj texObj;
TPLFile TPLfile;

void initialise();
void playMod();
u8 setupModel();
void loadTexture();
void setupCamera();
void SetLight(Mtx view);


int main(int argc, char **argv) {

	initialise();

	setupCamera();

	playMod();
	loadTexture();

	if (setupModel() != TRUE) {
		printf("Error generating model..\n");
		exit(0);
	}
	

	printf("\nChecking pads..\n");

	u32 connected = PAD_ScanPads();
	if ((connected & PAD1) == PAD1) printf("\nPlayer 1 connected\n");
	if ((connected & PAD2) == PAD2) printf("\nPlayer 2 connected\n");
	if ((connected & PAD3) == PAD3) printf("\nPlayer 3 connected\n");
	if ((connected & PAD4) == PAD4) printf("\nPlayer 4 connected\n");

	u32 firstFrame = 1;
	Mtx modelMtx, modelviewMtx;
	while (1) {
		guMtxIdentity(modelMtx);
		guMtxTransApply(modelMtx, modelMtx, 0, 0, -10);

		guMtxConcat(modelMtx, viewMtx, modelviewMtx);
		GX_LoadPosMtxImm(modelviewMtx, GX_PNMTX0);

		GX_SetNumChans(1);

		// Input here
		PAD_ScanPads();

		int buttonsDown = PAD_ButtonsDown(PAD1);

		if (buttonsDown & PAD_BUTTON_A) {
			printf("Button A pressed.\n");
		}

		if (buttonsDown & PAD_BUTTON_START) {
			
		}

		if (firstFrame) {
			firstFrame = 0;
			VIDEO_SetBlack(FALSE);
		}

		// Draw here
		GX_CallDispList(modelList, modelListSize);

		//Finish up
		GX_SetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
		GX_SetColorUpdate(GX_TRUE);
		GX_CopyDisp(xfb[fbi], GX_TRUE);

		GX_DrawDone();

		VIDEO_SetNextFramebuffer(xfb[fbi]);
		VIDEO_Flush();
		VIDEO_WaitVSync();
		fbi ^= 1;
	}

	return 0;
}

void initialise() {

	// Initialize systems
	VIDEO_Init();
	PAD_Init();
	AESND_Init(NULL);

	// Get render mode
	rmode = VIDEO_GetPreferredMode(NULL);

	// allocate the fifo buffer
	gpfifo = memalign(32, DEFAULT_FIFO_SIZE);
	memset(gpfifo, 0, DEFAULT_FIFO_SIZE);

	// Allocate frame buffers
	xfb[0] = SYS_AllocateFramebuffer(rmode);
	xfb[1] = SYS_AllocateFramebuffer(rmode);

	//console_init(framebuffer, 20, 20, rmode->fbWidth, rmode->xfbHeight, rmode->fbWidth*VI_DISPLAY_PIX_SZ);

	VIDEO_Configure(rmode);
	VIDEO_SetNextFramebuffer(xfb[fbi]);
	VIDEO_SetBlack(FALSE);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	if (rmode->viTVMode&VI_NON_INTERLACE) VIDEO_WaitVSync();

	//CON_InitEx(rmode, 20, 20, rmode->fbWidth / 2 - 20, rmode->xfbHeight - 40);

	// Swap frames
	fbi ^= 1;

	// init the flipper
	GX_Init(gpfifo, DEFAULT_FIFO_SIZE);

	// clears the bg to color and clears the z buffer
	GXColor background = { 0x00, 0x00, 0x00, 0xFF };
	GX_SetCopyClear(background, 0x00FFFFFF);
	
	// fullscreen viewport setup
	// Gx Setup
	GX_SetViewport(0, 0, rmode->fbWidth, rmode->efbHeight, 0, 1);
	f32 yscale = GX_GetYScaleFactor(rmode->efbHeight, rmode->xfbHeight);
	u32 xfbHeight = GX_SetDispCopyYScale(yscale);
	GX_SetScissor(0, 0, rmode->fbWidth, rmode->efbHeight);
	GX_SetDispCopySrc(0, 0, rmode->fbWidth, rmode->efbHeight);
	GX_SetDispCopyDst(rmode->fbWidth, xfbHeight);
	GX_SetCopyFilter(rmode->aa, rmode->sample_pattern, GX_TRUE, rmode->vfilter);
	GX_SetFieldMode(rmode->field_rendering, ((rmode->viHeight == 2 * rmode->xfbHeight) ? GX_ENABLE : GX_DISABLE));

	if (rmode->aa)
		GX_SetPixelFmt(GX_PF_RGB565_Z16, GX_ZC_LINEAR);
	else
		GX_SetPixelFmt(GX_PF_RGB8_Z24, GX_ZC_LINEAR);

	GX_SetCullMode(GX_CULL_NONE);
	GX_CopyDisp(xfb[fbi], GX_TRUE);
	GX_SetDispCopyGamma(GX_GM_1_0);
}

void setupCamera() {
	// Setup camera view and perspective
	guVector
		cam = { 0.0F, 0.0F, 0.0F },
		up = { 0.0F, 1.0F, 0.0F },
		look = { 0.0F, 0.0F, -1.0F };

	guLookAt(viewMtx, &cam, &up, &look);
	f32 w = rmode->viWidth;
	f32 h = rmode->viHeight;
	guPerspective(perspectiveMtx, 60, (f32)w / h, 0.1F, 300.0F);
	GX_LoadProjectionMtx(perspectiveMtx, GX_PERSPECTIVE);
}

void playMod() {
	MODPlay_Init(&play);
	MODPlay_SetMOD(&play, menumusic_mod);
	// Fixed in Dolphin PR666 // MODPlay_SetFrequency(&play, 32000);
	MODPlay_Start(&play);
}

u8 setupModel() {
	binheader_t* header = (binheader_t*)hovercraft_bmb;

	u32 posOffset = sizeof(binheader_t);
	u32 nrmOffset = posOffset + (sizeof(f32)* header->vcount * 3);
	u32 texOffset = nrmOffset + (sizeof(f32)* header->vcount * 3);
	u32 indOffset = texOffset + (sizeof(f32)* header->vcount * 2);

	f32* positions = (f32*)(hovercraft_bmb + posOffset);
	f32* normals = (f32*)(hovercraft_bmb + nrmOffset);
	f32* texcoords = (f32*)(hovercraft_bmb + texOffset);
	u16* indices = (u16*)(hovercraft_bmb + indOffset);

	//Calculate cost
	/*
		Setup:
		 ?
		Indices:
		header->fcount * 3 * sizeof(u16) * 3
	
	*/

	u32 indicesCount = header->fcount * 3;
	u32 dispSize = 5312;

	// Build displaylist
	// Allocate and clear
	u32 i;
	modelList = memalign(32, dispSize);
	memset(modelList, 0, dispSize);

	// Set buffer data
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

	//Texture?

	// Fill
	DCInvalidateRange(modelList, dispSize);
	GX_BeginDispList(modelList, dispSize);
	GX_Begin(GX_TRIANGLES, GX_VTXFMT0, indicesCount);
	for (i = 0; i < indicesCount; i++) {
		GX_Position1x16(indices[i]);
		GX_Normal1x16(indices[i]);
		GX_TexCoord1x16(indices[i]);
	}
	GX_End();

	// Close
	modelListSize = GX_EndDispList();
	if (modelListSize == 0) {
		return FALSE;
	}

	printf("modelListSize is %u\n", modelListSize);
	return TRUE;
}

void loadTexture() {
	// This has something to do on how the textures are generated (format, probably?)
	GX_SetNumTexGens(1);
	GX_SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);

	// Free Texture cache
	GX_InvalidateTexAll();

	// Open TPL file from memory (statically linked in)
	TPL_OpenTPLFromMemory(&TPLfile, (void *) textures_tpl, textures_tpl_size);

	// Get my fabulous texture out
	TPL_GetTexture(&TPLfile, hovercraftTex, &texObj);

	// Load it into the first Texture map
	GX_LoadTexObj(&texObj, GX_TEXMAP0);

	GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
}
