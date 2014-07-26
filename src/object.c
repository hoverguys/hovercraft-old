#include "object.h"

#include <malloc.h>
#include <string.h>
#include <stdio.h>

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
	object->transform.dirty = DIRTY_POSITION | DIRTY_ROTATION | DIRTY_SCALE;
	return object;
}

void OBJECT_destroy(object_t* object) {
	free(object);
}

void OBJECT_render(object_t* object) {
	if (object->mesh == NULL) {
		printf("Can't render inexistent mesh");
		return;
	}

	MODEL_render(object->mesh);
}