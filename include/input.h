/*! \file input.h
*  \brief Input management and detection
*/

#ifndef _INPUT_H
#define _INPUT_H

#include <gctypes.h>

#ifdef USE_WIIMOTE
#include <wiiuse/wpad.h>
#else
#include <ogc/pad.h>
#endif

#define INPUT_DEADZONE 10
#define INPUT_STICK_THRESHOLD 70
#define INPUT_TRIGGER_THRESHOLD 250
#define INPUT_STICK_MULTIPLIER 0.0143 // 1/70
#define INPUT_TRIGGER_MULTIPLIER 0.004  // 1/250

#ifdef USE_WIIMOTE
#define INPUT_BTN_JUMP WPAD_BUTTON_B
#else
#define INPUT_BTN_JUMP PAD_BUTTON_X
#endif

/*! \brief Initialize input
 */
void INPUT_init();

/*! \brief Scan for new input and detect new gamepads
 */
void INPUT_update();

/*! \brief Check if a specific pad is connected
 *  \param id Gamepad slot
 *  \return TRUE if connected, FALSE otherwise
 */
inline BOOL INPUT_isConnected(const u8 id);

/*! \brief Get Analog stick's X axis value
 *  \param id Gamepad slot
 *  \return Current analog value, normalized from -1 to 1 
 */
f32 INPUT_AnalogX(const u8 id);

/*! \brief Get Analog stick's Y axis value
*  \param id Gamepad slot
*  \return Current analog value, normalized from -1 to 1
*/
f32 INPUT_AnalogY(const u8 id);

/*! \brief Get C-stick's X axis value
*  \param id Gamepad slot
*  \return Current analog value, normalized from -1 to 1
*/
f32 INPUT_CStickX(const u8 id);

/*! \brief Get C-stick's Y axis value
*  \param id Gamepad slot
*  \return Current analog value, normalized from -1 to 1
*/
f32 INPUT_CStickY(const u8 id);

/*! \brief Get L trigger analog value
*  \param id Gamepad slot
*  \return Current analog value, normalized from -1 to 1
*/
f32 INPUT_TriggerL(const u8 id);

/*! \brief Get R trigger analog value
 *  \param id Gamepad slot
 *  \return Current analog value, normalized from -1 to 1
 */
f32 INPUT_TriggerR(const u8 id);

/*! \brief Get button status 
 *  \param padId    Gamepad slot
 *  \param buttonId Button id
 *  \return TRUE if the button has been pressed, FALSE otherwise
 */
BOOL INPUT_getButton(const u8 padId, const u16 buttonId);

/*! \brief Block until at least one controller is found
 */
void INPUT_waitForControllers();

#endif