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

// Audio
#include <aesndlib.h>
#include <gcmodplay.h>

static void *xfb = NULL;
static GXRModeObj *rmode = NULL;
static MODPlay play;

void *initialise();
void playMod();
void checkModel();

#define PAD1 1<<0
#define PAD2 1<<1
#define PAD3 1<<2
#define PAD4 1<<3

int main(int argc, char **argv) {

	xfb = initialise();

	playMod();

	checkModel();

	printf("\nHello World!\n");

	printf("\nChecking pads..\n");

	u32 connected = PAD_ScanPads();
	if ((connected & PAD1) == PAD1) printf("\nPlayer 1 connected\n");
	if ((connected & PAD2) == PAD2) printf("\nPlayer 2 connected\n");
	if ((connected & PAD3) == PAD3) printf("\nPlayer 3 connected\n");
	if ((connected & PAD4) == PAD4) printf("\nPlayer 4 connected\n");

	while (1) {

		VIDEO_WaitVSync();
		PAD_ScanPads();

		int buttonsDown = PAD_ButtonsDown(PAD1);

		if (buttonsDown & PAD_BUTTON_A) {
			printf("Button A pressed.\n");
		}

		if (buttonsDown & PAD_BUTTON_START) {
			exit(0);
		}
	}

	return 0;
}

void* initialise() {

	void *framebuffer;

	VIDEO_Init();
	PAD_Init();
	AESND_Init(NULL);

	rmode = VIDEO_GetPreferredMode(NULL);

	framebuffer = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
	console_init(framebuffer, 20, 20, rmode->fbWidth, rmode->xfbHeight, rmode->fbWidth*VI_DISPLAY_PIX_SZ);

	VIDEO_Configure(rmode);
	VIDEO_SetNextFramebuffer(framebuffer);
	VIDEO_SetBlack(FALSE);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	if (rmode->viTVMode&VI_NON_INTERLACE) VIDEO_WaitVSync();

	return framebuffer;
}

void playMod() {
	MODPlay_Init(&play);
	MODPlay_SetMOD(&play, menumusic_mod);
	MODPlay_SetFrequency(&play, 32000);
	MODPlay_Start(&play);
}

void checkModel() {
	binheader_t* header = (binheader_t*)hovercraft_bmb;

	u32 posOffset = sizeof(binheader_t);
	u32 nrmOffset = posOffset + (sizeof(f32)* header->vcount * 3);
	u32 texOffset = nrmOffset + (sizeof(f32)* header->vcount * 3);
	u32 indOffset = texOffset + (sizeof(f32)* header->vcount * 2);

	f32* positions = (f32*)(hovercraft_bmb + posOffset);
	f32* normals = (f32*)(hovercraft_bmb + nrmOffset);
	f32* texcoords = (f32*)(hovercraft_bmb + texOffset);
	u16* indices = (u16*)(hovercraft_bmb + indOffset);
}
