#include "object.h"

#include <malloc.h>
#include <string.h>
#include <stdio.h>

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
	rotation.x = rotation.y = rotation.z = rotation.w = 0;

	return OBJECT_createEx(mesh, position, rotation, scale);
}

object_t* OBJECT_createEx(model_t*     mesh,
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

	/* Rotate, Scale, Translate */
	c_guQuatNormalize(&t->rotation, &t->rotation);
	c_guMtxQuat(t->matrix, &t->rotation);
	ps_guMtxScaleApply(t->matrix, t->matrix, t->scale.x, t->scale.y, t->scale.z);
	ps_guMtxTransApply(t->matrix, t->matrix, t->position.x, t->position.y, t->position.z);

	object->transform.dirty = FALSE;
}

void OBJECT_moveTo(object_t* object, f32 tX, f32 tY, f32 tZ) {
	transform_t* t = &object->transform;
	t->position.x = tX;
	t->position.y = tY;
	t->position.z = tZ;
	t->dirty = TRUE;
}

void OBJECT_move(object_t* object, f32 tX, f32 tY, f32 tZ) {
	transform_t* t = &object->transform;
	t->position.x += tX;
	t->position.y += tY;
	t->position.z += tZ;
#ifdef TRANSLATE_DIRECT
	/* ps_* doesn't do src/dst checks, so this is faster */
	c_guMtxTransApply(t->matrix, t->matrix, tX, tY, tZ);
#else
	t->dirty = TRUE;
#endif
}

void OBJECT_rotateTo(object_t* object, f32 rX, f32 rY, f32 rZ, f32 rW) {
	transform_t* t = &object->transform;
	t->rotation.x = rX;
	t->rotation.y = rY;
	t->rotation.z = rZ;
	t->rotation.w = rW;
	t->dirty = TRUE;
}

void OBJECT_rotate(object_t* object, f32 rX, f32 rY, f32 rZ, f32 rW) {
	transform_t* t = &object->transform;
	t->rotation.x += rX;
	t->rotation.y += rY;
	t->rotation.z += rZ;
	t->rotation.w += rW;
	t->dirty = TRUE;
}

void OBJECT_scaleTo(object_t* object, f32 sX, f32 sY, f32 sZ) {
	transform_t* t = &object->transform;
	t->scale.x = sX;
	t->scale.y = sY;
	t->scale.z = sZ;
	t->dirty = TRUE;
}

void OBJECT_scale(object_t* object, f32 sX, f32 sY, f32 sZ) {
	transform_t* t = &object->transform;
	t->scale.x *= sX;
	t->scale.y *= sY;
	t->scale.z *= sZ;
	t->dirty = TRUE;
}

