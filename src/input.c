#include "input.h"
#include <ogc/video.h>
#include <math.h>
#include <stdlib.h>

u32 _GCConnected, _Wiimotes;
inline f32 _CLAMP(const f32 value, const f32 minVal, const f32 maxVal);
void OnResetCalled();

void INPUT_update() {
#ifdef WII
	WPAD_ScanPads();
	/* Probe each wiimote to see if it changed status*/
	u8 i;
	for (i = 0; i < 4; i++) {
		_Wiimotes = 0;
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
#endif
	PAD_Init();

	/* Setup reset function */
	SYS_SetResetCallback(OnResetCalled);
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
	u8 exp;
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

f32 INPUT_AnalogX(const u8 id) {
	const f32 raw = PAD_StickX(id);
	if (fabs(raw) < INPUT_DEADZONE) return 0;
	return _CLAMP(raw, -INPUT_STICK_THRESHOLD, INPUT_STICK_THRESHOLD) * INPUT_STICK_MULTIPLIER;
}

f32 INPUT_AnalogY(const u8 id) {
	const f32 raw = PAD_StickY(id);
	if (fabs(raw) < INPUT_DEADZONE) return 0;
	return _CLAMP(raw, -INPUT_STICK_THRESHOLD, INPUT_STICK_THRESHOLD) * INPUT_STICK_MULTIPLIER;
}

f32 INPUT_CStickX(const u8 id) {
	const f32 raw = PAD_SubStickX(id);
	if (fabs(raw) < INPUT_DEADZONE) return 0;
	return _CLAMP(raw, -INPUT_STICK_THRESHOLD, INPUT_STICK_THRESHOLD) * INPUT_STICK_MULTIPLIER;
}

f32 INPUT_CStickY(const u8 id) {
	const f32 raw = PAD_SubStickY(id);
	if (fabs(raw) < INPUT_DEADZONE) return 0;
	return _CLAMP(raw, -INPUT_STICK_THRESHOLD, INPUT_STICK_THRESHOLD) * INPUT_STICK_MULTIPLIER;
}

f32 INPUT_TriggerL(const u8 id) {
	const f32 raw = PAD_TriggerL(id);
	if (fabs(raw) < INPUT_DEADZONE) return 0;
	return _CLAMP(raw, 0, INPUT_TRIGGER_THRESHOLD) * INPUT_TRIGGER_MULTIPLIER;
}

f32 INPUT_TriggerR(const u8 id) {
	const f32 raw = PAD_TriggerR(id);
	if (fabs(raw) < INPUT_DEADZONE) return 0;
	return _CLAMP(raw, 0, INPUT_TRIGGER_THRESHOLD) * INPUT_TRIGGER_MULTIPLIER;
}

BOOL INPUT_getButton(const u8 padId, const u16 buttonId) {
#ifdef USE_WIIMOTE
	return WPAD_ButtonsDown(padId) & buttonId ? TRUE : FALSE;
#else
	return PAD_ButtonsDown(padId) & buttonId ? TRUE : FALSE;
#endif
}

void INPUT_waitForControllers() {
#ifdef WII
	/* Wait for the WPAD subsystem to have initialized */
	s8 status = WPAD_GetStatus();
	while (status != WPAD_STATE_ENABLED) {
		status = WPAD_GetStatus();
		VIDEO_WaitVSync();
	}
#endif

	/* Wait for at least one controller to show up*/
	while (_GCConnected == 0 && _Wiimotes == 0) {
		INPUT_update();
		VIDEO_WaitVSync();
	}
	return;
}

inline f32 _CLAMP(const f32 value, const f32 minVal, const f32 maxVal) {
	return value < minVal ? minVal : value > maxVal ? maxVal : value;
}

void OnResetCalled() {
	exit(0);
}