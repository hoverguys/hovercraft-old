#include "audioutil.h"

#include <aesndlib.h>
#include <gcmodplay.h>

static MODPlay play;

void AU_init() {
	AESND_Init(NULL);
}

void AU_playMusic(const void* music) {
	MODPlay_Init(&play);
	MODPlay_SetMOD(&play, music);
	MODPlay_Start(&play);
}