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
	transform_t* t = &object->transform;
	ps_guMtxIdentity(t->matrix);

	/* Current order: Scale, Rotation, Translation *
	* based on http://bit.ly/1ppBW33              *
	* Might be wrong, beware of bugs.             */
	c_guMtxQuat(t->matrix, &t->rotation);
	ps_guMtxScaleApply(t->matrix, t->matrix, t->scale.x, t->scale.y, t->scale.z);
	ps_guMtxTransApply(t->matrix, t->matrix, t->position.x, t->position.y, t->position.z);

	object->transform.dirty = FALSE;
}