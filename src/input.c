#include "input.h"
#include <ogc/video.h>
#include <math.h>

u32 _GCConnected, _Wiimotes;

inline f32 _CLAMP(const f32 value, const f32 minVal, const f32 maxVal);

void INPUT_update() {
#ifdef WII
	/* Read and process incoming wiimote data */
	WPAD_ScanPads();
	/* Probe each wiimote to see if it changed status */
	u8 i;
	_Wiimotes = 0;
	for (i = 0; i < 4; i++) {
		if (INPUT_isConnected(INPUT_CONTROLLER_WIIMOTE, i)) {
			_Wiimotes |= 1 << i;
		}
	}
#endif
	_GCConnected = PAD_ScanPads();
}

void INPUT_init() {
#ifdef WII
	WPAD_Init();
	WPAD_SetDataFormat(WPAD_CHAN_ALL, WPAD_FMT_BTNS_ACC);

#endif
	PAD_Init();
}


inline BOOL INPUT_isConnected(const Input_ControllerType type, const u8 id) {
	u8 err, padId;
	switch (type) {
	case INPUT_CONTROLLER_GAMECUBE:
		padId = 1 << id;
		return (_GCConnected & padId) == padId ? TRUE : FALSE;
#ifdef WII
	case INPUT_CONTROLLER_WIIMOTE:
		err = WPAD_Probe(id, NULL);
		return err == WPAD_ERR_NONE ? TRUE : FALSE;
#endif
	default:
		return FALSE;
	}
}

void INPUT_getExpansion(controller_t* controller) {
	switch (controller->type) {
#ifdef WII
	case INPUT_CONTROLLER_WIIMOTE:
		WPAD_Probe(controller->slot, &controller->expansion);
		return;
#endif
	default:
		return;
	}
}

f32 INPUT_steering(controller_t* controller) {
	f32 raw;
#ifdef WII
	orient_t orientation;
#endif
	switch (controller->type) {
	case INPUT_CONTROLLER_GAMECUBE:
		raw = PAD_StickX(controller->slot);
		if (fabs(raw) < INPUT_GC_DEADZONE) return 0;
		return _CLAMP(raw, -INPUT_GC_STICK_THRESHOLD, INPUT_GC_STICK_THRESHOLD) * INPUT_GC_STICK_MULTIPLIER;
#ifdef WII
	case INPUT_CONTROLLER_WIIMOTE:
		WPAD_Orientation(controller->slot, &orientation);
		return _CLAMP(-orientation.pitch / 20.f, -1, 1);
#endif
	default:
		return 0;
	}
}

f32 INPUT_acceleration(controller_t* controller) {
	f32 raw;
	switch (controller->type) {
	case INPUT_CONTROLLER_GAMECUBE:
		raw = PAD_TriggerR(controller->slot);
		if (fabs(raw) < INPUT_GC_DEADZONE) return 0;
		return _CLAMP(raw, 0, INPUT_GC_TRIGGER_THRESHOLD) * INPUT_GC_TRIGGER_MULTIPLIER;
#ifdef WII
	case INPUT_CONTROLLER_WIIMOTE:
		return WPAD_ButtonsHeld(controller->slot) & WPAD_BUTTON_A ? 1 : 0;
#endif
	default:
		return 0;
	}
}

BOOL INPUT_jump(controller_t* controller) {
	switch (controller->type) {
	case INPUT_CONTROLLER_GAMECUBE:
		return PAD_ButtonsDown(controller->slot) & INPUT_GC_BTN_JUMP ? TRUE : FALSE;
#ifdef WII
	case INPUT_CONTROLLER_WIIMOTE:
		return WPAD_ButtonsDown(controller->slot) & INPUT_WII_BTN_JUMP ? TRUE : FALSE;
#endif
	default:
		return 0;
	}
}

BOOL INPUT_checkControllers() {
#ifdef WII
	/* Wait for the WPAD subsystem to have initialized */
	s8 status = WPAD_GetStatus();
	while (status != WPAD_STATE_ENABLED) {
		return FALSE;
	}
#endif
	INPUT_update();
	return (_GCConnected == 0 && _Wiimotes == 0) ? FALSE : TRUE;
}

inline f32 _CLAMP(const f32 value, const f32 minVal, const f32 maxVal) {
	return value < minVal ? minVal : value > maxVal ? maxVal : value;
}