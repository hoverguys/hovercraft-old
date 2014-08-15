#include "mathutil.h"
#include <math.h>
#include <ogc/gu.h>

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
	guLookAt(tmpMatrix, forward, up, &zero);
	c_guQuatMtx(out, tmpMatrix);
}

void QUAT_slerp(guQuaternion* q0, guQuaternion* q1, const float t, guQuaternion* out) {
	if (t <= 0.0f) {
		*out = *q0;
		return;
	} else if (t >= 1.0f) {
		*out = *q1;
		return;
	}

	f32 cosOmega;
	QUAT_dotProduct(q0, q1, &cosOmega);
	f32 q1w = q1->w;
	f32 q1x = q1->x;
	f32 q1y = q1->y;
	f32 q1z = q1->z;
	if (cosOmega < 0.0f) {
		q1w = -q1w;
		q1x = -q1x;
		q1y = -q1y;
		q1z = -q1z;
		cosOmega = -cosOmega;
	}

	f32 k0, k1;
	if (cosOmega > 0.9999f) {
		k0 = 1.0f - t;
		k1 = t;
	} else {
		f32 sinOmega = sqrt(1.0f - cosOmega*cosOmega);
		f32 omega = atan2(sinOmega, cosOmega);
		f32 oneOverSinOmega = 1.0f / sinOmega;

		k0 = sin((1.0f - t) * omega) * oneOverSinOmega;
		k1 = sin(t * omega) * oneOverSinOmega;
	}

	out->x = k0*q0->x + k1*q1x;
	out->y = k0*q0->y + k1*q1y;
	out->z = k0*q0->z + k1*q1z;
	out->w = k0*q0->w + k1*q1w;
	return;
}

/* These should be in gu.h or something */

inline f32 guVecMag(guVector* vec) {
	return sqrt(guVecDotProduct(vec, vec));
}

inline f32 vecDistance(guVector* point1, guVector* point2) {
	guVector sub;
	guVecSub(point2, point1, &sub);
	return guVecMag(&sub);
}

// How big is the player's collision sphere?
#define RADIUS 2

BOOL CalculateBounce(player_t* a, player_t* b) {
	guVector collision;
	guVecSub(&b->hovercraft->transform.position,
			 &a->hovercraft->transform.position,
			 &collision);
	f32 distance = guVecMag(&collision);

	if (distance == 0) distance = 1;
	if (distance > RADIUS) return FALSE;

	guVecScale(&collision, &collision, 1 / distance);
	f32 dota = guVecDotProduct(&a->velocity, &collision);
	f32 dotb = guVecDotProduct(&b->velocity, &collision);
	f32 scaleFac = dotb - dota;

	guVector deltaA, deltaB;
	guVecScale(&collision, &deltaA, scaleFac);
	guVecScale(&collision, &deltaB, -scaleFac);

	guVecAdd(&a->velocity, &deltaA, &a->velocity);
	guVecAdd(&b->velocity, &deltaB, &b->velocity);

	return TRUE;
}