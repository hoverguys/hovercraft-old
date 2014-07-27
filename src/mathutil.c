#include "mathutil.h"
#include <math.h>

void EulerToQuaternion(guQuaternion* q, const f32 rX, const f32 rY, const f32 rZ) {
	guVector vec;
	vec.x = rX; vec.y = rY; vec.z = rZ;
	ps_guVecScale(&vec, &vec, 0.5f);
	f32 c1, s1, c2, s2, c3, s3;
	sincosf(vec.y, &c1, &s1);
	sincosf(vec.x, &c2, &s2);
	sincosf(vec.z, &c3, &s3);
	f32 c1c2 = c1*c2;
	f32 s1s2 = s1*s3;
	q->w = c1c2*c3 - s1s2*s3;
	q->x = c1c2*s3 + s1s2*c3;
	q->y = s1*c2*c3 + c1*s2*s3;
	q->z = c1*s2*c3 - s1*c2*s3;
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