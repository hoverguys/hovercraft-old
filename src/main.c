/* SDK Libraries */
#include <gccore.h>
#include <aesndlib.h>  /*  Audio       */
#include <gcmodplay.h> /* Mod playback */

/* Internal libraries */
#include "scene.h"
#include "game.h"
#include "input.h"

int main(int argc, char **argv) {
	//AESND_Init(NULL);
	INPUT_init();
	SCENE_load();

	while (1) {
		INPUT_update();
		SCENE_render();
	}

	return 0;
}