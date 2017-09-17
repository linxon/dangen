/* $Id: player-shot.h,v 1.5 2004/07/06 02:49:50 oohara Exp $ */

#ifndef __DANGEN_PLAYER_SHOT_H__
#define __DANGEN_PLAYER_SHOT_H__

#include "tenm_object.h"

tenm_object *player_shot_new(double x, double y, int n);
int deal_damage(tenm_object *my, tenm_object *your, int n);

#endif /* not __DANGEN_PLAYER_SHOT_H__ */
