#include "raycast.h"

#include <math.h>

#define EPSILON 0.000001f

BOOL Raycast(object_t* object, guVector* raydir, guVector* rayorigin, f32* distanceOut, guVector* normalOut) {
	/* Init data */
	model_t * const mesh = object->mesh;
	u16 *baseindices = mesh->modelIndices;
	guVector *vertices = (guVector*) mesh->modelPositions;
	guVector *normals = (guVector*) mesh->modelNormals;
	Mtx InverseObjMtx;
	guVector rayO, rayD;

	OBJECT_flush(object);

	/* Get the raycast into object space */
	guMtxInverse(object->transform.matrix, InverseObjMtx);

	guVecMultiply(InverseObjMtx, rayorigin, &rayO);
	guVecMultiplySR(InverseObjMtx, raydir, &rayD);
	f32 rayScale = sqrtf(guVecDotProduct(&rayD, &rayD));
	guVecNormalize(&rayD);

	/* Temporary variables */
	guVector e1, e2;
	guVector P, Q, T;
	float inv_det, u, v;
	float t;

	BOOL hit = FALSE;
	f32 sdist = 0;

	guVector *normal = 0;

	/* Iterate over every triangle */
	u32 f = 0;
	for (; f < mesh->modelFaceCount; ++f) {
		u16 *indices = baseindices + (f * 3);

		/* Get data */
		guVector *point0 = &vertices[indices[0]],
				 *point1 = &vertices[indices[1]],
				 *point2 = &vertices[indices[2]];

		guVecSub(point1, point0, &e1);
		guVecSub(point2, point0, &e2);

		guVecCross(&rayD, &e2, &P);

		float det = guVecDotProduct(&e1, &P);

		/* NOT CULLING */
		if (det > -EPSILON && det < EPSILON) {
			continue;
		}
		inv_det = 1.f / det;

		/* Calculate distance from V1 to ray origin */
		guVecSub(&rayO, point0, &T);

		/* Calculate u parameter and test bound */
		u = guVecDotProduct(&T, &P) * inv_det;
		/* The intersection lies outside of the triangle */
		if (u < 0.f || u > 1.f) {
			continue;
		}

		/* Prepare to test v parameter */
		guVecCross(&T, &e1, &Q);

		/* Calculate V parameter and test bound */
		v = guVecDotProduct(&rayD, &Q) * inv_det;
		/* The intersection lies outside of the triangle */
		if (v < 0.f || u + v  > 1.f) {
			continue;
		}

		t = guVecDotProduct(&e2, &Q) * inv_det;

		if (t > EPSILON) { /* Got a ray intersection! */
			if (t < sdist || hit == 0) {
				sdist = t;
				normal = &normals[indices[0]]; //TODO Interpolate 3 normals to get the positional one?
				hit = TRUE;
			}
		}
	}

	if (hit == TRUE) {
		*distanceOut = sdist / rayScale;
		if (normalOut != NULL) {
			*normalOut = *normal;
		}
	}

	return hit;
}