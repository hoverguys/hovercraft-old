/* System and SDK libraries */
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <gccore.h>
#include <ogcsys.h>
#include <aesndlib.h>  /*  Audio  */
#include <gcmodplay.h> /* Modplay */

/* Generated assets headers */
#include "menumusic_mod.h"
#include "hovercraft_bmb.h"
#include "plane_bmb.h"
#include "terrain_bmb.h"

/* Textures */
#include "textures_tpl.h"
#include "textures.h"

#include "input.h"
#include "model.h"
#include "object.h"
#include "mathutil.h"
#include "game.h"

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
model_t *modelHover, *modelTerrain, *modelPlane;
object_t *objectTerrain, *objectPlane;

/* Texture vars */
GXTexObj hoverTexObj, terrainTexObj, waterTexObj;
TPLFile TPLfile;

/* Light */
static GXColor lightColor[] = {
		{ 0x4f, 0x4f, 0x4f, 0xff }, // Light color
		{ 0x0f, 0x0f, 0x0f, 0xff }, // Ambient color
		{ 0xff, 0xff, 0xff, 0xff }  // Mat color
};

BOOL firstFrame = TRUE;
guVector speedVec;

void initialise();
void playMod();
void loadTextures();
void SetLight(Mtx view);
void followCamera(transform_t* target, float distance);

void SCENE_load() {
	initialise();
	playMod();
	loadTextures();

	modelHover = MODEL_setup(hovercraft_bmb);
	modelTerrain = MODEL_setup(terrain_bmb);
	modelPlane = MODEL_setup(plane_bmb);

	MODEL_setTexture(modelHover, &hoverTexObj);
	MODEL_setTexture(modelTerrain, &terrainTexObj);
	MODEL_setTexture(modelPlane, &waterTexObj);

	objectTerrain = OBJECT_create(modelTerrain);
	OBJECT_scaleTo(objectTerrain, 200, 200, 200);

	objectPlane = OBJECT_create(modelPlane);
	OBJECT_scaleTo(objectPlane, 1000, 1, 1000);
	OBJECT_moveTo(objectPlane, -500, .5f, -500);

	GAME_init(objectTerrain, objectPlane);
	GAME_createPlayer(0, modelHover);
}

void SCENE_render() {
	INPUT_update();

	GAME_updatePlayer(0);

	/* Render time */
	GX_SetNumChans(1);

	if (firstFrame) {
		firstFrame = FALSE;
		VIDEO_SetBlack(FALSE);
	}

	/* Enable Light */
	SetLight(viewMtx);

	/* Draw terrain */
	OBJECT_render(objectTerrain, viewMtx);
	OBJECT_render(objectPlane, viewMtx);

	/* Follow hovercraft */
	GAME_renderPlayer(0, viewMtx);

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

void initialise() {
	/* Initialize systems */
	VIDEO_Init();
	INPUT_init();
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

	CON_EnableGecko(1, FALSE);

	/* Swap frames */
	fbi ^= 1;

	/* Init flipper */
	GX_Init(gpfifo, DEFAULT_FIFO_SIZE);

	/* Clear the background to black and clear the Z buf */
	GXColor background = { 0xa0, 0xe0, 0xf0, 0xff };
	GX_SetCopyClear(background, 0x00ffffff);

	/* Fullscreen viewport setup */
	GX_SetViewport(0, 0, rmode->fbWidth, rmode->efbHeight, 0, 1);
	f32 yscale = GX_GetYScaleFactor(rmode->efbHeight, rmode->xfbHeight);
	u32 xfbHeight = GX_SetDispCopyYScale(yscale);
	GX_SetScissor(0, 0, rmode->fbWidth, rmode->efbHeight);
	GX_SetDispCopySrc(0, 0, rmode->fbWidth, rmode->efbHeight);
	GX_SetDispCopyDst(rmode->fbWidth, xfbHeight);
	GX_SetCopyFilter(rmode->aa, rmode->sample_pattern, GX_TRUE, rmode->vfilter);
	GX_SetFieldMode(rmode->field_rendering, ((rmode->viHeight == 2 * rmode->xfbHeight) ? GX_ENABLE : GX_DISABLE));

	GX_SetPixelFmt(rmode->aa ? GX_PF_RGB565_Z16 : GX_PF_RGB8_Z24, GX_ZC_LINEAR);

	GX_SetCullMode(GX_CULL_NONE);
	GX_CopyDisp(xfb[fbi], GX_TRUE);
	GX_SetDispCopyGamma(GX_GM_1_0);


	f32 w = rmode->viWidth;
	f32 h = rmode->viHeight;
	guPerspective(perspectiveMtx, 60.f, (f32) w / h, .1f, 300.f);
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
	TPL_GetTexture(&TPLfile, waterTex, &waterTexObj);
}

void SetLight(Mtx view) {
	guVector lpos;
	GXLightObj lobj;

	lpos.x = 0.0f;	lpos.y = -1.0f; lpos.z = -0.3f;
	guVecNormalize(&lpos);
	guVecMultiplySR(view, &lpos, &lpos);

	GX_InitSpecularDirv(&lobj, &lpos);
	GX_InitLightColor(&lobj, lightColor[0]);
	GX_LoadLightObj(&lobj, GX_LIGHT0);

	// set number of rasterized color channels
	GX_SetNumChans(1);
	GX_SetChanCtrl(GX_COLOR0A0, GX_ENABLE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT0, GX_DF_CLAMP, GX_AF_NONE);
	GX_SetChanAmbColor(GX_COLOR0A0, lightColor[1]);
	GX_SetChanMatColor(GX_COLOR0A0, lightColor[2]);
	GX_SetTevOp(GX_TEVSTAGE0, GX_BLEND);
}
