/*! \file input.h
*  \brief Input management and detection
*/

#ifndef _INPUT_H
#define _INPUT_H

#include <gctypes.h>

#ifdef WII
#include <wiiuse/wpad.h>
#endif

#include <ogc/pad.h>

#define INPUT_GC_DEADZONE 10
#define INPUT_GC_STICK_THRESHOLD 70
#define INPUT_GC_TRIGGER_THRESHOLD 250
#define INPUT_GC_STICK_MULTIPLIER 0.0143 /* 1/70 */
#define INPUT_GC_TRIGGER_MULTIPLIER 0.004  /* 1/250 */

#ifdef WII
#define INPUT_WII_BTN_JUMP WPAD_BUTTON_2
#endif
#define INPUT_GC_BTN_JUMP PAD_BUTTON_X


typedef enum {
	INPUT_CONTROLLER_GAMECUBE = 0,  /*< Gamecube controller */
	INPUT_CONTROLLER_WIIMOTE = 1    /*< Wiimote controller  */
} Input_ControllerType;

/*! Controller structure */
typedef struct {
	Input_ControllerType type;      /*< Type of controller (Gamecube pad / Wiimote) */
	u8                   slot;      /*< Slot/Channel id                             */
	u32                  expansion; /*< Expansion, if any (Nunchuck etc)            */
} controller_t;

/*! \brief Initialize input
 */
void INPUT_init();

/*! \brief Scan for new input and detect new gamepads
 */
void INPUT_update();

/*! \brief Check if a specific pad is connected
 *  \param type Controller type
 *  \param id Gamepad slot / Wiimote channel
 *  \return TRUE if connected, FALSE otherwise
 */
inline BOOL INPUT_isConnected(const Input_ControllerType type, const u8 id);

/*! \brief Checks for a gamepad expansion
 *  \param controller Controller to check expansion of
 */
void INPUT_getExpansion(controller_t* controller);

f32 INPUT_steering(controller_t* controller);
f32 INPUT_acceleration(controller_t* controller);
f32 INPUT_brakes(controller_t* controller);
BOOL INPUT_jump(controller_t* controller);


/*! \brief Block until at least one controller is found
 */
void INPUT_waitForControllers();

#endif