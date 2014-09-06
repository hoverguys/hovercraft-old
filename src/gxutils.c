#include "gxutils.h"

/* SDK libs */
#include <string.h>
#include <malloc.h>
#include <gccore.h>

/* Internal libs */
#include "sprite.h"

/* Texture definition */
#include "textures_tpl.h"

const TexSize TexSize256 = 1.f / 256.f;

/* GX vars */
#define DEFAULT_FIFO_SIZE	(256*1024)
static void *xfb[2] = { NULL, NULL };
static u32 fbi = 0;
static GXRModeObj *rmode = NULL;
BOOL first_frame = FALSE;
void *gpfifo = NULL;
f32 aspectRatio;
Mtx44 orthographicMatrix;

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
	if (rmode->viTVMode & VI_NON_INTERLACE) VIDEO_WaitVSync();

	/* Enable USBGecko debugging */
	//CON_EnableGecko(1, FALSE);
	//CON_InitEx(rmode, 0, 0, rmode->viWidth, rmode->viHeight);

	/* Set aspect ratio */
	aspectRatio = 4.f / 3.f;
#ifdef WII
	/* If 16:9 we need some hacks */
	if (CONF_GetAspectRatio()) {
		rmode->viWidth = 678;
		rmode->viXOrigin = (VI_MAX_WIDTH_PAL - 678) / 2;
		aspectRatio = 16.f / 9.f;
	}
#endif


	/* Swap frames */
	fbi ^= 1;

	/* Init flipper */
	GX_Init(gpfifo, DEFAULT_FIFO_SIZE);

	/* Clear the background to black and clear the Z buf */
	GXColor background = { 0xa0, 0xe0, 0xf0, 0xff };
	GX_SetCopyClear(background, 0x00ffffff);

	f32 yscale = GX_GetYScaleFactor(rmode->efbHeight, rmode->xfbHeight);
	u32 xfbHeight = GX_SetDispCopyYScale(yscale);
	GX_SetDispCopySrc(0, 0, rmode->fbWidth, rmode->efbHeight);
	GX_SetDispCopyDst(rmode->fbWidth, xfbHeight);
	GX_SetCopyFilter(rmode->aa, rmode->sample_pattern, GX_TRUE, rmode->vfilter);
	GX_SetFieldMode(rmode->field_rendering, ((rmode->viHeight == 2 * rmode->xfbHeight) ? GX_ENABLE : GX_DISABLE));

	GX_SetPixelFmt(rmode->aa ? GX_PF_RGB565_Z16 : GX_PF_RGB8_Z24, GX_ZC_LINEAR);

	GX_SetCullMode(GX_CULL_FRONT);
	GX_CopyDisp(xfb[fbi], GX_TRUE);
	GX_SetDispCopyGamma(GX_GM_1_0);

	/* Clear texture cache */
	GX_InvalidateTexAll();

	/* Open TPL file from memory (statically linked in) */
	TPL_OpenTPLFromMemory(&TPLfile, (void *) textures_tpl, textures_tpl_size);

	GX_SetNumTexGens(1);
	GX_SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
	GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);

	first_frame = TRUE;

	GXU_SetViewport(0, 0, rmode->viWidth, rmode->viHeight, 0, 1);
}

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
	if (first_frame) {
		first_frame = 0;
		VIDEO_SetBlack(FALSE);
	}

	VIDEO_Flush();
	VIDEO_WaitVSync();
	fbi ^= 1;
}

void GXU_setLight(Mtx view, GXColor lightColor[]) {
	guVector lpos = { 0, -1, -0.3f };
	GXLightObj lobj;

	guVecNormalize(&lpos);
	guVecMultiplySR(view, &lpos, &lpos);

	GX_InitSpecularDirv(&lobj, &lpos);
	GX_InitLightColor(&lobj, lightColor[0]);
	GX_InitLightShininess(&lobj, 12.0f);
	GX_LoadLightObj(&lobj, GX_LIGHT0);

	/* Set number of rasterized color channels */
	GX_SetNumChans(1);
	GX_SetChanCtrl(GX_COLOR0A0, GX_ENABLE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT0, GX_DF_CLAMP, GX_AF_SPEC);
	GX_SetChanAmbColor(GX_COLOR0A0, lightColor[1]);
	GX_SetChanMatColor(GX_COLOR0A0, lightColor[2]);
	GX_SetTevOp(GX_TEVSTAGE0, GX_MODULATE);
}

GXRModeObj* GXU_getMode() {
	return rmode;
}

f32 GXU_getAspectRatio() {
	return aspectRatio;
}

void GXU_setupCamera(camera_t* camera, u8 splitCount, u8 splitPlayer) {
	camera->width = splitCount > 2 ? rmode->viWidth >> 1 : rmode->viWidth;
	camera->height = splitCount > 1 ? rmode->efbHeight >> 1 : rmode->efbHeight;
	camera->offsetLeft = splitCount > 2 && splitPlayer % 2 == 0 ? rmode->viWidth >> 1 : 0;
	camera->offsetTop = splitPlayer > (splitCount > 2 ? 2 : 1) ? rmode->efbHeight >> 1 : 0;

	guPerspective(camera->perspectiveMtx, 60, aspectRatio * (splitCount == 2 ? 2.f : 1.f), 0.1f, 300.0f);
}

void GXU_2DMode() {
	GX_LoadProjectionMtx(orthographicMatrix, GX_ORTHOGRAPHIC);
}

void GXU_SetViewport(f32 xOrig, f32 yOrig, f32 wd, f32 ht, f32 nearZ, f32 farZ) {
	GX_SetScissor(xOrig, yOrig, wd, ht);
	GX_SetViewport(xOrig, yOrig, wd, ht, nearZ, farZ);

	guOrtho(orthographicMatrix, 0, ht, 0,wd, 0, rmode->viWidth);
}