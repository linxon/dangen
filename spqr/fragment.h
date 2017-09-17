/* $Id: fragment.h,v 1.6 2003/08/13 16:33:59 oohara Exp $ */

#ifndef __DANGEN_FRAGMENT_H__
#define __DANGEN_FRAGMENT_H__

tenm_object *fragment_new(double x, double y, double dx, double dy,
                          double size_fragment, int number_fragment,
                          int color, double speed_fragment, double speed_theta,
                          int life);

#endif /* not __DANGEN_FRAGMENT_H__ */
