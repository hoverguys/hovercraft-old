#include "object.h"

#include <malloc.h>
#include <string.h>
#include <stdio.h>

void _MakeMatrix(object_t* object);

object_t* OBJECT_create(model_t*     mesh,
						guVector     position,
						guQuaternion rotation,
						guVector     scale) {
	object_t* object = malloc(sizeof(object_t));
	memset(object, 0, sizeof(object_t));
	object->mesh = mesh;
	object->transform.position = position;
	object->transform.rotation = rotation;
	object->transform.scale = scale;
	object->transform.dirty = TRUE;
	return object;
}

void OBJECT_destroy(object_t* object) {
	free(object);
}

void OBJECT_render(object_t* object, Mtx viewMtx) {
	if (object->mesh == NULL) return;

	/* Check dirty flag */
	if (object->transform.dirty == TRUE)
		_MakeMatrix(object);

	Mtx modelviewMtx;
	ps_guMtxConcat(object->transform.matrix, viewMtx, modelviewMtx);
	GX_LoadPosMtxImm(modelviewMtx, GX_PNMTX0);

	MODEL_render(object->mesh);
}

inline void _MakeMatrix(object_t* object) {
	/* Reset matrix to identity */
	Mtx* m = &object->transform.matrix;
	ps_guMtxIdentity(*m);

	/* TODO Can we merge translation and scale matrix? *
	 *      guMtxTrans and guMtxScale do very trivial  *
	 *      matrix assignments that could be merged.   */

	/* NOTE Currently using ps_* as they do FP/PS optimizations *
	 *      The implementation of those functions are in ASM,   *
	 *      considering swtching to c_* for debugging.          */

	/* Create translation matrix */
	Mtx translationMatrix;
	ps_guMtxTrans(translationMatrix,
			   object->transform.position.x,
			   object->transform.position.y,
			   object->transform.position.z);

	/* Create scale matrix */
	Mtx scaleMatrix;
	ps_guMtxScale(scaleMatrix,
			   object->transform.scale.x,
			   object->transform.scale.y,
			   object->transform.scale.z);

	/* Create rotation matrix */
	Mtx rotationMatrix;
	/* Couldn't find the ps_* version */
	c_guMtxQuat(rotationMatrix,
			    &object->transform.rotation);

	/* Merge all matrices */

	/* Current order: Scale, Rotation, Translation *
	* based on http://bit.ly/1ppBW33              *
	* Might be wrong, beware of bugs.             */
	ps_guMtxConcat(*m, scaleMatrix, *m);
	ps_guMtxConcat(*m, rotationMatrix, *m);
	ps_guMtxConcat(*m, translationMatrix, *m);

	object->transform.dirty = FALSE;
}