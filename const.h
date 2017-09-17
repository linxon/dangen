/* $Id: const.h,v 1.33 2011/08/23 09:46:02 oohara Exp $ */

#ifndef __DANGEN_CONST_H__
#define __DANGEN_CONST_H__

#define COPYRIGHT_STRING "Copyright (C) 2005, 2011 Oohara Yuuma"

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

/* attribute
 * these values must be one of 2^n
 */
#define ATTR_PLAYER 1
#define ATTR_PLAYER_SHOT 2
/* this object is not killed automatically when the stage target is dead */
#define ATTR_BOSS 4
#define ATTR_ENEMY 8
#define ATTR_ENEMY_SHOT 16
/* "weak" enemy gets hit by it */
#define ATTR_OBSTACLE 32
/* normal enemy shot cannot go through it */
#define ATTR_OPAQUE 64

/* background color */
/* pure white is bad for your eyes */
#define DEFAULT_BACKGROUND_RED 255
#define DEFAULT_BACKGROUND_GREEN 245
#define DEFAULT_BACKGROUND_BLUE 192

#endif /* __DANGEN_CONST_H__ */
