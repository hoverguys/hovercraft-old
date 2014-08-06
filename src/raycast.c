#include "raycast.h"

#include <stdio.h>

#define EPSILON 0.001f

f32 Raycast(object_t* object, guVector* raydir, guVector* rayorigin) {
	//Init data
	model_t * const mesh = object->mesh;
	guVector *point0, *point1, *point2;
	u16 *indices = mesh->modelIndices;
	Mtx InverseObjMtx;
	guVector rayO, rayD;

	//Get the raycast into object space
	guMtxInverse(object->transform.matrix, InverseObjMtx);
	guVecMultiply(InverseObjMtx, rayorigin, &rayO);
	guVecMultiplySR(InverseObjMtx, raydir, &rayD);
	guVecNormalize(&rayD);

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
		//guVecCross(&rayD, &edge2, &pvec);
		guVecCross(&edge2, &rayD, &pvec);
		det = guVecDotProduct(&edge1, &pvec);

		//printf("det %f\n", det);

		/* hit check */
		if (det < EPSILON) {
			goto next; /* no hit */
		}

		guVecSub(&rayO, point0, &tvec);
		u = guVecDotProduct(&tvec, &pvec);

		//guVecCross(&tvec, &edge2, &qvec);
		guVecCross(&edge2, &tvec, &qvec);
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
			printf("HIT %f\n", t);
		}

next:
		//Next
		indices += 3;
	}

	if (hit == 1) {
		return sdist;
	}

	return -1.0f;
}