#include "mathutil.h"
#include <math.h>

void EulerToQuaternion(guQuaternion* q, const f32 rX, const f32 rY, const f32 rZ) {
	guVector vec;
	vec.x = rX; vec.y = rY; vec.z = rZ;
	ps_guVecScale(&vec, &vec, 0.5f);

	f32 x[2], y[2], z[2];

	sincosf(vec.x, &x[0], &x[1]);
	sincosf(vec.y, &y[0], &y[1]);
	sincosf(vec.z, &z[0], &z[1]);

	f32 xy[2] = { x[0], x[1] };
	f32 yz[2] = { y[0], y[1] };
	f32 xz[2] = { z[0], z[1] };
	ps_float3Mul(xy, yz, xz);

	q->w = xy[1] * z[1] - xy[0] * z[0];
	q->x = xy[1] * z[0] + xy[0] * z[1];
	q->z = yz[1] * x[0] - yz[0] * x[1];
	q->y = xz[1] * y[0] + xz[0] * y[1];
}

void AxisAngleToQuaternion(guQuaternion* q, guVector rAxis, const f32 rAngle) {
	f32 s, c;
	sincosf(rAngle / 2.0f, &s, &c);
	guVector out;
	ps_guVecScale(&rAxis, &out, s);
	q->x = out.x;
	q->y = out.y;
	q->z = out.z;
	q->w = c;
}