#include "mathutil.h"
#include <math.h>

void EulerToQuaternion(guQuaternion* q, const f32 rX, const f32 rY, const f32 rZ) {
	guVector vec;
	vec.x = rX; vec.y = rY; vec.z = rZ;
	guVecScale(&vec, &vec, 0.5f);

	f32 x[2], y[2], z[2];

	sincosf(vec.x, &x[0], &x[1]);
	sincosf(vec.y, &y[0], &y[1]);
	sincosf(vec.z, &z[0], &z[1]);

	ps_eulerQuat(x, y, z, q);
}

void AxisAngleToQuaternion(guQuaternion* q, guVector rAxis, const f32 rAngle) {
	f32 s, c;
	sincosf(rAngle / 2.0f, &s, &c);
	guVector out;
	guVecScale(&rAxis, &out, s);
	q->x = out.x;
	q->y = out.y;
	q->z = out.z;
	q->w = c;
}

void QUAT_lookat(guVector* forward, guVector *up, guQuaternion* out) {
	Mtx tmpMatrix;
	guVector zero = { 0, 0, 0 };
	guLookAt(tmpMatrix, &zero, up, forward);
	c_guQuatMtx(out, tmpMatrix);
}