/* SDK Libraries */
#include <gccore.h>
#include <stdlib.h>

/* Internal libraries */
#include "scene.h"
#include "input.h"
#include "audioutil.h"

#include "menumusic_mod.h"

BOOL isRunning;
void OnResetCalled();

int main(int argc, char **argv) {

	/* Setup reset function */
	SYS_SetResetCallback(OnResetCalled);

	INPUT_init();
	SCENE_load();

	AU_init();
	AU_playMusic(menumusic_mod);

	isRunning = TRUE;
	while (isRunning) {
		INPUT_update();
		SCENE_render();
	}

	exit(0);
	return 0;
}

void OnResetCalled() {
	isRunning = FALSE;
}