#include "object.h"

#include <malloc.h>
#include "mathutil.h"

/* Rebuild matrix on translate rather than setting dirty flag     *
 * Might be faster as we don't rebuild scale and rotation as well *
 * Can become a waste if we do more translations per frame        */
#define TRANSLATE_DIRECT

object_t* OBJECT_create(model_t* mesh) {
	guVector position = { 0, 0, 0 }, scale = { 1, 1, 1 };
	guQuaternion rotation;
	EulerToQuaternion(&rotation, 0, 0, 0);

	return OBJECT_createEx(mesh, position, rotation, scale);
}

object_t* OBJECT_createEx(model_t*     mesh,
						  const guVector     position,
						  const guQuaternion rotation,
						  const guVector     scale) {
	object_t* object = malloc(sizeof(object_t));
	object->mesh = mesh;

	object->transform.position = position;
	object->transform.rotation = rotation;
	object->transform.scale = scale;
	object->transform.dirty = TRUE;

	MakeMatrix(&object->transform);

	return object;
}

void OBJECT_destroy(object_t* object) {
	free(object);
}

void OBJECT_flush(object_t* object) {
	if (object->transform.dirty == TRUE) {
		MakeMatrix(&object->transform);
	}
}

void OBJECT_render(object_t* object, Mtx viewMtx) {
	if (object->mesh == NULL) return;

	/* Check dirty flag */
	OBJECT_flush(object);

	Mtx modelviewMtx;
	guMtxConcat(viewMtx, object->transform.matrix, modelviewMtx);
	GX_LoadPosMtxImm(modelviewMtx, GX_PNMTX0);

	MODEL_render(object->mesh);
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
	guVecAdd(&t->position, &deltaPos, &t->position);
#ifdef TRANSLATE_DIRECT
	/* ps_* doesn't do src/dst checks, so this is faster */
	guMtxTransApply(t->matrix, t->matrix, tX, tY, tZ);
#else
	t->dirty = TRUE;
#endif
}

void OBJECT_rotateTo(object_t* object, const f32 rX, const f32 rY, const f32 rZ) {
	transform_t* t = &object->transform;
	EulerToQuaternion(&t->rotation, rX, rY, rZ);
	t->dirty = TRUE;
}

void OBJECT_rotateSet(object_t* object, guQuaternion *rotation) {
	transform_t* t = &object->transform;
	t->rotation = *rotation;
	t->dirty = TRUE;
}

void OBJECT_rotateAxis(object_t* object, guVector* axis, const f32 angle) {
	transform_t* t = &object->transform;
	Mtx angleaxis;
	guQuaternion deltaq;
	guMtxRotAxisRad(angleaxis, axis, angle);
	c_guQuatMtx(&deltaq, angleaxis);
	guQuatMultiply(&deltaq, &t->rotation, &t->rotation);
	t->dirty = TRUE;
}

void OBJECT_rotate(object_t* object, const f32 rX, const f32 rY, const f32 rZ) {
	transform_t* t = &object->transform;
	guQuaternion deltaq;
	EulerToQuaternion(&deltaq, rX, rY, rZ);
	guQuatMultiply(&deltaq, &t->rotation, &t->rotation);
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

