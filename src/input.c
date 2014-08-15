#include "input.h"
#include <ogc/video.h>
#include <math.h>

u32 _Connected;
inline f32 _CLAMP(const f32 value, const f32 minVal, const f32 maxVal);

void INPUT_update() {
#ifdef USE_WIIMOTE
	WPAD_ScanPads();
	//todo Wiimote detection
#else
	_Connected = PAD_ScanPads();
#endif
}

void INPUT_init() {
#ifdef USE_WIIMOTE
	WPAD_Init();
#else
	PAD_Init();
#endif
}

inline BOOL INPUT_isConnected(const u8 id) {
	const u8 padId = 1 << id;
	return (_Connected & padId) == padId;
}

f32 INPUT_AnalogX(const u8 id) {
	const f32 raw = PAD_StickX(id);
	if (fabs(raw) < INPUT_DEADZONE) return 0;
	return _CLAMP(raw, -INPUT_STICK_THRESHOLD, INPUT_STICK_THRESHOLD) / INPUT_STICK_THRESHOLD;
}

f32 INPUT_AnalogY(const u8 id) {
	const f32 raw = PAD_StickY(id);
	if (fabs(raw) < INPUT_DEADZONE) return 0;
	return _CLAMP(raw, -INPUT_STICK_THRESHOLD, INPUT_STICK_THRESHOLD) / INPUT_STICK_THRESHOLD;
}

f32 INPUT_CStickX(const u8 id) {
	const f32 raw = PAD_SubStickX(id);
	if (fabs(raw) < INPUT_DEADZONE) return 0;
	return _CLAMP(raw, -INPUT_STICK_THRESHOLD, INPUT_STICK_THRESHOLD) / INPUT_STICK_THRESHOLD;
}

f32 INPUT_CStickY(const u8 id) {
	const f32 raw = PAD_SubStickY(id);
	if (fabs(raw) < INPUT_DEADZONE) return 0;
	return _CLAMP(raw, -INPUT_STICK_THRESHOLD, INPUT_STICK_THRESHOLD) / INPUT_STICK_THRESHOLD;
}

f32 INPUT_TriggerL(const u8 id) {
	const f32 raw = PAD_TriggerL(id);
	if (fabs(raw) < INPUT_DEADZONE) return 0;
	return _CLAMP(raw, 0, INPUT_TRIGGER_THRESHOLD) / INPUT_TRIGGER_THRESHOLD;
}

f32 INPUT_TriggerR(const u8 id) {
	const f32 raw = PAD_TriggerR(id);
	if (fabs(raw) < INPUT_DEADZONE) return 0;
	return _CLAMP(raw, 0, INPUT_TRIGGER_THRESHOLD) / INPUT_TRIGGER_THRESHOLD;
}

BOOL INPUT_getButton(const u8 padId, const u16 buttonId) {
#ifdef USE_WIIMOTE
	return WPAD_ButtonsDown(padId) & buttonId ? TRUE : FALSE;
#else
	return PAD_ButtonsDown(padId) & buttonId ? TRUE : FALSE;
#endif
}

void INPUT_waitForControllers() {
	while (_Connected == 0) {
		INPUT_update();
		VIDEO_WaitVSync();
	}
	return;
}

inline f32 _CLAMP(const f32 value, const f32 minVal, const f32 maxVal) {
	return value < minVal ? minVal : value > maxVal ? maxVal : value;
}