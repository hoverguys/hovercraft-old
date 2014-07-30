#include "object.h"

#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "mathutil.h"

/* Rebuild matrix on translate rather than setting dirty flag     *
 * Might be faster as we don't rebuild scale and rotation as well *
 * Can become a waste if we do more translations per frame        */
#define TRANSLATE_DIRECT

void _MakeMatrix(object_t* object);

object_t* OBJECT_create(model_t* mesh) {
	guVector position, scale;
	position.x = position.y = position.z = 0;
	scale.x = scale.y = scale.z = 1;
	guQuaternion rotation;
	EulerToQuaternion(&rotation, 0, 0, 0);

	return OBJECT_createEx(mesh, position, rotation, scale);
}

object_t* OBJECT_createEx(model_t*     mesh,
						  const guVector     position,
						  const guQuaternion rotation,
						  const guVector     scale) {
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
	ps_guMtxConcat(viewMtx, object->transform.matrix, modelviewMtx);
	GX_LoadPosMtxImm(modelviewMtx, GX_PNMTX0);

	MODEL_render(object->mesh);
}

inline void _MakeMatrix(object_t* object) {
	/* Reset matrix to identity */
	transform_t* t = &object->transform;
	MtxP matrix = t->matrix;
	ps_guMtxIdentity(matrix);

	/* Rotate, Scale, Translate */
	c_guQuatNormalize(&t->rotation, &t->rotation);
	c_guMtxQuat(matrix, &t->rotation);
	ps_guMtxScaleApply(matrix, matrix, t->scale.x, t->scale.y, t->scale.z);
	ps_guMtxTransApply(matrix, matrix, t->position.x, t->position.y, t->position.z);

	/* Calculate forward/up/left vectors */
	t->forward.x = matrix[0][2];
	t->forward.y = matrix[1][2];
	t->forward.z = matrix[2][2];

	t->up.x = matrix[0][1];
	t->up.y = matrix[1][1];
	t->up.z = matrix[2][1];

	t->right.x = matrix[0][0];
	t->right.y = matrix[1][0];
	t->right.z = matrix[2][0];

	object->transform.dirty = FALSE;
}

void OBJECT_moveTo(object_t* object, const f32 tX, const f32 tY, const f32 tZ) {
	transform_t* t = &object->transform;
	t->position.x = tX;
	t->position.y = tY;
	t->position.z = tZ;
	t->dirty = TRUE;
}

void OBJECT_move(object_t* object, const f32 tX, const f32 tY, const f32 tZ) {
	transform_t* t = &object->transform;
	guVector deltaPos;
	deltaPos.x = tX; deltaPos.y = tY; deltaPos.z = tZ;
	ps_guVecAdd(&t->position, &deltaPos, &t->position);
#ifdef TRANSLATE_DIRECT
	/* ps_* doesn't do src/dst checks, so this is faster */
	c_guMtxTransApply(t->matrix, t->matrix, tX, tY, tZ);
#else
	t->dirty = TRUE;
#endif
}

void OBJECT_rotateTo(object_t* object, const f32 rX, const f32 rY, const f32 rZ) {
	transform_t* t = &object->transform;
	EulerToQuaternion(&t->rotation, rX, rY, rZ);
	t->dirty = TRUE;
}

void OBJECT_rotate(object_t* object, const f32 rX, const f32 rY, const f32 rZ) {
	transform_t* t = &object->transform;
	guQuaternion deltaq;
	EulerToQuaternion(&deltaq, rX, rY, rZ);
	c_guQuatMultiply(&t->rotation, &deltaq, &t->rotation);
	t->dirty = TRUE;
}

void OBJECT_scaleTo(object_t* object, const f32 sX, const f32 sY, const f32 sZ) {
	transform_t* t = &object->transform;
	t->scale.x = sX;
	t->scale.y = sY;
	t->scale.z = sZ;
	t->dirty = TRUE;
}

void OBJECT_scale(object_t* object, const f32 sX, const f32 sY, const f32 sZ) {
	transform_t* t = &object->transform;
	t->scale.x *= sX;
	t->scale.y *= sY;
	t->scale.z *= sZ;
	t->dirty = TRUE;
}

