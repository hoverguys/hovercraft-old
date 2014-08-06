/* System and SDK libraries */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <ogcsys.h>
#include <gccore.h>
#include <aesndlib.h>  /*  Audio  */
#include <gcmodplay.h> /* Modplay */
#include <math.h>

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
#include "raycast.h"
#include "mathutil.h"

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
object_t *objectHover, *objectTerrain, *objectPlane;

/* Texture vars */
GXTexObj hoverTexObj, terrainTexObj, waterTexObj;
TPLFile TPLfile;

/* Light */
static GXColor lightColor[] = {
	{ 0x4f, 0x4f, 0x4f, 0xFF }, // Light color
	{ 0x0f, 0x0f, 0x0f, 0xFF }, // Ambient color
	{ 0xff, 0xff, 0xff, 0xFF }  // Mat color
};

guVector oldcam;

void initialise();
void playMod();
void loadTextures();
void SetLight (Mtx view);
void followCamera(transform_t* target, float distance);

int main(int argc, char **argv) {
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

	objectHover = OBJECT_create(modelHover);
	OBJECT_moveTo(objectHover, 100, 26, 100);

	objectPlane = OBJECT_create(modelPlane);
	OBJECT_scaleTo(objectPlane, 1000, 1, 1000);
	OBJECT_moveTo(objectPlane, -500, .05f, -500);
	OBJECT_render(objectHover, viewMtx);

	oldcam.x = oldcam.y = oldcam.z = 0;
	BOOL firstFrame = TRUE;
	guVector speedVec;

	while (1) {
		INPUT_update();

		/* Move hovercraft */
		f32 rot = INPUT_AnalogX(0) * .033f;
		OBJECT_rotate(objectHover, 0, rot, 0);
		f32 accel = INPUT_TriggerR(0) * .02f;
		f32 decel = 0.02f + INPUT_TriggerL(0) * .033f;
		const f32 maxspeed = .3f;
		guVector momem, decelVec;
		guVecScale(&objectHover->transform.forward, &momem, accel);
		guVecScale(&speedVec, &decelVec, decel);
		guVecSub(&speedVec, &decelVec, &speedVec);
		guVecAdd(&speedVec, &momem, &speedVec);
		if (sqrtf(speedVec.x*speedVec.x + speedVec.y*speedVec.y + speedVec.z*speedVec.z) > maxspeed) {
			guVecNormalize(&speedVec);
			guVecScale(&speedVec, &speedVec, maxspeed);
		}
		OBJECT_move(objectHover, speedVec.x, speedVec.y, speedVec.z);

		guVector raydir = { 0, -1, 0 };
		guVector raypos = { 0, 10, 0};
		guVector rayhit;
		guVecAdd(&objectHover->transform.position, &raypos, &raypos);
		f32 dist = 0;

		if (Raycast(objectTerrain, &raydir, &raypos, &dist)) {
			guVecScale(&raydir, &rayhit, dist);
			guVecAdd(&rayhit, &raypos, &rayhit);
			//OBJECT_moveTo(objectHover, rayhit.x, rayhit.y-1, rayhit.z);
			printf("HIT dist %f\n", rayhit.y);
		}
		else {
			printf("NO HIT \n");
		}
		/* Render time */
		GX_SetNumChans(1);

		if (firstFrame) {
			firstFrame = FALSE;
			VIDEO_SetBlack(FALSE);
		}

		/* Follow hovercraft */
		followCamera(&objectHover->transform, 5.f);

		/* Enable Light */
		SetLight(viewMtx);

		/* Draw models */
		OBJECT_render(objectTerrain, viewMtx);
		OBJECT_render(objectHover, viewMtx);
		//OBJECT_render(objectPlane, viewMtx);

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
	GXColor background = { 0xA0, 0xE0, 0xF0, 0xFF };
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


	f32 w = rmode->viWidth;
	f32 h = rmode->viHeight;
	guPerspective(perspectiveMtx, 60, (f32) w / h, 0.1F, 300.0F);
	GX_LoadProjectionMtx(perspectiveMtx, GX_PERSPECTIVE);
}

void followCamera(transform_t* target, float distance) {
	/* Setup camera view and perspective */
	guVector targetPos = { target->position.x - target->forward.x * distance,
		target->position.y - target->forward.y * distance + distance * .5f,
		target->position.z - target->forward.z * distance };
	float t = 1.f / 10.f;
	guVector up = { 0, 1, 0 };
	guVector cam = { oldcam.x + t * (targetPos.x - oldcam.x),
					 oldcam.y + t * (targetPos.y - oldcam.y),
					 oldcam.z + t * (targetPos.z - oldcam.z) };

	guLookAt(viewMtx, &cam, &up, &target->position);
	f32 w = rmode->viWidth;
	f32 h = rmode->viHeight;
	guPerspective(perspectiveMtx, 60, (f32) w / h, 0.1F, 300.0F);
	GX_LoadProjectionMtx(perspectiveMtx, GX_PERSPECTIVE);
	oldcam = cam;
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
