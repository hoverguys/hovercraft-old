#include "raycast.h"

#include <stdio.h>
#include <math.h>

#define EPSILON 0.000001f

u8 Raycast(object_t* object, guVector* raydir, guVector* rayorigin, f32* distanceOut) {
	//Init data
	model_t * const mesh = object->mesh;
	guVector *point0, *point1, *point2;
	u16 *indices = mesh->modelIndices;
	Mtx InverseObjMtx;
	guVector rayO, rayD;

	OBJECT_flush(object);

	//Get the raycast into object space
	guMtxInverse(object->transform.matrix, InverseObjMtx);

	guVecMultiply(InverseObjMtx, rayorigin, &rayO);
	guVecMultiplySR(InverseObjMtx, raydir, &rayD);

	//Temps
	guVector edge1, edge2;
	guVector pvec, tvec, qvec;
	f32 det, u, v, t;

	//TODO: Replace with better number
	u8 hit = 0;
	f32 sdist = 100000;
	
	//Iterate over very triangle
	u32 f = 0;
	for (; f < mesh->modelFaceCount; ++f ) {
		//Get data
		point0 = (guVector*)&mesh->modelPositions[indices[0]];
		point1 = (guVector*)&mesh->modelPositions[indices[1]];
		point2 = (guVector*)&mesh->modelPositions[indices[2]];

		guVecSub(point1, point0, &edge1);
		guVecSub(point2, point0, &edge2);
		guVecCross(&rayD, &edge2, &pvec);
		det = guVecDotProduct(&edge1, &pvec);

		/* hit check */
		if (det < EPSILON) {
			goto next; /* no hit */
		}

		guVecSub(&rayO, point0, &tvec);
		u = guVecDotProduct(&tvec, &pvec);

		//printf("u %f", u);

		if (u < 0.0f || u > det) {
			goto next;
		}

		guVecCross(&tvec, &edge1, &qvec);
		v = guVecDotProduct(&rayD, &qvec);

		/* hit check */
		if (v < 0 || (u + v) > det) {
			goto next; /* no hit */
		}

		t = guVecDotProduct(&edge2, &qvec) / det;

		// We have a hit, check if the data needs updating
		if (t < sdist || hit == 0) {
			sdist = t;
			hit = 1;
		}

next:
		//Next
		indices += 3;
	}

	if (hit == 1) {
		*distanceOut = sdist;
		return 1;
	}

	return 0;
}

u8 Raycast2(object_t* object, guVector* raydir, guVector* rayorigin, f32* distanceOut) {
	//Init data
	model_t * const mesh = object->mesh;
	guVector *point0, *point1, *point2;
	u16 *indices = mesh->modelIndices;
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

	u8 hit = 0;
	f32 sdist = 0;

	//Iterate over very triangle
	u32 f = 0;
	for (; f < mesh->modelFaceCount; ++f) {
		//Get data
		point0 = (guVector*)&mesh->modelPositions[indices[0]];
		point1 = (guVector*)&mesh->modelPositions[indices[1]];
		point2 = (guVector*)&mesh->modelPositions[indices[2]];

		guVecSub(point1, point0, &e1);
		guVecSub(point2, point0, &e2);

		guVecCross(&rayD, &e2, &P);

		det = guVecDotProduct(&e1, &P);

		//NOT CULLING
		if (det > -EPSILON && det < EPSILON) {
			goto next;
		}
		inv_det = 1.f / det;

		//calculate distance from V1 to ray origin
		guVecSub(&rayO, point0, &T);

		//Calculate u parameter and test bound
		u = guVecDotProduct(&T, &P) * inv_det;
		//The intersection lies outside of the triangle
		if (u < 0.f || u > 1.f) {
			goto next;
		}

		//Prepare to test v parameter
		guVecCross(&T, &e1, &Q);

		//Calculate V parameter and test bound
		v = guVecDotProduct(&rayD, &Q) * inv_det;
		//The intersection lies outside of the triangle
		if (v < 0.f || u + v  > 1.f) {
			goto next;
		}

		t = guVecDotProduct(&e2, &Q) * inv_det;

		if (t > EPSILON) { //ray intersection
			if (t < sdist || hit == 0) {
				sdist = t;
				hit = 1;
			}
		}

	next:
		//Next
		indices += 3;
	}

	if (hit == 1) {
		*distanceOut = sdist / rayScale;
		printf("scale %f\n", *distanceOut);
		return 1;
	}

	return 0;
}