/* SDK Libraries */
#include <gccore.h>
#include <aesndlib.h>  /*  Audio       */
#include <gcmodplay.h> /* Mod playback */

/* Internal libraries */
#include "scene.h"
#include "game.h"
#include "input.h"

int main(int argc, char **argv) {
	VIDEO_Init();
	AESND_Init(NULL);
	SCENE_load();

	while (1) {
		SCENE_update();
	}

	return 0;
}