#include "mathutil.h"
#include <math.h>

void EulerToQuaternion(guQuaternion* q, const f32 rX, const f32 rY, const f32 rZ) {
	guVector vec;
	vec.x = rX; vec.y = rY; vec.z = rZ;
	ps_guVecScale(&vec, &vec, 0.5f);

	/*           S  C  */
	f32 x[2] = { 0, 0 }, /* 1 */
		y[2] = { 0, 0 }, /* 2 */
		z[2] = { 0, 0 }; /* 3 */

	sincosf(vec.x, &x[0], &x[1]);
	sincosf(vec.y, &y[0], &y[1]);
	sincosf(vec.z, &z[0], &z[1]);

	f32 cs[2] = { 0, 0 };
	ps_float2Mul(x, y, cs);
	/* const f32 c1c2 = c1*c2; => f32 c1c2 = x[1] * y[1]; => cs[1] *
	 * const f32 s1s2 = s1*s2; => f32 s1s2 = x[0] * y[0]; => cs[0] */

	/*	q->w = c1c2*c3 - s1s2*s3; => cs[1] * z[1] - cs[0] * z[0]
	 *  q->x = c1c2*s3 + s1s2*c3; => cs[1] * z[0] - cs[0] * z[1]
	 *  q->y = s1*c2*c3 + c1*s2*s3; => cs[1] * z[1] - cs[0] * z[0]
	 *  q->z = c1*s2*c3 - s1*c2*s3; => cs[1] * z[1] - cs[0] * z[0]
	 */
	q->w = cs[1] * z[1] - cs[0] * z[0];
	q->x = cs[1] * z[0] + cs[0] * z[1];
	q->y = y[0] * x[1] * z[1] + y[1] * x[0] * z[0];
	q->z = y[1] * x[0] * z[1] - y[0] * x[1] * z[0];
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