#include "object.h"

#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

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

void OBJECT_rotateTo(object_t* object, f32 rX, f32 rY, f32 rZ) {
	transform_t* t = &object->transform;
	_EulerToQuaternion(&t->rotation, rX, rY, rZ);
	t->dirty = TRUE;
}

void OBJECT_rotate(object_t* object, f32 rX, f32 rY, f32 rZ) {
	transform_t* t = &object->transform;
	guQuaternion deltaq;
	_EulerToQuaternion(&deltaq, rX, rY, rZ);
	c_guQuatMultiply(&t->rotation, &deltaq, &t->rotation);
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

void _EulerToQuaternion(guQuaternion* q, const f32 rX, const f32 rY, const f32 rZ) {
	guVector vec;
	vec.x = rX; vec.y = rY; vec.z = rZ;
	ps_guVecScale(&vec, &vec, 0.5f);
	f32 c1, s1, c2, s2, c3, s3;
	sincosf(vec.y, &c1, &s1);
	sincosf(vec.x, &c2, &s2);
	sincosf(vec.z, &c3, &s3);
	f32 c1c2 = c1*c2;
	f32 s1s2 = s1*s3;
	q->w = c1c2*c3 - s1s2*s3;
	q->x = c1c2*s3 + s1s2*c3;
	q->y = s1*c2*c3 + c1*s2*s3;
	q->z = c1*s2*c3 - s1*c2*s3;
}


void _AxisAngleToQuaternion(guQuaternion* q, const guVector rAxis, const f32 rAngle) {
	f32 s, c;
	sincosf(rAngle / 2.0f, &s, &c);
	guVector out;
	ps_guVecScale(&rAxis, &out, s);
	q->x = out.x;
	q->y = out.y;
	q->z = out.z;
	q->w = c;
}