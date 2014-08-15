/*! \file audioutil.h
*  \brief Audio / Music / SFX utilities
*/
#ifndef _AUDIOUTIL_H
#define _AUDIOUTIL_H

/*! \brief Initialize audio subsystem
 */
void AU_init();

/*! \brief Play a specific music
 *  \param music Music Id, from generated assets
 */
void AU_playMusic(const void* music);

#endif