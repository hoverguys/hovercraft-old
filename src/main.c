/* SDK Libraries */
#include <gccore.h>
#include <stdlib.h>
#include <time.h>

/* Internal libraries */
#include "game.h"
#include "input.h"
#include "audioutil.h"
#include "mathutil.h"

BOOL isRunning;
void OnResetCalled();

int main() {

	fioraSeed(time(NULL));

	/* Setup reset function */
	SYS_SetResetCallback(OnResetCalled);

	INPUT_init();
	GAME_init();

	AU_init();

	isRunning = TRUE;
	while (isRunning) {
		INPUT_update();
		GAME_render();
	}

	return 0;
}

void OnResetCalled() {
	isRunning = FALSE;
}