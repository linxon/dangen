/* $Id: laser.h,v 1.7 2004/07/06 02:50:57 oohara Exp $ */

#ifndef __DANGEN_LASER_H__
#define __DANGEN_LASER_H__

#include "tenm_object.h"

tenm_object *laser_angle_new(double x, double y, double speed, int theta,
                              double length, int color);
tenm_object *laser_point_new(double x, double y, double speed,
                             double target_x, double target_y,
                             double length, int color);
tenm_object *laser_new(double x, double y,
                       double speed_x, double speed_y,
                       double length_x, double length_y,
                       int color, int life, int immortal_time);

#endif /* not __DANGEN_LASER_H__ */
