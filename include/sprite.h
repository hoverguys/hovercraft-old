/*! \file sprite.h
*  \brief Sprites and other 2D elements
*/

#ifndef _SPRITE_H
#define _SPRITE_H

#include <ogcsys.h>

typedef struct {
	GXTexObj* texture;
	f32 UVoffsetX, UVoffsetY;
	f32 UVwidth, UVheight;
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
 *  \param position   f32[2] UV offset/position
 *  \param texSize    f32[2] UV total size
 *  \param spriteSize f32[2] Sprite size in the texture
 */
void SPRITE_setTexture(sprite_t* sprite, GXTexObj* texture, f32* position, f32* spriteSize, f32* texSize);

#endif