/* System and SDK libraries */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <ogcsys.h>
#include <gccore.h>
#include <aesndlib.h>  /*  Audio  */
#include <gcmodplay.h> /* Modplay */

/* Generated assets headers */
#include "menumusic_mod.h"
#include "hovercraft_bmb.h"
#include "terrain_bmb.h"

/* Textures */
#include "textures_tpl.h"
#include "textures.h"

#include "input.h"
#include "model.h"
#include "object.h"

/* Music variable */
static MODPlay play;

/* GX vars */
#define DEFAULT_FIFO_SIZE	(256*1024)
static void *xfb[2] = { NULL, NULL };
static u32 fbi = 0;
static GXRModeObj *rmode = NULL;
void *gpfifo = NULL;

/* Matrices */
Mtx viewMtx;
Mtx44 perspectiveMtx;

/* Model info */
model_t *modelHover, *modelTerrain;
object_t *objectHover, *objectTerrain;

/* Texture vars */
GXTexObj hoverTexObj, terrainTexObj;
TPLFile TPLfile;

void initialise();
void playMod();
void loadTextures();
void setupCamera();
void SetLight(Mtx view);

int main(int argc, char **argv) {
	initialise();
	setupCamera();
	playMod();
	loadTextures();

	modelHover = MODEL_setup(hovercraft_bmb);
	modelTerrain = MODEL_setup(terrain_bmb);

	MODEL_setTexture(modelHover, &hoverTexObj);
	MODEL_setTexture(modelTerrain, &terrainTexObj);

	guVector zero, one, ten;
	zero.x = 0; zero.y = -1; zero.z = -10;
	ten.x = 100; ten.y = 100; ten.z = 100;
	one.x = 1; one.y = 1; one.z = 1;
	guQuaternion zeroq;
	zeroq.x = 0; zeroq.y = 0; zeroq.z = 0; zeroq.w = 1;

	/* THIS BREAKS FOR POSITION != zero */
	objectTerrain = OBJECT_create(modelTerrain, zero, zeroq, ten);
	objectHover = OBJECT_create(modelHover, zero, zeroq, one);

	printf("\nChecking pads..\n");

	u32 connected = PAD_ScanPads();
	if ((connected & PAD1) == PAD1) printf("\nPlayer 1 connected\n");
	if ((connected & PAD2) == PAD2) printf("\nPlayer 2 connected\n");
	if ((connected & PAD3) == PAD3) printf("\nPlayer 3 connected\n");
	if ((connected & PAD4) == PAD4) printf("\nPlayer 4 connected\n");

	u32 firstFrame = 1;
	while (1) {
		GX_SetNumChans(1);

		if (firstFrame) {
			firstFrame = 0;
			VIDEO_SetBlack(FALSE);
		}

		/* Draw models */
		OBJECT_render(objectHover, viewMtx);
		OBJECT_render(objectTerrain, viewMtx);

		/* Finish up */
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

	/* Initialize systems */
	VIDEO_Init();
	PAD_Init();
	AESND_Init(NULL);

	/* Get render mode */
	rmode = VIDEO_GetPreferredMode(NULL);

	/* Allocate the fifo buffer */
	gpfifo = memalign(32, DEFAULT_FIFO_SIZE);
	memset(gpfifo, 0, DEFAULT_FIFO_SIZE);

	/* Allocate frame buffers */
	xfb[0] = SYS_AllocateFramebuffer(rmode);
	xfb[1] = SYS_AllocateFramebuffer(rmode);

	VIDEO_Configure(rmode);
	VIDEO_SetNextFramebuffer(xfb[fbi]);
	VIDEO_SetBlack(FALSE);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	if (rmode->viTVMode&VI_NON_INTERLACE) VIDEO_WaitVSync();

	//CON_InitEx(rmode, 20, 20, rmode->fbWidth / 2 - 20, rmode->xfbHeight - 40);

	/* Swap frames */
	fbi ^= 1;

	/* Init flipper */
	GX_Init(gpfifo, DEFAULT_FIFO_SIZE);

	/* Clear the background to black and clear the Z buf */
	GXColor background = { 0x00, 0x00, 0x00, 0xFF };
	GX_SetCopyClear(background, 0x00FFFFFF);

	/* Fullscreen viewport setup */
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
	/* Setup camera view and perspective */
	guVector cam = { 0.0F, 0.0F, 0.0F },
			 up = { 0.0F, 1.0F, 0.0F },
			 look = { 0.0F, 0.0F, -1.0F };

	guLookAt(viewMtx, &cam, &up, &look);
	f32 w = rmode->viWidth;
	f32 h = rmode->viHeight;
	guPerspective(perspectiveMtx, 60, (f32) w / h, 0.1F, 300.0F);
	GX_LoadProjectionMtx(perspectiveMtx, GX_PERSPECTIVE);
}

void playMod() {
	MODPlay_Init(&play);
	MODPlay_SetMOD(&play, menumusic_mod);
	MODPlay_Start(&play);
}

void setupTexture() {
	/* Setup 1 texture channel */
	GX_SetNumTexGens(1);
	GX_SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
	GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
}

void loadTextures() {
	/* Clear texture cache */
	GX_InvalidateTexAll();

	/* Open TPL file from memory (statically linked in) */
	TPL_OpenTPLFromMemory(&TPLfile, (void *) textures_tpl, textures_tpl_size);

	/* Get texture from TPL */
	TPL_GetTexture(&TPLfile, hovercraftTex, &hoverTexObj);
	TPL_GetTexture(&TPLfile, terrainTex, &terrainTexObj);
}
