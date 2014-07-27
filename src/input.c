#include "input.h"

#include <ogc/pad.h>
#include <math.h>

u32 _Connected;
inline f32 _CLAMP(const f32 value, const f32 minVal, const f32 maxVal);

void INPUT_update() {
	_Connected = PAD_ScanPads();
}

void INPUT_init() {
	PAD_Init();
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

inline f32 _CLAMP(const f32 value, const f32 minVal, const f32 maxVal) {
	return value < minVal ? minVal : value > maxVal ? maxVal : value;
}