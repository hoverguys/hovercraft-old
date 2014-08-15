/* SDK Libraries */
#include <gccore.h>

/* Internal libraries */
#include "scene.h"
#include "input.h"
#include "audioutil.h"

#include "menumusic_mod.h"

int main(int argc, char **argv) {

	INPUT_init();
	SCENE_load();

	AU_init();
	AU_playMusic(menumusic_mod);

	while (1) {
		if (SYS_ResetButtonDown()) return 0;
		INPUT_update();
		SCENE_render();
	}

	return 0;
}