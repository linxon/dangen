/* $Id: player.h,v 1.3 2004/07/06 02:49:29 oohara Exp $ */

#ifndef __DANGEN_PLAYER_H__
#define __DANGEN_PLAYER_H__

#include "tenm_object.h"

tenm_object *player_new(int tutorial);
void player_neutral_position(tenm_object *my);

#endif /* not __DANGEN_PLAYER_H__ */
