/*! \file sprite.h
*  \brief Sprites and other 2D elements
*/

#ifndef _SPRITE_H
#define _SPRITE_H

#include <ogcsys.h>
#include "gxutils.h"
#include "object.h"

typedef struct {
	GXTexObj*   texture;
	transform_t transform;
	f32         UVoffsetX, UVoffsetY;
	f32         UVwidth, UVheight;
} sprite_t;

/*! \brief Initialize sprite system, including 2D camera
 */
void SPRITE_init();

/*! \brief Create empty sprite
 *  \return Pointer to newly sprite structure
 */
sprite_t* SPRITE_create();

/*! \brief Frees a sprite
 *  \param sprite Sprite to destroy
 */
void SPRITE_free(sprite_t* sprite);

/*! \brief Assign a texture to a sprite, including offset/size
 *  \param sprite     Sprite to assign texture to
 *  \param texture    Texture object to use
 *  \param position   UV offset/position
 *  \param texSize    UV total size
 *  \param spriteSize Sprite size in the texture
 */
void SPRITE_setTexture(sprite_t* sprite, GXTexObj* texture, f32 position[2], f32 spriteSize[2], TexSize texSize);

/*! \brief Renders a sprite on screen 
 *  \param sprite Sprite to render
 */
void SPRITE_render(sprite_t* sprite);

/*! \brief Force Matrix regeneration o sprite
*  \param sprite Sprite to process
*/
void SPRITE_flush(sprite_t* sprite);

#endif