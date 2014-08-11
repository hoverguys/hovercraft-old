#include "raycast.h"

#include <stdio.h>
#include <math.h>

#define EPSILON 0.000001f

BOOL Raycast(object_t* object, guVector* raydir, guVector* rayorigin, f32* distanceOut) {
	//Init data
	model_t * const mesh = object->mesh;
	guVector *point0, *point1, *point2;
	u16 *baseindices = mesh->modelIndices;
	guVector *vertices = (guVector*)mesh->modelPositions;
	Mtx InverseObjMtx;
	guVector rayO, rayD;

	OBJECT_flush(object);

	//Get the raycast into object space
	guMtxInverse(object->transform.matrix, InverseObjMtx);

	guVecMultiply(InverseObjMtx, rayorigin, &rayO);
	guVecMultiplySR(InverseObjMtx, raydir, &rayD);
	f32 rayScale = sqrtf(guVecDotProduct(&rayD, &rayD));
	guVecNormalize(&rayD);

	//Temps
	guVector e1, e2;
	guVector P, Q, T;
	float det, inv_det, u, v;
	float t;

	BOOL hit = FALSE;
	f32 sdist = 0;
	u16 *indices = 0;

	//Iterate over very triangle
	u32 f = 0;
	for (; f < mesh->modelFaceCount; ++f) {
		indices = baseindices + (f * 3);

		//Get data
		point0 = &vertices[indices[0]];
		point1 = &vertices[indices[1]];
		point2 = &vertices[indices[2]];

		guVecSub(point1, point0, &e1);
		guVecSub(point2, point0, &e2);

		guVecCross(&rayD, &e2, &P);

		det = guVecDotProduct(&e1, &P);

		//NOT CULLING
		if (det > -EPSILON && det < EPSILON) {
			continue;
		}
		inv_det = 1.f / det;

		//calculate distance from V1 to ray origin
		guVecSub(&rayO, point0, &T);

		//Calculate u parameter and test bound
		u = guVecDotProduct(&T, &P) * inv_det;
		//The intersection lies outside of the triangle
		if (u < 0.f || u > 1.f) {
			continue;
		}

		//Prepare to test v parameter
		guVecCross(&T, &e1, &Q);

		//Calculate V parameter and test bound
		v = guVecDotProduct(&rayD, &Q) * inv_det;
		//The intersection lies outside of the triangle
		if (v < 0.f || u + v  > 1.f) {
			continue;
		}

		t = guVecDotProduct(&e2, &Q) * inv_det;

		if (t > EPSILON) { //ray intersection
			if (t < sdist || hit == 0) {
				sdist = t;
				hit = TRUE;
			}
		}
	}

	if (hit == TRUE) {
		*distanceOut = sdist / rayScale;
		return TRUE;
	}

	return FALSE;
}