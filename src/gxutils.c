#include "gxutils.h"

/* SDK libs */
#include <string.h>
#include <malloc.h>
#include <gccore.h>

/* Texture definition */
#include "textures_tpl.h"

/* GX vars */
#define DEFAULT_FIFO_SIZE	(256*1024)
static void *xfb[2] = { NULL, NULL };
static u32 fbi = 0;
static GXRModeObj *rmode = NULL;
void *gpfifo = NULL;

/* Texture file */
TPLFile TPLfile;

void GXU_init() {
	VIDEO_Init();

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

	/* Enable USBGecko debugging */
	CON_EnableGecko(1, FALSE);

	/* Swap frames */
	fbi ^= 1;

	/* Init flipper */
	GX_Init(gpfifo, DEFAULT_FIFO_SIZE);

	/* Clear the background to black and clear the Z buf */
	GXColor background = { 0xa0, 0xe0, 0xf0, 0xff };
	GX_SetCopyClear(background, 0x00ffffff);

	/* Fullscreen viewport setup (replace with per-player viewport */
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

	/* Clear texture cache */
	GX_InvalidateTexAll();

	/* Open TPL file from memory (statically linked in) */
	TPL_OpenTPLFromMemory(&TPLfile, (void *) textures_tpl, textures_tpl_size);
}

/* Why do we have this? Is this even used?! */
/*void setupTexture() {
	GX_SetNumTexGens(1);
	GX_SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
	GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
}*/

void GXU_loadTexture(s32 texId, GXTexObj* texObj) {
	TPL_GetTexture(&TPLfile, texId, texObj);
}

void GXU_done() {
	/* Finish up rendering */
	GX_SetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
	GX_SetColorUpdate(GX_TRUE);
	GX_CopyDisp(xfb[fbi], GX_TRUE);

	GX_DrawDone();

	/* Flush and swap buffers */
	VIDEO_SetNextFramebuffer(xfb[fbi]);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	fbi ^= 1;
}

void GXU_setLight(Mtx view, GXColor lightColor[]) {
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

GXRModeObj* GXU_getMode() {
	return rmode;
}

void GXU_setupCamera(camera_t* camera, u8 playerId) {
	camera->width = rmode->viWidth;
	camera->height = rmode->viHeight;
	camera->offsetTop = camera->offsetLeft = 0;
	guPerspective(camera->perspectiveMtx, 60, (f32) camera->width / camera->height, 0.1f, 300.0f);
	players[playerId].camera = camera;
}