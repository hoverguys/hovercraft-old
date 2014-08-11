#include "scene.h"

int main(int argc, char **argv) {
	SCENE_load();

	while (1) {
		SCENE_render();
	}

	return 0;
}