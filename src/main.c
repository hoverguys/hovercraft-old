#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <ogcsys.h>
#include <gccore.h>

// include generated header
// rename from modfile.mod to modfile_mod.h
#include "menumusic_mod.h"

// Audio
#include <aesndlib.h>
#include <gcmodplay.h>

static void *xfb = NULL;
static GXRModeObj *rmode = NULL;
static MODPlay play;

void *initialise();

int main(int argc, char **argv) {

	xfb = initialise();

	playMod();

	printf("\nHello World!\n");

	while (1) {

		VIDEO_WaitVSync();
		PAD_ScanPads();

		int buttonsDown = PAD_ButtonsDown(0);

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
	MODPlay_Start(&play);
}
