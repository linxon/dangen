/* $Id: explosion.h,v 1.3 2004/07/06 02:51:14 oohara Exp $ */

#ifndef __DANGEN_EXPLOSION_H__
#define __DANGEN_EXPLOSION_H__
tenm_object *explosion_new(double x, double y, double dx, double dy,
                           int size_debris, int number_debris,
                           int color, double speed_debris, int life);

#endif /* not __DANGEN_EXPLOSION_H__ */
